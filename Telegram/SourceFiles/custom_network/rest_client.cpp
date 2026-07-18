/*
Custom Messenger - REST API Client
*/
#include "custom_network/rest_client.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QHttpPart>
#include <QFile>
#include <QFileInfo>

namespace Custom {

RestClient::RestClient(const QString &baseUrl, QObject *parent)
	: QObject(parent)
	, _baseUrl(baseUrl) {
}

RestClient::~RestClient() = default;

void RestClient::setAccessToken(const QString &token) {
	_accessToken = token;
	emit authenticatedChanged();
}

void RestClient::clearAccessToken() {
	_accessToken.clear();
	emit authenticatedChanged();
}

bool RestClient::isAuthenticated() const {
	return !_accessToken.isEmpty();
}

QNetworkRequest RestClient::createRequest(const QString &endpoint, bool auth) const {
	QUrl url(_baseUrl + endpoint);
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	if (auth && !_accessToken.isEmpty()) {
		request.setRawHeader("Authorization", ("Bearer " + _accessToken).toUtf8());
	}
	return request;
}

void RestClient::get(const QString &endpoint, SuccessCallback onsuccess, ErrorCallback onerror) {
	auto request = createRequest(endpoint);
	auto *reply = _manager.get(request);

	connect(reply, &QNetworkReply::finished, this, [=]() {
		if (reply->error() != QNetworkReply::NoError) {
			if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
				emit refreshTokenRequired();
			}
			auto errorData = reply->readAll();
			auto doc = QJsonDocument::fromJson(errorData);
			auto errorMsg = doc.object()["detail"].toString();
			onerror(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), errorMsg);
		} else {
			auto data = reply->readAll();
			auto doc = QJsonDocument::fromJson(data);
			onsuccess(doc.object());
		}
		reply->deleteLater();
	});
}

void RestClient::post(const QString &endpoint, const QJsonObject &body, SuccessCallback onsuccess, ErrorCallback onerror) {
	auto request = createRequest(endpoint);
	auto *reply = _manager.post(request, QJsonDocument(body).toJson());

	connect(reply, &QNetworkReply::finished, this, [=]() {
		if (reply->error() != QNetworkReply::NoError) {
			if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
				emit refreshTokenRequired();
			}
			auto errorData = reply->readAll();
			auto doc = QJsonDocument::fromJson(errorData);
			auto errorMsg = doc.object()["detail"].toString();
			onerror(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), errorMsg);
		} else {
			auto data = reply->readAll();
			auto doc = QJsonDocument::fromJson(data);
			onsuccess(doc.object());
		}
		reply->deleteLater();
	});
}

void RestClient::patch(const QString &endpoint, const QJsonObject &body, SuccessCallback onsuccess, ErrorCallback onerror) {
	auto request = createRequest(endpoint);
	auto *reply = _manager.sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());

	connect(reply, &QNetworkReply::finished, this, [=]() {
		if (reply->error() != QNetworkReply::NoError) {
			if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
				emit refreshTokenRequired();
			}
			auto errorData = reply->readAll();
			auto doc = QJsonDocument::fromJson(errorData);
			auto errorMsg = doc.object()["detail"].toString();
			onerror(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), errorMsg);
		} else {
			auto data = reply->readAll();
			auto doc = QJsonDocument::fromJson(data);
			onsuccess(doc.object());
		}
		reply->deleteLater();
	});
}

void RestClient::del(const QString &endpoint, SuccessCallback onsuccess, ErrorCallback onerror) {
	auto request = createRequest(endpoint);
	auto *reply = _manager.sendCustomRequest(request, "DELETE");

	connect(reply, &QNetworkReply::finished, this, [=]() {
		if (reply->error() != QNetworkReply::NoError) {
			if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
				emit refreshTokenRequired();
			}
			auto errorData = reply->readAll();
			auto doc = QJsonDocument::fromJson(errorData);
			auto errorMsg = doc.object()["detail"].toString();
			onerror(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), errorMsg);
		} else {
			auto data = reply->readAll();
			auto doc = QJsonDocument::fromJson(data);
			onsuccess(doc.object());
		}
		reply->deleteLater();
	});
}

void RestClient::upload(const QString &endpoint, const QString &filePath, const QByteArray &mimeType, SuccessCallback onsuccess, ErrorCallback onerror) {
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		onerror(0, "Cannot open file");
		return;
	}

	auto request = createRequest(endpoint);
	QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	QHttpPart filePart;
	filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
		QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(filePath).fileName() + "\""));
	filePart.setBodyDevice(&file);
	multiPart->append(filePart);

	auto *reply = _manager.post(request, multiPart);

	connect(reply, &QNetworkReply::finished, this, [=]() {
		if (reply->error() != QNetworkReply::NoError) {
			if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
				emit refreshTokenRequired();
			}
			auto errorData = reply->readAll();
			auto doc = QJsonDocument::fromJson(errorData);
			auto errorMsg = doc.object()["detail"].toString();
			onerror(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), errorMsg);
		} else {
			auto data = reply->readAll();
			auto doc = QJsonDocument::fromJson(data);
			onsuccess(doc.object());
		}
		reply->deleteLater();
		multiPart->deleteLater();
	});
}

} // namespace Custom
