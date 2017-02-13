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

#ifndef ASYNQT_CONS_CANCELED_FUTURE_H
#define ASYNQT_CONS_CANCELED_FUTURE_H

#include <QFuture>
#include <QFutureInterface>
#include <QObject>
#include <QTimer>

#ifndef QT_NO_EXCEPTIONS
#include <QException>
#endif

#include "../private/basic/canceledfuture_p.h"

namespace AsynQt {

/**
 * Creates a canceled future.
 */
template <typename _Result = void>
QFuture<_Result> makeCanceledFuture()
{
    // No need to decay the type, expecting the user not to try
    // and make a future of ref-to-type or anything that funny
    return detail::makeCanceledFuture<_Result>();
}

#ifndef QT_NO_EXCEPTIONS
/**
 * Creates a canceled future.
 */
template <typename _Result = void>
QFuture<_Result> makeCanceledFuture(const QException &exception)
{
    // No need to decay the type, expecting the user not to try
    // and make a future of ref-to-type or anything that funny
    return detail::makeCanceledFuture<_Result>(exception);
}

#ifdef ENABLE_EVIL_QFUTURE_HACKS_THAT_SHOULD_BE_IN_QT

namespace evil {

// TODO: Remove these

template <typename T>
bool hasException(const QFuture<T> &future)
{
    return
        reinterpret_cast<const QFuture<AsynQt_QFuturePrivacyHack_hasException>*>(
               &future)
        ->isCanceled();
}

template <typename T>
void throwPossibleException(QFuture<T> &future)
{
    reinterpret_cast<QFuture<AsynQt_QFuturePrivacyHack_throwPossibleException>*>(
            &future)
        ->cancel();
}

} // namespace evil

#endif


#else
#warning "Exceptions are disabled. If you enable them, you'll open a whole new world"
#endif

} // namespace AsynQt

#endif // ASYNQT_CONS_CANCELED_FUTURE_H

