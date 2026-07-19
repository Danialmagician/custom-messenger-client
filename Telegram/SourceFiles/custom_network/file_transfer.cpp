#include "custom_network/file_transfer.h"
#include "custom_network/rest_client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>

namespace CustomNetwork {

FileTransfer::FileTransfer(RestClient *client, QObject *parent)
    : QObject(parent)
    , _client(client)
{
}

void FileTransfer::upload(const QString &filePath) {
    _cancelled = false;
    QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        emit error(u"File not found: "_q + filePath);
        return;
    }

    _client->upload(
        u"/api/v1/files/upload"_q,
        filePath,
        u"file"_q,
        [this](const QByteArray &response) {
            if (_cancelled) return;
            auto json = QJsonDocument::fromJson(response).object();
            auto fileId = json["id"].toString();
            auto url = json["url"].toString();
            emit uploadFinished(fileId, url);
        },
        [this](const QString &error) {
            if (_cancelled) return;
            emit this->error(error);
        },
        [this](qint64 sent, qint64 total) {
            if (_cancelled || total <= 0) return;
            emit uploadProgress(static_cast<float>(sent) / static_cast<float>(total));
        });
}

void FileTransfer::download(const QString &fileId, const QString &savePath) {
    _cancelled = false;
    auto reply = _client->manager()->get(
        QNetworkRequest(QUrl(_client->manager()->property("baseUrl").toString()
            + u"/api/v1/files/"_q + fileId + u"/download"_q)));

    connect(reply, &QNetworkReply::downloadProgress, this,
        [this](qint64 received, qint64 total) {
            if (_cancelled || total <= 0) return;
            emit downloadProgress(static_cast<float>(received) / static_cast<float>(total));
        });

    connect(reply, &QNetworkReply::finished, this, [this, reply, savePath]() {
        reply->deleteLater();
        if (_cancelled) return;

        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            return;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            emit error(u"Could not write to: "_q + savePath);
            return;
        }
        file.write(reply->readAll());
        file.close();
        emit downloadFinished();
    });
}

void FileTransfer::cancel() {
    _cancelled = true;
}

} // namespace CustomNetwork
