#include "custom_network/data_adapter.h"

namespace CustomNetwork {

QString DataAdapter::adaptChatType(const QString &backendType) {
    if (backendType == u"private"_q) return u"private"_q;
    if (backendType == u"group"_q) return u"group"_q;
    if (backendType == u"channel"_q) return u"channel"_q;
    if (backendType == u"supergroup"_q) return u"supergroup"_q;
    return u"private"_q;
}

QString DataAdapter::adaptMessageType(const QString &backendType) {
    if (backendType == u"text"_q) return u"text"_q;
    if (backendType == u"photo"_q) return u"photo"_q;
    if (backendType == u"video"_q) return u"video"_q;
    if (backendType == u"audio"_q) return u"audio"_q;
    if (backendType == u"document"_q) return u"document"_q;
    if (backendType == u"sticker"_q) return u"sticker"_q;
    if (backendType == u"system"_q) return u"system"_q;
    return u"text"_q;
}

} // namespace CustomNetwork
