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

#ifndef ASYNQT_BASE_FLATTEN_H
#define ASYNQT_BASE_FLATTEN_H

#include <QFuture>
#include <QFutureWatcher>

#include <type_traits>
#include <memory>

#include "../private/operations/flatten_p.h"

namespace AsynQt {

/**
 * Takes a future of a future, and flattens it out.
 *
 * If any of the futures is canceled, the resulting future
 * will be canceled as well.
 *
 * @arg future future that contains another future of type T
 * @returns a single-level future of type T
 */
template <typename _Result>
QFuture<_Result> flatten(const QFuture<QFuture<_Result>> &future)
{
    return detail::flatten_impl(future);
}

namespace operators {

inline
detail::operators::FlattenModifier flatten()
{
    return detail::operators::FlattenModifier();
}

} // namespace operators

} // namespace AsynQt

#endif // ASYNQT_BASE_FLATTEN_H

