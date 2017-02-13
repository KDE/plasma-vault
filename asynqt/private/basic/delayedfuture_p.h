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

namespace AsynQt {
namespace detail {

template <typename _Result>
class DelayedFutureInterface
    : public QObject
    , public QFutureInterface<_Result> {

public:
   DelayedFutureInterface(_Result value, int milliseconds)
        : m_value(value)
        , m_milliseconds(milliseconds)
    {
    }

    QFuture<_Result> start()
    {
        auto future = this->future();

        this->reportStarted();

        QTimer::singleShot(m_milliseconds, [this] {
            this->reportResult(m_value);
            this->reportFinished();
            deleteLater();
        });

        return future;
    }

private:
    _Result m_value;
    int m_milliseconds;

};

template <typename T = void>
class DelayedVoidFutureInterface
    : public QObject
    , QFutureInterface<void> {

public:
    DelayedVoidFutureInterface(int milliseconds)
        : m_milliseconds(milliseconds)
    {
    }

    QFuture<void> start()
    {
        auto future = this->future();

        this->reportStarted();

        QTimer::singleShot(m_milliseconds, [this] {
            this->reportFinished();
            deleteLater();
        });

        deleteLater();

        return future;
    }

private:
    int m_milliseconds;
};

template <typename _Result>
DelayedFutureInterface<typename std::decay<_Result>::type> *
newDelayedFutureInterface(_Result &&result, int milliseconds)
{
    return new DelayedFutureInterface<typename std::decay<_Result>::type>(
        std::forward<_Result>(result), milliseconds);
}

} // namespace detail
} // namespace AsynQt

