/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

PlasmoidItem {
    property var vaultsModel: Plasmoid.vaultsModel
    property var vaultsModelActions: Plasmoid.vaultsModel.actionsModel()

    property var expandedItem: null

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    Plasmoid.busy: vaultsModelActions.isBusy

    Plasmoid.icon: {
        let iconName = (vaultsModelActions.hasError ? "plasmavault_error" : "plasmavault");

        if (inPanel) {
            return iconName += "-symbolic"
        }

        return iconName;
    }

    Plasmoid.status: vaultsModelActions.hasOpenVaults ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus

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

                model: KItemModels.KSortFilterProxyModel {
                    sortRoleName: "name"
                    sourceModel: vaultsModel
                }

                clip: true
                currentIndex: -1
                spacing: Kirigami.Units.smallSpacing
                topMargin: Kirigami.Units.largeSpacing
                leftMargin: Kirigami.Units.largeSpacing
                rightMargin: Kirigami.Units.largeSpacing
                bottomMargin: Kirigami.Units.largeSpacing

                highlight: PlasmaExtras.Highlight {}
                highlightMoveDuration: Kirigami.Units.shortDuration
                highlightResizeDuration: Kirigami.Units.shortDuration
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
                icon.name: "list-add-symbolic"

                onClicked: { createAction.trigger() }
                Layout.alignment: Qt.AlignLeft
            }
        }
    }
}
