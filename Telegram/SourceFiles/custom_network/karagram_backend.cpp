#ifdef CUSTOM_BACKEND

#include "custom_network/karagram_backend.h"
#include "custom_network/rest_client.h"

namespace Karagram {

Backend &Backend::instance() {
    static Backend backend;
    return backend;
}

void Backend::initialize(const QString &backendUrl) {
    if (_initialized) return;

    _api = new CustomNetwork::CustomApi(nullptr);
    _auth = new CustomNetwork::CustomAuth(_api, nullptr);
    _session = new CustomNetwork::SessionManager(nullptr);
    _notifications = new CustomNetwork::NotificationHandler(nullptr);

    _api->setBackendUrl(backendUrl);

    connect(_api, &CustomNetwork::CustomApi::messageReceived,
        _notifications, [this](const QJsonObject &msg) {
            Q_UNUSED(msg);
        });

    connect(_api, &CustomNetwork::CustomApi::connected, []() {
    });

    connect(_api, &CustomNetwork::CustomApi::disconnected, []() {
    });

    if (_session->isAuthenticated()) {
        _api->setBackendUrl(backendUrl);
        _api->connectWebSocket();
    }

    _initialized = true;
}

void Backend::shutdown() {
    if (!_initialized) return;

    _api->disconnectWebSocket();

    delete _notifications;
    delete _session;
    delete _auth;
    delete _api;

    _notifications = nullptr;
    _session = nullptr;
    _auth = nullptr;
    _api = nullptr;

    _initialized = false;
}

} // namespace Karagram

#endif // CUSTOM_BACKEND
