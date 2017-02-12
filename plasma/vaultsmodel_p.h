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

#ifndef PLASMAVAULT_PLASMA_VAULTSMODEL_P_H
#define PLASMAVAULT_PLASMA_VAULTSMODEL_P_H

#include <QDBusInterface>
#include <QDBusServiceWatcher>

#include "plasmavault_interface.h"
#include <common/vaultinfo.h>

class VaultsModel::Private: public QObject {
    Q_OBJECT

public:
    Private(VaultsModel *parent);
    QStringList vaultKeys;
    QMap<QString, PlasmaVault::VaultInfo> vaults;

public Q_SLOTS:
    void onVaultAdded(const PlasmaVault::VaultInfo &vaultData);
    void onVaultRemoved(const QString &device);
    void onVaultChanged(const PlasmaVault::VaultInfo &vaultData);

    void loadData();
    void clearData();

public:
    org::kde::plasmavault service;
    QDBusServiceWatcher serviceWatcher;
    VaultsModel *const q;
};

#endif // include guard

