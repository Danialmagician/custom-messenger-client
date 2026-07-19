#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include <optional>

namespace CustomNetwork {

struct UserProfile {
    QString id;
    QString email;
    QString username;
    QString displayName;
    QString avatarUrl;
    bool isActive = true;
    QString createdAt;
};

struct ChatInfo {
    QString id;
    QString title;
    QString description;
    QString chatType;
    QString avatarUrl;
    QString createdBy;
    QString createdAt;
    int unreadCount = 0;
};

struct MessageInfo {
    QString id;
    QString chatId;
    QString senderId;
    QString content;
    QString messageType;
    QString mediaUrl;
    QString replyToId;
    bool isEdited = false;
    bool isDeleted = false;
    QString createdAt;
};

struct TokenPair {
    QString accessToken;
    QString refreshToken;
};

using SuccessCallback = std::function<void()>;
using ErrorCallback = std::function<void(const QString &error)>;

class RestClient;
class WebSocketClient;
class SessionManager;
class CustomAuth;
class FileTransfer;

class CustomApi : public QObject {
    Q_OBJECT

public:
    explicit CustomApi(QObject *parent = nullptr);
    ~CustomApi();

    void setBackendUrl(const QString &url);
    [[nodiscard]] QString backendUrl() const;

    void login(
        const QString &email,
        const QString &password,
        bool rememberMe,
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail);

    void registerUser(
        const QString &email,
        const QString &username,
        const QString &displayName,
        const QString &password,
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail);

    void refreshToken(
        std::function<void()> &&done,
        ErrorCallback &&fail);

    void logout();

    void getMe(
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail);

    void updateProfile(
        const QString &displayName,
        const std::optional<QString> &avatarUrl,
        std::function<void(const UserProfile &user)> &&done,
        ErrorCallback &&fail);

    void getChats(
        std::function<void(const QVector<ChatInfo> &chats)> &&done,
        ErrorCallback &&fail);

    void createChat(
        const QString &title,
        const QString &description,
        const QString &chatType,
        const QVector<QString> &memberIds,
        std::function<void(const ChatInfo &chat)> &&done,
        ErrorCallback &&fail);

    void getChat(
        const QString &chatId,
        std::function<void(const ChatInfo &chat)> &&done,
        ErrorCallback &&fail);

    void getMessages(
        const QString &chatId,
        int offset,
        int limit,
        std::function<void(const QVector<MessageInfo> &messages)> &&done,
        ErrorCallback &&fail);

    void sendMessage(
        const QString &chatId,
        const QString &content,
        const QString &messageType,
        const std::optional<QString> &mediaUrl,
        const std::optional<QString> &replyToId,
        std::function<void(const MessageInfo &message)> &&done,
        ErrorCallback &&fail);

    void editMessage(
        const QString &messageId,
        const QString &content,
        std::function<void(const MessageInfo &message)> &&done,
        ErrorCallback &&fail);

    void deleteMessage(
        const QString &messageId,
        std::function<void()> &&done,
        ErrorCallback &&fail);

    void searchUsers(
        const QString &query,
        std::function<void(const QVector<UserProfile> &users)> &&done,
        ErrorCallback &&fail);

    void markRead(
        const QString &chatId,
        const QString &messageId,
        std::function<void()> &&done,
        ErrorCallback &&fail);

    void uploadFile(
        const QString &filePath,
        std::function<void(const QString &fileId, const QString &url)> &&done,
        ErrorCallback &&fail,
        std::function<void(float progress)> &&progress = nullptr);

    void downloadFile(
        const QString &fileId,
        const QString &savePath,
        std::function<void()> &&done,
        ErrorCallback &&fail,
        std::function<void(float progress)> &&progress = nullptr);

    [[nodiscard]] RestClient *restClient() const { return _restClient; }
    [[nodiscard]] WebSocketClient *wsClient() const { return _wsClient; }
    [[nodiscard]] SessionManager *session() const { return _session; }

    void connectWebSocket();
    void disconnectWebSocket();

signals:
    void messageReceived(const MessageInfo &message);
    void typingReceived(const QString &userId, const QString &chatId);
    void readReceiptReceived(const QString &userId, const QString &chatId, const QString &messageId);
    void presenceReceived(const QString &userId, const QString &status);
    void notificationReceived(const QJsonObject &notification);
    void connected();
    void disconnected();

private:
    QString _backendUrl;
    RestClient *_restClient = nullptr;
    WebSocketClient *_wsClient = nullptr;
    SessionManager *_session = nullptr;
};

} // namespace CustomNetwork
