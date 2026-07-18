/*
Custom Messenger - REST Data Loader
Fetches data from the custom REST backend and populates
the existing tdesktop data structures.
*/
#pragma once

#include "custom_network/api_facade.h"
#include "base/basic_types.h"
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <functional>

namespace Main {
class Session;
} // namespace Main

namespace Data {

class Session;

class RestDataLoader {
public:
	RestDataLoader(not_null<Main::Session*> session);

	void loadDialogs();
	void loadMessages(const QString &chatId, int limit = 50);
	void loadGroups();
	void loadChannels();
	void loadContacts();
	void loadUser(const QString &userId);

private:
	not_null<Main::Session*> _session;
	Custom::ApiFacade *_api;
};

} // namespace Data
