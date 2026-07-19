/*
Custom Messenger - REST Notification Handler Implementation
*/
#include "custom_network/rest_notification_handler.h"
#include "custom_network/mtp_override.h"

namespace Custom {

RestNotificationHandler::RestNotificationHandler(QObject *parent)
	: QObject(parent) {
}

void RestNotificationHandler::initialize(Window::Controller *controller) {
	_controller = controller;
	_api = MtpOverride::Instance().api();

	if (_api) {
		QObject::connect(_api, &ApiFacade::messageSent, this,
			&RestNotificationHandler::handleNewMessage);
	}
}

void RestNotificationHandler::handleNewMessage(const QJsonObject &message) {
	emit messageReceived(message);
}

void RestNotificationHandler::handleTyping(const QString &userId, bool isTyping) {
	emit typingIndicator(userId, isTyping);
}

void RestNotificationHandler::handleReadReceipt(const QString &messageId, const QString &readBy) {
	emit readReceiptUpdated(messageId, readBy);
}

} // namespace Custom
