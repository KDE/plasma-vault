/*
 *   Copyright (C) 2016 Ivan Cukic <ivan.cukic(at)kde.org>
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

//
// W A R N I N G
// -------------
//
// This file is not part of the AsynQt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace AsynQt {
namespace detail {

template <typename _Result>
QFuture<typename std::decay<_Result>::type>
makeReadyFuture(_Result &&value)
{
    QFutureInterface<_Result> interface;
    auto future = interface.future();

    interface.reportStarted();
    interface.reportResult(std::forward<_Result>(value));
    interface.reportFinished();

    return future;
}

inline
QFuture<void> makeReadyFuture()
{
    QFutureInterface<void> interface;
    auto future = interface.future();

    interface.reportStarted();
    interface.reportFinished();

    return future;
}

} // namespace detail
} // namespace AsynQt

