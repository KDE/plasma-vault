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
#include "engine/backend_p.h"

class BackendChooserWidget::Private {
public:
    Private(BackendChooserWidget *parent)
        : q(parent)
    {
    }

    Ui::BackendChooserWidget ui;

    bool vaultNameValid = false;
    bool backendValid = false;

    void setVaultNameValid(bool valid)
    {
        vaultNameValid = valid;
        q->setIsValid(vaultNameValid && backendValid);
    }

    void setBackendValid(bool valid)
    {
        backendValid = valid;
        q->setIsValid(vaultNameValid && backendValid);
    }

    BackendChooserWidget * const q;
};



BackendChooserWidget::BackendChooserWidget()
    : DialogDsl::DialogModule(false)
    , d(new Private(this))
{
    d->ui.setupUi(this);
    d->ui.textStatus->hide();

    connect(d->ui.editVaultName, &QLineEdit::textChanged,
            this, [&] (const QString &vaultName) {
                d->setVaultNameValid(!vaultName.isEmpty());
            });

    connect(d->ui.comboBackend, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &BackendChooserWidget::checkCurrentBackend);
}



BackendChooserWidget::~BackendChooserWidget()
{
}



void BackendChooserWidget::checkCurrentBackend()
{
    const auto backendId = d->ui.comboBackend->currentData().toString();
    const auto backend = PlasmaVault::Backend::instance(backendId);
    bool backendValid = false;

    d->setBackendValid(false);

    if (!backend) {
        d->ui.textStatus->setHtml(i18n("The specified backend is not available"));
        d->ui.textStatus->show();

    } else {
        d->ui.textStatus->hide();

        const auto result = AsynQt::await(backend->validateBackend());

        if (!result) {
            d->ui.textStatus->setHtml(result.error().message());
            d->ui.textStatus->show();

        } else {
            backendValid = true;
        }
    }

    d->setBackendValid(backendValid);
}



void BackendChooserWidget::addItem(const QByteArray &id, const QString &title)
{
    d->ui.comboBackend->addItem(title, id);

    checkCurrentBackend();
}



PlasmaVault::Vault::Payload BackendChooserWidget::fields() const
{
    return {
        { KEY_BACKEND, d->ui.comboBackend->currentData() },
        { KEY_NAME,    d->ui.editVaultName->text() }
    };
}

