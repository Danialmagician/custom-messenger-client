/*
Custom Messenger - Data Adapter Implementation
*/
#include "custom_network/data_adapter.h"

namespace Custom {

MTPUser DataAdapter::buildUser(const QJsonObject &json) {
	const auto id = getUserId(json);
	const auto username = getString(json, "username").toUtf8();
	const auto firstName = getString(json, "first_name").toUtf8();
	const auto lastName = getString(json, "last_name").toUtf8();
	const auto phone = getString(json, "phone").toUtf8();
	const auto statusStr = getString(json, "status");

	const auto hasPhone = !phone.isEmpty();
	const auto hasFirstName = !firstName.isEmpty();
	const auto hasLastName = !lastName.isEmpty();
	const auto hasUsername = !username.isEmpty();

	using Flag = MTPDuser::Flag;
	const auto flags = MTP_flags(
		Flag::f_self
		| (hasPhone ? Flag::f_phone : Flag())
		| (hasFirstName ? Flag::f_first_name : Flag())
		| (hasLastName ? Flag::f_last_name : Flag())
		| (hasUsername ? Flag::f_username : Flag()));

	MTPUserStatus status;
	if (statusStr == "online") {
		status = MTP_userStatusOnline(MTP_int(0));
	} else if (statusStr == "offline") {
		status = MTP_userStatusOffline(MTP_int(0));
	} else {
		status = MTP_userStatusRecently(MTP_flags());
	}

	return MTP_user(
		flags,
		MTP_long(id),
		MTPlong(),
		MTP_string(firstName),
		MTP_string(lastName),
		MTP_string(username),
		MTP_string(phone),
		MTPUserProfilePhoto(),
		status,
		MTPint(),
		MTPVector<MTPRestrictionReason>(),
		MTPstring(),
		MTPstring(),
		MTPEmojiStatus(),
		MTPVector<MTPUsername>(),
		MTPRecentStory(),
		MTPPeerColor(),
		MTPPeerColor(),
		MTPint(),
		MTPlong(),
		MTPlong(),
		MTPlong());
}

UserId DataAdapter::getUserId(const QJsonObject &json) {
	const auto idStr = json["id"].toString();
	bool ok = false;
	const auto id = idStr.toLongLong(&ok);
	return ok ? UserId(id) : UserId(0);
}

QString DataAdapter::getString(const QJsonObject &json, const QString &key) {
	return json[key].toString();
}

int DataAdapter::getInt(const QJsonObject &json, const QString &key, int defaultValue) {
	const auto val = json[key];
	return val.isDouble() ? val.toInt() : defaultValue;
}

bool DataAdapter::getBool(const QJsonObject &json, const QString &key, bool defaultValue) {
	const auto val = json[key];
	return val.isBool() ? val.toBool() : defaultValue;
}

} // namespace Custom
