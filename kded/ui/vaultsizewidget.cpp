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

#include "vaultsizewidget.h"
#include "ui_vaultsizewidget.h"

#include "vault.h"

class VaultSizeWidget::Private {
public:
    Ui::VaultSizeWidget ui;

    VaultSizeWidget *const q;
    Private(VaultSizeWidget *parent)
        : q(parent)
    {
    }

};



VaultSizeWidget::VaultSizeWidget()
    : DialogDsl::DialogModule(true), d(new Private(this))
{
    d->ui.setupUi(this);

    connect(d->ui.sliderVaultSize, &QSlider::valueChanged,
            this, [this] (int value) {
                d->ui.labelVaultSizeValue->setText(
                        i18n("%1 MB", value));
            });

    d->ui.labelVaultSizeValue->setText(
            i18n("%1 MB", d->ui.sliderVaultSize->value()));
}



VaultSizeWidget::~VaultSizeWidget()
{
}



PlasmaVault::Vault::Payload VaultSizeWidget::fields() const
{
    return {
        { PAYLOAD_VAULT_SIZE, d->ui.sliderVaultSize->value() }
    };
}



void VaultSizeWidget::init(
    const PlasmaVault::Vault::Payload &payload)
{
}


