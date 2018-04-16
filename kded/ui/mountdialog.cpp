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
#include "engine/vault.h"

#include <QCloseEvent>
#include <QStyleOption>


MountDialog::MountDialog(PlasmaVault::Vault *vault, const std::function<void()> &function)
    : m_vault(vault),
      m_function(function)
{
    m_ui.setupUi(this);
    m_ui.errorLabel->setVisible(false);
    m_ui.vaultName->setText(vault->name());

    QStyleOption option;
    option.initFrom(this);
    const int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, this);
    m_ui.icon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-password")).pixmap(iconSize));
}

void MountDialog::accept()
{
    setCursor(Qt::WaitCursor);
    setEnabled(false);

    m_ui.password->lineEdit()->setCursor(Qt::WaitCursor);
    QString pwd = m_ui.password->password();
    auto future = m_vault->open({ { KEY_PASSWORD, pwd } });
    const auto result = AsynQt::await(future);

    unsetCursor();
    setEnabled(true);
    m_ui.password->lineEdit()->unsetCursor();

    if (result) {
        m_function();
        QDialog::accept();
    } else {
        qDebug() << "We've got an error" <<  result.error().message();
        // m_ui.errorLabel->setText(i18n("Failed to open: %1").arg(result.error().message()));
        m_ui.errorLabel->setText(i18n("Failed to open: %1", result.error().message()));
        m_ui.errorLabel->setVisible(true);
    }
}
