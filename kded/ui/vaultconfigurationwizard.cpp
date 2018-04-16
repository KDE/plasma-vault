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

#include "vaultconfigurationwizard.h"
#include "ui_vaultconfigurationwizard.h"

#include <QPushButton>
#include <QMap>
#include <QVector>
#include <QStackedLayout>
#include <QTabWidget>

#include "dialogdsl.h"
#include "vault.h"

using namespace DialogDsl;
using namespace DialogDsl::operators;

#include "backendchooserwidget.h"
#include "activitieslinkingwidget.h"
#include "cryfscypherchooserwidget.h"
#include "directorychooserwidget.h"
#include "noticewidget.h"
#include "namechooserwidget.h"
#include "passwordchooserwidget.h"

using PlasmaVault::Vault;

class VaultConfigurationWizard::Private {
public:
    VaultConfigurationWizard *const q;
    Vault *vault;

    Ui::VaultConfigurationWizard ui;
    QStackedLayout *layout;

    steps currentSteps;
    QVector<DialogDsl::DialogModule*> currentModuleDialogs;

    steps defaultSteps
    {
        i18n("General") / step {
            nameChooser(),
            directoryChooser(DirectoryChooserWidget::RequireEmptyMountPoint)
        },

        i18n("Activities") / step {
            activitiesChooser()
        }

        /*
        i18n("Dismantle") / step {
            notice(
               "dismantle-message",
               i18n("Note that Plasma Vault will not delete any of the files,\n\
                     the dismantling process only removes the vault from Plasma.\n\
                     You will need to remove the files manually."),
               NoticeWidget::ShowAlways)
        }
        */
    };

    Logic logic
    {
        { "encfs" / i18n("EncFS"),
            defaultSteps
        },

        { "cryfs" / i18n("CryFS"),
            defaultSteps
        }
    };

    Private(Vault *vault, VaultConfigurationWizard *parent)
        : q(parent)
        , vault(vault)
    {
        ui.setupUi(parent);
        ui.message->hide();

        layout = new QStackedLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        ui.container->setLayout(layout);

        auto tabs = new QTabWidget();
        layout->addWidget(tabs);

        // Loading the backends
        auto modules = logic[Key(vault->backend().toLatin1())];

        Vault::Payload payload {
            { KEY_NAME,        QVariant(vault->name()) },
            { KEY_MOUNT_POINT, QVariant(vault->mountPoint()) },
            { KEY_ACTIVITIES,  QVariant(vault->activities()) }
        };

        for (const auto& module: modules) {
            DialogModule *stepWidget = new CompoundDialogModule(module);
            stepWidget->init(payload);
            tabs->addTab(stepWidget, module.title());
            currentModuleDialogs << stepWidget;
        }
    }

    void setVaultOpened(bool vaultOpened)
    {
        bool configurationEnabled = !vaultOpened;
        ui.buttons->button(QDialogButtonBox::Ok)->setEnabled(configurationEnabled);
        ui.frameUnlockVault->setVisible(!configurationEnabled);
        ui.container->setEnabled(configurationEnabled);
    }

    void saveConfiguration()
    {
        Vault::Payload collectedPayload;
        qDebug() << "Getting the data";
        for (const auto* module: currentModuleDialogs) {
            qDebug() << "Data: " << module->fields();
            collectedPayload.unite(module->fields());
        }

        const auto name = collectedPayload[KEY_NAME].toString();
        const PlasmaVault::MountPoint mountPoint(collectedPayload[KEY_MOUNT_POINT].toString());
        const auto activities = collectedPayload[KEY_ACTIVITIES].toStringList();

        if (name.isEmpty() || mountPoint.isEmpty()) return;

        vault->setName(name);
        vault->setMountPoint(mountPoint);
        vault->setActivities(activities);
    }
};



VaultConfigurationWizard::VaultConfigurationWizard(Vault *vault, QWidget *parent)
    : QDialog(parent)
    , d(new Private(vault, this))
{
    setWindowTitle(i18n("Configure"));

    d->setVaultOpened(vault->isOpened());

    connect(d->ui.buttonCloseVault, &QPushButton::clicked,
            this, [=] () {
                vault->close();
            });

    connect(vault, &Vault::isOpenedChanged,
            this, [=] (bool isOpened) {
                d->setVaultOpened(isOpened);
            });

    connect(d->ui.buttons, &QDialogButtonBox::accepted,
            this, [=] {
                d->saveConfiguration();
            });
}



VaultConfigurationWizard::~VaultConfigurationWizard()
{
}



