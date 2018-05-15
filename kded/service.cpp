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
#include "ui/mountdialog.h"

#include <functional>

#include <NetworkManagerQt/Manager>

K_PLUGIN_FACTORY_WITH_JSON(PlasmaVaultServiceFactory,
                           "plasmavault.json",
                           registerPlugin<PlasmaVaultService>();)

using namespace PlasmaVault;

using AsynQt::Expected;

class PlasmaVaultService::Private {
public:
    QHash<Device, Vault*> knownVaults;
    QSet<Device> openVaults;
    KActivities::Consumer kamd;

    struct NetworkingState {
        bool wasNetworkingEnabled;
        QVector<QString> devicesInhibittingNetworking;
    };
    // Ideally, this would be std::optional... lovely C++17
    Expected<NetworkingState, int> savedNetworkingState =
        Expected<NetworkingState, int>::error(0);


    void saveNetworkingState()
    {
        // Ignore the request if we already have a saved state
        if (savedNetworkingState) {
            return;
        }

        savedNetworkingState = Expected<NetworkingState, int>::success(
                NetworkingState {
                    NetworkManager::isNetworkingEnabled() || true,
                    {}
                });
    }


    void restoreNetworkingState()
    {
        // Ignore the request if we do not have a state saved
        // or if there are more devices inhibitting networking
        if (!savedNetworkingState || !savedNetworkingState->devicesInhibittingNetworking.isEmpty()) {
            return;
        }

        NetworkManager::setNetworkingEnabled(savedNetworkingState->wasNetworkingEnabled);
    }


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

    if (vault->status() == VaultInfo::Opened) {
        d->openVaults << vault->device();
    }
}



void PlasmaVaultService::forgetVault(Vault* vault)
{
    // Can not be open
    // d->openVaults.remove(vault.device());
    // and therefore can not inhibit networking
    // ... d->savedNetworkingState ...

    emit vaultRemoved(vault->device().data());

    d->knownVaults.remove(vault->device());
    vault->deleteLater();
}



void PlasmaVaultService::onVaultStatusChanged(VaultInfo::Status status)
{
    const auto vault = qobject_cast<Vault*>(sender());

    if (status == VaultInfo::Dismantled) {
        forgetVault(vault);

    } else if (status == VaultInfo::Opened) {
        d->openVaults << vault->device();
        if (d->openVaults.size() == 1) {
            emit hasOpenVaultsChanged(true);
        }

    } else {
        d->openVaults.remove(vault->device());
        if (d->openVaults.isEmpty()) {
            emit hasOpenVaultsChanged(false);
        }

    }

    if (vault->isOfflineOnly()) {
        d->saveNetworkingState();
        auto& devicesInhibittingNetworking = d->savedNetworkingState->devicesInhibittingNetworking;

        // We need to check whether this vault
        // should be added or removed from the
        // inhibitors list
        const bool alreadyInhibiting =
            devicesInhibittingNetworking.contains(vault->device());

        if (status == VaultInfo::Opened && !alreadyInhibiting) {
            devicesInhibittingNetworking << vault->device();
        }

        if (status != VaultInfo::Opened && alreadyInhibiting) {
            devicesInhibittingNetworking.removeAll(vault->device());
        }

        // qDebug() << "Vaults inhibitting networking: " << devicesInhibittingNetworking;

        // Now, let's handle the networking part
        if (!devicesInhibittingNetworking.isEmpty()) {
            NetworkManager::setNetworkingEnabled(false);
        }

        d->restoreNetworkingState();
    }

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


void showPasswordMountDialog(Vault *vault, const std::function<void()> &function)
{
    auto dialog = new MountDialog(vault, function);
    dialog->open();
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
            new KRun(QUrl::fromLocalFile((QString)vault->mountPoint()), nullptr);

        } else {
            showPasswordMountDialog(vault, [this, vault] {
                emit vaultChanged(vault->info());
                new KRun(QUrl::fromLocalFile((QString)vault->mountPoint()), nullptr);
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



bool PlasmaVaultService::hasOpenVaults() const
{
    return !d->openVaults.isEmpty();
}



void PlasmaVaultService::closeAllVaults()
{
    for (const auto& device: d->openVaults) {
        closeVault(device.data());
    }
}



void PlasmaVaultService::forceCloseAllVaults()
{
    for (const auto& device: d->openVaults) {
        forceCloseVault(device.data());
    }
}



void PlasmaVaultService::deleteVault(const QString &device, const QString &name)
{
    if (!d->knownVaults.contains(Device(device))) {
        qWarning() << "The specified vault does not exist: " << device;
        return;
    }

    auto vault = d->knownVaults[Device(device)];

    if (vault->status() == VaultInfo::Opened) {
        qWarning() << "Can not delete an open vault: " << device;
        return;
    }

    if (vault->name() != name) {
        qWarning() << "Name is not correct: " << device;
        return;
    }

    vault->dismantle({});
}


#include "service.moc"

