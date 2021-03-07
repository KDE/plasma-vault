/*
 *   Copyright 2017, 2018, 2019 by Ivan Cukic <ivan.cukic (at) kde.org>
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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents // for Highlight
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    property var vaultsModel: plasmoid.nativeInterface.vaultsModel
    property var vaultsModelActions: plasmoid.nativeInterface.vaultsModel.actionsModel()

    property var expandedItem: null

    Plasmoid.busy: vaultsModelActions.isBusy

    Plasmoid.icon: vaultsModelActions.hasError ? "plasmavault_error" : "plasmavault";

    Plasmoid.status: vaultsModelActions.count > 0 ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus

    Plasmoid.onExpandedChanged: {
        plasmoid.nativeInterface.vaultsModel.reloadDevices();
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

            model:
                PlasmaCore.SortFilterModel {
                    sortRole: "name"
                    sourceModel: vaultsModel
                }

            currentIndex: -1

            Layout.fillWidth: true
            Layout.fillHeight: true

            highlight: PlasmaComponents.Highlight {}
            highlightMoveDuration: units.longDuration
            highlightResizeDuration: units.longDuration
            delegate: VaultItem {
                width: vaultsList.width
            }

            interactive: false

            PlasmaExtras.PlaceholderMessage {
                id: noVaultsMessage

                anchors.centerIn: parent
                width: parent.width - (units.largeSpacing * 4)

                visible: vaultsList.count === 0

                text: i18nd("plasmavault-kde", "No Vaults have been set up")

                helpfulAction: Action {
                    text: i18nd("plasmavault-kde", "Create a New Vault...")
                    icon.name: "list-add"
                    onTriggered: vaultsModelActions.requestNewVault()
                }
            }
        }

        PlasmaComponents3.Button {
            id: buttonCreateNewVault

            visible: vaultsList.count > 0

            text: i18nd("plasmavault-kde", "Create a New Vault...")
            icon.name: "list-add"

            onClicked: vaultsModelActions.requestNewVault()
            Layout.alignment: Qt.AlignLeft
        }
    }
}
