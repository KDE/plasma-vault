/*
 *   Copyright (C) 2017 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#include "service.h"

#include <QDBusObjectPath>

#include <KPluginFactory>
#include <KPasswordDialog>
#include <KLocalizedString>

#include "encfscreatedialog.h"

#include "vault.h"
#include "commandresult.h"

#include "ui/vaultcreationwizard.h"

K_PLUGIN_FACTORY_WITH_JSON(PlasmaVaultServiceFactory,
                           "plasmavault.json",
                           registerPlugin<PlasmaVaultService>();)

using namespace PlasmaVault;

class PlasmaVaultService::Private {
public:
    QHash<QString, Vault*> knownVaults;

};



PlasmaVaultService::PlasmaVaultService(QObject * parent, const QVariantList&)
    : KDEDModule(parent)
    , d(new Private())
{
    connect(this, &KDEDModule::moduleRegistered, this,
            &PlasmaVaultService::slotRegistered);
    init();
}



PlasmaVaultService::~PlasmaVaultService()
{
}



void PlasmaVaultService::init()
{
    qDebug() << "Called init";

    for (const auto &device: Vault::availableDevices()) {
        registerVault(new Vault(device, this));
    }
}



PlasmaVault::VaultDataList PlasmaVaultService::availableDevices() const
{
    PlasmaVault::VaultDataList result;
    for (const auto &vault: d->knownVaults.values()) {
        result << VaultData::from(vault);
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
    if (d->knownVaults.contains(vault->device())) {
        qWarning() << "Warning: This one is already registered: " << vault->device();
        return;
    }

    vault->setParent(this);

    qDebug() << "Registering a new mount object for : " << vault->device();
    d->knownVaults[vault->device()] = vault;

    connect(vault, &Vault::statusChanged,
            this,  &PlasmaVaultService::onVaultStatusChanged);

    emit vaultAdded(VaultData::from(vault));
}



void PlasmaVaultService::onVaultStatusChanged(Vault::Status status)
{
    const auto vault = qobject_cast<Vault*>(sender());
    qDebug() << "Vault status changed: " << vault->device() << " status = " << status;

    emit vaultChanged(VaultData::from(vault));
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
    if (!d->knownVaults.contains(device)) return;
    const auto vault = d->knownVaults[device];

    showPasswordMountDialog(vault,
            [this, device] {
            qDebug() << "emit vaultChanged <----------------------------------";
                emit vaultChanged(VaultData::from(d->knownVaults[device]));
            });
}



void PlasmaVaultService::closeVault(const QString &device)
{
    if (!d->knownVaults.contains(device)) return;
    auto vault = d->knownVaults[device];

    vault->close();
}


#include "service.moc"

