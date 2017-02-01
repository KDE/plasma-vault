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

#ifndef VAULT_H
#define VAULT_H

#include "plasmavault_export.h"

#include <QString>
#include <QObject>

#include "common.h"

#include "commandresult.h"

namespace PlasmaVault {

class PLASMAVAULT_EXPORT Vault: public QObject {
    Q_OBJECT

    Q_ENUMS(Status)

    Q_PROPERTY(QString device     READ device)
    Q_PROPERTY(QString mountPoint READ mountPoint    NOTIFY mountPointChanged)
    Q_PROPERTY(Status status      READ status        NOTIFY statusChanged)
    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY isInitializedChanged)
    Q_PROPERTY(bool isOpened      READ isOpened      NOTIFY isOpenedChanged)
    Q_PROPERTY(bool isBusy        READ isBusy        NOTIFY isBusyChanged)

public:
    Vault(const QString &device, QObject *parent = nullptr);
    ~Vault();

    enum Status {
        NotInitialized = 0,
        Opened         = 1,
        Closed         = 2,

        Creating       = 3,
        Opening        = 4,
        Closing        = 5,
        Destroying     = 6,

        Error          = 255
    };

    static QStringList availableDevices();

    FutureResult<> create(const QString &name, const QString &mountPoint,
                          const QString &password, const QString &backend);

    FutureResult<> open(const QString &password);
    FutureResult<> close();

    FutureResult<> destroy(const QString &password);

public Q_SLOTS:
    QString errorMessage() const;
    Status status() const;
    bool isInitialized() const;
    bool isOpened() const;
    bool isBusy() const;

    QString name() const;
    QString device() const;
    QString mountPoint() const;

Q_SIGNALS:
    void mountPointChanged(const QString &mountPoint);
    void statusChanged(Vault::Status status);
    void isInitializedChanged(bool isInitialized);
    void isOpenedChanged(bool isOpened);
    void isBusyChanged(bool isBusy);

private:
    class Private;
    Private * const d;
};

} // namespace PlasmaVault

#endif // VAULT_H
