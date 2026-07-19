/*
Custom Messenger - Custom Session
Replaces MTP::Instance with a custom session using REST/WebSocket.
*/
#pragma once

#include "custom_network/api_facade.h"
#include "custom_network/websocket_client.h"

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QString>

namespace Main {
class Session;
} // namespace Main

namespace Custom {

class CustomSession : public QObject {
	Q_OBJECT

public:
	CustomSession(QObject *parent = nullptr);
	~CustomSession();

	void initialize(const QString &apiUrl, const QString &wsUrl);
	void connectWebSocket(const QString &token);
	void disconnectWebSocket();

	void login(const QString &email, const QString &password);
	void registerUser(const QString &username, const QString &email, const QString &password);
	void logout();
	void refreshToken();
	void getMe();

	void sendMessage(const QString &recipientId, const QString &content, const QString &contentType = "text");
	void sendGroupMessage(const QString &groupId, const QString &content, const QString &contentType = "text");
	void sendChannelMessage(const QString &channelId, const QString &content, const QString &contentType = "text");
	void getDirectMessages(const QString &userId, int limit = 50);
	void getGroupMessages(const QString &groupId, int limit = 50);
	void getChannelMessages(const QString &channelId, int limit = 50);
	void editMessage(const QString &messageId, const QString &content);
	void deleteMessage(const QString &messageId);
	void pinMessage(const QString &messageId);
	void reactToMessage(const QString &messageId, const QString &emoji);

	void createGroup(const QString &name, const QString &description);
	void getMyGroups();
	void createChannel(const QString &name, const QString &description);
	void getMyChannels();
	void searchUsers(const QString &query);
	void getUser(const QString &userId);

signals:
	void loginSuccess(const QJsonObject &user, const QJsonObject &tokens);
	void registerSuccess(const QJsonObject &user, const QJsonObject &tokens);
	void loginFailed(const QString &error);
	void registerFailed(const QString &error);
	void authenticatedChanged();
	void meLoaded(const QJsonObject &user);
	void profileUpdated(const QJsonObject &user);

	void messageSent(const QJsonObject &message);
	void messagesLoaded(const QJsonArray &messages, bool hasMore);
	void messageEdited(const QJsonObject &message);
	void messageDeleted(const QString &messageId);
	void messagePinned(const QJsonObject &message);
	void messageReacted(const QJsonObject &message);

	void groupCreated(const QJsonObject &group);
	void groupsLoaded(const QJsonArray &groups);
	void channelCreated(const QJsonObject &channel);
	void channelsLoaded(const QJsonArray &channels);

	void usersSearched(const QJsonArray &users);
	void userLoaded(const QJsonObject &user);

	void typingIndicator(const QString &userId, bool isTyping);
	void readReceipt(const QString &messageId, const QString &readBy);
	void callSignalReceived(const QString &fromUserId, const QString &signalType, const QJsonObject &payload);

	void errorOccurred(const QString &error);

private:
	ApiFacade *_api;
	WebSocketClient *_ws;
};

} // namespace Custom
