/*
 *   Copyright 2017 by Kees vd Broek <cryptodude@libertymail.net>
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
#include "mountdialog.h"

#include <QCloseEvent>
#include <QStyle>
#include <QStyleOption>
#include <QAction>
#include <QMessageBox>

#include <KMessageWidget>

MountDialog::MountDialog(PlasmaVault::Vault *vault)
    : m_vault(vault)
{
    m_ui.setupUi(this);
    setWindowTitle(i18nc("%1 is the name of the Plasma Vault to open", "Open '%1' Plasma Vault", vault->name()));

    m_errorLabel = new KMessageWidget(this);
    m_errorLabel->setMessageType(KMessageWidget::Error);
    m_errorLabel->setCloseButtonVisible(false);
    m_errorLabel->setIcon(QIcon::fromTheme("dialog-error"));
    m_errorLabel->setVisible(false);

    m_detailsAction = new QAction(this);
    m_detailsAction->setToolTip(i18n("Details..."));
    m_detailsAction->setIcon(QIcon::fromTheme("view-list-details"));

    connect(m_detailsAction, &QAction::triggered,
            this, [this] {
                QString message;
                const auto out = m_lastError.out().trimmed();
                const auto err = m_lastError.err().trimmed();

                if (!out.isEmpty() && !err.isEmpty()) {
                    message = i18n("Command output:\n%1\n\nError output: %2", m_lastError.out(), m_lastError.err());
                } else {
                    message = out + err;
                }

                auto messageBox = new QMessageBox(QMessageBox::Critical, i18n("Error details"), message, QMessageBox::Ok, this);
                messageBox->setAttribute(Qt::WA_DeleteOnClose);
                messageBox->show();

            });

    auto errorLabelSizePolicy = m_errorLabel->sizePolicy();
    errorLabelSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_errorLabel->setSizePolicy(errorLabelSizePolicy);
    m_errorLabel->setVisible(false);
    m_ui.formLayout->addRow(QString(), m_errorLabel);
    m_ui.vaultName->setText(vault->name());

    QStyleOption option;
    option.initFrom(this);
    const int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, this);
    m_ui.icon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-password")).pixmap(iconSize));
}

void MountDialog::accept()
{
    setCursor(Qt::WaitCursor);
    m_errorLabel->setVisible(false);
    setEnabled(false);

    m_ui.password->lineEdit()->setEchoMode(QLineEdit::Password);
    m_ui.password->lineEdit()->setCursor(Qt::WaitCursor);
    QString pwd = m_ui.password->password();
    auto future = m_vault->open({ { KEY_PASSWORD, pwd } });
    const auto result = AsynQt::await(future);

    unsetCursor();
    setEnabled(true);
    m_ui.password->lineEdit()->unsetCursor();

    if (result) {
        QDialog::accept();
    } else {
        m_lastError = result.error();
        m_ui.password->lineEdit()->setText({});

        m_errorLabel->setText(i18n("Failed to open: %1", m_lastError.message()));
        m_errorLabel->setVisible(true);

        if (!m_lastError.out().isEmpty() || !m_lastError.err().isEmpty()) {
            m_errorLabel->addAction(m_detailsAction);

        } else {
            m_errorLabel->removeAction(m_detailsAction);

        }
    }
}
