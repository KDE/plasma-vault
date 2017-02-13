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

#ifndef ASYNQT_CONS_DBUSFUTURE_H
#define ASYNQT_CONS_DBUSFUTURE_H

#include <QDBusAbstractInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <QFuture>
#include <QFutureInterface>
#include <QObject>

#include <memory>

#include "../private/wrappers/dbus_p.h"

namespace AsynQt {

/**
 * Creates a future from the specified dbus reply
 */
template <typename _Result>
QFuture<_Result> makeFuture(QDBusPendingReply<_Result> dbusReply)
{
    using namespace detail;

    return (new DBusCallFutureInterface<_Result>(dbusReply))->start();
}

namespace DBus {

/**
 * Makes an asynchronous call to the specified DBus interface,
 * and wraps the result in a future.
 */
template <typename _Result>
QFuture<_Result>
asyncCall(QDBusAbstractInterface *interface, const QString &method,
          const QVariant &arg1 = QVariant(), const QVariant &arg2 = QVariant(),
          const QVariant &arg3 = QVariant(), const QVariant &arg4 = QVariant(),
          const QVariant &arg5 = QVariant(), const QVariant &arg6 = QVariant(),
          const QVariant &arg7 = QVariant(), const QVariant &arg8 = QVariant())
{
    using namespace detail;

    auto callFutureInterface = new DBusCallFutureInterface
        <_Result>(interface->asyncCall(method, arg1, arg2, arg3, arg4, arg5,
                                       arg6, arg7, arg8));

    return callFutureInterface->start();
}

} // namespace DBus

} // namespace AsynQt

#endif // ASYNQT_CONS_DBUSFUTURE_H

