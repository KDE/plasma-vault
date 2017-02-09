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

#include "encfscreatedialog.h"

#include "ui_encfscreatedialog.h"

#include <QItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPointer>
#include <QTimer>

#include <asynqt/basic/all.h>
#include <asynqt/operations/transform.h>

#include <KActivities/ActivitiesModel>
#include <KColorScheme>

#include "vault.h"
#include "commandresult.h"

using namespace PlasmaVault;

namespace Encfs {


class CreateDialog::Private {
public:
    Ui::EncfsCreateDialog ui;

    bool autoDevice = true;
    bool autoMountPoint = true;
    bool inhibitUpdates = false;

    QColor errorColor = KColorScheme(QPalette::Active, KColorScheme::View)
                            .background(KColorScheme::NegativeBackground)
                            .color();
};



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
        return QSize(100, 22);
    }
}; //^



CreateDialog::CreateDialog(QWidget *parent)
    : QDialog(parent)
    , d(new Private())
{
    d->ui.setupUi(this);

    // Message
    d->ui.message->setCloseButtonVisible(false);
    d->ui.message->hide();

    // Yes button should be called Create
    d->ui.buttons->button(QDialogButtonBox::Ok)->setText(i18n("Create"));

    // Password component is not as nice as it should be...
    d->ui.editPassword->setBackgroundWarningColor(d->errorColor);
    d->ui.editPassword->setAllowEmptyPasswords(false);
    connect(d->ui.editPassword, &KNewPasswordWidget::passwordStatusChanged,
            this, [&] {
                const auto status = d->ui.editPassword->passwordStatus();
                d->ui.buttons->button(QDialogButtonBox::Ok)->setEnabled(
                        status == KNewPasswordWidget::StrongPassword ||
                        status == KNewPasswordWidget::WeakPassword);
            });


    // activities list init
    d->ui.listActivities->setModel(new KActivities::ActivitiesModel(this));
    d->ui.listActivities->setItemDelegate(new CheckboxDelegate(this));


    //_ automatically generating device and mount points
    connect(d->ui.editTitle, &QLineEdit::textChanged,
            this, [&] (const QString &newText) {
                d->inhibitUpdates = true;

                QString name = newText;
                name.replace(QRegularExpression("[^a-zA-Z0-9]+"), "-");

                if (d->autoDevice) {
                    d->ui.editDevice->setUrl("~/.vaults/" + name + ".enc");
                }

                if (d->autoMountPoint) {
                    d->ui.editMountPoint->setUrl("~/Vaults/" + name);
                }

                d->inhibitUpdates = false;
            });

    connect(d->ui.editDevice, &KUrlRequester::textEdited,
            this, [&] (const QString &url) {
                if (!d->inhibitUpdates) {
                    d->autoDevice = false;
                }
            });

    connect(d->ui.editMountPoint, &KUrlRequester::textEdited,
            this, [&] (const QString &url) {
                if (!d->inhibitUpdates) {
                    d->autoMountPoint = false;
                }
            });
    //^
}



CreateDialog::~CreateDialog()
{
}



void CreateDialog::accept()
{
    using namespace AsynQt::operators;

    const auto name       = d->ui.editTitle->text();
    const auto device     = d->ui.editDevice->url().toLocalFile();
    const auto mountPoint = d->ui.editMountPoint->url().toLocalFile();
    const auto password   = d->ui.editPassword->password();

    const auto selection = d->ui.listActivities->selectionModel();
    QStringList selectedActivities;

    for (const auto &selectedIndex: selection->selectedIndexes()) {
        selectedActivities
            << selectedIndex.data(KActivities::ActivitiesModel::ActivityId)
                            .toString();
    }

    auto vault = new Vault(device, this);

    auto future = vault->create(name, mountPoint, { { KEY_PASSWORD, password } }, "encfs");

    auto result = AsynQt::await(future);

    if (result) {
        emit createdVault(vault);
        QDialog::accept();

    } else {
        d->ui.message->setText(result.error().message());
        d->ui.message->setMessageType(KMessageWidget::Error);
        d->ui.message->show();
        delete vault;
    }
}



void CreateDialog::reject()
{
    // Nothing to do
    QDialog::reject();
}


} // namespace Encfs


