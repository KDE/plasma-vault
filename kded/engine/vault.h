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

#ifndef PLASMAVAULT_KDED_ENGINE_VAULT_H
#define PLASMAVAULT_KDED_ENGINE_VAULT_H

#include <QString>
#include <QObject>
#include <QVariant>
#include <QDBusArgument>

#include <common/vaultinfo.h>

#include "common.h"

#include "commandresult.h"

class QDBusArgument;

namespace PlasmaVault {

#define KEY_NAME        "vault-name"
#define KEY_BACKEND     "vault-backend"
#define KEY_PASSWORD    "vault-password"
#define KEY_DEVICE      "vault-device"
#define KEY_MOUNT_POINT "vault-mount-point"
#define KEY_ACTIVITIES  "vault-activities"

class Vault: public QObject {
    Q_OBJECT

    Q_PROPERTY(PlasmaVault::Device device READ device)

    Q_PROPERTY(QString mountPoint       READ mountPoint    NOTIFY mountPointChanged)
    Q_PROPERTY(VaultInfo::Status status READ status        NOTIFY statusChanged)

    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY isInitializedChanged)
    Q_PROPERTY(bool isOpened      READ isOpened      NOTIFY isOpenedChanged)
    Q_PROPERTY(bool isBusy        READ isBusy        NOTIFY isBusyChanged)

public:
    Vault(const Device &device, QObject *parent = nullptr);
    ~Vault();

    typedef QHash<QByteArray, QVariant> Payload;

    bool isValid() const;
    static QList<Device> availableDevices();
    static QStringList statusMessage();

    FutureResult<> create(const QString &name, const MountPoint &mountPoint,
                          const Payload &payload);

    FutureResult<> open(const Payload &payload);
    FutureResult<> close();

    FutureResult<> destroy(const Payload &payload);

    VaultInfo info() const;

public Q_SLOTS:
    QString errorMessage() const;
    VaultInfo::Status status() const;

    bool isInitialized() const;
    bool isOpened() const;
    bool isBusy() const;

    QString name() const;
    Device device() const;
    MountPoint mountPoint() const;

Q_SIGNALS:
    void mountPointChanged(const QString &mountPoint);
    void statusChanged(VaultInfo::Status status);
    void isInitializedChanged(bool isInitialized);
    void isOpenedChanged(bool isOpened);
    void isBusyChanged(bool isBusy);

private:
    class Private;
    QScopedPointer<Private> d;
};

} // namespace PlasmaVault

#endif // include guard

