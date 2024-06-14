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
    iconEmblem: {
        if (model.message.length !== 0) {
            return "emblem-error";
        } else if (model.isOpened) {
            return "emblem-mounted";
        } else if (model.isOfflineOnly) {
            return "network-disconnect-symbolic";
        } else {
            return "";
        }
    }

    title: model.name
    subtitle: model.message
    subtitleCanWrap: true
    defaultActionButtonAction: QQC2.Action {
        icon.name: model.isOpened ? "lock-symbolic" : "unlock-symbolic"
        text: model.isOpened ? i18nd("plasmavault-kde", "Lock Vault") : i18nd("plasmavault-kde", "Unlock and Open")
        onTriggered: source => {
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
            onTriggered: source => {
                vaultsModelActions.openInFileManager(model.device);
            }
        },
        QQC2.Action {
            icon.name: model.isOpened ? "window-close-symbolic" : "unlock-symbolic"
            text: model.isOpened ? i18nd("plasmavault-kde", "Forcefully Lock Vault") : i18nd("plasmavault-kde", "Unlock Vault")
            onTriggered: source => {
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
            onTriggered: source => vaultsModelActions.configure(model.device);
        }
    ]
}
