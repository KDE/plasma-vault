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

#include <type_traits>

#include "../utils_p.h"

namespace AsynQt {
namespace detail {

template <typename _Type, typename _Predicate>
class FilterFutureInterface
    : public QObject
    , public QFutureInterface<_Type> {

public:

    FilterFutureInterface(QFuture<_Type> future,
                          _Predicate predicate)
        : m_future(future)
        , m_predicate(predicate)
    {
    }

    QFuture<_Type> start()
    {
        m_futureWatcher.reset(new QFutureWatcher<_Type>());

        onFinished(m_futureWatcher, [this]() {
            this->reportFinished();
        });

        onCanceled(m_futureWatcher, [this]() { this->reportCanceled(); });

        onResultReadyAt(m_futureWatcher, [this](int index) {
            auto result = m_future.resultAt(index);
            if (m_predicate(result)) {
                this->reportResult(result);
            }
        });

        m_futureWatcher->setFuture(m_future);

        this->reportStarted();

        return this->future();
    }

private:
    QFuture<_Type> m_future;
    _Predicate m_predicate;
    std::unique_ptr<QFutureWatcher<_Type>> m_futureWatcher;
};

template <typename _Type, typename _Predicate>
QFuture<_Type>
filter_impl(const QFuture<_Type> &future, _Predicate &&predicate)
{
    return (new FilterFutureInterface<_Type, _Predicate>(
                future, std::forward<_Predicate>(predicate)))
        ->start();
}


namespace operators {


    template <typename _Predicate>
    class FilterModifier {
    public:
        FilterModifier(_Predicate predicate)
            : m_predicate(predicate)
        {
        }

        _Predicate m_predicate;
    };

    template <typename _Type, typename _Predicate>
    auto operator | (const QFuture<_Type> &future,
                     FilterModifier<_Predicate> &&modifier)
        -> decltype(filter_impl(future, modifier.m_predicate))
    {
        return filter_impl(future, modifier.m_predicate);
    }

} // namespace operators

} // namespace detail
} // namespace AsynQt

