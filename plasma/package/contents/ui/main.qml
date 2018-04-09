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

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    // For quick mockups in plasmoidviewer
    // property var vaultsModel: ListModel {
    //     ListElement {
    //         name: "TestVault"
    //         icon: "folder-decrypted"
    //         device: ""
    //         mountPoint: ""
    //         isBusy: false
    //         isOpened: true
    //         activities: []
    //         status: 1 // VaultInfo::Opened
    //         message: ""
    //     }
    // }

    property var vaultsModel: plasmoid.nativeInterface.vaultsModel
    property var vaultsModelActions: plasmoid.nativeInterface.vaultsModel.source()

    property var expandedItem: null

    Binding {
        target: plasmoid
        property: "busy"
        value: vaultsModelActions.isBusy
    }

    Binding {
        target: plasmoid
        property: "icon"
        value: {
            return vaultsModelActions.hasError ? "plasmavault_error" : "plasmavault";
        }
    }

    Plasmoid.fullRepresentation: ColumnLayout {
        anchors {
            fill: parent
        }

        Layout.minimumWidth: 300 * units.devicePixelRatio
        Layout.minimumHeight: 200 * units.devicePixelRatio

        // PlasmaExtras.Heading {
        //    text: i18nd("plasmavault-kde", "Encrypted vaults")
        // }



        ListView {
            id: vaultsList
            model: vaultsModel

            currentIndex: -1

            Layout.fillWidth: true
            Layout.fillHeight: true

            highlight: PlasmaComponents.Highlight {
                id: highlight
            }

            delegate: VaultItem {
                icon: model.icon
                name: model.name
                message: model.message
                isOpened: model.isOpened
                device: model.device
                isOfflineOnly: model.isOfflineOnly

                width: parent.width

                onItemExpanded: {
                    if (expandedItem != null) {
                        expandedItem.collapse();
                    }
                    expandedItem = item;
                }
            }

            visible: count > 0

            interactive: false
        }

        PlasmaComponents.Button {
            id: buttonCreateNewVault

            text: i18nd("plasmavault-kde", "Create a New Vault...")
            iconSource: "list-add"

            onClicked: vaultsModelActions.requestNewVault()
            Layout.alignment: vaultsList.visible ? Qt.AlignLeft : Qt.AlignCenter
        }
    }
}
