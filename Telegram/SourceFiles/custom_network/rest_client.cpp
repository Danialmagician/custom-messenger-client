#include "custom_network/rest_client.h"
#include <QJsonDocument>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

namespace CustomNetwork {

RestClient::RestClient(QObject *parent)
    : QObject(parent)
    , _manager(new QNetworkAccessManager(this))
{
}

void RestClient::setBaseUrl(const QString &url) {
    _baseUrl = url;
}

void RestClient::setAuthToken(const QString &token) {
    _authToken = token;
}

QNetworkRequest RestClient::createRequest(const QString &path) const {
    QNetworkRequest request(QUrl(_baseUrl + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/json"_q);
    if (!_authToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    }
    return request;
}

void RestClient::get(
        const QString &path,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail) {
    auto reply = _manager->get(createRequest(path));
    handleReply(reply, std::move(done), std::move(fail));
}

void RestClient::post(
        const QString &path,
        const QByteArray &body,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail) {
    auto reply = _manager->post(createRequest(path), body);
    handleReply(reply, std::move(done), std::move(fail));
}

void RestClient::put(
        const QString &path,
        const QByteArray &body,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail) {
    auto reply = _manager->put(createRequest(path), body);
    handleReply(reply, std::move(done), std::move(fail));
}

void RestClient::patch(
        const QString &path,
        const QByteArray &body,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail) {
    auto reply = _manager->sendCustomRequest(createRequest(path), "PATCH", body);
    handleReply(reply, std::move(done), std::move(fail));
}

void RestClient::del(
        const QString &path,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail) {
    auto reply = _manager->deleteResource(createRequest(path));
    handleReply(reply, std::move(done), std::move(fail));
}

void RestClient::upload(
        const QString &path,
        const QString &filePath,
        const QString &fieldName,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail,
        std::function<void(qint64 sent, qint64 total)> &&progress) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (fail) fail(u"Could not open file: "_q + filePath);
        return;
    }

    auto fileInfo = QFileInfo(filePath);
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant(u"form-data; name=\"file\"; filename=\""_q + fileInfo.fileName() + u"\""_q));
    filePart.setBodyDevice(file.release());
    multiPart->append(filePart);

    auto request = createRequest(path);
    request.setRawHeader("Content-Type",
        ("multipart/form-data; boundary=" + multiPart->boundary()).toUtf8());

    auto reply = _manager->post(request, multiPart);
    multiPart->setParent(reply);

    if (progress) {
        connect(reply, &QNetworkReply::uploadProgress, this,
            [progress](qint64 sent, qint64 total) { progress(sent, total); });
    }

    handleReply(reply, std::move(done), std::move(fail));
}

void RestClient::handleReply(
        QNetworkReply *reply,
        std::function<void(const QByteArray &response)> &&done,
        std::function<void(const QString &error)> &&fail) {
    connect(reply, &QNetworkReply::finished, this, [this, reply, done = std::move(done), fail = std::move(fail)]() mutable {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() == QNetworkReply::AuthenticationRequiredError
                || reply->error() == QNetworkReply::ContentAccessDenied) {
                emit tokenExpired();
            }
            if (fail) {
                fail(reply->errorString());
            }
            return;
        }

        if (done) {
            done(reply->readAll());
        }
    });
}

} // namespace CustomNetwork
