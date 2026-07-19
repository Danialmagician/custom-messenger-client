#include "custom_network/session_manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

namespace CustomNetwork {

namespace {

constexpr auto kTokenFile = "karagram_session.json";

QString sessionFilePath() {
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    return path + u"/"_q + kTokenFile;
}

} // namespace

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , _settings(u"Karagram"_q, u"Karagram"_q)
{
    connect(&_refreshTimer, &QTimer::timeout, this, &SessionManager::checkTokenExpiry);
    loadFromStorage();
}

void SessionManager::setTokens(const QString &accessToken, const QString &refreshToken) {
    _accessToken = accessToken;
    _refreshToken = refreshToken;
    saveToStorage();
    _refreshTimer.start(60000);
}

QString SessionManager::accessToken() const {
    return _accessToken;
}

QString SessionManager::refreshToken() const {
    return _refreshToken;
}

void SessionManager::clear() {
    _accessToken.clear();
    _refreshToken.clear();
    _userId.clear();
    _refreshTimer.stop();
    QFile::remove(sessionFilePath());
}

bool SessionManager::isAuthenticated() const {
    return !_accessToken.isEmpty();
}

bool SessionManager::needsRefresh() const {
    if (_accessToken.isEmpty()) return false;
    auto parts = _accessToken.split(u'.');
    if (parts.size() != 3) return false;
    auto payload = QByteArray::fromBase64(parts[1].toUtf8());
    auto json = QJsonDocument::fromJson(payload).object();
    auto exp = json["exp"].toVariant().toLongLong();
    auto now = QDateTime::currentSecsSinceEpoch();
    return (exp - now) < 300;
}

void SessionManager::loadFromStorage() {
    QFile file(sessionFilePath());
    if (!file.open(QIODevice::ReadOnly)) return;
    auto json = QJsonDocument::fromJson(file.readAll()).object();
    _accessToken = json["access_token"].toString();
    _refreshToken = json["refresh_token"].toString();
    _userId = json["user_id"].toString();
    if (!_accessToken.isEmpty()) {
        _refreshTimer.start(60000);
    }
}

void SessionManager::saveToStorage() {
    QJsonObject json;
    json["access_token"] = _accessToken;
    json["refresh_token"] = _refreshToken;
    json["user_id"] = _userId;
    QFile file(sessionFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(json).toJson());
    }
}

void SessionManager::setUserId(const QString &userId) {
    _userId = userId;
    saveToStorage();
}

QString SessionManager::userId() const {
    return _userId;
}

void SessionManager::checkTokenExpiry() {
    if (needsRefresh()) {
        emit tokenRefreshNeeded();
    }
}

} // namespace CustomNetwork
