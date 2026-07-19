#include "custom_network/websocket_client.h"
#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>

namespace CustomNetwork {

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent)
{
    connect(&_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(&_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::onReconnectTimeout);
}

WebSocketClient::~WebSocketClient() {
    _reconnectTimer.stop();
    _socket.close();
}

void WebSocketClient::setUrl(const QString &baseUrl) {
    auto url = baseUrl;
    url.replace(u"http://"_q, u"ws://"_q);
    url.replace(u"https://"_q, u"wss://"_q);
    _url = url;
}

void WebSocketClient::connectToServer(const QString &token) {
    _token = token;
    QUrl wsUrl(_url + u"/ws?token="_q + token);
    _socket.open(wsUrl);
}

void WebSocketClient::disconnect() {
    _reconnectTimer.stop();
    _socket.close();
}

bool WebSocketClient::isConnected() const {
    return _socket.isValid();
}

void WebSocketClient::onConnected() {
    _reconnectDelay = 1000;
    emit connected();
}

void WebSocketClient::onDisconnected() {
    emit disconnected();
    _reconnectTimer.start(_reconnectDelay);
    _reconnectDelay = qMin(_reconnectDelay * 2, kMaxReconnectDelay);
}

void WebSocketClient::onReconnectTimeout() {
    if (!_token.isEmpty()) {
        connectToServer(_token);
    }
}

void WebSocketClient::onTextMessageReceived(const QString &message) {
    auto json = QJsonDocument::fromJson(message.toUtf8()).object();
    auto type = json["type"].toString();

    if (type == u"message"_q) {
        emit messageReceived(json);
    } else if (type == u"typing"_q) {
        emit typingReceived(json["user_id"].toString(), json["chat_id"].toString());
    } else if (type == u"read_receipt"_q) {
        emit readReceiptReceived(json["user_id"].toString(), json["chat_id"].toString(), json["message_id"].toString());
    } else if (type == u"presence"_q) {
        emit presenceReceived(json["user_id"].toString(), json["status"].toString());
    } else if (type == u"notification"_q) {
        emit notificationReceived(json["notification"].toObject());
    } else if (type == u"pong"_q) {
    }
}

void WebSocketClient::sendTyping(const QString &chatId) {
    sendJson({{u"type"_q, u"typing"_q}, {u"chat_id"_q, chatId}});
}

void WebSocketClient::sendReadReceipt(const QString &chatId, const QString &messageId) {
    sendJson({{u"type"_q, u"read_receipt"_q}, {u"chat_id"_q, chatId}, {u"message_id"_q, messageId}});
}

void WebSocketClient::sendPresence(const QString &status) {
    sendJson({{u"type"_q, u"presence"_q}, {u"status"_q, status}});
}

void WebSocketClient::joinChat(const QString &chatId) {
    sendJson({{u"type"_q, u"join_chat"_q}, {u"chat_id"_q, chatId}});
}

void WebSocketClient::leaveChat(const QString &chatId) {
    sendJson({{u"type"_q, u"leave_chat"_q}, {u"chat_id"_q, chatId}});
}

void WebSocketClient::sendJson(const QJsonObject &obj) {
    if (_socket.isValid()) {
        _socket.sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    }
}

} // namespace CustomNetwork
