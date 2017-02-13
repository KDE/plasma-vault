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

#ifndef ASYNQT_EITHER_H
#define ASYNQT_EITHER_H

#include "expected.h"

namespace AsynQt {

template<typename T1, typename T2>
class Either: private Expected<T1, T2> {
public:
    template <typename ...ConsParams>
    static Either left(ConsParams && ...params)
    {
        return Expected<T1, T2>::success(std::forward<ConsParams>(params)...);
    }

    template <typename ...ConsParams>
    static Either right(ConsParams && ...params)
    {
        return Expected<T1, T2>::error(std::forward<ConsParams>(params)...);
    }

    Either(T1 value)
    {
        auto tmp = Either::left(value);
        this->swap(tmp);
    }

    Either(T2 value)
    {
        auto tmp = Either::right(value);
        this->swap(tmp);
    }

    T1& left()
    {
        return Expected<T1, T2>::get();
    }

    T1 left() const
    {
        return Expected<T1, T2>::get();
    }

    T2& right()
    {
        return Expected<T1, T2>::error();
    }

    T2 right() const
    {
        return Expected<T1, T2>::error();
    }

    Either(const Either &other) = default;
    Either(Either &&other) = default;

    Either &operator= (const Either &other) = default;
    Either &operator= (Either &&other) = default;
};

} // namespace AsynQt

#endif // ASYNQT_EITHER_H

