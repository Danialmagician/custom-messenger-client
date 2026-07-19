#include "custom_network/custom_api.h"
#include "custom_network/rest_client.h"
#include "custom_network/websocket_client.h"
#include "custom_network/session_manager.h"
#include "custom_network/custom_auth.h"
#include "custom_network/file_transfer.h"
#include <QUrlQuery>

namespace CustomNetwork {

CustomApi::CustomApi(QObject *parent)
    : QObject(parent)
    , _restClient(new RestClient(this))
    , _wsClient(new WebSocketClient(this))
    , _session(new SessionManager(this))
{
    connect(_wsClient, &WebSocketClient::messageReceived, this, &CustomApi::messageReceived);
    connect(_wsClient, &WebSocketClient::typingReceived, this, &CustomApi::typingReceived);
    connect(_wsClient, &WebSocketClient::readReceiptReceived, this, &CustomApi::readReceiptReceived);
    connect(_wsClient, &WebSocketClient::presenceReceived, this, &CustomApi::presenceReceived);
    connect(_wsClient, &WebSocketClient::notificationReceived, this, &CustomApi::notificationReceived);
    connect(_wsClient, &WebSocketClient::connected, this, &CustomApi::connected);
    connect(_wsClient, &WebSocketClient::disconnected, this, &CustomApi::disconnected);
}

CustomApi::~CustomApi() = default;

void CustomApi::setBackendUrl(const QString &url) {
    _backendUrl = url;
    _restClient->setBaseUrl(url);
    _wsClient->setUrl(url);
}

QString CustomApi::backendUrl() const {
    return _backendUrl;
}

void CustomApi::login(
        const QString &email,
        const QString &password,
        bool rememberMe,
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail) {
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;
    body["remember_me"] = rememberMe;

    _restClient->post(
        u"/auth/login"_q,
        QJsonDocument(body).toJson(),
        [this, done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            auto tokens = TokenPair{
                json["access_token"].toString(),
                json["refresh_token"].toString(),
            };
            _session->setTokens(tokens.accessToken, tokens.refreshToken);
            _restClient->setAuthToken(tokens.accessToken);

            _restClient->get(
                u"/auth/me"_q,
                [done = std::move(done)](const QByteArray &meResponse) {
                    auto meJson = QJsonDocument::fromJson(meResponse).object();
                    auto user = UserProfile{
                        meJson["id"].toString(),
                        meJson["email"].toString(),
                        meJson["username"].toString(),
                        meJson["display_name"].toString(),
                        meJson["avatar_url"].toString(),
                        meJson["is_active"].toBool(),
                        meJson["created_at"].toString(),
                    };
                    done(user);
                },
                [fail = std::move(fail)](const QString &error) {
                    if (fail) fail(error);
                });
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::registerUser(
        const QString &email,
        const QString &username,
        const QString &displayName,
        const QString &password,
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail) {
    QJsonObject body;
    body["email"] = email;
    body["username"] = username;
    body["display_name"] = displayName;
    body["password"] = password;

    _restClient->post(
        u"/auth/register"_q,
        QJsonDocument(body).toJson(),
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            auto user = UserProfile{
                json["id"].toString(),
                json["email"].toString(),
                json["username"].toString(),
                json["display_name"].toString(),
                json["avatar_url"].toString(),
                json["is_active"].toBool(),
                json["created_at"].toString(),
            };
            done(user);
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::refreshToken(
        std::function<void()> &&done,
        ErrorCallback &&fail) {
    QJsonObject body;
    body["refresh_token"] = _session->refreshToken();

    _restClient->post(
        u"/auth/refresh"_q,
        QJsonDocument(body).toJson(),
        [this, done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            _session->setTokens(
                json["access_token"].toString(),
                json["refresh_token"].toString());
            _restClient->setAuthToken(_session->accessToken());
            if (done) done();
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::logout() {
    _restClient->post(
        u"/auth/logout"_q,
        QByteArray(),
        [](const QByteArray &) {},
        [](const QString &) {});
    _session->clear();
    _restClient->setAuthToken(QString());
    _wsClient->disconnect();
}

void CustomApi::getMe(
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail) {
    _restClient->get(
        u"/api/v1/users/me"_q,
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            done(UserProfile{
                json["id"].toString(),
                json["email"].toString(),
                json["username"].toString(),
                json["display_name"].toString(),
                json["avatar_url"].toString(),
                json["is_active"].toBool(),
                json["created_at"].toString(),
            });
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::updateProfile(
        const QString &displayName,
        const std::optional<QString> &avatarUrl,
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail) {
    QJsonObject body;
    if (!displayName.isEmpty()) body["display_name"] = displayName;
    if (avatarUrl.has_value()) body["avatar_url"] = avatarUrl.value();

    QUrlQuery params;
    if (!displayName.isEmpty()) params.addQueryItem("display_name", displayName);
    if (avatarUrl.has_value()) params.addQueryItem("avatar_url", avatarUrl.value());

    _restClient->patch(
        u"/api/v1/users/me?"_q + params.toString(),
        QJsonDocument(body).toJson(),
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            done(UserProfile{
                json["id"].toString(),
                json["email"].toString(),
                json["username"].toString(),
                json["display_name"].toString(),
                json["avatar_url"].toString(),
                json["is_active"].toBool(),
                json["created_at"].toString(),
            });
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::getChats(
        std::function<void(const QVector<ChatInfo> &chats)> &&done,
        ErrorCallback &&fail) {
    _restClient->get(
        u"/api/v1/chats/"_q,
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).array();
            QVector<ChatInfo> chats;
            chats.reserve(json.size());
            for (const auto &item : json) {
                auto obj = item.toObject();
                chats.append(ChatInfo{
                    obj["id"].toString(),
                    obj["title"].toString(),
                    obj["description"].toString(),
                    obj["chat_type"].toString(),
                    obj["avatar_url"].toString(),
                    obj["created_by"].toString(),
                    obj["created_at"].toString(),
                    obj["unread_count"].toInt(),
                });
            }
            done(chats);
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::createChat(
        const QString &title,
        const QString &description,
        const QString &chatType,
        const QVector<QString> &memberIds,
        std::function<void(const ChatInfo &chat)> &&done,
        ErrorCallback &&fail) {
    QJsonObject body;
    body["title"] = title;
    body["description"] = description;
    body["chat_type"] = chatType;
    QJsonArray members;
    for (const auto &id : memberIds) {
        members.append(id);
    }
    body["member_ids"] = members;

    _restClient->post(
        u"/api/v1/chats/"_q,
        QJsonDocument(body).toJson(),
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            done(ChatInfo{
                json["id"].toString(),
                json["title"].toString(),
                json["description"].toString(),
                json["chat_type"].toString(),
                json["avatar_url"].toString(),
                json["created_by"].toString(),
                json["created_at"].toString(),
            });
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::getChat(
        const QString &chatId,
        std::function<void(const ChatInfo &chat)> &&done,
        ErrorCallback &&fail) {
    _restClient->get(
        u"/api/v1/chats/"_q + chatId,
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            done(ChatInfo{
                json["id"].toString(),
                json["title"].toString(),
                json["description"].toString(),
                json["chat_type"].toString(),
                json["avatar_url"].toString(),
                json["created_by"].toString(),
                json["created_at"].toString(),
            });
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::getMessages(
        const QString &chatId,
        int offset,
        int limit,
        std::function<void(const QVector<MessageInfo> &messages)> &&done,
        ErrorCallback &&fail) {
    _restClient->get(
        u"/api/v1/chats/"_q + chatId + u"/messages?offset="_q + QString::number(offset) + u"&limit="_q + QString::number(limit),
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).array();
            QVector<MessageInfo> messages;
            messages.reserve(json.size());
            for (const auto &item : json) {
                auto obj = item.toObject();
                messages.append(MessageInfo{
                    obj["id"].toString(),
                    obj["chat_id"].toString(),
                    obj["sender_id"].toString(),
                    obj["content"].toString(),
                    obj["message_type"].toString(),
                    obj["media_url"].toString(),
                    obj["reply_to_id"].toString(),
                    obj["is_edited"].toBool(),
                    obj["is_deleted"].toBool(),
                    obj["created_at"].toString(),
                });
            }
            done(messages);
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::sendMessage(
        const QString &chatId,
        const QString &content,
        const QString &messageType,
        const std::optional<QString> &mediaUrl,
        const std::optional<QString> &replyToId,
        std::function<void(const MessageInfo &message)> &&done,
        ErrorCallback &&fail) {
    QJsonObject body;
    body["content"] = content;
    body["message_type"] = messageType;
    if (mediaUrl.has_value()) body["media_url"] = mediaUrl.value();
    if (replyToId.has_value()) body["reply_to_id"] = replyToId.value();

    _restClient->post(
        u"/api/v1/chats/"_q + chatId + u"/messages"_q,
        QJsonDocument(body).toJson(),
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            done(MessageInfo{
                json["id"].toString(),
                json["chat_id"].toString(),
                json["sender_id"].toString(),
                json["content"].toString(),
                json["message_type"].toString(),
                json["media_url"].toString(),
                json["reply_to_id"].toString(),
                json["is_edited"].toBool(),
                json["is_deleted"].toBool(),
                json["created_at"].toString(),
            });
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::editMessage(
        const QString &messageId,
        const QString &content,
        std::function<void(const MessageInfo &message)> &&done,
        ErrorCallback &&fail) {
    QJsonObject body;
    body["content"] = content;

    _restClient->patch(
        u"/api/v1/messages/"_q + messageId,
        QJsonDocument(body).toJson(),
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).object();
            done(MessageInfo{
                json["id"].toString(),
                json["chat_id"].toString(),
                json["sender_id"].toString(),
                json["content"].toString(),
                json["message_type"].toString(),
                json["media_url"].toString(),
                json["reply_to_id"].toString(),
                json["is_edited"].toBool(),
                json["is_deleted"].toBool(),
                json["created_at"].toString(),
            });
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::deleteMessage(
        const QString &messageId,
        std::function<void()> &&done,
        ErrorCallback &&fail) {
    _restClient->del(
        u"/api/v1/messages/"_q + messageId,
        [done = std::move(done)](const QByteArray &) {
            if (done) done();
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::searchUsers(
        const QString &query,
        std::function<void(const QVector<UserProfile> &users)> &&done,
        ErrorCallback &&fail) {
    _restClient->get(
        u"/api/v1/users/search/?q="_q + query,
        [done = std::move(done)](const QByteArray &response) {
            auto json = QJsonDocument::fromJson(response).array();
            QVector<UserProfile> users;
            users.reserve(json.size());
            for (const auto &item : json) {
                auto obj = item.toObject();
                users.append(UserProfile{
                    obj["id"].toString(),
                    obj["email"].toString(),
                    obj["username"].toString(),
                    obj["display_name"].toString(),
                    obj["avatar_url"].toString(),
                    obj["is_active"].toBool(),
                    obj["created_at"].toString(),
                });
            }
            done(users);
        },
        [fail = std::move(fail)](const QString &error) {
            if (fail) fail(error);
        });
}

void CustomApi::markRead(
        const QString &chatId,
        const QString &messageId,
        std::function<void()> &&done,
        ErrorCallback &&fail) {
    if (_wsClient->isConnected()) {
        _wsClient->sendReadReceipt(chatId, messageId);
        if (done) done();
    } else {
        if (fail) fail(u"WebSocket not connected"_q);
    }
}

void CustomApi::uploadFile(
        const QString &filePath,
        std::function<void(const QString &fileId, const QString &url)> &&done,
        ErrorCallback &&fail,
        std::function<void(float progress)> &&progress) {
    auto transfer = new FileTransfer(_restClient, this);
    connect(transfer, &FileTransfer::uploadFinished, this, [transfer, done = std::move(done)](const QString &fileId, const QString &url) {
        if (done) done(fileId, url);
        transfer->deleteLater();
    });
    connect(transfer, &FileTransfer::error, this, [transfer, fail = std::move(fail)](const QString &error) {
        if (fail) fail(error);
        transfer->deleteLater();
    });
    if (progress) {
        connect(transfer, &FileTransfer::uploadProgress, this, [progress](float p) { progress(p); });
    }
    transfer->upload(filePath);
}

void CustomApi::downloadFile(
        const QString &fileId,
        const QString &savePath,
        std::function<void()> &&done,
        ErrorCallback &&fail,
        std::function<void(float progress)> &&progress) {
    auto transfer = new FileTransfer(_restClient, this);
    connect(transfer, &FileTransfer::downloadFinished, this, [transfer, done = std::move(done)]() {
        if (done) done();
        transfer->deleteLater();
    });
    connect(transfer, &FileTransfer::error, this, [transfer, fail = std::move(fail)](const QString &error) {
        if (fail) fail(error);
        transfer->deleteLater();
    });
    if (progress) {
        connect(transfer, &FileTransfer::downloadProgress, this, [progress](float p) { progress(p); });
    }
    transfer->download(fileId, savePath);
}

void CustomApi::connectWebSocket() {
    if (!_session->accessToken().isEmpty()) {
        _wsClient->connectToServer(_session->accessToken());
    }
}

void CustomApi::disconnectWebSocket() {
    _wsClient->disconnect();
}

} // namespace CustomNetwork
