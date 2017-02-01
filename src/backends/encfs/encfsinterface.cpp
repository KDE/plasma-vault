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

#include "encfsinterface.h"

#include <QFutureInterface>
#include <QDir>

#include <asynqt/wrappers/process.h>

#include <KMountPoint>
#include <KLocalizedString>

#include <algorithm>

using AsynQt::makeFuture;

namespace PlasmaVault {
namespace EncFs {

namespace detail {

    QProcess *process(const QString &executable, const QStringList &arguments)
    {
        auto result = new QProcess();
        // result->setProcessChannelMode(QProcess::ForwardedChannels);
        result->setProgram(executable);
        result->setArguments(arguments);
        return result;
    }



#define DEFINE_PROCESS(Name, Executable)                                       \
    inline QProcess *Name(const QStringList &arguments = QStringList())        \
    {                                                                          \
        return process(Executable, arguments);                                 \
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

} // namespace detail



bool isOpened(const MountPoint &mountPoint)
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr
        = KMountPoint::currentMountPoints().findByPath(mountPoint);

    if (ptr) {
        qDebug() << "Mount point for " << mountPoint << " is "
                 << "Device " << ptr->realDeviceName()
                 << "Mount " << ptr->mountPoint();
    }

    qDebug() << "Result is " << (ptr && ptr->mountPoint() == mountPoint);

    return ptr && ptr->mountPoint() == mountPoint;
}



bool isInitialized(const Device &device)
{
    auto process = detail::encfsctl({ device });

    process->start();
    process->waitForFinished();

    return process->exitCode() == 0;
}



FutureResult<> initialize(const QString &name, const Device &device,
                          const MountPoint &mountPoint, const QString &password)
{
    Q_UNUSED(name)

    return detail::open(device, mountPoint, password);
}



FutureResult<> open(const Device &device, const MountPoint &mountPoint,
                    const QString &password)
{
    return detail::open(device, mountPoint, password);
}



FutureResult<> close(const Device &device, const MountPoint &mountPoint)
{
    Q_UNUSED(device)

    return makeFuture(detail::fusermount({ "-u", mountPoint }),
                      detail::hasFinishedSuccessfully);
}

} // namespace EncFs
} // namespace PlasmaVault



