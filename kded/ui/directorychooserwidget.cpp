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

#include "directorychooserwidget.h"
#include "ui_directorychooserwidget.h"

#include "vault.h"

class DirectoryChooserWidget::Private {
public:
    Ui::DirectoryChooserWidget ui;
    DirectoryChooserWidget::Flags flags;

    bool mountPointValid = false;

    DirectoryChooserWidget *const q;
    Private(DirectoryChooserWidget *parent)
        : q(parent)
    {
    }

    void setMountPointValid(bool valid)
    {
        if (mountPointValid == valid) return;

        mountPointValid = valid;

        q->setIsValid(valid);
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



DirectoryChooserWidget::DirectoryChooserWidget(
    DirectoryChooserWidget::Flags flags)
    : DialogDsl::DialogModule(false), d(new Private(this))
{
    d->ui.setupUi(this);
    d->flags = flags;

    connect(d->ui.editMountPoint, &KUrlRequester::textEdited,
            this, [&] () {
                d->setMountPointValid(d->isDirectoryValid(d->ui.editMountPoint->url()));
            });
}



DirectoryChooserWidget::~DirectoryChooserWidget()
{
}



PlasmaVault::Vault::Payload DirectoryChooserWidget::fields() const
{
    return {
        { KEY_MOUNT_POINT, d->ui.editMountPoint->url().toLocalFile() }
    };
}



void DirectoryChooserWidget::init(
    const PlasmaVault::Vault::Payload &payload)
{
    const auto mountPoint = payload[KEY_MOUNT_POINT].toString();

    d->ui.editMountPoint->setText(mountPoint);

    d->setMountPointValid(d->isDirectoryValid(d->ui.editMountPoint->url()));
}



