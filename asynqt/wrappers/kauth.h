/*
 *   Copyright (C) 2017 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef ASYNQT_CONS_KAUTH_H
#define ASYNQT_CONS_KAUTH_H

#include <kauth.h>
#include <QFuture>
#include <QFutureInterface>

#include "../private/wrappers/kauth_p.h"

namespace AsynQt {

template <typename _Function>
auto makeFuture(KAuth::ExecuteJob *job, _Function map)
        -> QFuture<decltype(map(0, Q_NULLPTR))>
{
    using namespace detail;

    auto futureInterface =
        new KAuthJobFutureInterface<decltype(map(0, Q_NULLPTR)), _Function>
            (job, map);

    return futureInterface->start();
}

inline
auto makeFuture(KAuth::ExecuteJob *job)
        -> QFuture<KAuth::ExecuteJob*>
{
    using namespace detail;

    auto id = [] (bool success, ::KAuth::ExecuteJob *job) {
        Q_UNUSED(success);
        return job;
    };

    auto futureInterface =
        new KAuthJobFutureInterface<KAuth::ExecuteJob*, decltype(id)>
            (job, id);

    return futureInterface->start();
}

namespace KAuth {

    /**
     * Executes the specified command, with the specified arguments.
     */
    template <typename _Function>
    auto exec(const QString &id, const QString &helper,
              const QVariantMap &arguments,
              _Function &&map)
        -> QFuture<decltype(map(0, Q_NULLPTR))>
    {
        ::KAuth::Action action(id);
        action.setHelperId(helper);
        action.setArguments(arguments);

        return AsynQt::makeFuture(action.execute(), std::forward<_Function>(map));
    }

    /**
     * Executes the specified command, with the specified arguments,
     * and returns the future containing the process it created.
     */
    inline
    QFuture<::KAuth::ExecuteJob*> exec(const QString &id, const QString &helper,
                                       const QVariantMap &arguments)
    {
        return exec(id, helper, arguments,
                    [] (bool success, ::KAuth::ExecuteJob *job) {
                        Q_UNUSED(success);
                        return job;
                    });
    }

} // namespace KAuth

} // namespace qfuture

#endif // ASYNQT_CONS_KAUTH_H

