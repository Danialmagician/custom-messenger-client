/*
Custom Messenger - WebSocket Client
*/
#include "custom_network/websocket_client.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>

namespace Custom {

WebSocketClient::WebSocketClient(const QString &wsUrl, QObject *parent)
	: QObject(parent)
	, _wsUrl(wsUrl) {
	connect(&_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
	connect(&_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
	connect(&_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
	connect(&_pingTimer, &QTimer::timeout, this, &WebSocketClient::onPingTimer);
}

WebSocketClient::~WebSocketClient() {
	_pingTimer.stop();
	_webSocket.close();
}

bool WebSocketClient::isConnected() const {
	return _webSocket.isValid();
}

void WebSocketClient::connectToServer(const QString &token) {
	QUrl url(_wsUrl + "?token=" + token);
	_webSocket.open(url);
}

void WebSocketClient::disconnect() {
	_pingTimer.stop();
	_webSocket.close();
}

void WebSocketClient::sendMessage(const QJsonObject &message) {
	QJsonObject msg = message;
	msg["type"] = "message";
	_webSocket.sendTextMessage(QJsonDocument(msg).toJson());
}

void WebSocketClient::sendTyping(const QString &chatId, bool isTyping) {
	QJsonObject msg;
	msg["type"] = "typing";
	msg["chat_id"] = chatId;
	msg["is_typing"] = isTyping;
	_webSocket.sendTextMessage(QJsonDocument(msg).toJson());
}

void WebSocketClient::sendReadReceipt(const QString &messageId) {
	QJsonObject msg;
	msg["type"] = "read_receipt";
	msg["message_id"] = messageId;
	_webSocket.sendTextMessage(QJsonDocument(msg).toJson());
}

void WebSocketClient::sendCallSignal(const QString &targetUserId, const QString &signalType, const QJsonObject &payload) {
	QJsonObject msg;
	msg["type"] = "call_signal";
	msg["target_user_id"] = targetUserId;
	msg["signal_type"] = signalType;
	msg["payload"] = payload;
	_webSocket.sendTextMessage(QJsonDocument(msg).toJson());
}

void WebSocketClient::onConnected() {
	emit connectedChanged();
	emit connectedToServer();
	_pingTimer.start(30000);
}

void WebSocketClient::onDisconnected() {
	_pingTimer.stop();
	emit connectedChanged();
	emit disconnectedFromServer();
}

void WebSocketClient::onTextMessageReceived(const QString &message) {
	QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
	QJsonObject obj = doc.object();
	auto type = obj["type"].toString();

	if (type == "new_message") {
		emit newMessage(obj["message"].toObject());
	} else if (type == "typing") {
		emit typingIndicator(obj["user_id"].toString(), obj["is_typing"].toBool());
	} else if (type == "read_receipt") {
		emit readReceipt(obj["message_id"].toString(), obj["read_by"].toString());
	} else if (type == "call_signal") {
		emit callSignal(obj["from_user"].toString(), obj["signal_type"].toString(), obj["payload"].toObject());
	}
}

void WebSocketClient::onPingTimer() {
	QJsonObject msg;
	msg["type"] = "ping";
	_webSocket.sendTextMessage(QJsonDocument(msg).toJson());
}

} // namespace Custom
