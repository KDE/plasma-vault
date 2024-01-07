/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vault.h"

#include <QDBusMetaType>
#include <QDir>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QPointer>
#include <QUrl>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QRegularExpression>

#include <processcore/process.h>
#include <processcore/processes.h>

#include <KLocalizedString>
#include <kdirnotify.h>

#include "backend_p.h"

#include "asynqt/basic/all.h"
#include "asynqt/operations/listen.h"
#include "asynqt/wrappers/process.h"
// Included after others
#include "asynqt/operations/cast.h"

#include <signal.h>

#define CFG_NAME "name"
#define CFG_LAST_STATUS "lastStatus"
#define CFG_LAST_ERROR "lastError"
#define CFG_MOUNT_POINT "mountPoint"
#define CFG_BACKEND "backend"
#define CFG_ACTIVITIES "activities"
#define CFG_OFFLINEONLY "offlineOnly"

namespace PlasmaVault
{
class Vault::Private
{
public:
    Vault *const q;

    KSharedConfigPtr config;
    const Device device;

    FILE *deviceDirectoryLock = nullptr;

    void lockDeviceDirectory()
    {
        if (!deviceDirectoryLock) {
            deviceDirectoryLock = fopen(device.data().toLocal8Bit().data(), "r");
        }
    }

    void unlockDeviceDirectory()
    {
        if (deviceDirectoryLock) {
            fclose(deviceDirectoryLock);
            deviceDirectoryLock = nullptr;
        }
    }

    QTimer savingDelay;

    enum DeletionState {
        Normal,
        DeletionBlocked,
        DeletionScheduled,
    };
    DeletionState deletionState = Normal;

    struct Data {
        QString name;
        MountPoint mountPoint;

        VaultInfo::Status status;
        QString message;

        QStringList activities;
        bool isOfflineOnly;

        QString backendName;
        Backend::Ptr backend;
    };
    using ExpectedData = AsynQt::Expected<Data, PlasmaVault::Error>;
    ExpectedData data;

    void updateMessage(const QString &message)
    {
        if (!data)
            return;

        data->message = message;
        Q_EMIT q->messageChanged(message);
    }

    void writeConfiguration()
    {
        if (data) {
            const auto deviceStr = device.data();
            const auto mountPointStr = data->mountPoint.data();

            Q_ASSERT(!deviceStr.isEmpty() && !mountPointStr.isEmpty());

            // Saving the data for the current mount
            KConfigGroup generalConfig(config, QStringLiteral("EncryptedDevices"));
            generalConfig.writeEntry(deviceStr, true);

            KConfigGroup vaultConfig(config, deviceStr);
            vaultConfig.writeEntry(CFG_LAST_STATUS, (int)data->status);
            vaultConfig.writeEntry(CFG_MOUNT_POINT, mountPointStr);
            vaultConfig.writeEntry(CFG_NAME, data->name);
            vaultConfig.writeEntry(CFG_BACKEND, data->backend->name());

            vaultConfig.writeEntry(CFG_ACTIVITIES, data->activities);
            vaultConfig.writeEntry(CFG_OFFLINEONLY, data->isOfflineOnly);

        } else {
            KConfigGroup generalConfig(config, QStringLiteral("EncryptedDevices"));
            generalConfig.writeEntry(device.data(), false);

            KConfigGroup vaultConfig(config, device.data());
            vaultConfig.writeEntry(CFG_LAST_STATUS, (int)VaultInfo::Error);
            vaultConfig.writeEntry(CFG_LAST_ERROR,
                                   QString(data.error().message() + QStringLiteral(" (code: ") + QString::number(data.error().code()) + QStringLiteral(")")));
            // vaultConfig.deleteEntry(CFG_MOUNT_POINT);
            // vaultConfig.deleteEntry(CFG_NAME);
            // vaultConfig.deleteEntry(CFG_BACKEND);
            // vaultConfig.deleteEntry(CFG_ACTIVITIES);
            // vaultConfig.deleteEntry(CFG_OFFLINEONLY);
        }

        config->sync();
    }

    void updateStatus()
    {
        if (data) {
            // Checking the status, and whether we should update it
            const auto oldStatus = data->status;

            if (oldStatus == VaultInfo::Dismantling) {
                // This means that the vault should be forgotten
                KConfigGroup generalConfig(config, QStringLiteral("EncryptedDevices"));
                generalConfig.deleteEntry(device.data());

                KConfigGroup vaultConfig(config, device.data());
                vaultConfig.deleteGroup();

                Q_EMIT q->statusChanged(data->status = VaultInfo::Dismantled);

            } else {
                QDir deviceDir(device.data());

                const auto newStatus = !deviceDir.exists() ? VaultInfo::DeviceMissing
                    : isOpened()                           ? VaultInfo::Opened
                    : isInitialized()                      ? VaultInfo::Closed
                                                           : VaultInfo::NotInitialized;

                if (oldStatus == newStatus)
                    return;

                data->status = newStatus;

                Q_EMIT q->statusChanged(data->status);

                if (newStatus == VaultInfo::Closed //
                    || newStatus == VaultInfo::Opened) {
                    Q_EMIT q->isOpenedChanged(newStatus == VaultInfo::Opened);
                }

                if (oldStatus == VaultInfo::NotInitialized //
                    || newStatus == VaultInfo::NotInitialized) {
                    Q_EMIT q->isInitializedChanged(newStatus);
                }

                if (oldStatus == VaultInfo::Creating //
                    || oldStatus == VaultInfo::Opening //
                    || oldStatus == VaultInfo::Closing //
                    || oldStatus == VaultInfo::Dismantling) {
                    Q_EMIT q->isBusyChanged(false);
                }

                writeConfiguration();

                org::kde::KDirNotify::emitFilesAdded(QUrl::fromLocalFile(data->mountPoint.data()));
            }

        } else {
            Q_EMIT q->isOpenedChanged(false);
            Q_EMIT q->isInitializedChanged(false);
            Q_EMIT q->isBusyChanged(false);

            writeConfiguration();

            Q_EMIT q->statusChanged(VaultInfo::Error);
        }

        if (data && data->status == VaultInfo::Opened) {
            lockDeviceDirectory();
        } else {
            unlockDeviceDirectory();
        }
    }

    ExpectedData errorData(Error::Code error, const QString &message) const
    {
        qWarning() << "error: " << message;
        return ExpectedData::error(error, message);
    }

    ExpectedData
    loadVault(const Device &device, const QString &name = QString(), const MountPoint &mountPoint = MountPoint(), const Payload &payload = Payload()) const
    {
        if (!config->hasGroup(device.data())) {
            return errorData(Error::DeviceError, i18n("Unknown device"));
        }

        Data vaultData;
        const QString backendName = payload[KEY_BACKEND].toString();
        const QStringList activities = payload[KEY_ACTIVITIES].toStringList();
        const bool isOfflineOnly = payload[KEY_OFFLINEONLY].toBool();

        // status should never be in this state, if we got an error,
        // d->data should not be valid
        vaultData.status = VaultInfo::Error;

        // Reading the mount data from the config
        KConfigGroup vaultConfig(config, device.data());
        vaultData.name = vaultConfig.readEntry(CFG_NAME, name);
        vaultData.backendName = vaultConfig.readEntry(CFG_BACKEND, backendName);
        vaultData.activities = vaultConfig.readEntry(CFG_ACTIVITIES, activities);
        vaultData.isOfflineOnly = vaultConfig.readEntry(CFG_OFFLINEONLY, isOfflineOnly);

        const QString configuredMountPoint = vaultConfig.readEntry(CFG_MOUNT_POINT, mountPoint.data());
        vaultData.mountPoint = MountPoint(configuredMountPoint);
        const QString actualMountPoint = vaultData.mountPoint.data();
        vaultConfig.writeEntry(CFG_MOUNT_POINT, actualMountPoint);

        const QDir mountPointDir(vaultData.mountPoint.data());

        return
            // If the backend is not known, we need to fail
            !Backend::availableBackends().contains(vaultData.backendName)
            ? errorData(Error::BackendError, i18n("Configured backend does not exist: %1", vaultData.backendName))
            : vaultData.mountPoint.isEmpty() // If the mount point is empty, we can not do anything
            ? errorData(Error::MountPointError, i18n("Mount point is not specified"))
            : !mountPointDir.exists() && !QDir().mkpath(vaultData.mountPoint.data()) // Lets try to create the mount point
                ? errorData(Error::MountPointError, i18n("Cannot create the mount point"))
                : !(vaultData.backend = Backend::instance(vaultData.backendName)) // Instantiate the backend if possible
                    ? errorData(Error::BackendError, i18n("Configured backend cannot be instantiated: %1", vaultData.backendName))
                    : ExpectedData::success(vaultData);
    }

    Private(Vault *parent, const Device &device)
        : q(parent)
        , config(KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE))
        , device(device)
        , data(loadVault(device))
    {
        updateStatus();
    }

    ~Private()
    {
        unlockDeviceDirectory();
    }

    template<typename T>
    T followFuture(VaultInfo::Status whileNotFinished, const T &future)
    {
        using namespace AsynQt::operators;

        Q_EMIT q->isBusyChanged(true);
        data->status = whileNotFinished;

        deletionState = DeletionBlocked;

        return future | onSuccess([this] {
                   updateStatus();

                   if (deletionState == DeletionScheduled) {
                       q->deleteLater();
                   }
                   deletionState = Normal;
               });
    }

    bool isInitialized() const
    {
        return data && data->backend->isInitialized(device);
    }

    bool isOpened() const
    {
        return data && data->backend->isOpened(data->mountPoint);
    }

    QList<int> parseResult(const QString &result)
    {
        const static QRegularExpression splitRegex(QStringLiteral("\\s+"));
        const QStringList pidList = result.split(splitRegex, Qt::SkipEmptyParts);
        QList<int> ret;
        for (const QString &pid : pidList) {
            if (int num = pid.toInt()) {
                ret << num;
            }
        }
        return ret;
    }
};

Vault::Vault(const Device &device, QObject *parent)
    : QObject(parent)
    , d(new Private(this, device))
{
    d->savingDelay.setInterval(300);
    d->savingDelay.setSingleShot(true);
    connect(&d->savingDelay, &QTimer::timeout, this, [&] {
        d->writeConfiguration();
        Q_EMIT infoChanged();
    });
}

Vault::~Vault()
{
    if (d->isOpened()) {
        close();
    }
}

void Vault::scheduleDeletion()
{
    if (d->deletionState == Private::Normal) {
        deleteLater();
    } else {
        d->deletionState = Private::DeletionScheduled;
    }
}

void Vault::saveConfiguration()
{
    d->savingDelay.start();
}

FutureResult<> Vault::create(const QString &name, const MountPoint &mountPoint, const Payload &payload)
{
    using namespace AsynQt::operators;

    return
        // If the backend is already known, and the device is initialized,
        // we do not want to do it again
        d->data && d->data->backend->isInitialized(d->device) ? errorResult(Error::DeviceError, i18n("This device is already registered. Cannot recreate it."))
        : !(d->data = d->loadVault(d->device, name, mountPoint, payload)) // Mount not open, check the error messages
        ? errorResult(Error::BackendError, i18n("Unknown error; unable to create the backend."))
        : d->followFuture(VaultInfo::Creating, d->data->backend->initialize(name, d->device, mountPoint, payload)) | onSuccess([mountPoint] {
              // If we have successfully created the vault,
              // lets try to set its icon
              QFile dotDir(mountPoint.data() + QStringLiteral("/.directory"));

              if (dotDir.open(QIODevice::WriteOnly | QIODevice::Text)) {
                  QTextStream out(&dotDir);
                  out << "[Desktop Entry]\nIcon=folder-decrypted\n";
              }
          });
}

FutureResult<> Vault::import(const QString &name, const MountPoint &mountPoint, const Payload &payload)
{
    using namespace AsynQt::operators;

    return
        // If the backend is already known, and the device is initialized,
        // we do not want to do it again
        d->data && (!d->data->backend->isInitialized(d->device))
        ? errorResult(Error::DeviceError, i18n("This device is not initialized. Cannot import it.")) // Mount not open, check the error messages
        : !(d->data = d->loadVault(d->device, name, mountPoint, payload))
        ? errorResult(Error::BackendError, i18n("Unknown error; unable to create the backend."))
        : d->followFuture(VaultInfo::Creating, d->data->backend->import(name, d->device, mountPoint, payload)) | onSuccess([mountPoint] {
              // If we have successfully created the vault,
              // lets try to set its icon
              QFile dotDir(mountPoint.data() + QStringLiteral("/.directory"));

              if (dotDir.open(QIODevice::WriteOnly | QIODevice::Text)) {
                  QTextStream out(&dotDir);
                  out << "[Desktop Entry]\nIcon=folder-decrypted\n";
              }
          });
}

FutureResult<> Vault::open(const Payload &payload)
{
    return
        // We can not mount something that has not been registered with us before
        !d->data ? errorResult(Error::BackendError, i18n("Cannot open an unknown vault."))
                 : d->followFuture(VaultInfo::Opening, d->data->backend->open(d->device, d->data->mountPoint, payload));
}

FutureResult<> Vault::close()
{
    using namespace AsynQt::operators;

    return
        // We can not mount something that has not been registered
        // with us before
        !d->data ? errorResult(Error::BackendError, i18n("The vault is unknown; cannot close it.")) :

                 // otherwise
        d->followFuture(VaultInfo::Closing, d->data->backend->close(d->device, d->data->mountPoint)) | onSuccess([this](const Result<> &result) {
            if (!isOpened() || result) {
                d->updateMessage(QString());

            } else {
                // We want to check whether there is an application
                // that is accessing the vault
                AsynQt::Process::getOutput(QStringLiteral("lsof"), {QStringLiteral("-t"), mountPoint().data()}) | cast<QString>() | onError([this] {
                    d->updateMessage(i18n("Unable to close the vault because an application is using it"));
                }) | onSuccess([this](const QString &result) {
                    // based on ksolidnotify.cpp
                    QStringList blockApps;

                    const QList<int> pidList = d->parseResult(result);

                    if (pidList.isEmpty()) {
                        d->updateMessage(i18n("Unable to close the vault because an application is using it"));
                        close();

                    } else {
                        KSysGuard::Processes procs;
                        for (int pid : pidList) {
                            procs.updateOrAddProcess(pid);

                            KSysGuard::Process *proc = procs.getProcess(pid);
                            if (!blockApps.contains(proc->name())) {
                                blockApps << proc->name();
                            }
                        }

                        blockApps.removeDuplicates();

                        d->updateMessage(i18n("Unable to close the vault because it is being used by %1", blockApps.join(QStringLiteral(", "))));
                    }
                });
            }
        });
}

// FutureResult<> Vault::configure()
// {
//     return close();
// }

FutureResult<> Vault::forceClose()
{
    using namespace AsynQt::operators;

    AsynQt::await(AsynQt::Process::getOutput(QStringLiteral("lsof"), {QStringLiteral("-t"), mountPoint().data()}) | cast<QString>() | onError([this] {
                      d->updateMessage(i18n("Failed to fetch the list of applications using this vault"));
                  })
                  | onSuccess([this](const QString &result) {
                        // based on ksolidnotify.cpp
                        const QList<int> pidList = d->parseResult(result);
                        KSysGuard::Processes procs;
                        for (int pid : pidList) {
                            procs.sendSignal(pid, SIGKILL);
                        }
                    }));

    return close();
}

FutureResult<> Vault::dismantle(const Payload &payload)
{
    const auto resolvedPath = [](const QString &path) {
        auto result = QDir(path).canonicalPath();
        if (!result.endsWith(QLatin1Char('/'))) {
            result += QLatin1Char('/');
        }
        return result;
    };

    const auto resolvedDevice = resolvedPath(d->device.data());

    const auto devices = availableDevices();
    const int matches = std::count_if(devices.cbegin(), devices.cend(), [&](const Device &device) {
        auto thisResolvedDevice = resolvedPath(device.data());

        auto diff = std::mismatch(resolvedDevice.cbegin(), resolvedDevice.cend(), thisResolvedDevice.cbegin(), thisResolvedDevice.cend());

        return diff.first == resolvedDevice.cend() || diff.second == thisResolvedDevice.cend();
    });

    return matches != 1 ? errorResult(Error::BackendError, i18n("Cannot delete the vault; there are other vaults with overlapping paths.")) :
                        // We can not mount something that has not been registered with us before
        !d->data ? errorResult(Error::BackendError, i18n("The vault is unknown; cannot dismantle it."))
        : QMessageBox::question(nullptr, // Let's confirm with the user once more
                                i18n("Are you sure you want to delete this vault"),
                                i18n("This operation will irreversibly delete the following:\n%1\n%2", d->device.data(), d->data->mountPoint.data()),
                                QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes
        ? errorResult(Error::OperationCancelled, i18n("Delete operation cancelled"))
        : d->followFuture(VaultInfo::Dismantling, d->data->backend->dismantle(d->device, d->data->mountPoint, payload));
}

VaultInfo::Status Vault::status() const
{
    return d->data->status;
}

bool Vault::isValid() const
{
    return d->data;
}

Device Vault::device() const
{
    return d->device;
}

QList<Device> Vault::availableDevices()
{
    const auto config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
    const KConfigGroup general(config, QStringLiteral("EncryptedDevices"));

    QList<Device> results;
    const QStringList keys = general.keyList();
    for (const auto &item : keys) {
        results << Device(item);
    }
    return results;
}

QStringList Vault::statusMessage()
{
    for (const auto &backendName : Backend::availableBackends()) {
        auto backend = Backend::instance(backendName);
        backend->validateBackend();
    }

    return {};
}

QString Vault::message() const
{
    if (!d->data) {
        return d->data.error().message();
    } else {
        return d->data->message;
    }
}

bool Vault::isInitialized() const
{
    return d->isInitialized();
}

bool Vault::isOpened() const
{
    return d->isOpened();
}

MountPoint Vault::mountPoint() const
{
    return d->data->mountPoint;
}

void Vault::setMountPoint(const MountPoint &mountPoint)
{
    if (d->data->mountPoint.data() != mountPoint.data()) {
        QDir().rmpath(d->data->mountPoint.data());
        QDir().mkpath(mountPoint.data());

        d->data->mountPoint = mountPoint;
        saveConfiguration();
    }
}

QStringList Vault::activities() const
{
    return d->data->activities;
}

void Vault::setActivities(const QStringList &activities)
{
    d->data->activities = activities;
    Q_EMIT activitiesChanged(activities);
    saveConfiguration();
}

bool Vault::isOfflineOnly() const
{
    return d->data->isOfflineOnly;
}

void Vault::setIsOfflineOnly(bool isOfflineOnly)
{
    d->data->isOfflineOnly = isOfflineOnly;
    Q_EMIT isOfflineOnlyChanged(isOfflineOnly);
    saveConfiguration();
}

QString Vault::name() const
{
    return d->data->name;
}

void Vault::setName(const QString &name)
{
    d->data->name = name;
    Q_EMIT nameChanged(name);
    saveConfiguration();
}

QString Vault::backend() const
{
    return d->data->backendName;
}

bool Vault::isBusy() const
{
    if (!d->data) {
        return false;
    }

    switch (status()) {
    case VaultInfo::Creating:
    case VaultInfo::Opening:
    case VaultInfo::Closing:
    case VaultInfo::Dismantling:
        return true;

    default:
        return false;
    }
}

VaultInfo Vault::info() const
{
    VaultInfo vaultInfo;

    vaultInfo.device = device().data();
    vaultInfo.name = name();

    vaultInfo.status = status();
    vaultInfo.message = message();

    vaultInfo.activities = activities();
    vaultInfo.isOfflineOnly = isOfflineOnly();

    return vaultInfo;
}

void Vault::updateStatus()
{
    d->updateStatus();
}

} // namespace PlasmaVault

#include "moc_vault.cpp"
