/*
Custom Messenger - Email Registration Step
Replaces the phone signup with email-based registration.
*/
#pragma once

#include "intro/intro_step.h"

namespace Ui {
class InputField;
class RoundButton;
} // namespace Ui

namespace Intro {
namespace details {

class EmailRegisterWidget final : public Step {
public:
	EmailRegisterWidget(
		QWidget *parent,
		not_null<Main::Account*> account,
		not_null<Data*> data);

	QString accessibilityName() override;

	void setInnerFocus() override;
	void activate() override;
	void finished() override;
	void cancelled() override;
	void submit() override;

	bool hasBack() const override {
		return true;
	}

	rpl::producer<QString> nextButtonText() const override;

protected:
	void resizeEvent(QResizeEvent *e) override;

private:
	void submitRegistration();
	void submitDone();
	void submitFail(const QString &error);

	object_ptr<Ui::InputField> _username;
	object_ptr<Ui::InputField> _email;
	object_ptr<Ui::InputField> _password;
	object_ptr<Ui::InputField> _confirmPassword;

	bool _submitting = false;

};

} // namespace details
} // namespace Intro
