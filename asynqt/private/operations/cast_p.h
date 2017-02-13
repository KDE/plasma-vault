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

template <typename _Out, typename _In>
QFuture<_Out> qfuture_cast_impl(const QFuture<_In> &future)
{
    return transform(future, [] (const _In &value) -> _Out {
            return static_cast<_Out>(value);
        });
}

namespace operators {

    template <typename _Out>
    struct CastModifier {
    };

    template <typename _In, typename _Out>
    auto operator | (const QFuture<_In> &future,
                     CastModifier<_Out> &&)
        -> decltype(qfuture_cast_impl<_Out>(future))
    {
        return qfuture_cast_impl<_Out>(future);
    }

} // namespace operators

}
}

