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

#include "offlineonlywidget.h"

#include "ui_offlineonlywidget.h"

#include <KSharedConfig>
#include <KConfigGroup>

class OfflineOnlyChooserWidget::Private {
public:
    Ui::OfflineOnlyChooserWidget ui;
    KSharedConfig::Ptr config;
};



OfflineOnlyChooserWidget::OfflineOnlyChooserWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);
}



OfflineOnlyChooserWidget::~OfflineOnlyChooserWidget()
{
}



PlasmaVault::Vault::Payload OfflineOnlyChooserWidget::fields() const
{
    return {
        { KEY_OFFLINEONLY, d->ui.checkShouldBeOffline->isChecked() }
    };
}

void OfflineOnlyChooserWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    d->ui.checkShouldBeOffline->setChecked(
        payload[KEY_OFFLINEONLY].toBool());
}

