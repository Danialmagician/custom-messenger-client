/*
Custom Messenger - MTP Instance Override Implementation
*/
#include "custom_network/mtp_override.h"

namespace Custom {

MtpOverride &MtpOverride::Instance() {
	static MtpOverride instance;
	return instance;
}

void MtpOverride::initialize(const QString &apiUrl, const QString &wsUrl) {
	if (_initialized) return;

	_api = new ApiFacade();
	_ws = new WebSocketClient(wsUrl);

	_api->initialize(apiUrl, wsUrl);
	_initialized = true;
}

void MtpOverride::setAccessToken(const QString &token) {
	if (_api) _api->setTokens(token, QString());
	if (_ws && !token.isEmpty()) _ws->connectToServer(token);
}

void MtpOverride::clearTokens() {
	if (_api) _api->clearTokens();
	if (_ws) _ws->disconnect();
}

} // namespace Custom
