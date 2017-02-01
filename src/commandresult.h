/*
 *   Copyother (C) 2017 by Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef PLASMAVAULT_COMMAND_RESULT_H
#define PLASMAVAULT_COMMAND_RESULT_H

#include <QFuture>

#include "asynqt/basic/all.h"
#include "asynqt/utils/expected.h"

#include "plasmavault_export.h"

namespace PlasmaVault {

class PLASMAVAULT_EXPORT Error {
public:
    enum Code {
        MountPointError,
        DeviceError,
        BackendError,
        CommandError
    };

    Error(Code code, const QString &message = QString());

    Code code() const;
    QString message() const;

private:
    Code m_code;
    QString m_message;
};

template <typename T = void>
using Result = AsynQt::Expected<T, Error>;

template <typename T = void>
using FutureResult = QFuture<Result<T>>;


inline
FutureResult<> errorResult(Error::Code error, const QString &message)
{
    qWarning() << message;
    return makeReadyFuture(Result<>::error(error, message));
}


} // namespace PlasmaVault

#endif // PLASMAVAULT_COMMAND_RESULT_H

