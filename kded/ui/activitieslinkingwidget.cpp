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

#include "activitieslinkingwidget.h"

#include "ui_activitieslinkingwidget.h"

#include <QItemDelegate>

#include <KActivities/ActivitiesModel>

class ActivitiesLinkingWidget::Private {
public:
    Ui::ActivitiesLinkingWidget ui;
};


namespace {
    class CheckboxDelegate: public QItemDelegate { //_
    public:
        CheckboxDelegate(QObject *parent)
            : QItemDelegate(parent)
        {
        }

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
        {
            auto wholeRect = option.rect;

            // Drawing the checkbox
            auto checkRect = wholeRect;
            checkRect.setWidth(checkRect.height());
            drawCheck(painter, option, checkRect,
                    option.state & QStyle::State_Selected ?
                        Qt::Checked : Qt::Unchecked);

            // Drawing the text
            auto textRect = wholeRect;
            textRect.setLeft(textRect.left() + 8 + textRect.height());
            drawDisplay(painter, option, textRect,
                        index.data(Qt::DisplayRole).toString());
        }

        QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override
        {
            Q_UNUSED(option);
            Q_UNUSED(index);
            return QSize(100, 22);
        }
    }; //^
}



ActivitiesLinkingWidget::ActivitiesLinkingWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);

    d->ui.listActivities->setModel(new KActivities::ActivitiesModel(this));
    d->ui.listActivities->setItemDelegate(new CheckboxDelegate(this));
}



ActivitiesLinkingWidget::~ActivitiesLinkingWidget()
{
}



PlasmaVault::Vault::Payload ActivitiesLinkingWidget::fields() const
{

    const auto selection = d->ui.listActivities->selectionModel();
    QStringList selectedActivities;
    for (const auto &selectedIndex: selection->selectedIndexes()) {
        selectedActivities
            << selectedIndex.data(KActivities::ActivitiesModel::ActivityId)
                            .toString();
    }

    return {
        { KEY_ACTIVITIES, selectedActivities }
    };
}



