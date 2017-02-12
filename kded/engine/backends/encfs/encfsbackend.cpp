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

#include "encfsbackend.h"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>

#include <KMountPoint>
#include <KLocalizedString>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/wrappers/process.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>

#include <singleton_p.h>

using namespace AsynQt;

namespace PlasmaVault {


EncFsBackend::EncFsBackend()
{
}



EncFsBackend::~EncFsBackend()
{
}



Backend::Ptr EncFsBackend::instance()
{
    return singleton::instance<EncFsBackend>();
}



FutureResult<> EncFsBackend::mount(const Device &device,
                                   const MountPoint &mountPoint,
                                   const Vault::Payload &payload)
{
    QDir dir;

    const auto password = payload[KEY_PASSWORD].toString();

    if (!dir.mkpath(device) || !dir.mkpath(mountPoint)) {
        return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
    }

    auto process = encfs({
            "-S", // read password from stdin
            "--standard", // If creating a file system, use the default options
            device, // source directory to initialize encfs in
            mountPoint // where to mount the file system
        });

    auto result = makeFuture(process, hasProcessFinishedSuccessfully);

    // Writing the password
    process->write(password.toUtf8());
    process->write("\n");

    return result;

}



FutureResult<> EncFsBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    return
        collect(checkVersion(encfs({ "--version" }), std::make_tuple(1, 9, 2)),
                checkVersion(encfsctl({ "--version" }), std::make_tuple(1, 9, 2)),
                checkVersion(fusermount({ "--version" }), std::make_tuple(2, 9, 7)))

        | transform([] (const QPair<bool, QString> &encfs,
                        const QPair<bool, QString> &encfsctl,
                        const QPair<bool, QString> &fusermount) {

              bool success     = encfs.first && encfsctl.first && fusermount.first;
              QString message  = i18n("encfs: %1", encfs.second) + "\n"
                               + i18n("encfsctl: %1", encfsctl.second) + "\n"
                               + i18n("fusermount: %1", fusermount.second) + "\n";

              qDebug() << "Summary: " << success << message;

              return Result<>::success();
          });
}



bool EncFsBackend::isInitialized(const Device &device) const
{
    auto process = encfsctl({ device.data() });

    process->start();
    process->waitForFinished();

    return process->exitCode() == 0;
}



QProcess *EncFsBackend::encfs(const QStringList &arguments) const
{
    return process("encfs", arguments, {});
}



QProcess *EncFsBackend::encfsctl(const QStringList &arguments) const
{
    return process("encfsctl", arguments, {});
}



} // namespace PlasmaVault

