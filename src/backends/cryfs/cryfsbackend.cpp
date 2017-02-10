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



FutureResult<> CryFsBackend::mount(const Device &device,
                                   const MountPoint &mountPoint,
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
        = makeFuture(process, hasProcessFinishedSuccessfully);

    // Writing the password
    process->write(password.toUtf8());
    process->write("\n");

    return result;
}



FutureResult<> CryFsBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    return
        collect(checkVersion(cryfs({ "--version" }), std::make_tuple(0, 9, 6)),
                checkVersion(fusermount({ "--version" }), std::make_tuple(2, 9, 7)))

        | transform([] (const QPair<bool, QString> &cryfs,
                        const QPair<bool, QString> &fusermount) {

              qDebug() << "Called --version on everything <==========================";

              bool success     = cryfs.first && fusermount.first;
              QString message  = i18n("cryfs: %1", cryfs.second) + "\n"
                               + i18n("fusermount: %1", fusermount.second) + "\n";

              qDebug() << "Summary: " << success << message;

              return Result<>::success();
          });
}



bool CryFsBackend::isInitialized(const Device &device) const
{
    QFile cryFsConfig(device + "/cryfs.config");

    qDebug() << "Is initialized? " << cryFsConfig.fileName() << cryFsConfig.exists();
    return cryFsConfig.exists();
}



QProcess *CryFsBackend::cryfs(const QStringList &arguments) const
{
    return process("cryfs",
                   arguments,
                   { { "CRYFS_FRONTEND", "noninteractive" } });
}
} // namespace PlasmaVault

