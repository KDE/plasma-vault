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

#ifndef PLASMAVAULT_KDED_UI_VAULT_WIZARD_BASE_P_H
#define PLASMAVAULT_KDED_UI_VAULT_WIZARD_BASE_P_H

#include <QPushButton>

namespace PlasmaVault {
    class Vault;
} // namespace PlasmaVault

#define WBASE(Class) VaultWizardBase<Class, Ui::Class, Class::Private>

template <typename Class, typename Ui, typename Impl>
class VaultWizardBase {
public:
    Class *const q;
    Ui ui;

    QPushButton *buttonPrevious;
    QPushButton *buttonNext;
    QStackedLayout *layout;

    bool lastModule = false;
    QString lastButtonText;
    inline void setLastModule(bool last) {
        lastModule = last;
        if (last) {
            buttonNext->setText(lastButtonText);
            buttonNext->setIcon(QIcon::fromTheme("dialog-ok-apply"));
        } else {
            buttonNext->setText(i18n("Next"));
            buttonNext->setIcon(QIcon::fromTheme("go-next"));
        }
    }

    QVector<DialogDsl::DialogModule*> currentStepModules;
    steps currentSteps;
    BackendChooserWidget *firstStepModule = nullptr;
    DialogDsl::DialogModule *currentModule = nullptr;


    // to suggest the highest priority to the user as a starting value
    QMap<QString, int> priorities = {
        { "gocryptfs", 1 },
        { "encfs", 2 },
        { "cryfs", 3 },
    };

    template <typename ClickHandler>
    QPushButton *addDialogButton(const QString &icon, const QString &title,
                                 ClickHandler clickHandler)
    {
        auto button = new QPushButton(QIcon::fromTheme(icon), title);
        ui.buttons->addButton(button, QDialogButtonBox::ActionRole);
        QObject::connect(button, &QPushButton::clicked,
                         q, clickHandler);
        return button;
    }

    Impl* self()
    {
        return static_cast<Impl*>(this);
    }

    VaultWizardBase(Class *parent)
        : q(parent)
    {
        ui.setupUi(parent);
        ui.message->hide();

        layout = new QStackedLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        ui.container->setLayout(layout);

        lastButtonText = i18n("Create");
    }

    void initBase()
    {
        // The dialog buttons do not have previous/next by default
        // so we need to create them
        buttonPrevious = addDialogButton("go-previous", i18n("Previous"),
                [this] { previousStep(); });
        buttonNext = addDialogButton("go-next", i18n("Next"),
                [this] { if (lastModule) self()->finish(); else nextStep(); });

        // The 'Create' button should be hidden by default
        buttonPrevious->setEnabled(false);
        buttonNext->setEnabled(false);
        buttonNext->setDefault(true);

        // Loading the fist page of the wizard
        firstStepModule = new BackendChooserWidget();
        setCurrentModule(firstStepModule);
        layout->addWidget(firstStepModule);

        // Loading the backends to the combo box
        for (const auto& key: self()->logic.keys()) {
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
                buttonNext->setEnabled(valid);
            });

        // Lets update the button states

        // 1. next/create button is enabled only if the current
        //    module is in the valid state
        buttonNext->setEnabled(currentModule->isValid());

        // 2. previous button is enabled only if we are not on
        //    the first page
        buttonPrevious->setEnabled(currentStepModules.size() > 0);

        // 3. If we have loaded the last page, we want to show the
        //    'Create' button instead of 'Next'
        setLastModule(!currentSteps.isEmpty() && currentStepModules.size() == currentSteps.size());

        // Calling to initialize the module -- we are passing all the
        // previously collected data to it
        auto collectedPayload =
                firstStepModule == module ?
                    PlasmaVault::Vault::Payload{} :
                    firstStepModule->fields();
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
            currentSteps = self()->logic[fields[KEY_BACKEND].toByteArray()];
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

};


#endif // include guard

