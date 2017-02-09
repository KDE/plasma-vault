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

#include "backendchooserwidget.h"

#include "ui_backendchooserwidget.h"

#include <QDebug>

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



QHash<QString, QVariant> BackendChooserWidget::fields() const
{
    return {
        { VAULT_BACKEND, d->ui.comboBackend->currentData() },
        { VAULT_NAME,    d->ui.editVaultName->text() }
    };
}

