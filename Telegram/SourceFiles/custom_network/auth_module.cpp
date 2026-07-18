/*
Custom Messenger - Auth Module Implementation
*/
#include "custom_network/auth_module.h"
#include "custom_network/api_facade.h"

namespace Custom {

AuthModule::AuthModule(ApiFacade *api, QObject *parent)
	: QObject(parent)
	, _api(api)
	, _loggedIn(false) {
	connect(_api, &ApiFacade::loginSuccess, this, &AuthModule::onLoginSuccess);
	connect(_api, &ApiFacade::registerSuccess, this, &AuthModule::onRegisterSuccess);
	connect(_api, &ApiFacade::meLoaded, this, &AuthModule::onMeLoaded);
	connect(_api, &ApiFacade::tokenRefreshed, this, &AuthModule::onTokenRefreshed);
	connect(_api, &ApiFacade::errorOccurred, this, &AuthModule::onError);
	connect(_api, &ApiFacade::loginFailed, this, &AuthModule::onLoginFailed);
	connect(_api, &ApiFacade::registerFailed, this, &AuthModule::onRegisterFailed);
}

bool AuthModule::isLoggedIn() const {
	return _loggedIn;
}

QString AuthModule::userId() const {
	return _userId;
}

QString AuthModule::username() const {
	return _username;
}

QString AuthModule::error() const {
	return _error;
}

void AuthModule::login(const QString &email, const QString &password) {
	_error.clear();
	emit errorChanged();
	_api->login(email, password);
}

void AuthModule::registerUser(const QString &username, const QString &email, const QString &password) {
	_error.clear();
	emit errorChanged();
	_api->registerUser(username, email, password);
}

void AuthModule::logout() {
	_api->logout();
	_loggedIn = false;
	_userId.clear();
	_username.clear();
	emit loggedInChanged();
	emit userIdChanged();
	emit usernameChanged();
}

void AuthModule::checkAutoLogin() {
	if (_api->isAuthenticated()) {
		_api->getMe();
	}
}

void AuthModule::onLoginSuccess(const QJsonObject &user, const QJsonObject &tokens) {
	Q_UNUSED(tokens);
	_userId = user["id"].toString();
	_username = user["username"].toString();
	_loggedIn = true;
	emit loggedInChanged();
	emit userIdChanged();
	emit usernameChanged();
	emit loginSuccess();
	_api->getMe();
}

void AuthModule::onRegisterSuccess(const QJsonObject &user, const QJsonObject &tokens) {
	Q_UNUSED(tokens);
	_userId = user["id"].toString();
	_username = user["username"].toString();
	_loggedIn = true;
	emit loggedInChanged();
	emit userIdChanged();
	emit usernameChanged();
	emit registerSuccess();
}

void AuthModule::onMeLoaded(const QJsonObject &user) {
	_userId = user["id"].toString();
	_username = user["username"].toString();
	emit userIdChanged();
	emit usernameChanged();
}

void AuthModule::onTokenRefreshed(const QJsonObject &tokens) {
	Q_UNUSED(tokens);
	_loggedIn = true;
	emit loggedInChanged();
	_api->getMe();
}

void AuthModule::onError(const QString &error) {
	_error = error;
	emit errorChanged();
}

void AuthModule::onLoginFailed(const QString &error) {
	_error = error;
	emit errorChanged();
}

void AuthModule::onRegisterFailed(const QString &error) {
	_error = error;
	emit errorChanged();
}

} // namespace Custom
