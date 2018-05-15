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

#ifndef PLASMAVAULT_PLASMA_VAULT_APPLET_H
#define PLASMAVAULT_PLASMA_VAULT_APPLET_H

#include <QObject>

#include <Plasma/Applet>

class VaultsModel;


class VaultApplet: public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QObject* vaultsModel READ vaultsModel CONSTANT)

public:
    explicit VaultApplet(QObject *parent, const QVariantList &args);
    ~VaultApplet();

    QObject *vaultsModel();

private:
    // Applet will be the parent of this object
    QObject *m_vaultsModel;
};

#endif // include guard

