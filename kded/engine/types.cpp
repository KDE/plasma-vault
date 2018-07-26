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

#include "types.h"

#include <QFileInfo>

namespace PlasmaVault {

static QString normalizePath(const QString& path)
{
    QFileInfo fileInfo(path);

    auto result = fileInfo.canonicalFilePath();

    if (result.isEmpty()) {
        result = path;
    }

    if (result.endsWith('/')) {
        result.chop(1);
    }

    return result;
}

Device::Device(const QString &device)
    : m_device(device)
{
}

Device::operator QString() const
{
    // Done here because canonicalFilePath relies on file existence
    return normalizePath(m_device);
}

MountPoint::MountPoint(const QString &mountPoint)
    : m_mountPoint(mountPoint)
{
}

MountPoint::operator QString() const
{
    // Done here because canonicalFilePath relies on file existence
    return normalizePath(m_mountPoint);
}

} // namespace PlasmaVault

