/*
Custom Messenger - Custom Auth Module
Replaces Telegram's phone-based auth with email/password auth.
*/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QJsonObject>

namespace Custom {

class ApiFacade;

class AuthModule : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loggedInChanged)
	Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)
	Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
	Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
	AuthModule(ApiFacade *api, QObject *parent = nullptr);

	[[nodiscard]] bool isLoggedIn() const;
	[[nodiscard]] QString userId() const;
	[[nodiscard]] QString username() const;
	[[nodiscard]] QString error() const;

	void login(const QString &email, const QString &password);
	void registerUser(const QString &username, const QString &email, const QString &password);
	void logout();
	void checkAutoLogin();

signals:
	void loggedInChanged();
	void userIdChanged();
	void usernameChanged();
	void errorChanged();
	void loginSuccess();
	void registerSuccess();

private slots:
	void onLoginSuccess(const QJsonObject &user, const QJsonObject &tokens);
	void onRegisterSuccess(const QJsonObject &user, const QJsonObject &tokens);
	void onMeLoaded(const QJsonObject &user);
	void onTokenRefreshed(const QJsonObject &tokens);
	void onError(const QString &error);
	void onLoginFailed(const QString &error);
	void onRegisterFailed(const QString &error);

private:
	ApiFacade *_api;
	bool _loggedIn;
	QString _userId;
	QString _username;
	QString _error;
};

} // namespace Custom
