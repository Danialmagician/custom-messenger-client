/*
Custom Messenger - Custom Session Implementation
*/
#include "custom_network/custom_session.h"

namespace Custom {

CustomSession::CustomSession(QObject *parent)
	: QObject(parent)
	, _api(new ApiFacade(this))
	, _ws(nullptr) {
}

CustomSession::~CustomSession() = default;

void CustomSession::initialize(const QString &apiUrl, const QString &wsUrl) {
	_api->initialize(apiUrl, wsUrl);
	_ws = new WebSocketClient(wsUrl, this);

	connect(_ws, &WebSocketClient::connectedChanged, this, [=]() {
		emit authenticatedChanged();
	});

	connect(_ws, &WebSocketClient::newMessage, this, [=](const QJsonObject &msg) {
		emit messageSent(msg);
	});

	connect(_ws, &WebSocketClient::typingIndicator, this, [=](const QString &userId, bool isTyping) {
		emit typingIndicator(userId, isTyping);
	});

	connect(_ws, &WebSocketClient::readReceipt, this, [=](const QString &msgId, const QString &readBy) {
		emit readReceipt(msgId, readBy);
	});

	connect(_ws, &WebSocketClient::callSignal, this, [=](const QString &fromUserId, const QString &signalType, const QJsonObject &payload) {
		emit callSignalReceived(fromUserId, signalType, payload);
	});
}

void CustomSession::connectWebSocket(const QString &token) {
	if (_ws) _ws->connectToServer(token);
}

void CustomSession::disconnectWebSocket() {
	if (_ws) _ws->disconnect();
}

void CustomSession::login(const QString &email, const QString &password) {
	connect(_api, &ApiFacade::loginSuccess, this, [=](const QJsonObject &user, const QJsonObject &tokens) {
		disconnect(_api, &ApiFacade::loginSuccess, this, nullptr);
		connectWebSocket(tokens["access_token"].toString());
		emit loginSuccess(user, tokens);
	});

	connect(_api, &ApiFacade::loginFailed, this, [=](const QString &error) {
		disconnect(_api, &ApiFacade::loginFailed, this, nullptr);
		emit loginFailed(error);
	});

	_api->login(email, password);
}

void CustomSession::registerUser(const QString &username, const QString &email, const QString &password) {
	connect(_api, &ApiFacade::registerSuccess, this, [=](const QJsonObject &user, const QJsonObject &tokens) {
		disconnect(_api, &ApiFacade::registerSuccess, this, nullptr);
		connectWebSocket(tokens["access_token"].toString());
		emit registerSuccess(user, tokens);
	});

	connect(_api, &ApiFacade::registerFailed, this, [=](const QString &error) {
		disconnect(_api, &ApiFacade::registerFailed, this, nullptr);
		emit registerFailed(error);
	});

	_api->registerUser(username, email, password);
}

void CustomSession::logout() {
	_api->logout();
	if (_ws) _ws->disconnect();
}

void CustomSession::refreshToken() {
	_api->refreshToken();
}

void CustomSession::getMe() {
	_api->getMe();
}

void CustomSession::sendMessage(const QString &recipientId, const QString &content, const QString &contentType) {
	_api->sendMessage(recipientId, content, contentType);
}

void CustomSession::sendGroupMessage(const QString &groupId, const QString &content, const QString &contentType) {
	_api->sendGroupMessage(groupId, content, contentType);
}

void CustomSession::sendChannelMessage(const QString &channelId, const QString &content, const QString &contentType) {
	_api->sendChannelMessage(channelId, content, contentType);
}

void CustomSession::getDirectMessages(const QString &userId, int limit) {
	_api->getDirectMessages(userId, limit);
}

void CustomSession::getGroupMessages(const QString &groupId, int limit) {
	_api->getGroupMessages(groupId, limit);
}

void CustomSession::getChannelMessages(const QString &channelId, int limit) {
	_api->getChannelMessages(channelId, limit);
}

void CustomSession::editMessage(const QString &messageId, const QString &content) {
	_api->editMessage(messageId, content);
}

void CustomSession::deleteMessage(const QString &messageId) {
	_api->deleteMessage(messageId);
}

void CustomSession::pinMessage(const QString &messageId) {
	_api->pinMessage(messageId);
}

void CustomSession::reactToMessage(const QString &messageId, const QString &emoji) {
	_api->reactToMessage(messageId, emoji);
}

void CustomSession::createGroup(const QString &name, const QString &description) {
	_api->createGroup(name, description);
}

void CustomSession::getMyGroups() {
	_api->getMyGroups();
}

void CustomSession::createChannel(const QString &name, const QString &description) {
	_api->createChannel(name, description);
}

void CustomSession::getMyChannels() {
	_api->getMyChannels();
}

void CustomSession::searchUsers(const QString &query) {
	_api->searchUsers(query);
}

void CustomSession::getUser(const QString &userId) {
	_api->getUser(userId);
}

} // namespace Custom
