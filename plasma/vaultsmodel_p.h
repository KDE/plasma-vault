/*
 *   Copyright (C) 2017 Ivan Čukić <ivan.cukic(at)kde.org>
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

#ifndef PLASMAVAULT_PLASMA_VAULTSMODEL_P_H
#define PLASMAVAULT_PLASMA_VAULTSMODEL_P_H

#include <QDBusInterface>
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

public:
    org::kde::plasmavault service;
    VaultsModel *const q;
};

#endif // include guard
