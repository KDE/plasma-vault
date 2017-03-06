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

#include "tombhelper.h"
#include "tombexec.h"

#include <QDebug>

KAUTH_HELPER_MAIN("org.kde.plasma.vault.tomb", TombHelper)

ActionReply TombHelper::lock(QVariantMap args)
{
    const auto tomb     = args["tomb"].toString();
    const auto device   = args["device"].toString();
    const auto keyFile  = args["keyFile"].toString();
    const auto password = args["password"].toString();

    qDebug() << "Helper: trying to lock";

    auto process =
        execTomb(tomb, { "lock"
                       , device + "/tomb"
                       , "-k"
                       , keyFile
                       , "--unsafe"
                       , "--tomb-pwd"
                       , password
                       }
                );

    process->start();
    process->waitForFinished();

    qDebug() << process->readAllStandardError();
    qDebug() << process->readAllStandardOutput();

    return ActionReply::SuccessReply();
}



ActionReply TombHelper::open(QVariantMap args)
{
    const auto tomb       = args["tomb"].toString();
    const auto device     = args["device"].toString();
    const auto mountPoint = args["mountPoint"].toString();
    const auto keyFile    = args["keyFile"].toString();
    const auto password   = args["password"].toString();

    qDebug() << "Helper: trying to open";

    auto process =
        execTomb(tomb, { "open"
                       , device + "/tomb"
                       , "-k"
                       , keyFile
                       , mountPoint
                       , "--unsafe"
                       , "--tomb-pwd"
                       , password
                       }
                );

    process->start();
    process->waitForFinished();

    qDebug() << process->readAllStandardError();
    qDebug() << process->readAllStandardOutput();

    return ActionReply::SuccessReply();
}



ActionReply TombHelper::close(QVariantMap args)
{
    const auto tomb = args["tomb"].toString();

    qDebug() << "Helper: trying to close";

    auto process = execTomb(tomb, { "close" });

    process->start();
    process->waitForFinished();

    qDebug() << process->readAllStandardError();
    qDebug() << process->readAllStandardOutput();

    return ActionReply::SuccessReply();
}

