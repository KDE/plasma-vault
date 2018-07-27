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

#include "vaultimportingwizard.h"
#include "ui_vaultimportingwizard.h"

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
#include "offlineonlywidget.h"

#include "vaultwizardbase.h"

class VaultImportingWizard::Private: public WBASE(VaultImportingWizard) {
public:

    Logic logic
    {
        { "encfs" / i18n("EncFS"),
            {
                step { directoryPairChooser(
                           DirectoryPairChooserWidget::ShowDevicePicker |
                           DirectoryPairChooserWidget::ShowMountPointPicker |
                           DirectoryPairChooserWidget::RequireExistingDevice |
                           DirectoryPairChooserWidget::RequireEmptyMountPoint
                       ) },
                step { passwordChooser() },
                step {
                    activitiesChooser(),
                    offlineOnlyChooser()
                }
            }
        },

        { "cryfs" / i18n("CryFS"),
            {
                step { directoryPairChooser(
                           DirectoryPairChooserWidget::ShowDevicePicker |
                           DirectoryPairChooserWidget::ShowMountPointPicker |
                           DirectoryPairChooserWidget::RequireExistingDevice |
                           DirectoryPairChooserWidget::RequireEmptyMountPoint
                       ) },
                step { passwordChooser() },
                step {
                    activitiesChooser(),
                    offlineOnlyChooser()
                }
            }
        }
    };

    Private(VaultImportingWizard *parent)
        : WBASE(VaultImportingWizard)(parent)
    {
        lastButtonText = i18n("Import");
        initBase();
    }

    void finish()
    {
        auto collectedPayload = firstStepModule->fields();
        for (const auto* module: currentStepModules) {
            collectedPayload.unite(module->fields());
        }

        const auto name = collectedPayload[KEY_NAME].toString();
        const PlasmaVault::Device device(collectedPayload[KEY_DEVICE].toString());
        const PlasmaVault::MountPoint mountPoint(collectedPayload[KEY_MOUNT_POINT].toString());

        auto vault = new PlasmaVault::Vault(device, q);

        auto future = vault->import(name, mountPoint, collectedPayload);

        auto result = AsynQt::await(future);

        if (result) {
            emit q->importedVault(vault);
            q->QDialog::accept();

        } else {
            ui.message->setText(result.error().message());
            ui.message->setMessageType(KMessageWidget::Error);
            ui.message->show();
            vault->scheduleDeletion();
        }
    }
};



VaultImportingWizard::VaultImportingWizard(QWidget *parent)
    : QDialog(parent)
    , d(new Private(this))
{
    setWindowTitle(i18nc("@title:window", "Import an Existing Vault"));
}



VaultImportingWizard::~VaultImportingWizard()
{
}



