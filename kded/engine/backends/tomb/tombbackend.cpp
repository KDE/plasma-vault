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

#include "tombbackend.h"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>

#include <KMountPoint>
#include <KLocalizedString>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/wrappers/process.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>

#include <singleton_p.h>

using namespace AsynQt;

namespace PlasmaVault {


TombBackend::TombBackend()
{
}



TombBackend::~TombBackend()
{
}



Backend::Ptr TombBackend::instance()
{
    return singleton::instance<TombBackend>();
}



FutureResult<> TombBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    // return
    //     collect(checkVersion(tomb({ "--version" }), std::make_tuple(2, 3)))
    //
    //     | transform([this] (const QPair<bool, QString> &tomb) {
    //
    //           bool success     = tomb.first;
    //           QString message  = formatMessageLine("tomb", tomb);
    //
    //           return success ? Result<>::success()
    //                          : Result<>::error(Error::BackendError, message);
    //       });

    return FutureResult<>();
}



bool TombBackend::isInitialized(const Device &device) const
{
    return false;
}



bool TombBackend::isOpened(const MountPoint &mountPoint) const
{
    return false;
}



FutureResult<> TombBackend::initialize(const QString &name,
                                       const Device &device,
                                       const MountPoint &mountPoint,
                                       const Vault::Payload &payload)
{
    return FutureResult<>();
}



FutureResult<> TombBackend::open(const Device &device,
                                 const MountPoint &mountPoint,
                                 const Vault::Payload &payload)
{
    return FutureResult<>();
}



FutureResult<> TombBackend::close(const Device &device,
                                  const MountPoint &mountPoint)
{
    return FutureResult<>();
}



FutureResult<> TombBackend::destroy(const Device &device,
                                    const MountPoint &mountPoint,
                                    const Vault::Payload &payload)
{
    return FutureResult<>();
}



FutureResult<> validateBackend()
{
    return FutureResult<>();
}



} // namespace PlasmaVault

