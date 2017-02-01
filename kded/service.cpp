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



QStringList PlasmaVaultService::availableDevices() const
{
    return d->knownVaults.keys();
}



void PlasmaVaultService::requestNewVault()
{
    const auto dialog = new Encfs::CreateDialog();

    connect(dialog, &Encfs::CreateDialog::createdVault,
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
    vault->setParent(this);

    qDebug() << "Registering a new mount object for : " << vault->device();
    d->knownVaults[vault->device()] = vault;

    connect(vault, &Vault::statusChanged,
            this,  &PlasmaVaultService::onVaultStatusChanged);
}



void PlasmaVaultService::onVaultStatusChanged(Vault::Status status)
{
    const auto vault = qobject_cast<Vault*>(sender());
    qDebug() << "Vault status changed: " << vault->device() << " status = " << status;

    emit vaultStatusChanged(vault->device(), status);
}



class PasswordMountDialog: protected KPasswordDialog {
public:
    PasswordMountDialog(Vault *vault)
        : m_vault(vault)
    {
    }

    void show()
    {
        KPasswordDialog::show();
    }

private:
    bool checkPassword() override
    {
        auto future = m_vault->open(password());

        const auto result = AsynQt::await(future);

        if (result) {
            return true;

        } else {
            qDebug() << "Error message: " << result.error().message();
            showErrorMessage(result.error().message());
            return false;
        }
    }

    void hideEvent(QHideEvent *) override
    {
        deleteLater();
    }

    Vault *m_vault;
};

void PlasmaVaultService::openVault(const QString &device)
{
    if (!d->knownVaults.contains(device)) return;
    auto vault = d->knownVaults[device];

    (new PasswordMountDialog(vault))->show();

    // KPasswordDialog passwordDialog;
    // passwordDialog.setPrompt(i18n("Enter the password to unlock vault \"%1\"", vault->name()));
    // passwordDialog.show();
    //
    // while (passwordDialog.isVisible()) {
    //     QCoreApplication::processEvents();
    // }
    //
    // const auto password = passwordDialog.password();
    //
    // if (password.isEmpty()) {
    //     return;
    // }
    //
    // vault->open(password);
}



void PlasmaVaultService::closeVault(const QString &device)
{
    if (!d->knownVaults.contains(device)) return;
    auto vault = d->knownVaults[device];
    vault->close();
}


#include "service.moc"

