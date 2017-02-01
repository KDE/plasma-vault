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

#include <QDir>

#include "encfsbackend.h"
#include "encfsinterface.h"

#include <singleton_p.h>

#include <KLocalizedString>

namespace PlasmaVault {

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
    return
        EncFs::isInitialized(device) ?
            errorResult(Error::BackendError,
                        i18n("This directory already contains encrypted EncFS data")) :

        !isDirectoryEmpty(device) || !isDirectoryEmpty(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("You need to select empty directories for the encrypted storage and for the mount point")) :

        // otherwise
            EncFs::initialize(name, device, mountPoint, password);
}



FutureResult<> EncFsBackend::open(const Device &device,
                                  const MountPoint &mountPoint,
                                  const QString &password)
{
    return
        EncFs::isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is already open")) :

        // otherwise
            EncFs::open(device, mountPoint, password);
}



FutureResult<> EncFsBackend::close(const Device &device,
                                   const MountPoint &mountPoint)
{
    return
        !EncFs::isOpened(mountPoint) ?
            errorResult(Error::BackendError,
                        i18n("Device is not open")) :

        // otherwise
            EncFs::close(device, mountPoint);
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



bool EncFsBackend::isInitialized(const Device &device) const
{
    return EncFs::isInitialized(device);
}



bool EncFsBackend::isOpened(const MountPoint &mountPoint) const
{
    return EncFs::isOpened(mountPoint);
}

} // namespace PlasmaVault

