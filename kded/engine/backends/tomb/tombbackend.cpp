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

#include "tombbackend.h"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KMountPoint>
#include <KLocalizedString>

#include <algorithm>

#include <kauth.h>

#include <asynqt/basic/all.h>
#include <asynqt/wrappers/process.h>
#include <asynqt/wrappers/kauth.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>

#include <singleton_p.h>

#include "tombexec.h"

using namespace AsynQt;

namespace PlasmaVault {

inline
QString tombExecutable()
{
    const auto result = QStandardPaths::findExecutable("tomb");
    Q_ASSERT(!result.isEmpty());
    return result;
}


FutureResult<> createTomb(const Device &device,
                          const Vault::Payload &payload)
{
    QDir dir;

    if (!dir.mkpath(device)) {
        return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
    }

    return AsynQt::makeFuture(
        execTomb(tombExecutable(),
             QStringList {
                 "dig",
                 "-s",
                 QString::number(payload[PAYLOAD_VAULT_SIZE].toInt()),
                 device + "/tomb"
             }),
        Backend::hasProcessFinishedSuccessfully);
}


FutureResult<> createKey(const Vault::Payload &payload)
{
    return AsynQt::makeFuture(
        execTomb(tombExecutable(),
             QStringList {
                 "forge",
                 payload[PAYLOAD_KEY_FILE].toString(),
                 "--unsafe",
                 "--tomb-pwd",
                 payload[PAYLOAD_PASSWORD].toString()
             }),
        Backend::hasProcessFinishedSuccessfully);
}

FutureResult<> lockTomb(const Vault::Payload &payload)
{
    return AsynQt::KAuth::exec(
            "org.kde.plasma.vault.tomb.lock",
            "org.kde.plasma.vault.tomb",
            {
                { "tomb"     , tombExecutable() },
                { "device"   , payload[PAYLOAD_DEVICE] },
                { "keyFile"  , payload[PAYLOAD_KEY_FILE] },
                { "password" , payload[PAYLOAD_PASSWORD] }
            },
            [] (bool success, ::KAuth::ExecuteJob *job) {
            qDebug() << "lockTomb: success? " << success;
                if (success) {
                    return Result<>::success();
                } else {
                    return Result<>::error(Error::BackendError, i18n("Failed to lock the vault with the given key"));
                }
            }
        );
}

FutureResult<> openTomb(const Device &device,
                        const MountPoint &mountPoint,
                        const Vault::Payload &payload)
{
    QDir dir;

    if (!dir.mkpath(device) || !dir.mkpath(mountPoint)) {
        return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
    }

    return AsynQt::KAuth::exec(
            "org.kde.plasma.vault.tomb.open",
            "org.kde.plasma.vault.tomb",
            {
                { "tomb"       , tombExecutable() },
                { "device"     , device.data() },
                { "mountPoint" , mountPoint.data() },
                { "keyFile"    , payload[PAYLOAD_KEY_FILE] },
                { "password"   , payload[PAYLOAD_PASSWORD] }
            },
            [] (bool success, ::KAuth::ExecuteJob *job) {
                if (success) {
                    return Result<>::success();
                } else {
                    return Result<>::error(Error::BackendError, i18n("Failed to lock the vault with the given key"));
                }
            }
        );
}

FutureResult<> closeTomb(const Device &device,
                         const MountPoint &mountPoint)
{
    return AsynQt::KAuth::exec(
            "org.kde.plasma.vault.tomb.close",
            "org.kde.plasma.vault.tomb",
            {
                { "tomb"       , tombExecutable() },
                { "device"     , device.data() },
                { "mountPoint" , mountPoint.data() }
            },
            [] (bool success, ::KAuth::ExecuteJob *job) {
                if (success) {
                    return Result<>::success();
                } else {
                    return Result<>::error(Error::BackendError, i18n("Failed to lock the vault with the given key"));
                }
            }
        );
}



TombBackend::TombBackend()
{
}



TombBackend::~TombBackend()
{
}



Backend::Ptr TombBackend::instance()
{
    return singleton::instance<TombBackend>();
}



FutureResult<> TombBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // Check zsh 5.3.1
    // Check tomb 2.3
    // Check steghide 0.5.1
    // Check /sbin/cryptsetup 1.7.3

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    // return
    //     collect(checkVersion(tomb({ "--version" }), std::make_tuple(2, 3)))
    //
    //     | transform([this] (const QPair<bool, QString> &tomb) {
    //
    //           bool success     = tomb.first;
    //           QString message  = formatMessageLine("tomb", tomb);
    //
    //           return success ? Result<>::success()
    //                          : Result<>::error(Error::BackendError, message);
    //       });

    return makeReadyFuture(Result<>::success());
}



bool TombBackend::isInitialized(const Device &device) const
{
    QFile cryFsConfig(device + "/tomb");
    return cryFsConfig.exists();
}



bool TombBackend::isOpened(const MountPoint &mountPoint) const
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr
        = KMountPoint::currentMountPoints().findByPath(mountPoint);

    // we can not rely on ptr->realDeviceName() since it is empty,
    // KMountPoint can not get the source

    return ptr && ptr->mountPoint() == mountPoint;
}



FutureResult<> TombBackend::initialize(const QString &name,
                                       const Device &device,
                                       const MountPoint &mountPoint,
                                       const Vault::Payload &payload)
{
    Q_UNUSED(name);

    return
        isInitialized(device) ?
            errorResult(Error::BackendError,
                        i18n("This directory already contains encrypted data")) :

        !isDirectoryEmpty(device) || !isDirectoryEmpty(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("You need to select empty directories for the encrypted storage and for the mount point")) :

        // otherwise
           [=] {
               const auto tombCreated = AsynQt::await(createTomb(device, payload));
               if (!tombCreated) {
                   return errorResult(
                           Error::BackendError,
                           i18n("Unable to create the container file"));
               }
               qDebug() << "Created the container file: " << device;

               const auto keyCreated = AsynQt::await(createKey(payload));
               if (!keyCreated) {
                   return errorResult(
                           Error::BackendError,
                           i18n("Unable to create the key file"));
               }
               qDebug() << "Created the key file: " << payload[PAYLOAD_KEY_FILE].toString();

               const auto tombLocked = AsynQt::await(lockTomb(payload));
               if (!tombLocked) {
                   return errorResult(
                           Error::BackendError,
                           i18n("Unable to lock the container file"));
               }
               qDebug() << "Successfully locked the file";

               const auto tombOpened = AsynQt::await(openTomb(device, mountPoint, payload));
               if (!tombOpened) {
                   return errorResult(
                           Error::BackendError,
                           i18n("Failed to open the vault"));
               }
               qDebug() << "Opened the vault";

               return makeReadyFuture(Result<>::success());
           }();
}



FutureResult<> TombBackend::open(const Device &device,
                                 const MountPoint &mountPoint,
                                 const Vault::Payload &payload)
{
    return openTomb(device, mountPoint, payload);
}



FutureResult<> TombBackend::close(const Device &device,
                                  const MountPoint &mountPoint)
{
    return closeTomb(device, mountPoint);
}



FutureResult<> TombBackend::destroy(const Device &device,
                                    const MountPoint &mountPoint,
                                    const Vault::Payload &payload)
{
    return FutureResult<>();
}



FutureResult<> validateBackend()
{
    return FutureResult<>();
}



} // namespace PlasmaVault

