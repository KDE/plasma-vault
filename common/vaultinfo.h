/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMAVAULT_COMMON_VAULT_INFO_H
#define PLASMAVAULT_COMMON_VAULT_INFO_H

#include <QString>
#include <QDBusArgument>

namespace PlasmaVault {

class VaultInfo {
public:
    enum Status {
        NotInitialized = 0,
        Opened         = 1,
        Closed         = 2,

        Creating       = 3,
        Opening        = 4,
        Closing        = 5,
        Destroying     = 6,

        Error          = 255
    };
    // Q_ENUM(Status);

    QString name;
    QString device;
    QString mountPoint;
    Status status;

    inline bool isInitialized() const
    {
        return status == Closed
            || status == Opened
            || status == Closing
            || status == Opening;
    }

    inline bool isOpened() const
    {
        return status == Opened;
    }

    inline bool isBusy() const
    {
        return status == Creating
            || status == Opening
            || status == Closing
            || status == Destroying;
    }
};

typedef QList<VaultInfo> VaultInfoList;

inline
QDBusArgument &operator<<(QDBusArgument &argument, const VaultInfo &vaultData)
{
    argument.beginStructure();
    argument
        << vaultData.name
        << vaultData.device
        << vaultData.mountPoint
        << (quint16)vaultData.status
        ;
    argument.endStructure();
    return argument;
}

inline
const QDBusArgument &operator>>(const QDBusArgument &argument, VaultInfo &vaultData)
{
    quint16 status;
    argument.beginStructure();
    argument
        >> vaultData.name
        >> vaultData.device
        >> vaultData.mountPoint
        >> status
        ;
    vaultData.status = (VaultInfo::Status)status;
    argument.endStructure();
    return argument;
}

} // namespace PlasmaVault

Q_DECLARE_METATYPE(PlasmaVault::VaultInfo)
Q_DECLARE_METATYPE(PlasmaVault::VaultInfoList)

#endif // include guard

