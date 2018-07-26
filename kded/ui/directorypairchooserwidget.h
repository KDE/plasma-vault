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

#ifndef PLASMAVAULT_KDED_UI_DIRECTORY_PAIR_CHOOSER_WIDGET_H
#define PLASMAVAULT_KDED_UI_DIRECTORY_PAIR_CHOOSER_WIDGET_H

#include "dialogdsl.h"


class DirectoryPairChooserWidget: public DialogDsl::DialogModule {
    Q_OBJECT

public:
    enum Flag {
        ShowDevicePicker = 1,
        ShowMountPointPicker = 2,
        RequireEmptyDevice = 4,
        RequireExistingDevice = 8,
        RequireEmptyMountPoint = 16,
        RequireExistingMountPoint = 32,

        AutoFillPaths = 64
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    DirectoryPairChooserWidget(Flags flags);
    ~DirectoryPairChooserWidget();

    PlasmaVault::Vault::Payload fields() const override;
    void init(const PlasmaVault::Vault::Payload &payload) override;

private:
    class Private;
    QScopedPointer<Private> d;
};

inline DialogDsl::ModuleFactory directoryPairChooser(DirectoryPairChooserWidget::Flags flags)
{
    return [=] {
        return new DirectoryPairChooserWidget(flags);
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(DirectoryPairChooserWidget::Flags)

#endif // include guard

