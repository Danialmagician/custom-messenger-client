/*
Custom Messenger - Email Login Step Implementation
*/
#include "intro/intro_email_login.h"

#include "lang/lang_keys.h"
#include "intro/intro_email_register.h"
#include "intro/intro_step.h"
#include "styles/style_intro.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/fields/input_field.h"
#include "main/main_account.h"
#include "core/application.h"
#include "window/window_controller.h"
#include "custom_network/api_facade.h"
#include "custom_network/mtp_override.h"
#include "custom_network/data_adapter.h"

namespace Intro {
namespace details {

EmailLoginWidget::EmailLoginWidget(
	QWidget *parent,
	not_null<Main::Account*> account,
	not_null<Data*> data)
: Step(parent, account, data) {
	setTitleText(rpl::single(u"Sign In"_q));
	setDescriptionText(rpl::single(u"Enter your email and password to continue."_q));
	setErrorCentered(true);

	_email = object_ptr<Ui::InputField>(
		this,
		st::introPhone,
		Ui::InputField::Mode::SingleLine,
		QString());
	_password = object_ptr<Ui::InputField>(
		this,
		st::introPhone,
		Ui::InputField::Mode::SingleLine,
		QString());

	_email->setPlaceholderText(u"Email"_q);
	_password->setPlaceholderText(u"Password"_q);
	_password->setEchoMode(QLineEdit::Password);

	addChild(_email);
	addChild(_password);

	connect(_email, &Ui::InputField::submitted, [=] { _password->setFocus(); });
	connect(_password, &Ui::InputField::submitted, [=] { submit(); });

	_email->show();
	_password->show();
}

QString EmailLoginWidget::accessibilityName() {
	return u"Email Login"_q;
}

void EmailLoginWidget::setInnerFocus() {
	_email->setFocusFast();
}

void EmailLoginWidget::activate() {
	_email->selectAll();
}

void EmailLoginWidget::finished() {
}

void EmailLoginWidget::cancelled() {
}

void EmailLoginWidget::submit() {
	submitEmail();
}

rpl::producer<QString> EmailLoginWidget::nextButtonText() const {
	return rpl::single(u"Next"_q);
}

void EmailLoginWidget::resizeEvent(QResizeEvent *e) {
	Step::resizeEvent(e);
	updateControlsGeometry();

	const auto padding = st::introPhonePosition;
	const auto fieldWidth = st::introPhone.width;
	const auto fieldHeight = st::introPhone.height;
	const auto gap = st::introPhoneTop;

	const auto left = (width() - fieldWidth) / 2;
	const auto top = contentTop() + st::introDescriptionTop + gap;

	_email->setGeometry(left, top, fieldWidth, fieldHeight);
	_password->setGeometry(left, top + fieldHeight + gap, fieldWidth, fieldHeight);
}

void EmailLoginWidget::emailChanged() {
}

void EmailLoginWidget::submitEmail() {
	if (_submitting) return;

	auto email = _email->text().trimmed();
	auto password = _password->text();

	if (email.isEmpty() || password.isEmpty()) {
		showError(rpl::single(u"Please fill in all fields"_q));
		return;
	}

	_submitting = true;

	auto &override = Custom::MtpOverride::Instance();
	if (!override.isInitialized()) {
		override.initialize(
			u"http://localhost:8000/api/v1"_q,
			u"ws://localhost:8000/ws"_q);
	}

	auto api = override.api();
	const auto weak = base::make_weak(this);

	QObject::connect(api, &Custom::ApiFacade::loginSuccess, api,
		[=](const QJsonObject &user, const QJsonObject &) {
			if (weak) {
				_submitting = false;
				const auto mtpUser = Custom::DataAdapter::buildUser(user);
				finish(mtpUser);
			}
		});

	QObject::connect(api, &Custom::ApiFacade::loginFailed, api,
		[=](const QString &error) {
			if (weak) {
				_submitting = false;
				showError(rpl::single(error));
			}
		});

	api->login(email, password);
}

} // namespace details
} // namespace Intro
