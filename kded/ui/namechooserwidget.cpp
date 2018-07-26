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

#include "namechooserwidget.h"
#include "ui_namechooserwidget.h"

#include "vault.h"

class NameChooserWidget::Private {
public:
    Ui::NameChooserWidget ui;

    NameChooserWidget *const q;
    Private(NameChooserWidget *parent)
        : q(parent)
    {
    }
};



NameChooserWidget::NameChooserWidget()
    : DialogDsl::DialogModule(false), d(new Private(this))
{
    d->ui.setupUi(this);

    connect(d->ui.editVaultName, &QLineEdit::textChanged,
            this, [this] (const QString &text) {
                Q_UNUSED(text);
                setIsValid(!d->ui.editVaultName->text().isEmpty());
            });
}



NameChooserWidget::~NameChooserWidget()
{
}



PlasmaVault::Vault::Payload NameChooserWidget::fields() const
{
    return {
        { KEY_NAME, d->ui.editVaultName->text() }
    };
}



void NameChooserWidget::init(
    const PlasmaVault::Vault::Payload &payload)
{
    const auto name = payload[KEY_NAME].toString();

    d->ui.editVaultName->setText(name);
    setIsValid(!d->ui.editVaultName->text().isEmpty());
}



