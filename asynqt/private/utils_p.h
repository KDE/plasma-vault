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

#ifndef ASYNQT_PRIVATE_UTILS_P_H
#define ASYNQT_PRIVATE_UTILS_P_H

#include <memory>
#include <QFuture>
#include <QFutureWatcher>

namespace AsynQt {
namespace detail {

#define IMPLEMENT_FUTURE_WATCHER_HANDLER(SignalName, EventName)                \
    template <typename T, typename Function>                                   \
    inline void EventName(const std::unique_ptr<QFutureWatcher<T>> &watcher,   \
                          Function function)                                   \
    {                                                                          \
        QObject::connect(watcher.get(), &QFutureWatcherBase::SignalName,       \
                         watcher.get(), function, Qt::QueuedConnection);       \
    }                                                                          \
                                                                               \
    template <typename T, typename Function>                                   \
    inline void EventName(const QFutureWatcher<T> &watcher,                    \
                          Function function)                                   \
    {                                                                          \
        QObject::connect(&watcher, &QFutureWatcherBase::SignalName,            \
                         &watcher, function, Qt::QueuedConnection);            \
    }

IMPLEMENT_FUTURE_WATCHER_HANDLER(finished, onFinished)
IMPLEMENT_FUTURE_WATCHER_HANDLER(canceled, onCanceled)
IMPLEMENT_FUTURE_WATCHER_HANDLER(resultReadyAt, onResultReadyAt)

template <typename Function>
inline void
onResultReadyAt(const std::unique_ptr<QFutureWatcher<void>> &watcher,
                Function function)
{
    Q_UNUSED(watcher)
    Q_UNUSED(function)
}

#undef IMPLEMENT_FUTURE_WATCHER_HANDLER

} // namespace detail
} // namespace AsynQt

#endif // ASYNQT_PRIVATE_UTILS_P_H

