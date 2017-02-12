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

#ifndef PLASMAVAULT_PLASMA_VAULTSMODEL_H
#define PLASMAVAULT_PLASMA_VAULTSMODEL_H

#include <QAbstractListModel>

#include <common/vaultinfo.h>

class VaultsModel: public QAbstractListModel {
    Q_OBJECT

public:
    VaultsModel(QObject *parent = nullptr);
    ~VaultsModel();

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        VaultName = Qt::UserRole + 1, ///< The user defined name of the vault
        VaultDevice,                  ///< Path to the encrypted data
        VaultMountPoint,              ///< Where the data should be visible from
        VaultIcon,                    ///< Icon for the fault
        VaultIsBusy,                  ///< Is the vault currently being opened or closed
        VaultIsOpened,                ///< Is the vault open
        VaultStatus                   ///< Status of the vault, @see VaultInfo::Status
    };

public Q_SLOTS:
    // Refresh the model. This should be done automatically, no need to call
    // this function
    void refresh();

    // Open (unlock) a vault
    void open(const QString &device);

    // Close the vault
    void close(const QString &device);

    // Open the vault if it is closed, or close it otherwise
    void toggle(const QString &device);

    // Opens the new-vault wizard
    void requestNewVault();

private:
    class Private;
    friend class Private;
    QScopedPointer<Private> d;
};

#endif // include guard

