/*
 *   Copyright 2017 by Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vault.h"

#include <QDir>
#include <QFutureWatcher>
#include <QDBusMetaType>
#include <QUrl>
#include <QPointer>

#include <KSharedConfig>
#include <KConfig>
#include <KConfigGroup>

#include <processcore/process.h>
#include <processcore/processes.h>

#include <KLocalizedString>
#include <kdirnotify.h>

#include "backend_p.h"

#include "asynqt/basic/all.h"
#include "asynqt/wrappers/process.h"
#include "asynqt/operations/listen.h"
#include "asynqt/operations/cast.h"

#define CFG_NAME "name"
#define CFG_LAST_STATUS "lastStatus"
#define CFG_MOUNT_POINT "mountPoint"
#define CFG_BACKEND "backend"
#define CFG_ACTIVITIES "activities"

namespace PlasmaVault {

class Vault::Private {
public:
    Vault * const q;

    KSharedConfigPtr config;
    Device device;



    struct Data {
        QString name;
        MountPoint mountPoint;
        VaultInfo::Status status;
        QStringList activities;
        QString message;

        QString backendName;
        Backend::Ptr backend;
    };
    using ExpectedData = AsynQt::Expected<Data, PlasmaVault::Error>;
    ExpectedData data;



    void updateMessage(const QString &message)
    {
        if (!data) return;

        data->message = message;
        emit q->messageChanged(message);
    }


    void updateStatus()
    {
        if (data) {
            // Checking the status, and whether we should update it
            const auto oldStatus = data->status;
            const auto newStatus =
                           isOpened()      ? VaultInfo::Opened :
                           isInitialized() ? VaultInfo::Closed :
                                             VaultInfo::NotInitialized;

            if (oldStatus == newStatus) return;

            data->status = newStatus;

            emit q->statusChanged(data->status);

            if (oldStatus == VaultInfo::Opened || newStatus == VaultInfo::Opened) {
                emit q->isOpenedChanged(newStatus);
            }

            if (oldStatus == VaultInfo::NotInitialized || newStatus == VaultInfo::NotInitialized) {
                emit q->isInitializedChanged(newStatus);
            }

            if (oldStatus == VaultInfo::Creating
                    || oldStatus == VaultInfo::Opening
                    || oldStatus == VaultInfo::Closing
                    || oldStatus == VaultInfo::Destroying) {
                emit q->isBusyChanged(false);
            }

            // Saving the data for the current mount
            KConfigGroup generalConfig(config, "EncryptedDevices");
            generalConfig.writeEntry(device.data(), true);

            KConfigGroup vaultConfig(config, device.data());
            vaultConfig.writeEntry(CFG_LAST_STATUS, (int)data->status);
            vaultConfig.writeEntry(CFG_MOUNT_POINT, data->mountPoint.data());
            vaultConfig.writeEntry(CFG_NAME,        data->name);
            vaultConfig.writeEntry(CFG_BACKEND,     data->backend->name());
            vaultConfig.writeEntry(CFG_ACTIVITIES,  data->activities);

            org::kde::KDirNotify::emitFilesAdded(
                    QUrl::fromLocalFile(data->mountPoint.data()));

            config->sync();

        } else {
            emit q->isOpenedChanged(false);
            emit q->isInitializedChanged(false);
            emit q->isBusyChanged(false);

            KConfigGroup generalConfig(config, "EncryptedDevices");
            generalConfig.writeEntry(device.data(), false);

            KConfigGroup vaultConfig(config, device.data());
            vaultConfig.writeEntry(CFG_LAST_STATUS, (int)VaultInfo::Error);
            // vaultConfig.deleteEntry(CFG_MOUNT_POINT);
            // vaultConfig.deleteEntry(CFG_NAME);
            // vaultConfig.deleteEntry(CFG_BACKEND);
            // vaultConfig.deleteEntry(CFG_ACTIVITIES);

            emit q->statusChanged(VaultInfo::Error);
        }

        config->sync();
    }



    ExpectedData errorData(Error::Code error, const QString &message) const
    {
        qWarning() << "error: " << message;
        return ExpectedData::error(error, message);
    }



    ExpectedData loadVault(const Device &device,
                           const QString &name = QString(),
                           const MountPoint &mountPoint = MountPoint(),
                           const Payload &payload = Payload()) const
    {
        if (!config->hasGroup(device.data())) {
            return errorData(Error::DeviceError, i18n("Unknown device"));
        }

        Data vaultData;
        const QString backendName    = payload[KEY_BACKEND].toString();
        const QStringList activities = payload[KEY_ACTIVITIES].toStringList();

        // status should never be in this state, if we got an error,
        // d->data should not be valid
        vaultData.status = VaultInfo::Error;

        // Reading the mount data from the config
        const KConfigGroup vaultConfig(config, device.data());
        vaultData.name        = vaultConfig.readEntry(CFG_NAME, name);
        vaultData.mountPoint  = MountPoint(vaultConfig.readEntry(CFG_MOUNT_POINT, mountPoint.data()));
        vaultData.backendName = vaultConfig.readEntry(CFG_BACKEND, backendName);
        vaultData.activities  = vaultConfig.readEntry(CFG_ACTIVITIES, activities);

        const QDir mountPointDir(vaultData.mountPoint);


        return
            // If the backend is not known, we need to fail
            !Backend::availableBackends().contains(vaultData.backendName) ?
                errorData(Error::BackendError,
                          i18n("Configured backend does not exist: %1", vaultData.backendName)) :

            // If the mount point is empty, we can not do anything
            vaultData.mountPoint.isEmpty() ?
                errorData(Error::MountPointError,
                          i18n("Mount point is not specified")) :

            // Lets try to create the mount point
            !mountPointDir.exists() && !QDir().mkpath(vaultData.mountPoint) ?
                errorData(Error::MountPointError,
                          i18n("Can not create the mount point")) :

            // Instantiate the backend if possible
            !(vaultData.backend = Backend::instance(vaultData.backendName)) ?
                errorData(Error::BackendError,
                          i18n("Configured backend can not be instantiated: %1", vaultData.backendName)) :

            // otherwise
            ExpectedData::success(vaultData);
    }



    Private(Vault *parent, const Device &device)
        : q(parent)
        , config(KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE))
        , device(device)
        , data(loadVault(device))
    {
        updateStatus();
    }



    template <typename T>
    T followFuture(VaultInfo::Status whileNotFinished, const T &future)
    {
        using namespace AsynQt::operators;

        emit q->isBusyChanged(true);
        data->status = whileNotFinished;

        return future | onSuccess([this] {
                updateStatus();
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
};



Vault::Vault(const Device &device, QObject *parent)
    : QObject(parent)
    , d(new Private(this, device))
{
}



Vault::~Vault()
{
    close();
}



FutureResult<> Vault::create(const QString &name, const MountPoint &mountPoint,
                             const Payload &payload)
{
    using namespace AsynQt::operators;

    return
        // If the backend is already known, and the device is initialized,
        // we do not want to do it again
        d->data && d->data->backend->isInitialized(d->device) ?
            errorResult(Error::DeviceError,
                        i18n("This device is already registered. Can not recreate it.")) :

        // Mount not open, check the error messages
        !(d->data = d->loadVault(d->device, name, mountPoint, payload)) ?
            errorResult(Error::BackendError,
                        i18n("Unknown error, unable to create the backend.")) :

        // otherwise
        d->followFuture(VaultInfo::Creating,
                        d->data->backend->initialize(name, d->device, mountPoint, payload))
            | onSuccess([mountPoint] {
                // If we have successfully created the vault,
                // lets try to set its icon
                QFile dotDir(mountPoint + "/.directory");

                if (dotDir.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&dotDir);
                    out << "[Desktop Entry]\nIcon=folder-decrypted\n";
                }
            });
}


FutureResult<> Vault::open(const Payload &payload)
{
    return
        // We can not mount something that has not been registered
        // with us before
        !d->data ? errorResult(Error::BackendError,
                               i18n("Can not open an unknown vault.")) :

        // otherwise
        d->followFuture(VaultInfo::Opening,
                        d->data->backend->open(d->device, d->data->mountPoint, payload));
}



FutureResult<> Vault::close()
{
    using namespace AsynQt::operators;

    return
        // We can not mount something that has not been registered
        // with us before
        !d->data ? errorResult(Error::BackendError,
                               i18n("The vault is unknown, can not close it.")) :

        // otherwise
        d->followFuture(VaultInfo::Closing,
                        d->data->backend->close(d->device, d->data->mountPoint))
            | onSuccess([this] (const Result<> &result) {
                if (!isOpened() || result) {
                    d->updateMessage(QString());

                } else {
                    // We want to check whether there is an application
                    // that is accessing the vault
                    AsynQt::Process::getOutput("lsof", { "-t", mountPoint() })
                        | cast<QString>()
                        | onError([this] {
                            d->updateMessage(i18n("Unable to close the vault, an application is using it"));
                        })
                        | onSuccess([this] (const QString &result) {
                            // based on ksolidnotify.cpp
                            QStringList blockApps;

                            const auto &pidList =
                            result.split(QRegExp(QStringLiteral("\\s+")),
                                         QString::SkipEmptyParts);

                            KSysGuard::Processes procs;

                            for (const QString &pidStr: pidList) {
                                int pid = pidStr.toInt();
                                if (!pid) {
                                    continue;
                                }

                                procs.updateOrAddProcess(pid);

                                KSysGuard::Process *proc = procs.getProcess(pid);

                                if (!blockApps.contains(proc->name())) {
                                    blockApps << proc->name();
                                }
                            }

                            blockApps.removeDuplicates();

                            d->updateMessage(i18n("Unable to close the vault, it is used by %1", blockApps.join(", ")));
                        });
                    }
                });
}



FutureResult<> Vault::destroy(const Payload &payload)
{
    return
        // We can not mount something that has not been registered
        // with us before
        !d->data ? errorResult(Error::BackendError,
                               i18n("The vault is unknown, can not destroy it.")) :

        // otherwise
        d->followFuture(VaultInfo::Destroying,
                        d->data->backend->destroy(d->device, d->data->mountPoint, payload));
}



VaultInfo::Status Vault::status() const
{
    return d->data->status;
}



bool Vault::isValid() const
{
    return d->data;
}



QString Vault::name() const
{
    return d->data->name;
}



Device Vault::device() const
{
    return d->device;
}



MountPoint Vault::mountPoint() const
{
    return d->data->mountPoint;
}



QList<Device> Vault::availableDevices()
{
    const auto config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
    const KConfigGroup general(config, "EncryptedDevices");

    QList<Device> results;
    for (const auto& item: general.keyList()) {
        results << Device(item);
    }
    return results;
}



QStringList Vault::statusMessage()
{
    for (const auto& backendName: Backend::availableBackends()) {
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



QStringList Vault::activities() const
{
    return d->data->activities;
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
        case VaultInfo::Destroying:
            return true;

        default:
            return false;
    }
}



VaultInfo Vault::info() const
{
    VaultInfo vaultInfo;
    vaultInfo.device     = device();
    vaultInfo.name       = name();
    vaultInfo.status     = status();
    vaultInfo.activities = activities();
    vaultInfo.message    = message();
    return vaultInfo;
}

} // namespace PlasmaVault


