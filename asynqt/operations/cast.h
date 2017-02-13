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

#ifndef ASYNQT_BASE_CAST_H
#define ASYNQT_BASE_CAST_H

#include <QFuture>
#include <QFutureWatcher>

#include <type_traits>
#include <memory>

#include "transform.h"
#include "../private/operations/cast_p.h"

namespace AsynQt {

/**
 * Casts the future result into the specified type.
 *
 * <code>
 *     auto future = AsynQt::Process::getOutput("echo", { "Hello KDE" });
 *     auto castFuture = AsynQt::qfuture_cast<QString>(future);
 * </code>
 */
template <typename _Out, typename _In>
QFuture<_Out> qfuture_cast(const QFuture<_In> &future)
{
    return detail::qfuture_cast_impl<_Out>(future);
}

namespace operators {

template <typename _Out>
detail::operators::CastModifier<_Out> cast()
{
    return detail::operators::CastModifier<_Out>();
}

} // namespace operator

} // namespace AsynQt

#endif // ASYNQT_BASE_CAST_H

