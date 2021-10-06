/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    function action_createNewVault() {
        vaultsModelActions.requestNewVault()
    }

    Component.onCompleted: {
        plasmoid.setAction("createNewVault", i18nd("plasmavault-kde", "Create a New Vault…"), "list-add");
    }

    Plasmoid.fullRepresentation: ColumnLayout {

        Layout.minimumWidth: PlasmaCore.Units.gridUnit * 18
        Layout.minimumHeight: PlasmaCore.Units.gridUnit * 12

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
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            delegate: VaultItem {
                width: vaultsList.width
            }

            interactive: false

            PlasmaExtras.PlaceholderMessage {
                id: noVaultsMessage

                anchors.centerIn: parent
                width: parent.width - (PlasmaCore.Units.largeSpacing * 4)

                visible: vaultsList.count === 0

                text: i18nd("plasmavault-kde", "No Vaults have been set up")

                helpfulAction: Action {
                    text: plasmoid.action("createNewVault").text
                    icon.name: "list-add"

                    onTriggered: { plasmoid.action("createNewVault").trigger() }
                }
            }
        }

        PlasmaComponents3.Button {
            id: buttonCreateNewVault

            visible: vaultsList.count > 0 && !(plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)

            text: plasmoid.action("createNewVault").text
            icon.name: "list-add"

            onClicked: { plasmoid.action("createNewVault").trigger() }
            Layout.alignment: Qt.AlignLeft
        }
    }
}
