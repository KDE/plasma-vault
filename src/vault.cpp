/*
 *   Copyright (C) 2017 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "vault.h"

#include <QDir>
#include <QFutureWatcher>

#include <KSharedConfig>
#include <KConfig>
#include <KConfigGroup>

#include <KLocalizedString>

#include "backend_p.h"
#include "error.h"

#include "asynqt/basic/all.h"

#define CFG_NAME "name"
#define CFG_LAST_STATUS "lastStatus"
#define CFG_MOUNTPOINT "mountPoint"
#define CFG_BACKEND "backend"

namespace PlasmaVault {

class Vault::Private {
public:
    Vault * const q;

    KSharedConfigPtr config;
    QString device;



    struct Data {
        QString name;
        QString mountPoint;
        Vault::Status status;

        QString backendName;
        Backend::Ptr backend;
    };
    using ExpectedData = AsynQt::Expected<Data, PlasmaVault::Error>;
    ExpectedData data;



    void updateStatus()
    {
        if (data) {
            // Checking the status, and whether we should update it
            const auto oldStatus = data->status;
            const auto newStatus =
                           isOpened()      ? Vault::Opened :
                           isInitialized() ? Vault::Closed :
                                             Vault::NotInitialized;

            if (oldStatus == newStatus) return;

            data->status = newStatus;

            if (oldStatus == Vault::Opened || newStatus == Vault::Opened) {
                emit q->isOpenedChanged(newStatus);
            }

            if (oldStatus == Vault::NotInitialized || newStatus == Vault::NotInitialized) {
                emit q->isInitializedChanged(newStatus);
            }

            if (oldStatus == Vault::Creating
                    || oldStatus == Vault::Opening
                    || oldStatus == Vault::Closing
                    || oldStatus == Vault::Destroying) {
                emit q->isBusyChanged(false);
            }

            // Saving the data for the current mount
            KConfigGroup generalConfig(config, "EncryptedDevices");
            generalConfig.writeEntry(device, true);

            KConfigGroup vaultConfig(config, device);
            vaultConfig.writeEntry(CFG_LAST_STATUS, (int)data->status);
            vaultConfig.writeEntry(CFG_MOUNTPOINT,  data->mountPoint);
            vaultConfig.writeEntry(CFG_NAME,        data->name);
            vaultConfig.writeEntry(CFG_BACKEND,     data->backend->name());

            config->sync();

        } else {
            emit q->isOpenedChanged(false);
            emit q->isInitializedChanged(false);
            emit q->isBusyChanged(false);

            KConfigGroup generalConfig(config, "EncryptedDevices");
            generalConfig.writeEntry(device, false);

            KConfigGroup vaultConfig(config, device);
            vaultConfig.writeEntry(CFG_LAST_STATUS, (int)Vault::Error);
            vaultConfig.deleteEntry(CFG_MOUNTPOINT);
            vaultConfig.deleteEntry(CFG_NAME);
            vaultConfig.deleteEntry(CFG_BACKEND);
        }

        config->sync();
    }



    ExpectedData errorData(Error::Code error, const QString &message) const
    {
        qWarning() << "error: " << message;
        return ExpectedData::error(error, message);
    }



    ExpectedData openVault(const QString &device,
                           const QString &name = QString(),
                           const QString &mountPoint = QString(),
                           const QString &backendName = QString()) const
    {
        if (!config->hasGroup(device)) {
            return errorData(Error::DeviceError, i18n("Unknown device"));
        }

        Data vaultData;

        // status should never be in this state, if we got an error,
        // d->data should not be valid
        vaultData.status = Vault::Error;

        // Reading the mount data from the config
        const KConfigGroup vaultConfig(config, device);
        vaultData.name        = vaultConfig.readEntry(CFG_NAME, name);
        vaultData.mountPoint  = vaultConfig.readEntry(CFG_MOUNTPOINT, mountPoint);
        vaultData.backendName = vaultConfig.readEntry(CFG_BACKEND, backendName);

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



    Private(Vault *parent, const QString &device)
        : q(parent)
        , config(KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE))
        , device(device)
        , data(openVault(device))
    {
        updateStatus();
    }



    template <typename T>
    T followFuture(Vault::Status whileNotFinished, const T &future)
    {
        auto watcher = new QFutureWatcher<decltype(future.result())>(q);
        watcher->setFuture(future);

        emit q->isBusyChanged(true);
        data->status = whileNotFinished;

        if (!watcher->isFinished()) {

            QObject::connect(watcher, &QFutureWatcherBase::finished,
                             q, [=] () {
                                qDebug() << "Operation finished successfully <------------------------------";
                                updateStatus();
                                // watcher->deleteLater();
                             });

            QObject::connect(watcher, &QFutureWatcherBase::canceled,
                             q, [=] () {
                                qDebug() << "Operation was canceled, invalidating the object <------------------------------";
                                data->status = Vault::Error;
                                // watcher->deleteLater();
                             });

        } else {
            qDebug() << "Operation done even before we got to it <------------------------------";
            updateStatus();
        }

        return future;
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



Vault::Vault(const QString &device, QObject *parent)
    : QObject(parent)
    , d(new Private(this, device))
{
}



Vault::~Vault()
{
    close();
    delete d;
}



FutureResult<> Vault::create(const QString &name, const QString &mountPoint,
                             const QString &password,
                             const QString &backendName)
{
    return
        // If the backend is already known, and the device is initialized,
        // we do not want to do it again
        d->data && d->data->backend->isInitialized(d->device) ?
            errorResult(Error::DeviceError,
                        i18n("This device is already registered. Can not recreate it.")) :

        // Mount not open, check the error messages
        !(d->data = d->openVault(d->device, name, mountPoint, backendName)) ?
            errorResult(Error::BackendError,
                        i18n("Unknown error, unable to create the backend.")) :

        // otherwise
        d->followFuture(Creating,
                        d->data->backend->initialize(name, d->device, mountPoint, password));
}


FutureResult<> Vault::open(const QString &password)
{
    return
        // We can not mount something that has not been registered
        // with us before
        !d->data ? errorResult(Error::BackendError,
                               i18n("Can not open an unknown vault.")) :

        // otherwise
        d->followFuture(Opening,
                        d->data->backend->open(d->device, d->data->mountPoint, password));
}



FutureResult<> Vault::close()
{
    return
        // We can not mount something that has not been registered
        // with us before
        !d->data ? errorResult(Error::BackendError,
                               i18n("The vault is unknown, can not close it.")) :

        // otherwise
        d->followFuture(Closing,
                        d->data->backend->close(d->device, d->data->mountPoint));
}



FutureResult<> Vault::destroy(const QString &password)
{
    return
        // We can not mount something that has not been registered
        // with us before
        !d->data ? errorResult(Error::BackendError,
                               i18n("The vault is unknown, can not destroy it.")) :

        // otherwise
        d->followFuture(Destroying,
                        d->data->backend->destroy(d->device, d->data->mountPoint, password));
}



Vault::Status Vault::status() const
{
    return d->data->status;
}



QString Vault::name() const
{
    return d->data->name;
}



QString Vault::device() const
{
    return d->device;
}



QString Vault::mountPoint() const
{
    return d->data->mountPoint;
}



QStringList Vault::availableDevices()
{
    const auto config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
    const KConfigGroup general(config, "EncryptedDevices");
    return general.keyList();
}



QString Vault::errorMessage() const
{
    if (!d->data) {
        return d->data.error().message();
    } else {
        return {};
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



bool Vault::isBusy() const
{
    if (!d->data) {
        return false;
    }

    switch (status()) {
        case Creating:
        case Opening:
        case Closing:
        case Destroying:
            return true;

        default:
            return false;
    }
}

} // namespace PlasmaVault

