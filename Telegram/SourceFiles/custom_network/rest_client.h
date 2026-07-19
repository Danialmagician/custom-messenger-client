#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <functional>

namespace CustomNetwork {

class RestClient : public QObject {
    Q_OBJECT

public:
    explicit RestClient(QObject *parent = nullptr);

    void setBaseUrl(const QString &url);
    void setAuthToken(const QString &token);

    void get(
        const QString &path,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail);

    void post(
        const QString &path,
        const QByteArray &body,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail);

    void put(
        const QString &path,
        const QByteArray &body,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail);

    void patch(
        const QString &path,
        const QByteArray &body,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail);

    void del(
        const QString &path,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail);

    void upload(
        const QString &path,
        const QString &filePath,
        const QString &fieldName,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail,
        std::function<void(qint64 sent, qint64 total)> &&progress = nullptr);

    [[nodiscard]] QNetworkAccessManager *manager() const { return _manager; }

signals:
    void tokenExpired();

private:
    QNetworkRequest createRequest(const QString &path) const;
    void handleReply(
        QNetworkReply *reply,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail);

    QNetworkAccessManager *_manager;
    QString _baseUrl;
    QString _authToken;
};

} // namespace CustomNetwork
