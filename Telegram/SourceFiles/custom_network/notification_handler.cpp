#include "custom_network/notification_handler.h"

namespace CustomNetwork {

NotificationHandler::NotificationHandler(QObject *parent)
    : QObject(parent)
{
}

void NotificationHandler::handleNotification(const QJsonObject &notification) {
    auto type = notification["type"].toString();
    auto content = notification["content"].toString();
    auto chatId = notification["chat_id"].toString();

    emit showDesktopNotification(type, content, chatId);
    ++_totalUnread;
    emit updateUnreadBadge(_totalUnread);
}

void NotificationHandler::handleTyping(const QString &userId, const QString &chatId) {
    emit updateTypingIndicator(userId, chatId, true);
}

void NotificationHandler::handleReadReceipt(const QString &userId, const QString &chatId, const QString &messageId) {
    Q_UNUSED(userId);
    Q_UNUSED(chatId);
    Q_UNUSED(messageId);
}

void NotificationHandler::handlePresence(const QString &userId, const QString &status) {
    Q_UNUSED(userId);
    Q_UNUSED(status);
}

} // namespace CustomNetwork
