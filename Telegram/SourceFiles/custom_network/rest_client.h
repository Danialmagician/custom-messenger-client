/*
Custom Messenger - REST API Client
Replaces MTProto networking with standard HTTP/REST communication.
*/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <functional>

namespace Custom {

class RestClient : public QObject {
	Q_OBJECT

public:
	RestClient(const QString &baseUrl, QObject *parent = nullptr);
	~RestClient();

	void setAccessToken(const QString &token);
	void clearAccessToken();
	[[nodiscard]] bool isAuthenticated() const;

	using SuccessCallback = std::function<void(const QJsonObject &response)>;
	using ErrorCallback = std::function<void(int statusCode, const QString &error)>;

	void get(const QString &endpoint, SuccessCallback onsuccess, ErrorCallback onerror);
	void post(const QString &endpoint, const QJsonObject &body, SuccessCallback onsuccess, ErrorCallback onerror);
	void patch(const QString &endpoint, const QJsonObject &body, SuccessCallback onsuccess, ErrorCallback onerror);
	void del(const QString &endpoint, SuccessCallback onsuccess, ErrorCallback onerror);
	void upload(const QString &endpoint, const QString &filePath, const QByteArray &mimeType, SuccessCallback onsuccess, ErrorCallback onerror);

signals:
	void authenticatedChanged();
	void refreshTokenRequired();

private:
	QNetworkRequest createRequest(const QString &endpoint, bool auth = true) const;

	QNetworkAccessManager _manager;
	QString _baseUrl;
	QString _accessToken;
};

} // namespace Custom
