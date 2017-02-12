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

#include "cryfscypherchooserwidget.h"

#include "ui_cryfscypherchooserwidget.h"

#include <QStringList>
#include <QProcess>
#include <QTimer>

#include <asynqt/basic/all.h>
#include <asynqt/wrappers/process.h>
#include <asynqt/operations/transform.h>

class CryfsCypherChooserWidget::Private {
public:
    Ui::CryfsCypherChooserWidget ui;
};



CryfsCypherChooserWidget::CryfsCypherChooserWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);

    QTimer::singleShot(0, this, &CryfsCypherChooserWidget::initializeCyphers);
}

void CryfsCypherChooserWidget::initializeCyphers()
{
    using namespace AsynQt;
    using namespace AsynQt::operators;

    // TODO: This needs to be prettier -- for modules to be able
    // to reach their backends directly
    auto process = new QProcess();
    process->setProgram("cryfs");
    process->setArguments({ "--show-ciphers" });

    auto env = process->processEnvironment();
    env.insert("CRYFS_FRONTEND", "noninteractive");
    process->setProcessEnvironment(env);

    auto combo = d->ui.comboCypher;

    process->start();

    while (!process->waitForFinished(10)) {
        QCoreApplication::processEvents();
    }

    const auto err = process->readAllStandardError();

    combo->addItem(i18n("Use the default cipher"), QString());

    for (const auto& item: QString::fromLatin1(err).split('\n')) {
        if (!item.isEmpty()) {
            combo->addItem(item, item);
        }
    }
}



CryfsCypherChooserWidget::~CryfsCypherChooserWidget()
{
}



PlasmaVault::Vault::Payload CryfsCypherChooserWidget::fields() const
{
    return {
        { "cryfs-cipher", d->ui.comboCypher->currentData() }
    };
}



