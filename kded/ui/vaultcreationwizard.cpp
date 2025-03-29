/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultcreationwizard.h"
#include "ui_vaultcreationwizard.h"

#include <QList>
#include <QMap>
#include <QPushButton>
#include <QStackedLayout>

#include "dialogdsl.h"
#include "vault.h"

using namespace DialogDsl;
using namespace DialogDsl::operators;

#include "activitieslinkingwidget.h"
#include "backendchooserwidget.h"
#include "cryfscypherchooserwidget.h"
#include "directorypairchooserwidget.h"
#include "noticewidget.h"
#include "offlineonlywidget.h"
#include "passwordchooserwidget.h"

#include "vaultwizardbase.h"

class VaultCreationWizard::Private : public WBASE(VaultCreationWizard)
{
public:
    Logic logic{{"gocryptfs" / i18n("gocryptfs"),
                 {step{passwordChooser()},
                  step{directoryPairChooser(DirectoryPairChooserWidget::AutoFillPaths | DirectoryPairChooserWidget::ShowDevicePicker
                                            | DirectoryPairChooserWidget::ShowMountPointPicker | DirectoryPairChooserWidget::RequireEmptyDevice
                                            | DirectoryPairChooserWidget::RequireEmptyMountPoint)},
                  step{activitiesChooser(), offlineOnlyChooser()}}}};

    Private(VaultCreationWizard *parent)
        : WBASE(VaultCreationWizard)(parent)
    {
        initBase();
        hideBackendSelector();
    }

    void finish()
    {
        q->setEnabled(false);

        auto collectedPayload = firstStepModule->fields();
        for (const auto *module : currentStepModules) {
            collectedPayload.insert(module->fields());
        }

        const auto name = collectedPayload[KEY_NAME].toString();
        const PlasmaVault::Device device(collectedPayload[KEY_DEVICE].toString());
        const PlasmaVault::MountPoint mountPoint(collectedPayload[KEY_MOUNT_POINT].toString());

        auto vault = new PlasmaVault::Vault(device, q);

        auto future = vault->create(name, mountPoint, collectedPayload);

        auto result = AsynQt::await(future);

        if (result) {
            Q_EMIT q->createdVault(vault);
            q->QDialog::accept();

        } else {
            ui.message->setText(result.error().message());
            ui.message->setMessageType(KMessageWidget::Error);
            ui.message->show();
            vault->scheduleDeletion();
        }

        q->setEnabled(true);
    }
};

VaultCreationWizard::VaultCreationWizard(QWidget *parent)
    : QDialog(parent)
    , d(new Private(this))
{
    setWindowTitle(i18nc("@title:window", "Create a New Vault"));
}

VaultCreationWizard::~VaultCreationWizard()
{
}

#include "moc_vaultcreationwizard.cpp"
