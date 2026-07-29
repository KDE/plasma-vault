/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultapplet.h"

#include <QSortFilterProxyModel>

#include <KConfigGroup>
#include <KSharedConfig>
#include <PlasmaActivities/Consumer>
#include <klocalizedstring.h>

#include "../kded/engine/types.h"
#include "vaultsmodel.h"
#include <config-plasma-vault.h>

#if HAVE_NETWORKMANAGER
#include <NetworkManagerQt/Manager>
#else
void SortedVaultsModelProxy::checkAndEnableInternetConnection()
{
}
#endif

VaultApplet::VaultApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
{
}

VaultApplet::~VaultApplet()
{
}

void VaultApplet::restoreNetworking()
{
    // Bug #457680: allow restoring networking when the vault hasn't been closed before shutting down. Has to be launched ASAP after the system is up and running.

    auto config = KSharedConfig::openStateConfig(PLASMAVAULT_CONFIG_FILE);
    KConfigGroup networkConfig(config, "NetworkingConfig");

    if (!networkConfig.readEntry("is-networking-disabled", false)) {
        return;
    }

    NetworkManager::setNetworkingEnabled(true);

    //Update the config to avoid the unnecessary enabling in the future reboots
    networkConfig.writeEntry("is-networking-disabled", false);
}

QObject *VaultApplet::vaultsModel()
{
    if (!m_vaultsModel) {
        m_vaultsModel = new SortedVaultsModelProxy(this);
    }

    // Bug #457680: allow restoring networking when the vault hasn't been closed before shutting down. Has to be launched ASAP after the system is up and
    // running.
    VaultApplet::restoreNetworking();

    return m_vaultsModel;
}

K_PLUGIN_CLASS(VaultApplet)

#include "vaultapplet.moc"

#include "moc_vaultapplet.cpp"
