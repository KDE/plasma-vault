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


template <typename _Function>
class PassResult {
public:
    PassResult(_Function function)
        : m_function(function)
    {
    }

    template <typename _Result>
    void operator() (const QFuture<_Result> &future) const
    {
        qDebug() << "Passing the result";
        if (future.resultCount()) {
            qDebug() << "Passing the result, really";
            m_function(future.result());
        }
    }

private:
    _Function m_function;
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
        qDebug() << "Passing the error";
        if (future.isCanceled()) {
            qDebug() << "Passing the error, really";
            m_function();
        }
    }

private:
    _Function m_function;
};



template <typename _Type, typename _Function>
class OnFinishedFutureInterface
    : public QObject
    , public QFutureInterface<_Type> {

public:
    OnFinishedFutureInterface(QFuture<_Type> future,
                          _Function function)
        : m_future(future)
        , m_function(function)
    {
        qDebug() << "Created the future interface";
    }

    ~OnFinishedFutureInterface()
    {
        qDebug() << "Killed the future interface";
    }

    QFuture<_Type> start()
    {
        qDebug() << "Starting the future interface";
        m_futureWatcher.reset(new QFutureWatcher<_Type>());

        onFinished(m_futureWatcher, [this]() {
            qDebug() << "Reporting that the future interface has finished";
            m_function(this->future());
            this->reportFinished();
        });

        onCanceled(m_futureWatcher, [this]() { this->reportCanceled(); });

        onResultReadyAt(m_futureWatcher, [this](int index) {
            auto result = m_future.resultAt(index);
            this->reportResult(result);
        });

        m_futureWatcher->setFuture(m_future);

        this->reportStarted();

        return this->future();
    }

private:
    QFuture<_Type> m_future;
    _Function m_function;
    std::unique_ptr<QFutureWatcher<_Type>> m_futureWatcher;
};

template <typename _Type, typename _Function>
QFuture<_Type>
onFinished_impl(const QFuture<_Type> &future, _Function &&function)
{
    return (new OnFinishedFutureInterface<_Type, _Function>(
                future, std::forward<_Function>(function)))
        ->start();
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

