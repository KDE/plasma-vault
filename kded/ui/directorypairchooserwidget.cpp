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

#include "directorypairchooserwidget.h"
#include "ui_directorypairchooserwidget.h"

#include "vault.h"

#include <QStandardPaths>

class DirectoryPairChooserWidget::Private {
public:
    Ui::DirectoryPairChooserWidget ui;
    const DirectoryPairChooserWidget::Flags flags;

    bool mountPointValid = false;
    bool encryptedLocationValid = false;

    DirectoryPairChooserWidget *const q;
    Private(DirectoryPairChooserWidget *parent, DirectoryPairChooserWidget::Flags flags)
        : flags(flags), q(parent)
    {
        if (flags & DirectoryPairChooserWidget::SkipDevicePicker)
            encryptedLocationValid = true;
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
        // TODO check the vaults database to see if another vault already uses this dir

        QDir directory(url.toString());

        // TODO: Support alternative flags once they are needed
        if (!directory.exists() || directory.entryList().isEmpty()) return true;

        return false;
    }
};

DirectoryPairChooserWidget::DirectoryPairChooserWidget(
    DirectoryPairChooserWidget::Flags flags)
    : DialogDsl::DialogModule(false), d(new Private(this, flags))
{
    d->ui.setupUi(this);
    if (flags & DirectoryPairChooserWidget::SkipDevicePicker) {
        d->ui.editDevice->setVisible(false);
        d->ui.labelDevice->setVisible(false);
    } else {
        connect(d->ui.editDevice, &KUrlRequester::textEdited,
                this, [&] () {
                d->setEncryptedLocationValid(d->isDirectoryValid(d->ui.editDevice->url()));
                });
    }

    connect(d->ui.editMountPoint, &KUrlRequester::textEdited,
            this, [&] () {
                d->setMountPointValid(d->isDirectoryValid(d->ui.editMountPoint->url()));
            });
}


DirectoryPairChooserWidget::~DirectoryPairChooserWidget()
{
}


PlasmaVault::Vault::Payload DirectoryPairChooserWidget::fields() const
{
    return {
        { KEY_DEVICE,      d->ui.editDevice->url().toLocalFile() },
        { KEY_MOUNT_POINT, d->ui.editMountPoint->url().toLocalFile() }
    };
}



void DirectoryPairChooserWidget::init(
    const PlasmaVault::Vault::Payload &payload)
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            + QStringLiteral("/plasma-vault");

    const auto name = payload[KEY_NAME].toString();

    Q_ASSERT(!name.isEmpty());

    QString path = QString("%1/%2.enc").arg(basePath).arg(name);
    int index = 1;
    while (QDir(path).exists()) {
        path = QString("%1/%2_%3.enc").arg(basePath).arg(name).arg(index++);
    }

    d->ui.editDevice->setText(path);
    d->ui.editMountPoint->setText(QDir::homePath() + QStringLiteral("/Vaults/") + name);

    d->setEncryptedLocationValid(d->isDirectoryValid(d->ui.editDevice->url()));
    d->setMountPointValid(d->isDirectoryValid(d->ui.editMountPoint->url()));
}
