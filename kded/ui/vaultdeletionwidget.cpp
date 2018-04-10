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

#include "vaultdeletionwidget.h"

#include "ui_vaultdeletionwidget.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <KMessageWidget>

#include <QDBusInterface>
#include <QDBusPendingCall>

class VaultDeletionWidget::Private {
public:
    Ui::VaultDeletionWidget ui;
    QString vaultName;
    QString vaultDevice;
    KSharedConfig::Ptr config;
};



VaultDeletionWidget::VaultDeletionWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);

    auto messageWidget = new KMessageWidget(d->ui.labelWarning->text(), this);
    messageWidget->setMessageType(KMessageWidget::Warning);
    messageWidget->setCloseButtonVisible(false);
    messageWidget->setIcon(QIcon::fromTheme("dialog-warning"));
    static_cast<QBoxLayout*>(layout())->insertWidget(0, messageWidget);

    d->ui.labelWarning->hide();

    connect(d->ui.textVaultNameConfirmation, &QLineEdit::textEdited,
            this, [this] (const QString &newText) {
                d->ui.buttonDeleteVault->setEnabled(d->vaultName == newText);
            });

    connect(d->ui.buttonDeleteVault, &QPushButton::clicked,
            this, [this] {
                d->ui.buttonDeleteVault->setEnabled(false);
                emit requestCancellation();

                QDBusInterface(
                        QStringLiteral("org.kde.kded5"),
                        QStringLiteral("/modules/plasmavault"),
                        QStringLiteral("org.kde.plasmavault"))
                    .asyncCall("deleteVault", d->vaultDevice, d->vaultName);

            });

}



VaultDeletionWidget::~VaultDeletionWidget()
{
}



PlasmaVault::Vault::Payload VaultDeletionWidget::fields() const
{
    return {};
}

void VaultDeletionWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    d->vaultName = payload[KEY_NAME].toString();
    d->vaultDevice = payload[KEY_DEVICE].toString();
    d->ui.buttonDeleteVault->setEnabled(false);
}

