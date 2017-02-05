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

#include "encfsbackend.h"

#include <QDir>
#include <QProcess>

#include <singleton_p.h>

#include <KMountPoint>
#include <KLocalizedString>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/wrappers/process.h>

using namespace AsynQt;

namespace PlasmaVault {


namespace EncFs {

    QProcess *process(const QString &executable, const QStringList &arguments)
    {
        auto result = new QProcess();
        result->setProgram(executable);
        result->setArguments(arguments);
        return result;
    }


#define DEFINE_PROCESS(Name, Executable)                                   \
    inline QProcess *Name(const QStringList &arguments = QStringList())    \
    {                                                                      \
        return process(Executable, arguments);                             \
    }

    DEFINE_PROCESS(encfs,      "encfs")
    DEFINE_PROCESS(encfsctl,   "encfsctl")
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

            // If we have a message for the user, report it
            !out.isEmpty() ?
                Result<>::error(Error::CommandError,
                                out) :

            // otherwise just report that we failed
                Result<>::error(Error::CommandError,
                                i18n("Unable to open the vault"));
    }



    FutureResult<> open(const Device &device, const MountPoint &mountPoint,
                        const QString &password)
    {
        QDir dir;

        if (!dir.mkpath(device) || !dir.mkpath(mountPoint)) {
            return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
        }

        auto process = encfs({
                "-S", // read password from stdin
                "--standard", // If creating a file system, use the default options
                device, // source directory to initialize encfs in
                mountPoint // where to mount the file system
            });

        auto result
            = makeFuture(process, hasFinishedSuccessfully);

        // Writing the password
        process->write(password.toUtf8());
        process->write("\n");

        return result;
    }

} // namespace EncFs



EncFsBackend::EncFsBackend()
{
}



EncFsBackend::~EncFsBackend()
{
}



Backend::Ptr EncFsBackend::instance()
{
    return util::singleton::instance<EncFsBackend>();
}



bool isDirectoryEmpty(const QString &path)
{
    QDir dir(path);

    if (!dir.exists()) return true;

    return dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0;
}



FutureResult<> EncFsBackend::initialize(const QString &name,
                                        const Device &device,
                                        const MountPoint &mountPoint,
                                        const QString &password)
{
    Q_UNUSED(name);

    return
        isInitialized(device) ?
            errorResult(Error::BackendError,
                        i18n("This directory already contains encrypted EncFS data")) :

        !isDirectoryEmpty(device) || !isDirectoryEmpty(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("You need to select empty directories for the encrypted storage and for the mount point")) :

        // otherwise
            EncFs::open(device, mountPoint, password);
}



FutureResult<> EncFsBackend::open(const Device &device,
                                  const MountPoint &mountPoint,
                                  const QString &password)
{
    return
        isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is already open")) :

        // otherwise
            EncFs::open(device, mountPoint, password);
}



FutureResult<> EncFsBackend::close(const Device &device,
                                   const MountPoint &mountPoint)
{
    Q_UNUSED(device);

    return
        !isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is not open")) :

        // otherwise
            makeFuture(EncFs::fusermount({ "-u", mountPoint }),
                       EncFs::hasFinishedSuccessfully);
}



FutureResult<> EncFsBackend::destroy(const Device &device,
                                     const MountPoint &mountPoint,
                                     const QString &password)
{
    // TODO:
    // mount
    // unmount
    // remove the directories
    // return EncFs::destroy(device, mountPoint, password);

    Q_UNUSED(device)
    Q_UNUSED(mountPoint)
    Q_UNUSED(password)
    return {};
}



FutureResult<> EncFsBackend::validateBackend()
{
    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    // return AsynQt::collect(
    //         EncFs::encfs({}),
    //         EncFs::encfsctl({}),
    //         EncFs::fusermount({})
    //     );
    return {};
}



bool EncFsBackend::isInitialized(const Device &device) const
{
    auto process = EncFs::encfsctl({ device });

    process->start();
    process->waitForFinished();

    return process->exitCode() == 0;
}



bool EncFsBackend::isOpened(const MountPoint &mountPoint) const
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr
        = KMountPoint::currentMountPoints().findByPath(mountPoint);

    // we can not rely on ptr->realDeviceName() since it is empty,
    // KMountPoint can not get the encfs source

    return ptr && ptr->mountPoint() == mountPoint;
}

} // namespace PlasmaVault

