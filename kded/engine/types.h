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

#ifndef PLASMAVAULT_KDED_ENGINE_COMMON_TYPES_H
#define PLASMAVAULT_KDED_ENGINE_COMMON_TYPES_H

#include <QString>
#include <QHash>

#define PLASMAVAULT_CONFIG_FILE QStringLiteral("plasmavaultrc")

namespace PlasmaVault {

class Device {
public:
    explicit Device(const QString &device = QString());
    operator QString() const;

    inline QString data() const
    {
        return m_device;
    }

private:
    QString m_device;
};

inline uint qHash(const Device &value, uint seed = 0)
{
    return qHash(value.data(), seed);
}

inline bool operator== (const Device &left, const Device &right)
{
    return left.data() == right.data();
}



class MountPoint {
public:
    explicit MountPoint(const QString &mountPoint = QString());
    operator QString() const;

    inline bool isEmpty() const
    {
        return m_mountPoint.isEmpty();
    }

    inline QString data() const
    {
        return m_mountPoint;
    }

private:
    QString m_mountPoint;
};

} // namespace PlasmaVault

#endif // include guard

