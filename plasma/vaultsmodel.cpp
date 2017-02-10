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

#include "vaultsmodel.h"
#include "vaultsmodel_p.h"

#include <asynqt/wrappers/dbus.h>
#include <asynqt/operations/transform.h>

#include <QFileInfo>

using namespace PlasmaVault;
using namespace AsynQt::operators;


VaultsModel::Private::Private(VaultsModel *parent)
    : service( "org.kde.kded5"
             , "/modules/plasmavault"
             , QDBusConnection::sessionBus()
             )
    , q(parent)
{
    qDebug() << "---------------> Asking for vaults" << service.isValid();

    connect(&service, &org::kde::plasmavault::vaultAdded,
            this,     &Private::onVaultAdded);
    connect(&service, &org::kde::plasmavault::vaultChanged,
            this,     &Private::onVaultChanged);
    connect(&service, &org::kde::plasmavault::vaultRemoved,
            this,     &Private::onVaultRemoved);


#if 1
    auto pcall = service.asyncCall("availableDevices");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

    QObject::connect(
        watcher, &QDBusPendingCallWatcher::finished,
            this, [this] (QDBusPendingCallWatcher* watcher) {
                QDBusPendingReply<VaultDataList> reply = *watcher;
                const auto vaultList = reply.value();
                for (const auto& vault: vaultList) {
                    qDebug() << "---------------> Plasma recognizes vault: "
                             << vault.name
                             << vault.device
                             << vault.mountPoint
                             << vault.status;
                    vaults[vault.device] = vault;
                    vaultKeys << vault.device;
                }
            });

    return;
#endif

#if 0
    AsynQt::DBus::asyncCall<VaultDataList>(&service, "availableDevices")
        | transform([this] (const VaultDataList &vaultList) {
            for (const auto& vault: vaultList) {
                qDebug() << "---------------> Plasma recognizes vault: " << vault.name;
                vaults[vault.device] = vault;
                vaultKeys << vault.device;
            }
        });
#endif
}



void VaultsModel::Private::onVaultAdded(const PlasmaVault::VaultData &vaultData)
{
    const auto device = vaultData.device;

    if (vaults.contains(device)) return;

    q->beginInsertRows(QModelIndex(), vaultKeys.size(), vaultKeys.size());
    vaults[device] = vaultData;
    vaultKeys << device;
    q->endInsertRows();
}



void VaultsModel::Private::onVaultRemoved(const QString &device)
{
    if (vaults.contains(device)) return;

    const auto row = vaultKeys.indexOf(device);

    q->beginRemoveRows(QModelIndex(), row, row);
    vaultKeys.removeAt(row);
    vaults.remove(device);
    q->endRemoveRows();
}



void VaultsModel::Private::onVaultChanged(const PlasmaVault::VaultData &vaultData)
{
    const auto device = vaultData.device;
    qDebug() << "onVaultChanged: " << device << vaultData.status;

    qDebug() << "Known devices: " << vaultKeys;
    if (!vaultKeys.contains(device)) return;
    qDebug() << "YES! We can change the data";

    const auto row = vaultKeys.indexOf(device);

    vaults[device] = vaultData;
    q->dataChanged(q->index(row), q->index(row));
}



VaultsModel::VaultsModel(QObject *parent)
    : d(new Private(this))
{
}



VaultsModel::~VaultsModel()
{
}



int VaultsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->vaultKeys.size();
}



QVariant VaultsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qDebug() << "Index is not valid";
        return {};
    }

    const int row = index.row();

    if (row >= d->vaultKeys.count()) {
        qDebug() << "row" << row << "is not valid";
        return {};
    }

    const auto device = d->vaultKeys[row];
    const auto &vault = d->vaults[device];

    switch (role) {
        case VaultDevice:
            qDebug() << "Vault getting the device" << vault.device;
            return vault.device;

        case VaultMountPoint:
            qDebug() << "Vault getting the mount point" << vault.mountPoint;
            return vault.mountPoint;

        case VaultName:
            qDebug() << "Vault getting name: " << vault.name;
            return vault.name.isEmpty() ?
                          vault.device :
                          vault.name;

        case VaultIcon:
        {
            QFileInfo fileInfo(vault.device);
            if (!fileInfo.exists()) {
                return "document-close";

            } else switch (vault.status) {
                case Vault::Error:
                    return "document-close";

                case Vault::NotInitialized:
                    return "folder-gray";

                case Vault::Closed:
                    return "folder-encrypted-closed";

                case Vault::Opened:
                    return "folder-encrypted-open";

                default:
                    return "";
            }
        }

        case VaultBusy:
            return vault.isBusy();
    }

    return {};
}



QHash<int, QByteArray> VaultsModel::roleNames() const
{
    return {
        { VaultName, "name" },
        { VaultIcon, "icon" },
        { VaultDevice, "device" },
        { VaultMountPoint, "mountPoint" },
        { VaultBusy, "isBusy" },
        { VaultStatus, "status" }
    };
}



void VaultsModel::refresh()
{
}


void VaultsModel::open(const QString &device)
{
    if (!d->vaults.contains(device)) return;
    AsynQt::DBus::asyncCall<void>(&d->service, "openVault", device)
        | transform([] {
              qDebug() << "open request finished";
          });
}

void VaultsModel::close(const QString &device)
{
    if (!d->vaults.contains(device)) return;
    AsynQt::DBus::asyncCall<void>(&d->service, "closeVault", device)
        | transform([] {
              qDebug() << "close request finished";
          });
}

void VaultsModel::toggle(const QString &device)
{
    if (!d->vaults.contains(device)) return;
    const auto &vault = d->vaults[device];
    if (vault.status == Vault::Opened) {
        close(device);
    } else if (vault.status == Vault::Closed) {
        open(device);
    }
}


void VaultsModel::requestNewVault()
{
    AsynQt::DBus::asyncCall<void>(&d->service, "requestNewVault");
}




#if 0

inline QStringList difference(const QStringList &left, const QStringList &right)
{
    QStringList result;
    std::set_difference(left.cbegin(), left.cend(),
                        right.cbegin(), right.cend(),
                        std::back_inserter(result));
    return result;
}



void VaultsModel::refresh()
{
    beginResetModel();
    const auto oldDevices = m_vaults.keys();
    auto newDevices = Vault::availableDevices();
    newDevices.sort();

    const auto removedDevices = difference(oldDevices, newDevices);
    const auto addedDevices = difference(newDevices, oldDevices);

    for (const auto &device: removedDevices) {
        auto mount = m_vaults.take(device);
        delete mount;
    }

    for (const auto &device: addedDevices) {
        auto mount = new Vault(device, this);
        connect(mount, &Vault::statusChanged,
                this,  &VaultsModel::onVaultStatusChanged);
        m_vaults[device] = mount;
    }

    m_mountKeys = m_vaults.keys();

    qDebug() << "Vaults: " << m_vaults << m_mountKeys;
    endResetModel();
}



void VaultsModel::onVaultStatusChanged(Vault::Status status)
{
    const auto mount = qobject_cast<Vault*>(sender());

    if (!mount || !m_vaults.contains(mount->device())) return;

    setVaultStatus(mount->device(), status);
}



void VaultsModel::setVaultStatus(const QString& device, Vault::Status status)
{
    const auto row = m_mountKeys.indexOf(device);
    dataChanged(index(row), index(row));
}



void VaultsModel::open(const QString &device)
{
    if (!m_vaults.contains(device)) return;
    m_vaults[device]->mount("somepass");
}



void VaultsModel::close(const QString &device)
{
    if (!m_vaults.contains(device)) return;
    m_vaults[device]->unmount();
}



void VaultsModel::toggle(const QString &device)
{
    qDebug() << "toggleMounted " << device;
    if (!m_vaults.contains(device)) return;

    const auto mount = m_vaults[device];

    if (mount->isMounted()) {
        close(device);
    } else {
        open(device);
    }
}

#endif


