/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultapplet.h"

#include <QSortFilterProxyModel>

#include <KActivities/Consumer>
#include <klocalizedstring.h>

#include "vaultsmodel.h"

VaultApplet::VaultApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
{
}

VaultApplet::~VaultApplet()
{
}

QObject *VaultApplet::vaultsModel()
{
    if (!m_vaultsModel) {
        m_vaultsModel = new SortedVaultsModelProxy(this);
    }

    return m_vaultsModel;
}

K_PLUGIN_CLASS_WITH_JSON(VaultApplet, "package/metadata.json")

#include "vaultapplet.moc"
