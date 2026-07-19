/*
Custom Messenger - Email Registration Step Implementation
*/
#include "intro/intro_email_register.h"

#include "lang/lang_keys.h"
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

EmailRegisterWidget::EmailRegisterWidget(
	QWidget *parent,
	not_null<Main::Account*> account,
	not_null<Data*> data)
: Step(parent, account, data) {
	setTitleText(rpl::single(u"Create Account"_q));
	setDescriptionText(rpl::single(u"Fill in your details to register."_q));
	setErrorCentered(true);

	_username = object_ptr<Ui::InputField>(
		this, st::introPhone, Ui::InputField::Mode::SingleLine, QString());
	_email = object_ptr<Ui::InputField>(
		this, st::introPhone, Ui::InputField::Mode::SingleLine, QString());
	_password = object_ptr<Ui::InputField>(
		this, st::introPhone, Ui::InputField::Mode::SingleLine, QString());
	_confirmPassword = object_ptr<Ui::InputField>(
		this, st::introPhone, Ui::InputField::Mode::SingleLine, QString());

	_username->setPlaceholderText(u"Username"_q);
	_email->setPlaceholderText(u"Email"_q);
	_password->setPlaceholderText(u"Password (min 8 chars)"_q);
	_password->setEchoMode(QLineEdit::Password);
	_confirmPassword->setPlaceholderText(u"Confirm Password"_q);
	_confirmPassword->setEchoMode(QLineEdit::Password);

	addChild(_username);
	addChild(_email);
	addChild(_password);
	addChild(_confirmPassword);

	connect(_username, &Ui::InputField::submitted, [=] { _email->setFocus(); });
	connect(_email, &Ui::InputField::submitted, [=] { _password->setFocus(); });
	connect(_password, &Ui::InputField::submitted, [=] { _confirmPassword->setFocus(); });
	connect(_confirmPassword, &Ui::InputField::submitted, [=] { submit(); });

	_username->show();
	_email->show();
	_password->show();
	_confirmPassword->show();
}

QString EmailRegisterWidget::accessibilityName() {
	return u"Registration"_q;
}

void EmailRegisterWidget::setInnerFocus() {
	_username->setFocusFast();
}

void EmailRegisterWidget::activate() {
	_username->selectAll();
}

void EmailRegisterWidget::finished() {
}

void EmailRegisterWidget::cancelled() {
}

void EmailRegisterWidget::submit() {
	submitRegistration();
}

rpl::producer<QString> EmailRegisterWidget::nextButtonText() const {
	return rpl::single(u"Register"_q);
}

void EmailRegisterWidget::resizeEvent(QResizeEvent *e) {
	Step::resizeEvent(e);

	const auto fieldWidth = st::introPhone.width;
	const auto fieldHeight = st::introPhone.height;
	const auto gap = st::introPhoneTop;

	const auto left = (width() - fieldWidth) / 2;
	const auto top = contentTop() + st::introDescriptionTop + gap;

	_username->setGeometry(left, top, fieldWidth, fieldHeight);
	_email->setGeometry(left, top + (fieldHeight + gap), fieldWidth, fieldHeight);
	_password->setGeometry(left, top + 2 * (fieldHeight + gap), fieldWidth, fieldHeight);
	_confirmPassword->setGeometry(left, top + 3 * (fieldHeight + gap), fieldWidth, fieldHeight);
}

void EmailRegisterWidget::submitRegistration() {
	if (_submitting) return;

	auto username = _username->text().trimmed();
	auto email = _email->text().trimmed();
	auto password = _password->text();
	auto confirmPassword = _confirmPassword->text();

	if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
		showError(rpl::single(u"Please fill in all fields"_q));
		return;
	}

	if (password != confirmPassword) {
		showError(rpl::single(u"Passwords do not match"_q));
		return;
	}

	if (password.length() < 8) {
		showError(rpl::single(u"Password must be at least 8 characters"_q));
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

	QObject::connect(api, &Custom::ApiFacade::registerSuccess, api,
		[=](const QJsonObject &user, const QJsonObject &) {
			if (weak) {
				_submitting = false;
				const auto mtpUser = Custom::DataAdapter::buildUser(user);
				finish(mtpUser);
			}
		});

	QObject::connect(api, &Custom::ApiFacade::registerFailed, api,
		[=](const QString &error) {
			if (weak) {
				_submitting = false;
				showError(rpl::single(error));
			}
		});

	api->registerUser(username, email, password);
}

} // namespace details
} // namespace Intro
