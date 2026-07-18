/*
Custom Messenger - REST File Transfer Implementation
*/
#include "custom_network/rest_file_transfer.h"
#include "custom_network/api_facade.h"
#include "custom_network/mtp_override.h"

namespace Custom {

RestFileTransfer::RestFileTransfer(QObject *parent)
	: QObject(parent) {
}

void RestFileTransfer::setApi(ApiFacade *api) {
	_api = api;
}

void RestFileTransfer::uploadFile(const QString &filePath) {
	if (!_api) {
		_api = MtpOverride::Instance().api();
	}
	if (!_api) {
		emit uploadFailed(u"API not initialized"_q);
		return;
	}
	_api->uploadFile(filePath);
}

void RestFileTransfer::downloadFile(const QString &fileId, const QString &savePath) {
	Q_UNUSED(fileId);
	Q_UNUSED(savePath);
}

} // namespace Custom
