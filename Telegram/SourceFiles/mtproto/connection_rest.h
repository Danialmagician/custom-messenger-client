/*
Custom Messenger - REST Connection
Replaces TCP/HTTP connections to Telegram servers.
Routes all data through the custom REST backend instead.
*/
#pragma once

#include "mtproto/connection_abstract.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace MTP {
namespace details {

class RestConnection final : public AbstractConnection {
public:
	RestConnection(QThread *thread, const ProxyData &proxy);
	~RestConnection();

	ConnectionPointer clone(const ProxyData &proxy) override;

	void sendData(mtpBuffer &&buffer) override;
	void disconnectFromServer() override;
	void connectToServer(
		const QString &ip,
		int port,
		const bytes::vector &protocolSecret,
		int16 protocolDcId,
		bool protocolForFiles) override;

	[[nodiscard]] bool isConnected() const override;
	[[nodiscard]] crl::time pingTime() const override;
	[[nodiscard]] crl::time fullConnectTimeout() const override;

	[[nodiscard]] int32 debugState() const override;
	[[nodiscard]] QString transport() const override;
	[[nodiscard]] QString tag() const override;

	void setBackendUrl(const QString &url);

private:
	void handleReply(QNetworkReply *reply);

	QNetworkAccessManager _manager;
	QString _backendUrl;
	bool _connected = false;
};

} // namespace details
} // namespace MTP
