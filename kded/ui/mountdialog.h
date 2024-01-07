/*
 *   SPDX-FileCopyrightText: 2017 Kees vd Broek <cryptodude@libertymail.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#ifndef MOUNTDIALOG_H
#define MOUNTDIALOG_H

#include <QDialog>

#include "engine/vault.h"
#include "ui_mountdialog.h"

class KMessageWidget;
class QAction;

namespace PlasmaVault
{
class Vault;
}

class MountDialog : public QDialog // clazy:exclude=missing-qobject-macro
{
public:
    MountDialog(PlasmaVault::Vault *vault);

protected:
    void accept() override;

private:
    PlasmaVault::Vault *m_vault;
    Ui_MountDialog m_ui;
    KMessageWidget *m_errorLabel;
    QAction *m_detailsAction;
    PlasmaVault::Error m_lastError;
};

#endif
