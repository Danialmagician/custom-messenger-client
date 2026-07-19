#pragma once

#include "custom_network/custom_api.h"
#include <QString>

namespace Data {
class User;
class Peer;
} // namespace Data

namespace CustomNetwork {

class DataAdapter {
public:
    static Data::User *adaptUser(
        Main::Session *session,
        const UserProfile &profile);

    static QString adaptChatType(const QString &backendType);
    static QString adaptMessageType(const QString &backendType);
};

} // namespace CustomNetwork
