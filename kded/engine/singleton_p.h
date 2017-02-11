/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMAVAULT_KDED_ENGINE_SINGLETON_H
#define PLASMAVAULT_KDED_ENGINE_SINGLETON_H

#include <mutex>
#include <memory>

namespace util {
namespace singleton {

template <typename BackendType>
static std::shared_ptr<BackendType> instance()
{
    static std::mutex s_instanceMutex;
    static std::weak_ptr<BackendType> s_instance;

    std::unique_lock<std::mutex> locker;

    auto ptr = s_instance.lock();

    if (!ptr) {
        ptr = std::make_shared<BackendType>();
        s_instance = ptr;
    }

    return ptr;
}

} // namespace singleton
} // namespace util

#endif // include guard

