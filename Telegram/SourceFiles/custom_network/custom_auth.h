#pragma once

#include "custom_network/custom_api.h"
#include <QObject>

namespace CustomNetwork {

class CustomAuth : public QObject {
    Q_OBJECT

public:
    explicit CustomAuth(CustomApi *api, QObject *parent = nullptr);

    void login(
        const QString &email,
        const QString &password,
        bool rememberMe,
        std::function<void(const UserProfile &user)> &&done,
        std::function<void(const QString &error)> &&fail);

    void registerUser(
        const QString &email,
        const QString &username,
        const QString &displayName,
        const QString &password,
        std::function<void(const UserProfile &user)> &&done,
        std::function<void(const QString &error)> &&fail);

    void logout();
    [[nodiscard]] bool isAuthenticated() const;

private:
    CustomApi *_api;
};

} // namespace CustomNetwork
