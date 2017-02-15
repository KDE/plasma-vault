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

#include "vaultsmodel.h"
#include "vaultsmodel_p.h"

#include <asynqt/wrappers/dbus.h>
#include <asynqt/operations/listen.h>

#include <QFileInfo>

using namespace PlasmaVault;
using namespace AsynQt;
using namespace AsynQt::operators;


VaultsModel::Private::Private(VaultsModel *parent)
    : service( "org.kde.kded5"
             , "/modules/plasmavault"
             , QDBusConnection::sessionBus()
             )
    , serviceWatcher( "org.kde.kded5"
                    , QDBusConnection::sessionBus()
                    )
    , q(parent)
{
    connect(&service, &org::kde::plasmavault::vaultAdded,
            this,     &Private::onVaultAdded);
    connect(&service, &org::kde::plasmavault::vaultChanged,
            this,     &Private::onVaultChanged);
    connect(&service, &org::kde::plasmavault::vaultRemoved,
            this,     &Private::onVaultRemoved);

    connect(&serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged,
            this, [this] (const QString &service, const QString &oldOwner, const QString &newOwner) {
                Q_UNUSED(oldOwner);

                if (service != "org.kde.kded5") {
                    return;
                }

                // If kded is not running, just clear all vault info,
                // otherwise load the data
                if (newOwner.isEmpty()) {
                    clearData();
                } else {
                    loadData();
                }
            });

    // Try to load the data. This should start kded if it is not running
    // for some reason
    loadData();
}

void VaultsModel::Private::loadData()
{
    // Before loading the new data, lets forget everything
    clearData();

    // Asynchronously load the devices
    DBus::asyncCall<VaultInfoList>(&service, "availableDevices") |
        onSuccess([this] (const VaultInfoList &vaultList) {
            q->beginResetModel();

            vaults.clear();
            vaultKeys.clear();
            busyVaults.clear();
            errorVaults.clear();

            for (const auto& vault: vaultList) {
                vaults[vault.device] = vault;
                vaultKeys << vault.device;

                if (vault.isBusy()) {
                    busyVaults << vault.device;
                }

                if (!vault.message.isEmpty()) {
                    errorVaults << vault.device;
                }
            }

            q->endResetModel();

            emit q->isBusyChanged(busyVaults.count() != 0);
            emit q->hasErrorChanged(errorVaults.count() != 0);
        });
}



void VaultsModel::Private::clearData()
{
    q->beginResetModel();
    vaultKeys.clear();
    vaults.clear();
    q->endResetModel();
}



void VaultsModel::Private::onVaultAdded(const PlasmaVault::VaultInfo &vaultInfo)
{
    const auto device = vaultInfo.device;

    if (vaults.contains(device)) return;

    q->beginInsertRows(QModelIndex(), vaultKeys.size(), vaultKeys.size());
    vaults[device] = vaultInfo;
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



void VaultsModel::Private::onVaultChanged(
    const PlasmaVault::VaultInfo &vaultInfo)
{
    const auto device = vaultInfo.device;

    if (!vaultKeys.contains(device)) return;

    const auto row = vaultKeys.indexOf(device);

    // Lets see whether this warrants updates to the busy flag
    if (vaultInfo.isBusy() && !busyVaults.contains(device)) {
        busyVaults << device;
        if (busyVaults.count() == 1) {
            emit q->isBusyChanged(true);
        }
    }

    if (!vaultInfo.isBusy() && busyVaults.contains(device)) {
        busyVaults.remove(device);
        if (busyVaults.count() == 0) {
            emit q->isBusyChanged(false);
        }
    }

    // Lets see whether this warrants updates to the error flag
    if (!vaultInfo.message.isEmpty() && !errorVaults.contains(device)) {
        errorVaults << device;
        if (errorVaults.count() == 1) {
            emit q->hasErrorChanged(true);
        }
    }

    if (vaultInfo.message.isEmpty() && errorVaults.contains(device)) {
        errorVaults.remove(device);
        if (errorVaults.count() == 0) {
            emit q->hasErrorChanged(false);
        }
    }

    vaults[device] = vaultInfo;
    q->dataChanged(q->index(row), q->index(row));
}



VaultsModel::VaultsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
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
        return {};
    }

    const int row = index.row();

    if (row >= d->vaultKeys.count()) {
        return {};
    }

    const auto device = d->vaultKeys[row];
    const auto &vault = d->vaults[device];

    switch (role) {
        case VaultDevice:
            return vault.device;

        case VaultMountPoint:
            return vault.mountPoint;

        case VaultName:
            return vault.name.isEmpty() ?
                          vault.device :
                          vault.name;

        case VaultIcon:
        {
            QFileInfo fileInfo(vault.device);
            if (!fileInfo.exists()) {
                return "document-close";

            } else switch (vault.status) {
                case VaultInfo::Error:
                    return "document-close";

                case VaultInfo::NotInitialized:
                    return "folder-gray";

                case VaultInfo::Closed:
                    return "folder-encrypted-closed";

                case VaultInfo::Opened:
                    return "folder-encrypted-open";

                default:
                    return "";
            }
        }

        case VaultIsBusy:
            return vault.isBusy();

        case VaultIsOpened:
            return vault.isOpened();

        case VaultActivities:
            return vault.activities;

        case VaultMessage:
            return vault.message;
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
        { VaultIsBusy, "isBusy" },
        { VaultIsOpened, "isOpened" },
        { VaultActivities, "activities" },
        { VaultStatus, "status" },
        { VaultMessage, "message" }
    };
}



void VaultsModel::refresh()
{
    d->loadData();
}



void VaultsModel::open(const QString &device)
{
    if (!d->vaults.contains(device)) return;

    DBus::asyncCall<>(&d->service, "openVault", device);
}



void VaultsModel::close(const QString &device)
{
    if (!d->vaults.contains(device)) return;

    DBus::asyncCall<>(&d->service, "closeVault", device);
}



void VaultsModel::toggle(const QString &device)
{
    if (!d->vaults.contains(device)) return;

    const auto &vault = d->vaults[device];
    if (vault.status == VaultInfo::Opened) {
        close(device);
    } else if (vault.status == VaultInfo::Closed) {
        open(device);
    }
}



void VaultsModel::requestNewVault()
{
    DBus::asyncCall<>(&d->service, "requestNewVault");
}



bool VaultsModel::isBusy() const
{
    return !d->busyVaults.isEmpty();
}



bool VaultsModel::hasError() const
{
    return !d->errorVaults.isEmpty();
}





SortedVaultsModelProxy::SortedVaultsModelProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_source(new VaultsModel(this))
    , m_kamd(new KActivities::Consumer(this))
{
    setSourceModel(m_source);

    connect(m_kamd, &KActivities::Consumer::currentActivityChanged,
            this,   &SortedVaultsModelProxy::invalidate);
    connect(m_kamd, &KActivities::Consumer::serviceStatusChanged,
            this,   &SortedVaultsModelProxy::invalidate);
}



bool SortedVaultsModelProxy::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    const auto leftData = sourceModel()->data(left, VaultsModel::VaultName);
    const auto rightData = sourceModel()->data(right, VaultsModel::VaultName);

    return leftData < rightData;
}



bool SortedVaultsModelProxy::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    const auto activities =
        m_source->index(sourceRow).data(VaultsModel::VaultActivities).toStringList();

    const auto isOpened =
        m_source->index(sourceRow).data(VaultsModel::VaultIsOpened).toBool();

    return activities.size() == 0 || isOpened || activities.contains(m_kamd->currentActivity());
}



QObject *SortedVaultsModelProxy::source() const
{
    return sourceModel();
}


