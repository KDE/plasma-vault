/*
 *   Copyright (C) 2017 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#include "directorypairchooserwidget.h"
#include "ui_directorypairchooserwidget.h"

#include "vault.h"

class DirectoryPairChooserWidget::Private {
public:
    Ui::DirectoryPairChooserWidget ui;
    DirectoryPairChooserWidgetFlags flags;

    bool mountPointValid = false;
    bool encryptedLocationValid = false;

    DirectoryPairChooserWidget *const q;
    Private(DirectoryPairChooserWidget *parent)
        : q(parent)
    {
    }

    void setEncryptedLocationValid(bool valid)
    {
        if (encryptedLocationValid == valid) return;

        encryptedLocationValid = valid;

        // We only change the global valid state if
        // the mount point was already valid
        if (mountPointValid) {
            q->setIsValid(valid);
        }
    }

    void setMountPointValid(bool valid)
    {
        if (mountPointValid == valid) return;

        mountPointValid = valid;

        // We only change the global valid state if
        // the enc location was already valid
        if (encryptedLocationValid) {
            q->setIsValid(valid);
        }
    }

    bool isDirectoryValid(const QUrl &url) const
    {
        if (url.isEmpty()) return false;

        QDir directory(url.toString());

        // TODO: Support alternative flags once they are needed
        if (!directory.exists() || directory.entryList().isEmpty()) return true;

        return false;
    }
};



DirectoryPairChooserWidget::DirectoryPairChooserWidget(DirectoryPairChooserWidgetFlags flags)
    : DialogDsl::DialogModule(false)
    , d(new Private(this))
{
    d->ui.setupUi(this);
    d->flags = flags;

    connect(d->ui.editDevice, &KUrlRequester::textEdited,
            this, [&] (const QString &url) {
                d->setEncryptedLocationValid(d->isDirectoryValid(url));
            });

    connect(d->ui.editMountPoint, &KUrlRequester::textEdited,
            this, [&] (const QString &url) {
                d->setMountPointValid(d->isDirectoryValid(url));
            });

}



DirectoryPairChooserWidget::~DirectoryPairChooserWidget()
{
}



PlasmaVault::Vault::Payload DirectoryPairChooserWidget::fields() const
{
    return {
        { KEY_DEVICE,      d->ui.editDevice->text() },
        { KEY_MOUNT_POINT, d->ui.editMountPoint->text() }
    };
}



void DirectoryPairChooserWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    const auto name = payload[KEY_NAME].toString();

    d->ui.editDevice->setText("~/.vaults/" + name + ".enc");
    d->ui.editMountPoint->setText("~/Vaults/" + name);
}



