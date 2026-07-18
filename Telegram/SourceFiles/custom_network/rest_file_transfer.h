/*
Custom Messenger - REST File Transfer
Replaces MTProto file upload/download with REST API calls.
*/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <functional>

namespace Custom {

class ApiFacade;

class RestFileTransfer : public QObject {
	Q_OBJECT

public:
	RestFileTransfer(QObject *parent = nullptr);

	void uploadFile(const QString &filePath);
	void downloadFile(const QString &fileId, const QString &savePath);
	void setApi(ApiFacade *api);

signals:
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void uploadFinished(const QJsonObject &fileInfo);
	void uploadFailed(const QString &error);
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void downloadFinished(const QString &filePath);
	void downloadFailed(const QString &error);

private:
	ApiFacade *_api = nullptr;
};

} // namespace Custom
