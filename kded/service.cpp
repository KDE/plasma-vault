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

#include "service.h"

#include <QDBusObjectPath>

#include <KPluginFactory>
#include <KPasswordDialog>
#include <KLocalizedString>
#include <KActivities/Consumer>
#include <KRun>

#include "engine/vault.h"
#include "engine/commandresult.h"

#include "ui/vaultcreationwizard.h"
#include "ui/vaultconfigurationwizard.h"

K_PLUGIN_FACTORY_WITH_JSON(PlasmaVaultServiceFactory,
                           "plasmavault.json",
                           registerPlugin<PlasmaVaultService>();)

using namespace PlasmaVault;

class PlasmaVaultService::Private {
public:
    QHash<Device, Vault*> knownVaults;
    KActivities::Consumer kamd;

    Vault* vaultFor(const QString &device_) const
    {
        const Device device(device_);

        if (!knownVaults.contains(device)) {
            return nullptr;
        }

        return knownVaults[device];
    }

};



PlasmaVaultService::PlasmaVaultService(QObject * parent, const QVariantList&)
    : KDEDModule(parent)
    , d(new Private())
{
    connect(this, &KDEDModule::moduleRegistered, this,
            &PlasmaVaultService::slotRegistered);

    connect(&d->kamd, &KActivities::Consumer::currentActivityChanged,
            this,     &PlasmaVaultService::onCurrentActivityChanged);

    init();
}



PlasmaVaultService::~PlasmaVaultService()
{
}



void PlasmaVaultService::init()
{
    for (const Device &device: Vault::availableDevices()) {
        registerVault(new Vault(device, this));
    }
}



PlasmaVault::VaultInfoList PlasmaVaultService::availableDevices() const
{
    PlasmaVault::VaultInfoList result;
    for (const auto &vault: d->knownVaults.values()) {
        const auto vaultData = vault->info();
        result << vaultData;
    }
    return result;
}



void PlasmaVaultService::requestNewVault()
{
    const auto dialog = new VaultCreationWizard();

    connect(dialog, &VaultCreationWizard::createdVault,
            this,   &PlasmaVaultService::registerVault);

    dialog->show();
}



void PlasmaVaultService::slotRegistered(const QDBusObjectPath &path)
{
    if (path.path() == QLatin1String("/modules/plasmavault")) {
        emit registered();
    }
}



void PlasmaVaultService::registerVault(Vault *vault)
{
    if (!vault->isValid()) {
        qWarning() << "Warning: Trying to register an invalid vault: "
                   << vault->device();
        return;
    }

    if (d->knownVaults.contains(vault->device())) {
        qWarning() << "Warning: This one is already registered: "
                   << vault->device();
        return;
    }

    vault->setParent(this);

    d->knownVaults[vault->device()] = vault;

    connect(vault, &Vault::statusChanged,
            this,  &PlasmaVaultService::onVaultStatusChanged);
    connect(vault, &Vault::messageChanged,
            this,  &PlasmaVaultService::onVaultMessageChanged);
    connect(vault, &Vault::infoChanged,
            this,  &PlasmaVaultService::onVaultInfoChanged);

    emit vaultAdded(vault->info());
}



void PlasmaVaultService::onVaultStatusChanged(VaultInfo::Status status)
{
    Q_UNUSED(status);

    const auto vault = qobject_cast<Vault*>(sender());

    emit vaultChanged(vault->info());
}



void PlasmaVaultService::onVaultInfoChanged()
{
    const auto vault = qobject_cast<Vault*>(sender());
    emit vaultChanged(vault->info());
}



void PlasmaVaultService::onVaultMessageChanged(const QString &message)
{
    Q_UNUSED(message);

    const auto vault = qobject_cast<Vault*>(sender());

    emit vaultChanged(vault->info());
}


template <typename Function>
class PasswordMountDialog: protected KPasswordDialog { //_
public:
    PasswordMountDialog(Vault *vault, Function function)
        : m_vault(vault)
        , m_function(function)
    {
    }

    void show()
    {
        KPasswordDialog::show();
    }

private:
    bool checkPassword() override
    {
        auto future = m_vault->open({ { KEY_PASSWORD, password() } });

        const auto result = AsynQt::await(future);

        if (result) {
            m_function();
            return true;

        } else {
            showErrorMessage(result.error().message());
            return false;
        }
    }

    void hideEvent(QHideEvent *) override
    {
        deleteLater();
    }

    Vault *m_vault;
    Function m_function;
};

template <typename Function>
void showPasswordMountDialog(Vault *vault, Function &&function)
{
    auto dialog = new PasswordMountDialog<Function>(
        vault, std::forward<Function>(function));
    dialog->show();
}
//^

void PlasmaVaultService::openVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        if (vault->isOpened()) return;

        showPasswordMountDialog(vault,
                [this, vault] {
                    emit vaultChanged(vault->info());
                });
    }
}



void PlasmaVaultService::closeVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        if (!vault->isOpened()) return;

        vault->close();
    }
}



void PlasmaVaultService::configureVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        const auto dialog = new VaultConfigurationWizard(vault);

        // connect(dialog, &VaultConfigurationWizard::configurationChanged,
        //         this,   &PlasmaVaultService::registerVault);

        dialog->show();
    }
}



void PlasmaVaultService::forceCloseVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        if (!vault->isOpened()) return;

        vault->forceClose();
    }
}



void PlasmaVaultService::openVaultInFileManager(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        if (vault->isOpened()) {
            new KRun(QUrl::fromLocalFile((QString)vault->mountPoint()), 0);

        } else {
            showPasswordMountDialog(vault, [this, vault] {
                emit vaultChanged(vault->info());
                new KRun(QUrl::fromLocalFile((QString)vault->mountPoint()), 0);
            });
        }
    }
}



void PlasmaVaultService::onCurrentActivityChanged(
    const QString &currentActivity)
{
    for (auto* vault: d->knownVaults.values()) {
        const auto vaultActivities = vault->activities();
        if (!vaultActivities.isEmpty() && !vaultActivities.contains(currentActivity)) {
            vault->close();
        }
    }
}


#include "service.moc"

