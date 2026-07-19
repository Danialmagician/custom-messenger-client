#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QJsonObject>

namespace CustomNetwork {

class WebSocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebSocketClient(QObject *parent = nullptr);
    ~WebSocketClient();

    void setUrl(const QString &baseUrl);
    void connectToServer(const QString &token);
    void disconnect();
    [[nodiscard]] bool isConnected() const;

    void sendTyping(const QString &chatId);
    void sendReadReceipt(const QString &chatId, const QString &messageId);
    void sendPresence(const QString &status);
    void joinChat(const QString &chatId);
    void leaveChat(const QString &chatId);

signals:
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject &message);
    void typingReceived(const QString &userId, const QString &chatId);
    void readReceiptReceived(const QString &userId, const QString &chatId, const QString &messageId);
    void presenceReceived(const QString &userId, const QString &status);
    void notificationReceived(const QJsonObject &notification);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onReconnectTimeout();

private:
    void sendJson(const QJsonObject &obj);

    QWebSocket _socket;
    QString _url;
    QString _token;
    QTimer _reconnectTimer;
    int _reconnectDelay = 1000;
    static constexpr int kMaxReconnectDelay = 30000;
};

} // namespace CustomNetwork
