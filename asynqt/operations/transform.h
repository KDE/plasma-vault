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

#ifndef ASYNQT_BASE_TRANSFORM_H
#define ASYNQT_BASE_TRANSFORM_H

#include <QFuture>
#include <QFutureWatcher>

#include <type_traits>
#include <memory>

#include "../private/operations/transform_p.h"

namespace AsynQt {

/**
 * This method applies the specified transformation function to
 * the value stored in the given future. Since the value might not
 * yet be present, it returns a future that will contain the
 * transformed value as soon as the original future is finished.
 *
 * If the original future is canceled, the transformation function
 * will not be invoked, and the resulting future will also be canceled.
 *
 * Example:
 *
 * <code>
 *     QFuture<int> answer = meaningOfLife()
 *     // answer will eventually contain 42
 *
 *     QFuture<QString> text = transform(answer, toText)
 *     // text will eventually contain the result of toText(42)
 * </code>
 *
 * @arg future the future to transform
 * @arg transformation unary function to apply to the value in the future
 * @returns a future that will contain the transformed value
 */
template <typename _In, typename _Transformation>
QFuture<
    typename detail::TransformFutureInterface<_In, _Transformation>::result_type
    >
transform(const QFuture<_In> &future, _Transformation &&transormation)
{
    using namespace detail;
    return transform_impl(future, std::forward<_Transformation>(transormation));
}

namespace operators {

template <typename _Transformation>
detail::operators::TransformationModifier<_Transformation>
transform(_Transformation &&transormation)
{
    return detail::operators::TransformationModifier<_Transformation>(
        std::forward<_Transformation>(transormation));
}

} // namespace operators

} // namespace AsynQt

#endif // ASYNQT_BASE_TRANSFORM_H

