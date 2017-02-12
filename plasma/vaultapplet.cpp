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

#include "vaultapplet.h"

#include <QSortFilterProxyModel>

#include <klocalizedstring.h>
#include <KActivities/Consumer>

#include "vaultsmodel.h"



VaultsModelProxy::VaultsModelProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_source(new VaultsModel(this))
    , m_kamd(new KActivities::Consumer(this))
{
    setSourceModel(m_source);

    connect(m_kamd, &KActivities::Consumer::currentActivityChanged,
            this,   &VaultsModelProxy::invalidate);
    connect(m_kamd, &KActivities::Consumer::serviceStatusChanged,
            this,   &VaultsModelProxy::invalidate);
}



bool VaultsModelProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftData = sourceModel()->data(left, VaultsModel::VaultName);
    const auto rightData = sourceModel()->data(right, VaultsModel::VaultName);

    return leftData < rightData;
}



bool VaultsModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    const auto activities =
        m_source->index(sourceRow).data(VaultsModel::VaultActivities).toStringList();

    return activities.size() == 0 || activities.contains(m_kamd->currentActivity());
}



void VaultsModelProxy::open(const QString &device)
{
    m_source->open(device);
}



void VaultsModelProxy::close(const QString &device)
{
    m_source->close(device);
}



void VaultsModelProxy::toggle(const QString &device)
{
    m_source->toggle(device);
}



void VaultsModelProxy::requestNewVault()
{
    m_source->requestNewVault();
}



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
        m_vaultsModel = new VaultsModelProxy(this);
    }

    return m_vaultsModel;
}



K_EXPORT_PLASMA_APPLET_WITH_JSON(vaultapplet, VaultApplet, "metadata.json")

#include "vaultapplet.moc"

