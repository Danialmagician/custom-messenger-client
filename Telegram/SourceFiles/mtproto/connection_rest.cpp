/*
Custom Messenger - REST Connection Implementation
*/
#include "mtproto/connection_rest.h"

#include <QtNetwork/QNetworkRequest>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>

namespace MTP {
namespace details {

RestConnection::RestConnection(QThread *thread, const ProxyData &proxy)
	: AbstractConnection(thread, proxy) {
	_backendUrl = QString::fromLocal8Bit(
		qgetenv("CUSTOM MESSENGER_API_URL"));
	if (_backendUrl.isEmpty()) {
		_backendUrl = u"http://localhost:8000/api/v1"_q;
	}
}

RestConnection::~RestConnection() = default;

ConnectionPointer RestConnection::clone(const ProxyData &proxy) {
	return ConnectionPointer::New<RestConnection>(
		thread(),
		proxy);
}

void RestConnection::sendData(mtpBuffer &&buffer) {
	Q_UNUSED(buffer);
}

void RestConnection::disconnectFromServer() {
	_connected = false;
}

void RestConnection::connectToServer(
		const QString &ip,
		int port,
		const bytes::vector &protocolSecret,
		int16 protocolDcId,
		bool protocolForFiles) {
	Q_UNUSED(ip);
	Q_UNUSED(port);
	Q_UNUSED(protocolSecret);
	Q_UNUSED(protocolDcId);
	Q_UNUSED(protocolForFiles);

	_connected = true;
	emit connected();
}

bool RestConnection::isConnected() const {
	return _connected;
}

crl::time RestConnection::pingTime() const {
	return 0;
}

crl::time RestConnection::fullConnectTimeout() const {
	return 30000;
}

int32 RestConnection::debugState() const {
	return _connected ? 1 : 0;
}

QString RestConnection::transport() const {
	return u"REST"_q;
}

QString RestConnection::tag() const {
	return u"custom-rest"_q;
}

void RestConnection::setBackendUrl(const QString &url) {
	_backendUrl = url;
}

void RestConnection::handleReply(QNetworkReply *reply) {
	if (reply->error() != QNetworkReply::NoError) {
		emit error(kErrorCodeOther);
		return;
	}
	const auto data = reply->readAll();
	Q_UNUSED(data);
}

} // namespace details
} // namespace MTP
