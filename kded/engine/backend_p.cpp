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

#include "backend_p.h"

#include <QDir>

#include "backends/encfs/encfsbackend.h"
#include "backends/cryfs/cryfsbackend.h"

#include <KLocalizedString>

namespace PlasmaVault {

Backend::Backend()
{
}



Backend::~Backend()
{
}



QStringList Backend::availableBackends()
{
    return { "encfs", "cryfs" };
}



Backend::Ptr Backend::instance(const QString &backend)
{
    return
        backend == "encfs" ? PlasmaVault::EncFsBackend::instance() :
        backend == "cryfs" ? PlasmaVault::CryFsBackend::instance() :
        /* unknown backend */ nullptr;
}



bool Backend::isDirectoryEmpty(const QString &path)
{
    QDir dir(path);

    if (!dir.exists()) return true;

    return dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0;
}



QString Backend::formatMessageLine(
        const QString &command,
        const QPair<bool, QString> &result) const
{
    const auto valid = result.first;
    const auto message = result.second;

    QString htmlMessage =
        (valid ? QString() : "<b>") +
        message +
        (valid ? QString() : "</b>") +
        "<br />\n"
        ;

    return i18nc("formatting the message for a command, as in encfs: not found",
                 "%1: %2",
                 command,
                 htmlMessage);
}


} // namespace PlasmaVault

