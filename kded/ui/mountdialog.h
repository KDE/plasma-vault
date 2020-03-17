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
#ifndef MOUNTDIALOG_H
#define MOUNTDIALOG_H

#include <QDialog>
#include <functional>

#include "ui_mountdialog.h"

#include "engine/vault.h"

class KMessageWidget;
class QAction;

namespace PlasmaVault {
    class Vault;
}

class MountDialog : public QDialog
{
public:
    MountDialog(PlasmaVault::Vault *vault);

protected:
    void accept() override;

private:
    PlasmaVault::Vault *m_vault;
    Ui_MountDialog m_ui;
    KMessageWidget* m_errorLabel;
    QAction* m_detailsAction;
    PlasmaVault::Error m_lastError;
};

#endif
