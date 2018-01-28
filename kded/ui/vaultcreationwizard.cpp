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

#include "vaultcreationwizard.h"
#include "ui_vaultcreationwizard.h"

#include <QPushButton>
#include <QMap>
#include <QVector>
#include <QStackedLayout>

#include "dialogdsl.h"
#include "vault.h"

using namespace DialogDsl;
using namespace DialogDsl::operators;

#include "backendchooserwidget.h"
#include "activitieslinkingwidget.h"
#include "cryfscypherchooserwidget.h"
#include "directorypairchooserwidget.h"
#include "noticewidget.h"
#include "passwordchooserwidget.h"

class VaultCreationWizard::Private {
public:
    VaultCreationWizard *const q;
    Ui::VaultCreationWizard ui;
    QPushButton *buttonPrevious;
    QPushButton *buttonNext;
    QPushButton *buttonCreate;
    QStackedLayout *layout;

    inline void buttonNextSetEnabled(bool enabled) {
        buttonNext->setEnabled(enabled);
        buttonCreate->setEnabled(enabled);
    }

    QVector<DialogDsl::DialogModule*> currentStepModules;
    steps currentSteps;
    BackendChooserWidget *firstStepModule = nullptr;
    DialogDsl::DialogModule *currentModule = nullptr;

    Logic logic
    {
        { "encfs" / i18n("EncFS"),
            {
                step { notice("encfs-message",
                       i18n("<b>Security notice:</b>\n\
                             According to a security audit by Taylor Hornby (Defuse Security),\n\
                             the current implementation of Encfs is vulnerable or potentially vulnerable\n\
                             to multiple types of attacks.\n\
                             For example, an attacker with read/write access\n\
                             to encrypted data might lower the decryption complexity\n\
                             for subsequently encrypted data without this being noticed by a legitimate user,\n\
                             or might use timing analysis to deduce information.\n\
                             <br /><br />\n\
                             This means that you should not synchronize\n\
                             the encrypted data to a cloud storage service,\n\
                             or use it in other circumstances where the attacker\n\
                             can frequently access the encrypted data.\n\
                             <br /><br />\n\
                             See <a href='http://defuse.ca/audits/encfs.htm'>defuse.ca/audits/encfs.htm</a> for more information."))
                     },
                step { passwordChooser() },
                step { directoryPairChooser(DirectoryPairChooserWidget::SkipDevicePicker) },
                step { activitiesChooser() }
            }
        },

        { "cryfs" / i18n("CryFS"),
            {
                step { notice("cryfs-message",
                       i18n("<b>Security notice:</b>\n\
                             CryFS encrypts your files, so you can safely store them anywhere.\n\
                             It works well together with cloud services like Dropbox, iCloud, OneDrive and others.\n\
                             <br /><br />\n\
                             Unlike some other file-system overlay solutions,\n\
                             it does not expose the directory structure,\n\
                             the number of files nor the file sizes\n\
                             through the encrypted data format.\n\
                             <br /><br />\n\
                             One important thing to note is that,\n\
                             while CryFS is considered safe,\n\
                             there is no independent security audit\n\
                             which confirms this."))
                     },
                step { passwordChooser() },
                step { directoryPairChooser() },
                step {
                    cryfsCypherChooser(),
                    activitiesChooser()
                }
            }
        }
    };

    // to suggest the highest priority to the user as a starting value
    QMap<QString, int> priorities = {
        { "encfs", 2 },
        { "cryfs", 1 }
    };

    template <typename ClickHandler>
    QPushButton *addDialogButton(const QString &icon, const QString &title, ClickHandler clickHandler)
    {
        auto button = new QPushButton(QIcon::fromTheme(icon), title);
        ui.buttons->addButton(button, QDialogButtonBox::ActionRole);
        QObject::connect(button, &QPushButton::clicked,
                         q, clickHandler);
        return button;
    }

    Private(VaultCreationWizard *parent)
        : q(parent)
    {
        ui.setupUi(parent);
        ui.message->hide();

        layout = new QStackedLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        ui.container->setLayout(layout);

        // The dialog buttons do not have previous/next by default
        // so we need to create them
        buttonPrevious = addDialogButton("go-previous",     i18n("Previous"), [this] { previousStep(); });
        buttonNext     = addDialogButton("go-next",         i18n("Next"),     [this] { nextStep(); });
        buttonCreate   = addDialogButton("dialog-ok-apply", i18n("Create"),   [this] { createVault(); });

        // The 'Create' button should be hidden by default
        buttonCreate->hide();
        buttonPrevious->setEnabled(false);
        buttonNextSetEnabled(false);

        // Loading the fist page of the wizard
        firstStepModule = new BackendChooserWidget();
        setCurrentModule(firstStepModule);
        layout->addWidget(firstStepModule);

        // Loading the backends to the combo box
        for (const auto& key: logic.keys()) {
            firstStepModule->addItem(key, key.translation(), priorities.value(key));
        }
        firstStepModule->checkBackendAvailable();
    }

    void setCurrentModule(DialogDsl::DialogModule *module)
    {
        // If there is a current module already, disconnect it
        if (currentModule) {
            currentModule->aboutToBeHidden();
            currentModule->disconnect();
        }

        // The current module needs to be changed
        currentModule = module;
        currentModule->aboutToBeShown();

        QObject::connect(
            currentModule, &DialogModule::isValidChanged,
            q, [&] (bool valid) {
                buttonNextSetEnabled(valid);
            });

        // Lets update the button states

        // 1. next/create button is enabled only if the current
        //    module is in the valid state
        buttonNextSetEnabled(currentModule->isValid());

        // 2. previous button is enabled only if we are not on
        //    the first page
        buttonPrevious->setEnabled(currentStepModules.size() > 0);

        // 3. If we have loaded the last page, we want to show the
        //    'Create' button instead of 'Next'
        if (!currentSteps.isEmpty() && currentStepModules.size() == currentSteps.size()) {
            buttonNext->hide();
            buttonCreate->show();
        } else {
            buttonNext->show();
            buttonCreate->hide();
        }

        // Calling to initialize the module -- we are passing all the
        // previously collected data to it
        auto collectedPayload = firstStepModule->fields();
        for (const auto* module: currentStepModules) {
            collectedPayload.unite(module->fields());
        }
        currentModule->init(collectedPayload);
    }

    void previousStep()
    {
        if (currentStepModules.isEmpty()) return;

        // We want to kill the current module, and move to the previous one
        currentStepModules.takeLast();
        currentModule->deleteLater();;

        if (currentStepModules.size()) {
            setCurrentModule(currentStepModules.last());
        } else {
            setCurrentModule(firstStepModule);
        }

        if (!currentModule->shouldBeShown()) {
            previousStep();
        }
    }

    void nextStep()
    {
        // If the step modules are empty, this means that we
        // have just started - the user chose the backend
        // and we need to load the vault creation steps
        if (currentStepModules.isEmpty()) {
            const auto &fields = firstStepModule->fields();
            currentSteps = logic[fields[KEY_BACKEND].toByteArray()];
        }

        // Loading the modulws that we need to show now
        auto subModules = currentSteps[currentStepModules.size()];

        // If there is only one module on the current page,
        // lets not complicate things by creating the compound module
        DialogModule *stepWidget =
            (subModules.size() == 1) ? subModules.first()()
                                     : new CompoundDialogModule(subModules);

        // Adding the widget to the list and the layout
        currentStepModules << stepWidget;
        layout->addWidget(stepWidget);
        layout->setCurrentWidget(stepWidget);

        // Set the newly added module to be the current
        setCurrentModule(stepWidget);

        if (!currentModule->shouldBeShown()) {
            nextStep();
        }
    }

    void createVault()
    {
        auto collectedPayload = firstStepModule->fields();
        for (const auto* module: currentStepModules) {
            collectedPayload.unite(module->fields());
        }

        const auto name = collectedPayload[KEY_NAME].toString();
        const PlasmaVault::Device device(collectedPayload[KEY_DEVICE].toString());
        const PlasmaVault::MountPoint mountPoint(collectedPayload[KEY_MOUNT_POINT].toString());

        auto vault = new PlasmaVault::Vault(device, q);

        auto future = vault->create(name, mountPoint, collectedPayload);

        auto result = AsynQt::await(future);

        if (result) {
            emit q->createdVault(vault);
            q->QDialog::accept();

        } else {
            ui.message->setText(result.error().message());
            ui.message->setMessageType(KMessageWidget::Error);
            ui.message->show();
            delete vault;
        }
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



