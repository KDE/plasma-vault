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

#ifndef PLASMAVAULT_TOMBEXEC_H
#define PLASMAVAULT_TOMBEXEC_H

#include <QProcess>

inline
QProcess * execTomb(const QString &tombPath, const QStringList &args)
{
    auto zsh = new QProcess();
    zsh->setProgram("zsh");

    // Tomb is a ZSH script that wants us to pass it the password
    // as a command-line argument which is insane as it will be
    // shown in the process table.
    //
    // We are instead running our own zsh instance, and passing
    // arguments via $0, etc. This should be able to trick Tomb
    // to do what is right without the password ending up in the
    // process table.

    QObject::connect(zsh, &QProcess::started,
            [zsh,args,tombPath] {
                zsh->write("0=tomb\n");
                for (int i = 0; i < args.count(); ++i) {
                    // TODO: escape the ' character
                    zsh->write(QByteArray::number(i + 1) + "='" + args[i].toUtf8() + "'\n");
                }
                zsh->write(". " + tombPath.toUtf8() + "\n");
                zsh->closeWriteChannel();
            });

    return zsh;
}


#endif // include guard

