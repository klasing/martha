#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Connect2SQLite.hpp"
#include "SmtpClient.hpp"

//****************************************************************************
//*                     HandlerForResetPassword
//****************************************************************************
class HandlerForResetPassword {
	// this could be a dangerous variable, has to be seen!
	std::string generated_code = "";
	SmtpClient smtpClient;
public:
	void handle_reset_password(const std::string& user_email_address
		, const std::string& user_password
		, std::shared_ptr<Connect2SQLite> pSqlite
		, std::string& response_payload
	)
	{
		// check for registered user_email_address
		if (pSqlite->selectFromTable_(
			"user_email_address",
			user_email_address) == -1) {
			response_payload += "email address is unknown, try again.";
			return;
		}
		response_payload += "enter the code received by email.";
		// generate random string
		generated_code = generate_random_string();
		std::string message_with_code = "code " + generated_code;
		// send generated string from klasingsmtp@gmail.com
		// to user_email_address of registered user
		smtpClient.smtp_client(
			"klasingsmtp@gmail.com",
			user_email_address,
			"klasingsmtp@gmail.com",
			user_email_address,
			"no subject",
			message_with_code);
	}
	void handle_reset_password_confirm(
		const std::string& user_email_address,
		const std::string& user_password,
		const std::string& code_received_from_user,
		std::shared_ptr<Connect2SQLite> pSqlite,
		std::string& response_payload)
	{
		// verify code given by user with generated string
		if (code_received_from_user == generated_code) {
			pSqlite->updateUserPassword(
				user_email_address,
				user_password
			);
			response_payload += " password is reset.";
		}
		else
			response_payload += " the given code is not correct.";
	}
};