/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultapplet.h"

#include <QSortFilterProxyModel>

#include <klocalizedstring.h>
#include <KActivities/Consumer>

#include "vaultsmodel.h"



VaultApplet::VaultApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , m_vaultsModel(nullptr)
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



K_EXPORT_PLASMA_APPLET_WITH_JSON(vaultapplet, VaultApplet, "metadata.json")

#include "vaultapplet.moc"

