/*
Custom Messenger - Email Login Step
Replaces the phone number input with email/password login.
This integrates with the existing Intro::Widget framework.
*/
#pragma once

#include "intro/intro_step.h"

namespace Ui {
class FlatLabel;
class LinkButton;
class RoundButton;
class InputField;
} // namespace Ui

namespace Intro {
namespace details {

class EmailLoginWidget final : public Step {
public:
	EmailLoginWidget(
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
		return false;
	}

	rpl::producer<QString> nextButtonText() const override;

protected:
	void resizeEvent(QResizeEvent *e) override;

private:
	void emailChanged();
	void submitEmail();
	void submitDone();
	void submitFail(const QString &error);

	object_ptr<Ui::InputField> _email;
	object_ptr<Ui::InputField> _password;

	QString _sentEmail;
	bool _submitting = false;

};

} // namespace details
} // namespace Intro
