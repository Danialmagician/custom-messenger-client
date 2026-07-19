/*
Custom Messenger - REST Data Loader Implementation
*/
#include "custom_network/rest_data_loader.h"

#include "custom_network/mtp_override.h"
#include "custom_network/data_adapter.h"
#include "main/main_session.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "data/data_channel.h"
#include "data/data_chat.h"
#include "data/data_message.h"
#include "apiwrap.h"
#include "window/window_session_controller.h"

namespace Data {

RestDataLoader::RestDataLoader(not_null<Main::Session*> session)
	: _session(session)
	, _api(Custom::MtpOverride::Instance().api()) {
}

void RestDataLoader::loadDialogs() {
	if (!_api) return;

	_api->getMyGroups();
	QObject::connect(_api, &Custom::ApiFacade::groupsLoaded, _api,
		[=](const QJsonArray &groups) {
			for (const auto &group : groups) {
				const auto obj = group.toObject();
				const auto name = obj["name"].toString();
				const auto id = obj["id"].toString();
				Q_UNUSED(id);
				Q_UNUSED(name);
			}
		});

	_api->getMyChannels();
	QObject::connect(_api, &Custom::ApiFacade::channelsLoaded, _api,
		[=](const QJsonArray &channels) {
			for (const auto &channel : channels) {
				const auto obj = channel.toObject();
				const auto name = obj["name"].toString();
				const auto id = obj["id"].toString();
				Q_UNUSED(id);
				Q_UNUSED(name);
			}
		});
}

void RestDataLoader::loadMessages(const QString &chatId, int limit) {
	if (!_api) return;

	_api->getDirectMessages(chatId, limit);
}

void RestDataLoader::loadGroups() {
	if (!_api) return;

	_api->getMyGroups();
}

void RestDataLoader::loadChannels() {
	if (!_api) return;

	_api->getMyChannels();
}

void RestDataLoader::loadContacts() {
	if (!_api) return;

	_api->searchUsers("");
}

void RestDataLoader::loadUser(const QString &userId) {
	if (!_api) return;

	_api->getUser(userId);
}

} // namespace Data
