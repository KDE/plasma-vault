/*
 *   SPDX-FileCopyrightText: 2020 Martino Pilia <martino.pilia (at) gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "gocryptfsbackend.h"

#include <QDir>
#include <QProcess>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMountPoint>
#include <KSharedConfig>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>
#include <asynqt/wrappers/process.h>

#include <singleton_p.h>

using namespace AsynQt;

namespace PlasmaVault {

// See `man gocryptfs`, section EXIT CODES.
enum class ExitCode : int{
    Success = 0,

    // CIPHERDIR is not an emtpy directory (on "-init")
    NonEmptyCipherDir = 6,

    // MOUNTPOINT is not an empty directory
    NonEmptyMountPoint = 10,

    // Password incorrect
    WrongPassword = 12,

    // Password is empty (on "-init")
    EmptyPassword = 22,

    // Could not read gocryptfs.conf
    CannotReadConfig = 23,

    // Could not write gocryptfs.conf (on "-init" or "-password")
    CannotWriteConfig = 24,

    // fsck found errors
    FsckError = 26,
};



GocryptfsBackend::GocryptfsBackend()
{
}



GocryptfsBackend::~GocryptfsBackend()
{
}



Backend::Ptr GocryptfsBackend::instance()
{
    return singleton::instance<GocryptfsBackend>();
}



FutureResult<> GocryptfsBackend::mount(const Device &device,
                                       const MountPoint &mountPoint,
                                       const Vault::Payload &payload)
{
    QDir dir;

    const auto password = payload[KEY_PASSWORD].toString();

    if (!dir.mkpath(device.data()) || !dir.mkpath(mountPoint.data())) {
        return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
    }

    if (isInitialized(device)) {
        auto mountProcess = gocryptfs({
                device.data(),    // cypher data directory
                mountPoint.data() // mount point
                });

        auto mountResult = makeFuture(mountProcess, hasProcessFinishedSuccessfully);

        // Write password
        mountProcess->write(password.toUtf8() + "\n");

        return mountResult;
    } else {
        // Initialise cipherdir
        auto initProcess = gocryptfs({
                "-init",
                device.data(),
            });

        auto initResult = makeFuture(initProcess, [=] (QProcess *process) {
            auto const exitCode = static_cast<ExitCode>(process->exitCode());

            switch (exitCode) {
                case ExitCode::Success:
                    return AsynQt::await(mount(device, mountPoint, payload));

                case ExitCode::NonEmptyCipherDir:
                    return Result<>::error(Error::BackendError,
                                           i18n("The cipher directory is not empty, cannot initialise the vault."));

                case ExitCode::EmptyPassword:
                    return Result<>::error(Error::BackendError,
                                           i18n("The password is empty, cannot initialise the vault."));

                case ExitCode::CannotWriteConfig:
                    return Result<>::error(Error::BackendError,
                                           i18n("Cannot write gocryptfs.conf inside cipher directory, check your permissions."));

                default:
                    return Result<>::error(Error::CommandError,
                                           i18n("Unable to perform the operation (error code %1).", QString::number((int) exitCode)),
                                           process->readAllStandardOutput(),
                                           process->readAllStandardError());
            }
        });

        // Write password twice (insert and confirm)
        for (int i = 0; i < 2; ++i) {
            initProcess->write(password.toUtf8() + "\n");
        }

        return initResult;
    }
}



FutureResult<> GocryptfsBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    return
        collect(checkVersion(gocryptfs({ "--version" }), std::make_tuple(1, 7, 1)),
                checkVersion(fusermount({ "--version" }), std::make_tuple(2, 9, 7)))

        | transform([this] (const QPair<bool, QString> &gocryptfs,
                            const QPair<bool, QString> &fusermount) {

              bool success     = gocryptfs.first && fusermount.first;
              QString message  = formatMessageLine("gocryptfs", gocryptfs)
                               + formatMessageLine("fusermount", fusermount);

              return success ? Result<>::success()
                             : Result<>::error(Error::BackendError, message);
          });
}



bool GocryptfsBackend::isInitialized(const Device &device) const
{
    QFile gocryptfsConfig(getConfigFilePath(device));
    return gocryptfsConfig.exists();
}



QProcess *GocryptfsBackend::gocryptfs(const QStringList &arguments) const
{
    auto config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
    KConfigGroup backendConfig(config, "GocryptfsBackend");

    return process("gocryptfs",
                   arguments + backendConfig.readEntry("extraMountOptions", QStringList{}),
                   {});
}



QString GocryptfsBackend::getConfigFilePath(const Device &device) const
{
    return device.data() + QStringLiteral("/gocryptfs.conf");
}



} // namespace PlasmaVault

