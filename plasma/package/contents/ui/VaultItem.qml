/*
 *   Copyright 2017, 2018, 2019 by Ivan Cukic <ivan.cukic (at) kde.org>
 *   Copyright 2020 by Nate Graham <nate@kde.org>
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

import QtQuick 2.1
import QtQuick.Controls 2.12

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaExtras.ExpandableListItem {
    id: expandableListItem
    property var vaultsModelActions: plasmoid.nativeInterface.vaultsModel.actionsModel()

    icon: model.icon
    iconEmblem: model.message.length !== 0 ? "emblem-error" :
                            model.isOpened ? "emblem-mounted" :
                        model.isOfflineOnly ? "network-disconnect" :
                                                ""
    title: model.name
    subtitle: model.message
    defaultActionButtonAction: Action {
        icon.name: model.isOpened ? "media-eject" : "media-mount"
        text: model.isOpened ? i18nd("plasmavault-kde", "Close vault") : i18nd("plasmavault-kde", "Open vault")
        onTriggered: {
            vaultsModelActions.toggle(model.device);
        }
    }
    isBusy: Plasmoid.busy
    isEnabled: model.isEnabled
    contextualActionsModel: [
        Action {
            icon.name: "system-file-manager"
            text: i18nd("plasmavault-kde", "Open with File Manager")
            onTriggered: vaultsModelActions.openInFileManager(model.device);
        },
        Action {
            icon.name: "window-close"
            text: i18nd("plasmavault-kde", "Forcefully close")
            onTriggered: vaultsModelActions.forceClose(model.device);
            enabled: model.isOpened
        },
        Action {
            icon.name: "configure"
            text: i18nd("plasmavault-kde", "Configure Vault...")
            onTriggered: vaultsModelActions.configure(model.device);
        }
    ]
}
