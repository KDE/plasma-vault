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

#include "keylocationwidget.h"
#include "ui_keylocationwidget.h"

#include "vault.h"

class KeyLocationWidget::Private {
public:
    Ui::KeyLocationWidget ui;

    KeyLocationWidget *const q;
    Private(KeyLocationWidget *parent)
        : q(parent)
    {
    }
};



KeyLocationWidget::KeyLocationWidget()
    : DialogDsl::DialogModule(false), d(new Private(this))
{
    d->ui.setupUi(this);
}



KeyLocationWidget::~KeyLocationWidget()
{
}



PlasmaVault::Vault::Payload KeyLocationWidget::fields() const
{
    return {
    };
}



void KeyLocationWidget::init(
    const PlasmaVault::Vault::Payload &payload)
{
    const auto name = payload[KEY_NAME].toString();
}



