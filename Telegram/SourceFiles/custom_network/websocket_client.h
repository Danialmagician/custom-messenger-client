/*
Custom Messenger - WebSocket Client
Handles real-time messaging, typing indicators, read receipts, and call signaling.
*/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QTimer>

namespace Custom {

class WebSocketClient : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
	WebSocketClient(const QString &wsUrl, QObject *parent = nullptr);
	~WebSocketClient();

	[[nodiscard]] bool isConnected() const;

	void connectToServer(const QString &token);
	void disconnect();

	void sendMessage(const QJsonObject &message);
	void sendTyping(const QString &chatId, bool isTyping);
	void sendReadReceipt(const QString &messageId);
	void sendCallSignal(const QString &targetUserId, const QString &signalType, const QJsonObject &payload);

signals:
	void connectedChanged();
	void connectedToServer();
	void disconnectedFromServer();
	void newMessage(const QJsonObject &message);
	void typingIndicator(const QString &userId, bool isTyping);
	void readReceipt(const QString &messageId, const QString &readBy);
	void callSignal(const QString &fromUserId, const QString &signalType, const QJsonObject &payload);

private slots:
	void onConnected();
	void onDisconnected();
	void onTextMessageReceived(const QString &message);
	void onPingTimer();

private:
	QWebSocket _webSocket;
	QTimer _pingTimer;
	QString _wsUrl;
};

} // namespace Custom
