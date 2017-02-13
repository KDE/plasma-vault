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

#ifndef ASYNQT_CXXUTILS_P_H
#define ASYNQT_CXXUTILS_P_H

namespace AsynQt {
namespace detail {

// Some things we need to wait for... c++17

template <int ...Indices>
struct index_sequence {};

template <int N, int ...Indices>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Indices...> {};

template <int ...Indices>
struct make_index_sequence<0, Indices...> : index_sequence<Indices...> {};


} // namespace detail
} // namespace AsynQt

#endif // ASYNQT_CXXUTILS_P_H

