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
            delegate: VaultItem {}

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
