/*
Custom Messenger - Data Adapter
Converts custom API JSON responses to Telegram's MTP types.
This allows the existing UI code to display data from our backend.
*/
#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include "mtproto/scheme.h"

namespace Custom {

class DataAdapter {
public:
	[[nodiscard]] static MTPUser buildUser(const QJsonObject &json);
	[[nodiscard]] static UserId getUserId(const QJsonObject &json);
	[[nodiscard]] static QString getString(const QJsonObject &json, const QString &key);
	[[nodiscard]] static int getInt(const QJsonObject &json, const QString &key, int defaultValue = 0);
	[[nodiscard]] static bool getBool(const QJsonObject &json, const QString &key, bool defaultValue = false);
};

} // namespace Custom
