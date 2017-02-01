/*
 *   Copyright (C) 2017 by Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef PLASMAVAULT_BACKEND_P_H
#define PLASMAVAULT_BACKEND_P_H

#include <QList>
#include <QString>
#include <QStringList>

#include <memory>
#include <mutex>

#include "common.h"
#include "commandresult.h"

namespace PlasmaVault {

class Backend {
public:
    typedef std::shared_ptr<Backend> Ptr;

    Backend();
    virtual ~Backend();

    virtual bool isInitialized(const Device &device) const = 0;
    virtual bool isOpened(const MountPoint &mountPoint) const = 0;

    virtual FutureResult<> initialize(const QString &name,
                                      const Device &device,
                                      const MountPoint &mountPoint,
                                      const QString &password) = 0;

    virtual FutureResult<> open(const Device &device,
                                const MountPoint &mountPoint,
                                const QString &password) = 0;

    virtual FutureResult<> close(const Device &device,
                                 const MountPoint &mountPoint) = 0;

    virtual FutureResult<> destroy(const Device &device,
                                   const MountPoint &mountPoint,
                                   const QString &password) = 0;

    virtual QString name() const = 0;

    static QStringList availableBackends();

    static Ptr instance(const QString &backend);

private:

};

} // namespace PlasmaVault

#endif // PLASMAVAULT_BACKEND_P_H

