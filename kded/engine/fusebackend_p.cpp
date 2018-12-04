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

#include "fusebackend_p.h"

#include <QUrl>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>

#include <KMountPoint>
#include <KLocalizedString>
#include <KIO/DeleteJob>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/wrappers/process.h>
#include <asynqt/wrappers/kjob.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>

#include "singleton_p.h"

using namespace AsynQt;

namespace PlasmaVault {


Result<> FuseBackend::hasProcessFinishedSuccessfully(QProcess *process)
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

        // If we have a message for the user, report it
        // !out.isEmpty() ?
        //     Result<>::error(Error::CommandError,
        //                     out) :

        // otherwise just report that we failed
            Result<>::error(Error::CommandError,
                            i18n("Unable to perform the operation"));
}



FuseBackend::FuseBackend()
{
}



FuseBackend::~FuseBackend()
{
}



QProcess *FuseBackend::process(const QString &executable,
                               const QStringList &arguments,
                               const QHash<QString, QString> &environment) const
{
    auto result = new QProcess();
    result->setProgram(executable);
    result->setArguments(arguments);

    if (environment.count() > 0) {
        auto env = result->processEnvironment();
        for (const auto& key: environment.keys()) {
            env.insert(key, environment[key]);
        }
        result->setProcessEnvironment(env);
    }

    return result;
}



QProcess *FuseBackend::fusermount(const QStringList &arguments) const
{
    return process("fusermount", arguments, {});
}



FutureResult<> FuseBackend::initialize(const QString &name,
                                        const Device &device,
                                        const MountPoint &mountPoint,
                                        const Vault::Payload &payload)
{
    Q_UNUSED(name);

    return
        isInitialized(device) ?
            errorResult(Error::BackendError,
                        i18n("This directory already contains encrypted data")) :

        directoryExists(device) || directoryExists(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("You need to select empty directories for the encrypted storage and for the mount point")) :

        // otherwise
            mount(device, mountPoint, payload);
}



FutureResult<> FuseBackend::import(const QString &name,
                                   const Device &device,
                                   const MountPoint &mountPoint,
                                   const Vault::Payload &payload)
{
    Q_UNUSED(name);

    return
        !isInitialized(device) ?
            errorResult(Error::BackendError,
                        i18n("This directory doesn't contain encrypted data")) :

        !directoryExists(device) || directoryExists(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("You need to select an empty directory for the mount point")) :

        // otherwise
            mount(device, mountPoint, payload);
}



FutureResult<> FuseBackend::open(const Device &device,
                                  const MountPoint &mountPoint,
                                  const Vault::Payload &payload)
{
    return
        isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is already open")) :

        // otherwise
            mount(device, mountPoint, payload);
}



FutureResult<> FuseBackend::close(const Device &device,
                                   const MountPoint &mountPoint)
{
    Q_UNUSED(device);

    return
        !isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is not open")) :

        // otherwise
            makeFuture(fusermount({ "-u", mountPoint }),
                       hasProcessFinishedSuccessfully);
}



FutureResult<> FuseBackend::dismantle(const Device &device,
                                      const MountPoint &mountPoint,
                                      const Vault::Payload &payload)
{
    // TODO:
    // mount
    // unmount
    // remove the directories
    // return Fuse::dismantle(device, mountPoint, password);

    Q_UNUSED(payload)

    // Removing the data and the mount point
    return transform(makeFuture<KJob*>(
            KIO::del( { QUrl::fromLocalFile(device.data())
                      , QUrl::fromLocalFile(mountPoint.data())
                      } )),
            [] (KJob *job) {
                job->deleteLater();
                return job->error() == 0 ? Result<>::success()
                                         : Result<>::error(Error::DeletionError, job->errorString());
            }
        );
}



QFuture<QPair<bool, QString>> FuseBackend::checkVersion(
        QProcess *process,
        const std::tuple<int,int,int> &requiredVersion) const
{
    using namespace AsynQt::operators;

    return makeFuture(process, [=] (QProcess *process) {

              if (process->exitStatus() != QProcess::NormalExit) {
                  return qMakePair(
                              false,
                              i18n("Failed to execute"));
              }

              QRegularExpression versionMatcher("([0-9]+)[.]([0-9]+)[.]([0-9]+)");

              const auto out = process->readAllStandardOutput();
              const auto err = process->readAllStandardError();
              const auto all = out + err;

              const auto matches = versionMatcher.match(all);

              if (!matches.hasMatch()) {
                  return qMakePair(
                              false,
                              i18n("Unable to detect the version"));
              }

              const auto matchedVersion =
                  std::make_tuple(matches.captured(1).toInt(),
                                  matches.captured(2).toInt(),
                                  matches.captured(3).toInt());

              if (matchedVersion < requiredVersion) {
                  // Bad version, we need to notify the world
                  return qMakePair(
                              false,
                              i18n("Wrong version installed. The required version is %1.%2.%3",
                                  std::get<0>(requiredVersion),
                                  std::get<1>(requiredVersion),
                                  std::get<2>(requiredVersion)
                              ));
              }

              return qMakePair(
                          true,
                          i18n("Correct version found"));
        });
}



bool FuseBackend::isOpened(const MountPoint &mountPoint) const
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr
        = KMountPoint::currentMountPoints().findByPath(mountPoint);

    // we can not rely on ptr->realDeviceName() since it is empty,
    // KMountPoint can not get the source

    return ptr && ptr->mountPoint() == mountPoint;
}

} // namespace PlasmaVault

