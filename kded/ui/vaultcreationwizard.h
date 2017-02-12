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

#ifndef PLASMAVAULT_KDED_UI_VAULT_CREATION_WIZARD_H
#define PLASMAVAULT_KDED_UI_VAULT_CREATION_WIZARD_H

#include <QDialog>

namespace PlasmaVault {
    class Vault;
} // namespace PlasmaVault

class VaultCreationWizard: public QDialog {
    Q_OBJECT

public:
    VaultCreationWizard(QWidget *parent = nullptr);
    ~VaultCreationWizard();

Q_SIGNALS:
    void createdVault(PlasmaVault::Vault *vault);

private:
    class Private;
    QScopedPointer<Private> d;
};


#endif // include guard
