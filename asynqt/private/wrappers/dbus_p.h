/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
class DBusCallFutureInterface : public QObject,
                                public QFutureInterface<_Result> {
public:
    DBusCallFutureInterface(QDBusPendingReply<_Result> reply)
        : reply(reply)
    {
    }

    ~DBusCallFutureInterface()
    {
    }

    void callFinished();

    QFuture<_Result> start()
    {
        replyWatcher.reset(new QDBusPendingCallWatcher(reply));

        QObject::connect(replyWatcher.get(),
                         &QDBusPendingCallWatcher::finished,
                         replyWatcher.get(),
                         [this] () { callFinished(); },
                         Qt::QueuedConnection);

        this->reportStarted();

        if (reply.isFinished()) {
            this->callFinished();
        }

        return this->future();
    }

private:
    QDBusPendingReply<_Result> reply;
    std::unique_ptr<QDBusPendingCallWatcher> replyWatcher;
};

template <typename _Result>
void DBusCallFutureInterface<_Result>::callFinished()
{
    if (!reply.isError()) {
        this->reportResult(reply.value());
        this->reportFinished();

    } else {
        this->reportCanceled();

    }

    deleteLater();
}

template <>
inline
void DBusCallFutureInterface<void>::callFinished()
{
    if (!reply.isError()) {
        this->reportFinished();

    } else {
        this->reportCanceled();
    }

    deleteLater();
}

} // namespace detail
} // namespace AsynQt

