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

#ifndef ASYNQT_BASE_CONTINUEWITH_H
#define ASYNQT_BASE_CONTINUEWITH_H

#include <QFuture>
#include <QFutureWatcher>

#include <type_traits>
#include <memory>

#include "flatten.h"
#include "transform.h"

namespace AsynQt {

/**
 * This method is similar to `transform`,
 * but it takes a transformation function (in this
 * case, called a continuation) that does not
 * return a normal value, but also a new future.
 * It returns the future that will be returned
 * by the continuation.
 *
 * It is equivalent to calling `flatten` on the result
 * of the `transform` function when a continuation
 * is passed to it.
 *
 * Example:
 *
 * <code>
 *     QFuture<QString> input = getUserInput();
 *     QFuture<Status> result = continueWith(input, [] (QString message) {
 *          return server.send(message);
 *     });
 * </code>
 *
 * @arg future the future to connect the continuation to
 * @arg continuation the continuation function
 * @returns the future that the continuation will return
 */
template <typename _In, typename _Continuation>
auto continueWith(const QFuture<_In> &future, _Continuation &&continuation)
    -> decltype(flatten(transform(future, std::forward<_Continuation>(continuation))))
{
    return flatten(transform(future, std::forward<_Continuation>(continuation)));
}

namespace operators {

    template <typename _In, typename _Continuation>
    auto operator | (const QFuture<_In> &future, _Continuation &&continuation)
        -> decltype(continueWith(future, continuation))
    {
        return continueWith(future, continuation);
    }

} // namespace operators

} // namespace AsynQt

#endif // ASYNQT_BASE_CONTINUEWITH_H

