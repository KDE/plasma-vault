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

#include "backendchooserwidget.h"

#include "ui_backendchooserwidget.h"

#include "vault.h"

class BackendChooserWidget::Private {
public:
    Ui::BackendChooserWidget ui;
};



BackendChooserWidget::BackendChooserWidget()
    : DialogDsl::DialogModule(false)
    , d(new Private())
{
    d->ui.setupUi(this);

    connect(d->ui.editVaultName, &QLineEdit::textChanged,
            this, [&] (const QString &vaultName) {
                setIsValid(!vaultName.isEmpty());
            });
}



BackendChooserWidget::~BackendChooserWidget()
{
}



void BackendChooserWidget::addItem(const QByteArray &id, const QString &title)
{
    d->ui.comboBackend->addItem(title, id);
}



PlasmaVault::Vault::Payload BackendChooserWidget::fields() const
{
    return {
        { KEY_BACKEND, d->ui.comboBackend->currentData() },
        { KEY_NAME,    d->ui.editVaultName->text() }
    };
}

