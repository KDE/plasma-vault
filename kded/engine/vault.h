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

#ifndef PLASMAVAULT_KDED_ENGINE_VAULT_H
#define PLASMAVAULT_KDED_ENGINE_VAULT_H

#include <QString>
#include <QObject>
#include <QVariant>
#include <QDBusArgument>

#include <common/vaultinfo.h>

#include "types.h"

#include "commandresult.h"

class QDBusArgument;

namespace PlasmaVault {

#define PAYLOAD_NAME        "vault-name"
#define PAYLOAD_BACKEND     "vault-backend"
#define PAYLOAD_PASSWORD    "vault-password"
#define PAYLOAD_DEVICE      "vault-device"
#define PAYLOAD_MOUNT_POINT "vault-mount-point"
#define PAYLOAD_KEY_FILE    "vault-key-file"
#define PAYLOAD_VAULT_SIZE  "vault-size"
#define PAYLOAD_ACTIVITIES  "vault-activities"

class Vault: public QObject {
    Q_OBJECT

    Q_PROPERTY(PlasmaVault::Device device READ device)

    Q_PROPERTY(QString mountPoint       READ mountPoint    NOTIFY mountPointChanged)
    Q_PROPERTY(VaultInfo::Status status READ status        NOTIFY statusChanged)

    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY isInitializedChanged)
    Q_PROPERTY(bool isOpened      READ isOpened      NOTIFY isOpenedChanged)
    Q_PROPERTY(bool isBusy        READ isBusy        NOTIFY isBusyChanged)

    Q_PROPERTY(QStringList activities READ activities NOTIFY activitiesChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)

public:
    Vault(const Device &device, QObject *parent = nullptr);
    ~Vault();

    typedef QHash<QByteArray, QVariant> Payload;

    bool isValid() const;

    FutureResult<> create(const QString &name, const MountPoint &mountPoint,
                          const Payload &payload);

    FutureResult<> open(const Payload &payload);
    FutureResult<> close();

    FutureResult<> destroy(const Payload &payload);

    VaultInfo info() const;

public Q_SLOTS:
    QString message() const;
    VaultInfo::Status status() const;

    bool isInitialized() const;
    bool isOpened() const;
    bool isBusy() const;

    QString name() const;
    Device device() const;
    MountPoint mountPoint() const;
    QStringList activities() const;

Q_SIGNALS:
    void mountPointChanged(const QString &mountPoint);
    void statusChanged(VaultInfo::Status status);
    void isInitializedChanged(bool isInitialized);
    void isOpenedChanged(bool isOpened);
    void isBusyChanged(bool isBusy);
    void activitiesChanged(const QStringList &activities);
    void messageChanged(const QString &message);

public:
    static QList<Device> availableDevices();
    static QStringList statusMessage();

private:
    class Private;
    QScopedPointer<Private> d;
};

} // namespace PlasmaVault

#endif // include guard

