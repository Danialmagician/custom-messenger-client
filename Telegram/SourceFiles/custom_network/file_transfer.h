#pragma once

#include <QObject>
#include <QString>

namespace CustomNetwork {

class RestClient;

class FileTransfer : public QObject {
    Q_OBJECT

public:
    explicit FileTransfer(RestClient *client, QObject *parent = nullptr);

    void upload(const QString &filePath);
    void download(const QString &fileId, const QString &savePath);
    void cancel();

signals:
    void uploadFinished(const QString &fileId, const QString &url);
    void downloadFinished();
    void uploadProgress(float progress);
    void downloadProgress(float progress);
    void error(const QString &message);

private:
    RestClient *_client;
    bool _cancelled = false;
};

} // namespace CustomNetwork
