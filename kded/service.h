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

#ifndef PLASMAVAULT_KDED_SERVICE_H
#define PLASMAVAULT_KDED_SERVICE_H

#include <KDEDModule>

#include <QVariant>

#include <vault.h>


class Q_DECL_EXPORT PlasmaVaultService : public KDEDModule {
    Q_CLASSINFO("D-Bus Interface", "org.kde.plasmavault")
    Q_OBJECT

public:
    PlasmaVaultService(QObject *parent, const QVariantList&);
    ~PlasmaVaultService();

public Q_SLOTS:
    Q_SCRIPTABLE void init();

    Q_SCRIPTABLE void requestNewVault();
    Q_SCRIPTABLE void openVault(const QString &device);
    Q_SCRIPTABLE void closeVault(const QString &device);

    Q_SCRIPTABLE PlasmaVault::VaultDataList availableDevices() const;

Q_SIGNALS:
    void registered();

    Q_SCRIPTABLE void vaultAdded(const PlasmaVault::VaultData &vaultData);
    Q_SCRIPTABLE void vaultRemoved(const QString &device);
    Q_SCRIPTABLE void vaultChanged(const PlasmaVault::VaultData &vaultData);

private Q_SLOTS:
    void slotRegistered(const QDBusObjectPath &path);

    void registerVault(PlasmaVault::Vault *vault);

    void onVaultStatusChanged(PlasmaVault::Vault::Status status);

private:
    class Private;
    QScopedPointer<Private> d;

};

#endif // PLASMAVAULT_KDED_SERVICE_H
