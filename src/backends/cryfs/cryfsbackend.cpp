/*
 *   Copyright (C) 2017 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#include "cryfsbackend.h"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>

#include <singleton_p.h>

#include <KMountPoint>
#include <KLocalizedString>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/wrappers/process.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>

using namespace AsynQt;

namespace PlasmaVault {


namespace CryFs {

    static QProcess *process(const QString &executable, const QStringList &arguments)
    {
        auto result = new QProcess();
        result->setProgram(executable);
        result->setArguments(arguments);

        auto env = result->processEnvironment();
        env.insert("CRYFS_FRONTEND", "noninteractive");
        result->setProcessEnvironment(env);

        return result;
    }


#define DEFINE_PROCESS(Name, Executable)                                   \
    inline static QProcess *Name(const QStringList &arguments = QStringList())    \
    {                                                                      \
        return process(Executable, arguments);                             \
    }

    DEFINE_PROCESS(cryfs,      "cryfs")
    DEFINE_PROCESS(fusermount, "fusermount")
#undef DEFINE_PROCESS



    Result<> hasFinishedSuccessfully(QProcess *process)
    {
        const auto out = process->readAllStandardOutput();
        const auto err = process->readAllStandardError();

        return
            // If all went well, just return success
            (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) ?
                Result<>::success() :

            // If we tried to mount into a non-empty location, report
            err.contains("'nonempty'") ?
                Result<>::error(Error::CommandError,
                                i18n("The mount point directory is not empty, refusing to open the vault")) :

            // If we have a message for the user, report it ... or not, cryfs messages are useless
            // !out.isEmpty() ?
            //     Result<>::error(Error::CommandError,
            //                     out) :

            // otherwise just report that we failed
                Result<>::error(Error::CommandError,
                                i18n("Unable to open the vault"));
    }



    FutureResult<> open(const Device &device, const MountPoint &mountPoint,
                        const Vault::Payload &payload)
    {
        QDir dir;

        const auto password = payload[KEY_PASSWORD].toString();
        const auto cypher   = payload["cryfs-cipher"].toString();

        if (!dir.mkpath(device) || !dir.mkpath(mountPoint)) {
            return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
        }

        auto process =
            cypher.isEmpty() ?
                // Cypher is not specified, use the default, whatever it is
                cryfs({
                    device, // source directory to initialize cryfs in
                    mountPoint // where to mount the file system
                })

            :   // Cypher is specified, use it to create the device
                cryfs({
                    "--cipher",
                    cypher,
                    device, // source directory to initialize cryfs in
                    mountPoint // where to mount the file system
                })
            ;

        auto result
            = makeFuture(process, hasFinishedSuccessfully);

        // Writing the password
        process->write(password.toUtf8());
        process->write("\n");

        return result;
    }

} // namespace CryFs



CryFsBackend::CryFsBackend()
{
}



CryFsBackend::~CryFsBackend()
{
}



Backend::Ptr CryFsBackend::instance()
{
    return util::singleton::instance<CryFsBackend>();
}



FutureResult<> CryFsBackend::initialize(const QString &name,
                                        const Device &device,
                                        const MountPoint &mountPoint,
                                        const Vault::Payload &payload)
{
    Q_UNUSED(name);

    return
        isInitialized(device) ?
            errorResult(Error::BackendError,
                        i18n("This directory already contains encrypted CryFS data")) :

        !isDirectoryEmpty(device) || !isDirectoryEmpty(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("You need to select empty directories for the encrypted storage and for the mount point")) :

        // otherwise
            CryFs::open(device, mountPoint, payload);
}



FutureResult<> CryFsBackend::open(const Device &device,
                                  const MountPoint &mountPoint,
                                  const Vault::Payload &payload)
{
    return
        isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is already open")) :

        // otherwise
            CryFs::open(device, mountPoint, payload);
}



FutureResult<> CryFsBackend::close(const Device &device,
                                   const MountPoint &mountPoint)
{
    Q_UNUSED(device);

    return
        !isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is not open")) :

        // otherwise
            makeFuture(CryFs::fusermount({ "-u", mountPoint }),
                       CryFs::hasFinishedSuccessfully);
}



FutureResult<> CryFsBackend::destroy(const Device &device,
                                     const MountPoint &mountPoint,
                                     const Vault::Payload &payload)
{
    // TODO:
    // mount
    // unmount
    // remove the directories
    // return CryFs::destroy(device, mountPoint, payload);

    Q_UNUSED(device)
    Q_UNUSED(mountPoint)
    Q_UNUSED(payload)
    return {};
}



FutureResult<> CryFsBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    return
        collect(makeFuture(CryFs::cryfs({ "--version" })),
                makeFuture(CryFs::fusermount({ "--version" })))

        | transform([] (QProcess *cryfs, QProcess *fusermount) {
              qDebug() << "Called --version on everything <==========================";

              auto processTest = [] (QProcess *process, std::tuple<int, int, int> requiredVersion) {

                  if (process->exitStatus() != QProcess::NormalExit || process->exitCode() != 0) {
                      return std::make_pair(
                                  false,
                                  i18n("Failed to execute"));
                  }

                  QRegularExpression versionMatcher("([0-9]+)[.]([0-9]+)[.]([0-9]+)");

                  const auto out = process->readAllStandardOutput();
                  const auto err = process->readAllStandardError();
                  const auto all = out + err;

                  const auto matches = versionMatcher.match(all);

                  if (!matches.isValid()) {
                      return std::make_pair(
                                  false,
                                  i18n("Unable to detect the version"));
                  }

                  const auto matchedVersion =
                      std::make_tuple(matches.captured(1).toInt(),
                                      matches.captured(2).toInt(),
                                      matches.captured(3).toInt());

                  if (matchedVersion < requiredVersion) {
                      // Bad version, we need to notify the world
                      return std::make_pair(
                                  false,
                                  i18n("Error: Wrong version installed. The required version is %1.%2.%3",
                                      std::get<0>(requiredVersion),
                                      std::get<1>(requiredVersion),
                                      std::get<2>(requiredVersion)
                                  )
                              );
                  }

                  return std::make_pair(
                              true,
                              i18n("Correct version found"));

              };

              bool summarySuccess = true;
              QString summaryMessage;

              bool success;
              QString message;

              std::tie(success, message) = processTest(cryfs, std::make_tuple(0, 9, 6));
              summarySuccess &= success;
              summaryMessage += i18n("cryfs: %1", message) + "\n";

              std::tie(success, message) = processTest(fusermount, std::make_tuple(2, 9, 7));
              summarySuccess &= success;
              summaryMessage += i18n("fusermount: %1", message) + "\n";

              qDebug() << "Summary: " << summarySuccess << summaryMessage;

              return Result<>::success();
          });
}



bool CryFsBackend::isInitialized(const Device &device) const
{
    QFile cryFsConfig(device + "/cryfs.config");

    qDebug() << "Is initialized? " << cryFsConfig.fileName() << cryFsConfig.exists();
    return cryFsConfig.exists();
}



bool CryFsBackend::isOpened(const MountPoint &mountPoint) const
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr
        = KMountPoint::currentMountPoints().findByPath(mountPoint);

    // we can not rely on ptr->realDeviceName() since it is empty,
    // KMountPoint can not get the cryfs source

    return ptr && ptr->mountPoint() == mountPoint;
}


} // namespace PlasmaVault

