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
#include "../cxxutils_p.h"

namespace AsynQt {
namespace detail {

template <typename ... _Results>
class CollectFutureInterface : public QObject
                            , public QFutureInterface<std::tuple<_Results...>> {
public:
    CollectFutureInterface(QFuture<_Results>... futures)
        : m_waitingCount(m_count)
        , m_futures(std::make_tuple(futures...))
    {
    }

    template <int index>
    bool connectFuture()
    {
        auto future = std::get<index>(m_futures);
        auto &watcher = std::get<index>(m_watchers);

        onFinished(watcher, [this] {
            m_waitingCount--;

            // We are saving the result
            std::get<index>(m_results) =
                std::get<index>(m_futures).result();

            // If all futures are done, report the result
            if (m_waitingCount == 0) {
                this->reportResult(m_results);
                this->reportFinished();
            }
        });

        onCanceled(watcher, [this] {
            // If any of the futures fail, we can not return
            // the result :(
            this->reportCanceled();
        });

        watcher.setFuture(future);

        return true;
    }

    template <int ...Indices>
    void connectFutures(index_sequence<Indices...>)
    {
        auto ignore = { connectFuture<Indices>()... };
        Q_UNUSED(ignore);
    }

    QFuture<std::tuple<_Results...>> start()
    {
        // we need to call connectFuture for index = 0 to sizeof..._Results
        connectFutures(make_index_sequence<m_count>());

        this->reportStarted();

        return this->future();
    }

private:
    static const constexpr std::size_t m_count = sizeof...(_Results);
    int m_waitingCount;
    std::tuple<QFuture<_Results>...> m_futures;
    std::tuple<QFutureWatcher<_Results>...> m_watchers;
    std::tuple<_Results...> m_results;
};

template <typename ..._Results>
QFuture<std::tuple<_Results...>> collect_impl(QFuture<_Results>... futures)
{
    return (new CollectFutureInterface<_Results...>(futures...))->start();
}

} // namespace detail
} // namespace AsynQt


