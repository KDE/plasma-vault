/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.4
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmoidItem {
    property var vaultsModel: Plasmoid.vaultsModel
    property var vaultsModelActions: Plasmoid.vaultsModel.actionsModel()

    property var expandedItem: null

    Plasmoid.busy: vaultsModelActions.isBusy

    Plasmoid.icon: vaultsModelActions.hasError ? "plasmavault_error" : "plasmavault-symbolic";

    Plasmoid.status: vaultsModelActions.count > 0 ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus

    onExpandedChanged: {
        Plasmoid.vaultsModel.reloadDevices();
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            id: createAction
            text: i18nd("plasmavault-kde", "Create a New Vault…")
            icon.name: "list-add-symbolic"
            onTriggered: vaultsModelActions.requestNewVault()
        }
    ]

    fullRepresentation: PlasmaExtras.Representation {

        Layout.minimumWidth: Kirigami.Units.gridUnit * 18
        Layout.minimumHeight: Kirigami.Units.gridUnit * 12

        collapseMarginsHint: true

        PlasmaComponents3.ScrollView {
            anchors.fill: parent

            // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
            PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

            contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

            contentItem: ListView {
                id: vaultsList

                model:
                    PlasmaCore.SortFilterModel {
                        sortRole: "name"
                        sourceModel: vaultsModel
                    }

                currentIndex: -1
                topMargin: Kirigami.Units.smallSpacing * 2
                bottomMargin: Kirigami.Units.smallSpacing * 2
                leftMargin: Kirigami.Units.smallSpacing * 2
                rightMargin: Kirigami.Units.smallSpacing * 2
                spacing: Kirigami.Units.smallSpacing

                highlight: PlasmaExtras.Highlight {}
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                delegate: VaultItem {}

                PlasmaExtras.PlaceholderMessage {
                    id: noVaultsMessage

                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.gridUnit * 4)

                    visible: vaultsList.count === 0

                    iconName: "plasmavault"
                    text: i18nd("plasmavault-kde", "No Vaults have been set up")

                    helpfulAction: QQC2.Action {
                        text: createAction.text
                        icon.name: "list-add-symbolic"

                        onTriggered: createAction.trigger()
                    }
                }
            }
        }

        footer: RowLayout {
            PlasmaComponents3.Button {
                id: buttonCreateNewVault

                visible: vaultsList.count > 0 && !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)

                text: createAction.text
                icon.name: "list-add"

                onClicked: { createAction.trigger() }
                Layout.alignment: Qt.AlignLeft
            }
        }
    }
}
