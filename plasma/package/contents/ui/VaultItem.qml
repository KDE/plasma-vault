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

MouseArea {
    id: vaultItem

    property alias icon: vaultIcon.source
    property alias name: vaultName.text
    property alias message: vaultMessage.text

    property bool isOpened: false
    property bool isBusy: false

    property var vaultsModel: plasmoid.nativeInterface.vaultsModel

    hoverEnabled: true
    height: units.iconSizes.medium + (actionsList.visible ? actionsList.height : 0)

    onContainsMouseChanged: {
        vaultItem.ListView.view.currentIndex = (containsMouse ? index : -1)
    }

    PlasmaCore.IconItem {
        id: vaultIcon

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            margins: units.smallSpacing
        }

        width: units.iconSizes.medium
        height: units.iconSizes.medium
    }

    PlasmaComponents.ToolButton {
        id: buttonOpenClose

        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: units.smallSpacing
        }

        width: units.iconSizes.medium
        height: units.iconSizes.medium
        visible: !busyIndicator.visible

        iconName: isOpened ? "media-eject" : "media-mount"

        onClicked: vaultsModel.toggle(model.device)
    }

    PlasmaComponents.BusyIndicator {
        id: busyIndicator
        anchors.fill: buttonOpenClose
        visible: isBusy
    }

    Column {
        anchors {
            left: vaultIcon.right
            right: buttonOpenClose.left
            verticalCenter: parent.verticalCenter
            margins: units.smallSpacing
            leftMargin: 2 * units.smallSpacing
        }

        PlasmaComponents.Label {
            id: vaultName

            width: parent.width
            height: undefined
            elide: Text.ElideRight
        }

        PlasmaComponents.Label {
            id: vaultMessage

            width: parent.width
            height: undefined
            elide: Text.ElideRight
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.log("TODO: Mount and open dolphin");
        }
    }
}


