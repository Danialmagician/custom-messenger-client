#pragma once

#ifdef CUSTOM_BACKEND

#include "custom_network/custom_api.h"
#include "custom_network/custom_auth.h"
#include "custom_network/websocket_client.h"
#include "custom_network/session_manager.h"
#include "custom_network/notification_handler.h"
#include "custom_network/file_transfer.h"
#include "custom_network/data_adapter.h"

namespace Karagram {

class Backend {
public:
    static Backend &instance();

    void initialize(const QString &backendUrl);
    void shutdown();

    [[nodiscard]] CustomNetwork::CustomApi &api() { return *_api; }
    [[nodiscard]] CustomNetwork::CustomAuth &auth() { return *_auth; }
    [[nodiscard]] CustomNetwork::WebSocketClient &ws() { return *_ws; }
    [[nodiscard]] CustomNetwork::SessionManager &session() { return *_session; }
    [[nodiscard]] CustomNetwork::NotificationHandler &notifications() { return *_notifications; }

    [[nodiscard]] bool isInitialized() const { return _initialized; }

private:
    Backend() = default;
    ~Backend() = default;
    Backend(const Backend &) = delete;
    Backend &operator=(const Backend &) = delete;

    bool _initialized = false;
    CustomNetwork::CustomApi *_api = nullptr;
    CustomNetwork::CustomAuth *_auth = nullptr;
    CustomNetwork::WebSocketClient *_ws = nullptr;
    CustomNetwork::SessionManager *_session = nullptr;
    CustomNetwork::NotificationHandler *_notifications = nullptr;
};

} // namespace Karagram

#endif // CUSTOM_BACKEND
