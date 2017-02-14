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

#ifndef ASYNQT_BASE_LISTEN_H
#define ASYNQT_BASE_LISTEN_H

#include <QFuture>
#include <QFutureWatcher>

#include "../private/operations/listen_p.h"

namespace AsynQt {

/**
 *
 */
template <typename _Result, typename _Function>
QFuture<_Result> onFinished(const QFuture<_Result> &future,
                           _Function &&function)
{
    return detail::onFinished_impl(future, std::forward<_Function>(function));
}

/**
 *
 */
template <typename _Result, typename _Function>
QFuture<_Result> onSuccess(const QFuture<_Result> &future,
                           _Function &&function)
{
    return onFinished(future, detail::PassResult<_Function>(function));
}

/**
 *
 */
template <typename _Result, typename _Function>
QFuture<_Result> onError(const QFuture<_Result> &future,
                         _Function &&function)
{
    return onFinished(future, detail::PassError<_Function>(function));
}

namespace operators {

template <typename _Function>
inline
detail::operators::OnFinishedModifier<_Function> onFinished(_Function &&function)
{
    return detail::operators::OnFinishedModifier<_Function>(std::forward<_Function>(function));
}

template <typename _Function>
inline
detail::operators::OnFinishedModifier<detail::PassResult<_Function>> onSuccess(_Function &&function)
{
    return detail::operators::OnFinishedModifier<detail::PassResult<_Function>>(detail::PassResult<_Function>(function));
}

template <typename _Function>
inline
detail::operators::OnFinishedModifier<detail::PassError<_Function>> onError(_Function &&function)
{
    return detail::operators::OnFinishedModifier<detail::PassError<_Function>>(detail::PassError<_Function>(function));
}

} // namespace operators

} // namespace AsynQt

#endif // include guard

