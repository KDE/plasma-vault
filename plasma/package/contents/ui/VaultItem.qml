/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *   SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

PlasmaExtras.ExpandableListItem {
    id: expandableListItem
    property var vaultsModelActions: Plasmoid.vaultsModel.actionsModel()

    icon: model.icon
    iconEmblem: model.message.length !== 0 ? "emblem-error" :
                            model.isOpened ? "emblem-mounted" :
                        model.isOfflineOnly ? "network-disconnect-symbolic" :
                                                ""
    title: model.name
    subtitle: model.message
    subtitleCanWrap: true
    defaultActionButtonAction: QQC2.Action {
        icon.name: model.isOpened ? "lock-symbolic" : "unlock-symbolic"
        text: model.isOpened ? i18nd("plasmavault-kde", "Lock Vault") : i18nd("plasmavault-kde", "Unlock and Open")
        onTriggered: {
            if (model.isOpened) {
                vaultsModelActions.toggle(model.device);
            } else {
                vaultsModelActions.openInFileManager(model.device);
            }
        }
    }
    isBusy: Plasmoid.busy
    enabled: model.isEnabled
    contextualActions: [
        QQC2.Action {
            enabled: model.isOpened
            icon.name: "document-open-folder-symbolic"
            text: i18nd("plasmavault-kde", "Show in File Manager")
            onTriggered: {
                vaultsModelActions.openInFileManager(model.device);
            }
        },
        QQC2.Action {
            icon.name: model.isOpened ? "window-close-symbolic" : "unlock-symbolic"
            text: model.isOpened ? i18nd("plasmavault-kde", "Forcefully Lock Vault") : i18nd("plasmavault-kde", "Unlock Vault")
            onTriggered: {
                if (model.isOpened) {
                    vaultsModelActions.forceClose(model.device);
                } else {
                    vaultsModelActions.toggle(model.device);
                }
            }
        },
        QQC2.Action {
            icon.name: "configure-symbolic"
            text: i18nd("plasmavault-kde", "Configure Vault…")
            onTriggered: vaultsModelActions.configure(model.device);
        }
    ]
}
