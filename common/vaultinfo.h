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

#ifndef PLASMAVAULT_COMMON_VAULT_INFO_H
#define PLASMAVAULT_COMMON_VAULT_INFO_H

#include <QString>
#include <QDBusArgument>
#include <QDebug>

namespace PlasmaVault {

/**
 * Class used to serialize the vault data for DBus communication
 */
class VaultInfo {
public:
    enum Status {
        NotInitialized = 0,
        Opened         = 1,
        Closed         = 2,

        Creating       = 3,
        Opening        = 4,
        Closing        = 5,
        Dismantling    = 6,
        Dismantled     = 7,

        Error          = 255
    };
    // Q_ENUM(Status); // VaultInfo is not a QObject or anything like that


    QString name;
    QString device;
    QString mountPoint;

    Status status;
    QString message;

    QStringList activities;
    bool isOfflineOnly;


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
            || status == Dismantling;
    }
};



typedef QList<VaultInfo> VaultInfoList;



inline QDebug &operator<< (QDebug &debug,
                           const VaultInfo &vaultInfo)
{
    debug
        << vaultInfo.name
        << vaultInfo.device
        << vaultInfo.mountPoint

        << (quint16)vaultInfo.status
        << vaultInfo.message

        << vaultInfo.activities
        << vaultInfo.isOfflineOnly
        ;
    return debug;
}



inline QDBusArgument &operator<< (QDBusArgument &argument,
                                  const VaultInfo &vaultInfo)
{
    argument.beginStructure();
    argument
        << vaultInfo.name
        << vaultInfo.device
        << vaultInfo.mountPoint

        << (quint16)vaultInfo.status
        << vaultInfo.message

        << vaultInfo.activities
        << vaultInfo.isOfflineOnly
        ;
    argument.endStructure();
    return argument;
}



inline const QDBusArgument &operator>> (const QDBusArgument &argument,
                                        VaultInfo &vaultInfo)
{
    quint16 status;
    argument.beginStructure();
    argument
        >> vaultInfo.name
        >> vaultInfo.device
        >> vaultInfo.mountPoint

        >> status
        >> vaultInfo.message

        >> vaultInfo.activities
        >> vaultInfo.isOfflineOnly
        ;
    vaultInfo.status = (VaultInfo::Status)status;
    argument.endStructure();
    return argument;
}



} // namespace PlasmaVault

Q_DECLARE_METATYPE(PlasmaVault::VaultInfo)
Q_DECLARE_METATYPE(PlasmaVault::VaultInfoList)

#endif // include guard

