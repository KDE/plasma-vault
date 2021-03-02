/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
template <typename _Result = void>
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

