/*
 *   Copyright 2015 by Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLASMAVAULT_KDED_ENGINE_SINGLETON_P_H
#define PLASMAVAULT_KDED_ENGINE_SINGLETON_P_H

#include <mutex>
#include <memory>

namespace singleton {

template <typename BackendType>
static std::shared_ptr<BackendType> instance()
{
    static std::mutex s_instanceMutex;
    static std::weak_ptr<BackendType> s_instance;

    std::unique_lock<std::mutex> locker(s_instanceMutex);

    auto ptr = s_instance.lock();

    if (!ptr) {
        ptr = std::make_shared<BackendType>();
        s_instance = ptr;
    }

    return ptr;
}

} // namespace singleton

#endif // include guard

