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

//
// W A R N I N G
// -------------
//
// This file is not part of the AsynQt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "../utils_p.h"

namespace AsynQt {
namespace detail {

template <typename T>
struct isNullary {
private:
    template <typename U>
    static decltype(std::declval<U>()(), std::true_type()) test(int);

    template <typename>
    static std::false_type test(...);

public:
    typedef decltype(test<T>(0)) type;
    enum { value = type::value };
};


template <typename _Function>
class PassResult {
private:
    using isNullaryFunction = typename isNullary<_Function>::type;
    _Function m_function;

    template <typename _Result>
    void callFunction(const QFuture<_Result> &future, std::true_type) const
    {
        Q_UNUSED(future);
        if (!future.isCanceled()) {
            m_function();
        }
    }

    template <typename _Result>
    void callFunction(const QFuture<_Result> &future, std::false_type) const
    {
        if (future.resultCount()) {
            m_function(future.result());
        }
    }

public:
    PassResult(_Function function)
        : m_function(function)
    {
    }

    template <typename _Result>
    void operator() (const QFuture<_Result> &future) const
    {
        callFunction(future, isNullaryFunction());
    }
};

template <typename _Function>
class PassError {
public:
    PassError(_Function function)
        : m_function(function)
    {
    }

    template <typename _Result>
    void operator() (const QFuture<_Result> &future) const
    {
        if (future.isCanceled()) {
            m_function();
        }
    }

private:
    _Function m_function;
};


template <typename _Type, typename _Function>
QFuture<_Type>
onFinished_impl(const QFuture<_Type> &future, _Function &&function)
{
    auto watcher = new QFutureWatcher<_Type>();
    QObject::connect(watcher, &QFutureWatcherBase::finished,
                     [watcher,function] () {
                         function(watcher->future());
                         watcher->deleteLater();
                     });
    watcher->setFuture(future);

    return future;
}


namespace operators {

    template <typename _Function>
    class OnFinishedModifier {
    public:
        OnFinishedModifier(_Function function)
            : m_function(function)
        {
        }

        _Function m_function;
    };

    template <typename _Type, typename _Function>
    auto operator | (const QFuture<_Type> &future,
                     OnFinishedModifier<_Function> &&modifier)
        -> decltype(onFinished_impl(future, modifier.m_function))
    {
        return onFinished_impl(future, modifier.m_function);
    }

} // namespace operators





} // namespace detail
} // namespace AsynQt

