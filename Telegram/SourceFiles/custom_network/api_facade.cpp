/*
Custom Messenger - API Facade Implementation
*/
#include "custom_network/api_facade.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>

namespace Custom {

ApiFacade::ApiFacade(QObject *parent)
	: QObject(parent)
	, _rest(nullptr)
	, _ws(nullptr)
	, _authenticated(false) {
}

ApiFacade::~ApiFacade() = default;

void ApiFacade::initialize(const QString &apiBaseUrl, const QString &wsUrl) {
	_rest = new RestClient(apiBaseUrl, this);
	_ws = new WebSocketClient(wsUrl, this);

	connect(_rest, &RestClient::authenticatedChanged, this, [=]() {
		emit authenticatedChanged();
	});

	connect(_rest, &RestClient::refreshTokenRequired, this, [=]() {
		refreshToken();
	});

	connect(_ws, &WebSocketClient::connectedChanged, this, [=]() {
		emit wsConnectedChanged();
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

	loadTokens();
}

void ApiFacade::setTokens(const QString &accessToken, const QString &refreshToken) {
	_accessToken = accessToken;
	_refreshToken = refreshToken;
	_rest->setAccessToken(accessToken);

	QSettings settings("CustomMessenger", "Messenger");
	settings.setValue("access_token", accessToken);
	settings.setValue("refresh_token", refreshToken);
}

void ApiFacade::clearTokens() {
	_accessToken.clear();
	_refreshToken.clear();
	_rest->clearAccessToken();
	_authenticated = false;
	_userId.clear();
	_username.clear();
	emit authenticatedChanged();
	emit currentUserIdChanged();
	emit currentUserUsernameChanged();

	QSettings settings("CustomMessenger", "Messenger");
	settings.remove("access_token");
	settings.remove("refresh_token");
}

void ApiFacade::loadTokens() {
	QSettings settings("CustomMessenger", "Messenger");
	_accessToken = settings.value("access_token").toString();
	_refreshToken = settings.value("refresh_token").toString();
	if (!_accessToken.isEmpty()) {
		_rest->setAccessToken(_accessToken);
		_authenticated = true;
		emit authenticatedChanged();
	}
}

bool ApiFacade::isAuthenticated() const {
	return _authenticated;
}

bool ApiFacade::isWsConnected() const {
	return _ws ? _ws->isConnected() : false;
}

QString ApiFacade::currentUserId() const {
	return _userId;
}

QString ApiFacade::currentUserUsername() const {
	return _username;
}

void ApiFacade::connectWebSocket() {
	if (_ws && !_accessToken.isEmpty()) {
		_ws->connectToServer(_accessToken);
	}
}

void ApiFacade::login(const QString &email, const QString &password) {
	QJsonObject body;
	body["email"] = email;
	body["password"] = password;

	_rest->post("/auth/login", body,
		[=](const QJsonObject &response) {
			auto user = response["user"].toObject();
			auto tokens = response["tokens"].toObject();
			setTokens(tokens["access_token"].toString(), tokens["refresh_token"].toString());
			_userId = user["id"].toString();
			_username = user["username"].toString();
			_authenticated = true;
			emit authenticatedChanged();
			emit currentUserIdChanged();
			emit currentUserUsernameChanged();
			emit loginSuccess(user, tokens);
			connectWebSocket();
		},
		[=](int status, const QString &error) {
			emit loginFailed(error);
		}
	);
}

void ApiFacade::registerUser(const QString &username, const QString &email, const QString &password) {
	QJsonObject body;
	body["username"] = username;
	body["email"] = email;
	body["password"] = password;

	_rest->post("/auth/register", body,
		[=](const QJsonObject &response) {
			auto user = response["user"].toObject();
			auto tokens = response["tokens"].toObject();
			setTokens(tokens["access_token"].toString(), tokens["refresh_token"].toString());
			_userId = user["id"].toString();
			_username = user["username"].toString();
			_authenticated = true;
			emit authenticatedChanged();
			emit currentUserIdChanged();
			emit currentUserUsernameChanged();
			emit registerSuccess(user, tokens);
			connectWebSocket();
		},
		[=](int status, const QString &error) {
			emit registerFailed(error);
		}
	);
}

void ApiFacade::refreshToken() {
	QJsonObject body;
	body["refresh_token"] = _refreshToken;

	_rest->post("/auth/refresh", body,
		[=](const QJsonObject &response) {
			setTokens(response["access_token"].toString(), response["refresh_token"].toString());
			emit tokenRefreshed(response);
		},
		[=](int status, const QString &error) {
			clearTokens();
		}
	);
}

void ApiFacade::logout() {
	_rest->post("/auth/logout", {},
		[=](const QJsonObject &) {
			clearTokens();
			if (_ws) _ws->disconnect();
		},
		[=](int, const QString &) {
			clearTokens();
		}
	);
}

void ApiFacade::getMe() {
	_rest->get("/auth/me",
		[=](const QJsonObject &response) {
			_userId = response["id"].toString();
			_username = response["username"].toString();
			emit currentUserIdChanged();
			emit currentUserUsernameChanged();
			emit meLoaded(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::sendMessage(const QString &recipientId, const QString &content, const QString &contentType) {
	QJsonObject body;
	body["recipient_id"] = recipientId;
	body["content"] = content;
	body["content_type"] = contentType;

	_rest->post("/messages", body,
		[=](const QJsonObject &response) {
			emit messageSent(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::sendGroupMessage(const QString &groupId, const QString &content, const QString &contentType) {
	QJsonObject body;
	body["group_id"] = groupId;
	body["content"] = content;
	body["content_type"] = contentType;

	_rest->post("/messages", body,
		[=](const QJsonObject &response) {
			emit messageSent(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::sendChannelMessage(const QString &channelId, const QString &content, const QString &contentType) {
	QJsonObject body;
	body["channel_id"] = channelId;
	body["content"] = content;
	body["content_type"] = contentType;

	_rest->post("/messages", body,
		[=](const QJsonObject &response) {
			emit messageSent(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getDirectMessages(const QString &userId, int limit) {
	auto endpoint = "/messages/direct/" + userId + "?limit=" + QString::number(limit);
	_rest->get(endpoint,
		[=](const QJsonObject &response) {
			emit messagesLoaded(response["messages"].toArray(), response["has_more"].toBool());
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getGroupMessages(const QString &groupId, int limit) {
	auto endpoint = "/messages/group/" + groupId + "?limit=" + QString::number(limit);
	_rest->get(endpoint,
		[=](const QJsonObject &response) {
			emit messagesLoaded(response["messages"].toArray(), response["has_more"].toBool());
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getChannelMessages(const QString &channelId, int limit) {
	auto endpoint = "/messages/channel/" + channelId + "?limit=" + QString::number(limit);
	_rest->get(endpoint,
		[=](const QJsonObject &response) {
			emit messagesLoaded(response["messages"].toArray(), response["has_more"].toBool());
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::editMessage(const QString &messageId, const QString &content) {
	QJsonObject body;
	body["content"] = content;

	_rest->patch("/messages/" + messageId, body,
		[=](const QJsonObject &response) {
			emit messageEdited(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::deleteMessage(const QString &messageId) {
	_rest->del("/messages/" + messageId,
		[=](const QJsonObject &) {
			emit messageDeleted(messageId);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::pinMessage(const QString &messageId) {
	_rest->post("/messages/" + messageId + "/pin", {},
		[=](const QJsonObject &response) {
			emit messagePinned(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::reactToMessage(const QString &messageId, const QString &emoji) {
	QJsonObject body;
	body["emoji"] = emoji;

	_rest->post("/messages/" + messageId + "/react", body,
		[=](const QJsonObject &response) {
			emit messageReacted(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::forwardMessage(const QString &messageId, const QString &recipientId) {
	QJsonObject body;
	body["recipient_id"] = recipientId;

	_rest->post("/messages/" + messageId + "/forward", body,
		[=](const QJsonObject &response) {
			emit messageForwarded(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::loadThreadMessages(const QString &threadId, int limit) {
	auto endpoint = "/messages/thread/" + threadId + "?limit=" + QString::number(limit);
	_rest->get(endpoint,
		[=](const QJsonObject &response) {
			emit messagesLoaded(response["messages"].toArray(), response["has_more"].toBool());
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::createGroup(const QString &name, const QString &description) {
	QJsonObject body;
	body["name"] = name;
	body["description"] = description;

	_rest->post("/groups", body,
		[=](const QJsonObject &response) {
			emit groupCreated(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getMyGroups() {
	_rest->get("/groups",
		[=](const QJsonObject &response) {
			emit groupsLoaded(response.toArray());
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getGroup(const QString &groupId) {
	_rest->get("/groups/" + groupId,
		[=](const QJsonObject &response) {
			emit groupLoaded(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::updateGroup(const QString &groupId, const QJsonObject &data) {
	_rest->patch("/groups/" + groupId, data,
		[=](const QJsonObject &response) {
			emit groupUpdated(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::addGroupMember(const QString &groupId, const QString &userId) {
	QJsonObject body;
	body["user_id"] = userId;

	_rest->post("/groups/" + groupId + "/members", body,
		[=](const QJsonObject &) {},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::removeGroupMember(const QString &groupId, const QString &userId) {
	_rest->del("/groups/" + groupId + "/members/" + userId,
		[=](const QJsonObject &) {},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::leaveGroup(const QString &groupId) {
	_rest->post("/groups/" + groupId + "/leave", {},
		[=](const QJsonObject &) {},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::createChannel(const QString &name, const QString &description) {
	QJsonObject body;
	body["name"] = name;
	body["description"] = description;

	_rest->post("/channels", body,
		[=](const QJsonObject &response) {
			emit channelCreated(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getMyChannels() {
	_rest->get("/channels",
		[=](const QJsonObject &response) {
			emit channelsLoaded(response.toArray());
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getChannel(const QString &channelId) {
	_rest->get("/channels/" + channelId,
		[=](const QJsonObject &response) {
			emit channelLoaded(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::subscribeChannel(const QString &channelId) {
	_rest->post("/channels/" + channelId + "/subscribe", {},
		[=](const QJsonObject &) {},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::unsubscribeChannel(const QString &channelId) {
	_rest->post("/channels/" + channelId + "/unsubscribe", {},
		[=](const QJsonObject &) {},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::searchUsers(const QString &query) {
	_rest->get("/users/search?q=" + query,
		[=](const QJsonObject &response) {
			emit usersSearched(response.toArray());
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::getUser(const QString &userId) {
	_rest->get("/users/" + userId,
		[=](const QJsonObject &response) {
			emit userLoaded(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::updateProfile(const QJsonObject &data) {
	_rest->patch("/auth/me", data,
		[=](const QJsonObject &response) {
			emit profileUpdated(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

void ApiFacade::uploadFile(const QString &filePath) {
	_rest->upload("/files/upload", filePath, "application/octet-stream",
		[=](const QJsonObject &response) {
			emit fileUploaded(response);
		},
		[=](int status, const QString &error) {
			emit errorOccurred(error);
		}
	);
}

} // namespace Custom
