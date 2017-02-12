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
        VaultName = Qt::UserRole + 1,
        VaultDevice,
        VaultMountPoint,
        VaultIcon,
        VaultIsBusy,
        VaultIsOpened,
        VaultStatus
    };

public Q_SLOTS:
    void refresh();

    void open(const QString &device);
    void close(const QString &device);
    void toggle(const QString &device);

    void requestNewVault();

private:
    class Private;
    friend class Private;
    QScopedPointer<Private> d;
};

#endif // include guard

