/***************************************************************************
 *   Copyright (C) %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    property var vaultsModel: plasmoid.nativeInterface.vaultsModel

    Plasmoid.fullRepresentation: ColumnLayout {
        anchors.fill: parent

        Layout.minimumWidth: 200
        Layout.minimumHeight: 200

        // PlasmaExtras.Heading {
        //    text: i18n("Encrypted vaults")
        // }

        ListView {

            model: vaultsModel

            Layout.fillWidth: true
            Layout.fillHeight: true

            highlight: PlasmaComponents.Highlight {
                id: highlight
            }

            delegate: PlasmaComponents.ListItem {

                RowLayout {
                    PlasmaCore.IconItem {
                        source: model.icon

                        PlasmaComponents.BusyIndicator {
                            anchors.fill: parent
                            visible: model.isBusy
                        }
                    }

                    PlasmaComponents.Label {
                        text: model.name
                    }
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: vaultsModel.toggle(model.device)
                }
            }
        }
    }
}
