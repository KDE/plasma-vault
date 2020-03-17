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

#ifndef PLASMAVAULT_KDED_ENGINE_COMMAND_RESULT_H
#define PLASMAVAULT_KDED_ENGINE_COMMAND_RESULT_H

#include <QFuture>

#include "asynqt/basic/all.h"
#include "asynqt/utils/expected.h"

namespace PlasmaVault {

class Error {
public:
    enum Code {
        MountPointError,
        DeviceError,
        BackendError,
        CommandError,
        DeletionError,
        UnknownError
    };

    Error(Code code = UnknownError, const QString &message = {}, const QString &out = {}, const QString &err = {});

    Code code() const;
    QString message() const;
    QString out() const;
    QString err() const;

private:
    Code m_code;
    QString m_message;
    QString m_out;
    QString m_err;
};



template <typename T = void>
using Result = AsynQt::Expected<T, Error>;

template <typename T = void>
using FutureResult = QFuture<Result<T>>;



inline
FutureResult<> errorResult(Error::Code error, const QString &message, const QString &out = {}, const QString &err = {})
{
    qWarning() << message;
    return makeReadyFuture(Result<>::error(error, message, out, err));
}


} // namespace PlasmaVault

#endif // include guard

