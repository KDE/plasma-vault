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

#ifndef PLASMAVAULT_KDED_ENGINE_BACKENDS_FUSE_BACKEND_H
#define PLASMAVAULT_KDED_ENGINE_BACKENDS_FUSE_BACKEND_H

#include "backend_p.h"

#include <QProcess>

namespace PlasmaVault {

class FuseBackend: public Backend {
public:
    FuseBackend();
    ~FuseBackend();

    bool isOpened(const MountPoint &mountPoint) const override;

    FutureResult<> initialize(const QString &name,
                              const Device &device, const MountPoint &mountPoint,
                              const Vault::Payload &payload) override;

    FutureResult<> open(const Device &device,
                        const MountPoint &mountPoint,
                        const Vault::Payload &payload) override;

    FutureResult<> close(const Device &device,
                         const MountPoint &mountPoint) override;

    FutureResult<> destroy(const Device &device,
                           const MountPoint &mountPoint,
                           const Vault::Payload &payload) override;

protected:
    virtual FutureResult<> mount(const Device &device,
                                 const MountPoint &mountPoint,
                                 const Vault::Payload &payload) = 0;

    QProcess *process(const QString &command, const QStringList &args,
                      const QHash<QString, QString> &environment) const;

    QFuture<QPair<bool, QString>> checkVersion(
            QProcess *process,
            const std::tuple<int,int,int> &requiredVersion) const;

    QProcess *fusermount(const QStringList &arguments = QStringList()) const;

    static Result<> hasProcessFinishedSuccessfully(QProcess *process);
};

} // namespace PlasmaVault

#endif // include guard
