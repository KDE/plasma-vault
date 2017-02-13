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

template <typename _Result, typename _Function>
class ProcessFutureInterface : public QObject,
                               public QFutureInterface<_Result> {

public:
    ProcessFutureInterface(QProcess *process, _Function map)
        : m_process(process)
        , m_map(map)
    {
    }

    QFuture<_Result> start()
    {
        QObject::connect(
                m_process,
                // Pretty new Qt connect syntax :)
                (void (QProcess::*)(int, QProcess::ExitStatus)) &QProcess::finished,
                this, [this] (int, QProcess::ExitStatus) {
                    this->finished();
                });

        this->reportStarted();

        m_process->start();

        return this->future();
    }

    void finished();


private:
    QProcess *m_process;
    _Function m_map;

};

template <typename _Result, typename _Function>
void ProcessFutureInterface<_Result, _Function>::finished()
{
    this->reportResult(m_map(m_process));
    this->reportFinished();
}

} // namespace detail
} // namespace AsynQt

