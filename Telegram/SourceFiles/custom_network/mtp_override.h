/*
Custom Messenger - MTP Instance Override
This file provides the hooks to redirect MTProto networking
through the custom REST backend.

Instead of modifying mtp_instance.cpp directly, this file provides
helper functions that can be called from the modified initialization code.
*/
#pragma once

#include "custom_network/api_facade.h"
#include "custom_network/websocket_client.h"

#include <QtCore/QString>

namespace Custom {

class MtpOverride {
public:
	static MtpOverride &Instance();

	void initialize(const QString &apiUrl, const QString &wsUrl);
	void setAccessToken(const QString &token);
	void clearTokens();

	[[nodiscard]] ApiFacade *api() const { return _api; }
	[[nodiscard]] WebSocketClient *ws() const { return _ws; }
	[[nodiscard]] bool isInitialized() const { return _initialized; }

private:
	MtpOverride() = default;
	ApiFacade *_api = nullptr;
	WebSocketClient *_ws = nullptr;
	bool _initialized = false;
};

} // namespace Custom
