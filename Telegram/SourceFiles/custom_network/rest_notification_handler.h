/*
Custom Messenger - REST Notification Handler
Replaces MTProto update handling with WebSocket notifications.
*/
#pragma once

#include "custom_network/api_facade.h"
#include <QtCore/QObject>

namespace Window {
class Controller;
} // namespace Window

namespace Custom {

class RestNotificationHandler : public QObject {
	Q_OBJECT

public:
	RestNotificationHandler(QObject *parent = nullptr);

	void initialize(Window::Controller *controller);
	void handleNewMessage(const QJsonObject &message);
	void handleTyping(const QString &userId, bool isTyping);
	void handleReadReceipt(const QString &messageId, const QString &readBy);

signals:
	void messageReceived(const QJsonObject &message);
	void typingIndicator(const QString &userId, bool isTyping);
	void readReceiptUpdated(const QString &messageId, const QString &readBy);

private:
	Window::Controller *_controller = nullptr;
	ApiFacade *_api = nullptr;
};

} // namespace Custom
