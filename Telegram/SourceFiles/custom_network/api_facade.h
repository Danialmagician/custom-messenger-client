/*
Custom Messenger - API Facade
Replaces Telegram API with REST API calls to custom backend.
This is the main interface that the rest of the app uses.
*/
#pragma once

#include "custom_network/rest_client.h"
#include "custom_network/websocket_client.h"

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QString>
#include <functional>

namespace Custom {

class ApiFacade : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
	Q_PROPERTY(bool wsConnected READ isWsConnected NOTIFY wsConnectedChanged)
	Q_PROPERTY(QString currentUserId READ currentUserId NOTIFY currentUserIdChanged)
	Q_PROPERTY(QString currentUserUsername READ currentUserUsername NOTIFY currentUserUsernameChanged)

public:
	ApiFacade(QObject *parent = nullptr);
	~ApiFacade();

	void initialize(const QString &apiBaseUrl, const QString &wsUrl);
	void setTokens(const QString &accessToken, const QString &refreshToken);
	void clearTokens();
	void loadTokens();

	[[nodiscard]] bool isAuthenticated() const;
	[[nodiscard]] bool isWsConnected() const;
	[[nodiscard]] QString currentUserId() const;
	[[nodiscard]] QString currentUserUsername() const;

	void connectWebSocket();

	// Auth
	void login(const QString &email, const QString &password);
	void registerUser(const QString &username, const QString &email, const QString &password);
	void refreshToken();
	void logout();
	void getMe();

	// Messages
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
	void forwardMessage(const QString &messageId, const QString &recipientId);
	void loadThreadMessages(const QString &threadId, int limit = 50);

	// Groups
	void createGroup(const QString &name, const QString &description);
	void getMyGroups();
	void getGroup(const QString &groupId);
	void updateGroup(const QString &groupId, const QJsonObject &data);
	void addGroupMember(const QString &groupId, const QString &userId);
	void removeGroupMember(const QString &groupId, const QString &userId);
	void leaveGroup(const QString &groupId);

	// Channels
	void createChannel(const QString &name, const QString &description);
	void getMyChannels();
	void getChannel(const QString &channelId);
	void subscribeChannel(const QString &channelId);
	void unsubscribeChannel(const QString &channelId);

	// Users
	void searchUsers(const QString &query);
	void getUser(const QString &userId);
	void updateProfile(const QJsonObject &data);

	// Files
	void uploadFile(const QString &filePath);

signals:
	void authenticatedChanged();
	void wsConnectedChanged();
	void currentUserIdChanged();
	void currentUserUsernameChanged();

	void loginSuccess(const QJsonObject &user, const QJsonObject &tokens);
	void registerSuccess(const QJsonObject &user, const QJsonObject &tokens);
	void loginFailed(const QString &error);
	void registerFailed(const QString &error);
	void meLoaded(const QJsonObject &user);
	void profileUpdated(const QJsonObject &user);
	void tokenRefreshed(const QJsonObject &tokens);

	void messageSent(const QJsonObject &message);
	void messagesLoaded(const QJsonArray &messages, bool hasMore);
	void messageEdited(const QJsonObject &message);
	void messageDeleted(const QString &messageId);
	void messagePinned(const QJsonObject &message);
	void messageReacted(const QJsonObject &message);
	void messageForwarded(const QJsonObject &message);

	void groupCreated(const QJsonObject &group);
	void groupsLoaded(const QJsonArray &groups);
	void groupLoaded(const QJsonObject &group);
	void groupUpdated(const QJsonObject &group);

	void channelCreated(const QJsonObject &channel);
	void channelsLoaded(const QJsonArray &channels);
	void channelLoaded(const QJsonObject &channel);

	void usersSearched(const QJsonArray &users);
	void userLoaded(const QJsonObject &user);

	void fileUploaded(const QJsonObject &file);

	void typingIndicator(const QString &userId, bool isTyping);
	void readReceipt(const QString &messageId, const QString &readBy);
	void callSignalReceived(const QString &fromUserId, const QString &signalType, const QJsonObject &payload);

	void errorOccurred(const QString &error);

private:
	RestClient *_rest;
	WebSocketClient *_ws;
	QString _accessToken;
	QString _refreshToken;
	QString _userId;
	QString _username;
	bool _authenticated;
};

} // namespace Custom
