#pragma once

#include <QObject>
#include <QJsonObject>

namespace CustomNetwork {

class NotificationHandler : public QObject {
    Q_OBJECT

public:
    explicit NotificationHandler(QObject *parent = nullptr);

    void handleNotification(const QJsonObject &notification);
    void handleTyping(const QString &userId, const QString &chatId);
    void handleReadReceipt(const QString &userId, const QString &chatId, const QString &messageId);
    void handlePresence(const QString &userId, const QString &status);

signals:
    void showDesktopNotification(const QString &title, const QString &body, const QString &chatId);
    void updateUnreadBadge(int count);
    void updateTypingIndicator(const QString &userId, const QString &chatId, bool typing);

private:
    int _totalUnread = 0;
};

} // namespace CustomNetwork
