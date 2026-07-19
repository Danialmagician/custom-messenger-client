#include "custom_network/custom_auth.h"
#include "custom_network/session_manager.h"

namespace CustomNetwork {

CustomAuth::CustomAuth(CustomApi *api, QObject *parent)
    : QObject(parent)
    , _api(api)
{
}

void CustomAuth::login(
        const QString &email,
        const QString &password,
        bool rememberMe,
        std::function<void(const UserProfile &user)> &&done,
        std::function<void(const QString &error)> &&fail) {
    _api->login(email, password, rememberMe, std::move(done), std::move(fail));
}

void CustomAuth::registerUser(
        const QString &email,
        const QString &username,
        const QString &displayName,
        const QString &password,
        std::function<void(const UserProfile &user)> &&done,
        std::function<void(const QString &error)> &&fail) {
    _api->registerUser(email, username, displayName, password, std::move(done), std::move(fail));
}

void CustomAuth::logout() {
    _api->logout();
}

bool CustomAuth::isAuthenticated() const {
    return _api->session()->isAuthenticated();
}

} // namespace CustomNetwork
