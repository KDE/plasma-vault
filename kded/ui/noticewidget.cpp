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

#include "noticewidget.h"

#include "ui_noticewidget.h"

#include <KSharedConfig>
#include <KConfigGroup>

class NoticeWidget::Private {
public:
    Ui::NoticeWidget ui;
    KSharedConfig::Ptr config;
    bool shouldBeShown;
    QString noticeId;
};



NoticeWidget::NoticeWidget(const QString &noticeId, const QString &message,
                           Mode mode)
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);
    d->ui.textNotice->setHtml(message);
    d->ui.checkShouldBeHidden->setVisible(mode == DoNotShowAgainOption);

    d->noticeId = noticeId;

    d->config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
}



NoticeWidget::~NoticeWidget()
{
}



PlasmaVault::Vault::Payload NoticeWidget::fields() const
{
    return {};
}



void NoticeWidget::aboutToBeShown()
{
    KConfigGroup noticeUi(d->config, "UI-notice");
    d->shouldBeShown = !noticeUi.readEntry("SkipNotice-" + d->noticeId, false);
    d->ui.checkShouldBeHidden->setChecked(!d->shouldBeShown);
}



bool NoticeWidget::shouldBeShown() const
{
    return d->shouldBeShown;
}



void NoticeWidget::aboutToBeHidden()
{
    KConfigGroup noticeUi(d->config, "UI-notice");
    noticeUi.writeEntry("SkipNotice-" + d->noticeId,
                        d->ui.checkShouldBeHidden->isChecked());
    d->config->sync();
}



