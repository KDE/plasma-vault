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
class KJobFutureInterface : public QObject,
                            public QFutureInterface<_Result> {
public:
    KJobFutureInterface(KJob* job)
        : job(job)
    {
        job->setAutoDelete(false);
    }

    ~KJobFutureInterface()
    {
    }

    void callFinished();

    QFuture<_Result> start()
    {
        QObject::connect(job, &KJob::result,
                         this, [this] () { callFinished(); },
                         Qt::QueuedConnection);

        this->reportStarted();

        job->start();

        return this->future();
    }

private:
    KJob* job;
};

template <typename _Result>
void KJobFutureInterface<_Result>::callFinished()
{
    this->reportResult(job);
    this->reportFinished();

    deleteLater();
}

template <>
void KJobFutureInterface<void>::callFinished()
{
    this->reportFinished();

    job->deleteLater();
    deleteLater();
}

} // namespace detail
} // namespace AsynQt

