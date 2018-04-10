/*
 *   Copyright 2017 by Ivan Cukic <ivan.cukic (at) kde.org>
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

#ifndef PLASMAVAULT_KDED_ENGINE_BACKEND_P_H
#define PLASMAVAULT_KDED_ENGINE_BACKEND_P_H

#include <QList>
#include <QString>
#include <QStringList>

#include <memory>

#include "vault.h"
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
                                      const Vault::Payload &payload) = 0;

    virtual FutureResult<> open(const Device &device,
                                const MountPoint &mountPoint,
                                const Vault::Payload &payload) = 0;

    virtual FutureResult<> close(const Device &device,
                                 const MountPoint &mountPoint) = 0;

    virtual FutureResult<> dismantle(const Device &device,
                                     const MountPoint &mountPoint,
                                     const Vault::Payload &payload) = 0;

    virtual FutureResult<> validateBackend() = 0;

    virtual QString name() const = 0;

    static QStringList availableBackends();

    static Ptr instance(const QString &backend);

    QString formatMessageLine(
            const QString &command,
            const QPair<bool, QString> &result) const;

protected:
    static bool isDirectoryEmpty(const QString &path);

};

} // namespace PlasmaVault

#endif // include guard

