#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QSettings>

namespace CustomNetwork {

class SessionManager : public QObject {
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);

    void setTokens(const QString &accessToken, const QString &refreshToken);
    [[nodiscard]] QString accessToken() const;
    [[nodiscard]] QString refreshToken() const;
    void clear();

    [[nodiscard]] bool isAuthenticated() const;
    [[nodiscard]] bool needsRefresh() const;

    void loadFromStorage();
    void saveToStorage();
    void setUserId(const QString &userId);
    [[nodiscard]] QString userId() const;

signals:
    void tokenRefreshNeeded();

private:
    void checkTokenExpiry();

    QString _accessToken;
    QString _refreshToken;
    QString _userId;
    QTimer _refreshTimer;
    QSettings _settings;
};

} // namespace CustomNetwork
