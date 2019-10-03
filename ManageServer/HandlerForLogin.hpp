#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Connect2SQLite.hpp"

//****************************************************************************
//*                     HandlerForLogin
//****************************************************************************
class HandlerForLogin {
public:
	void handle_login(
		  const std::string& user_email_address
		, const std::string& user_password
		, std::shared_ptr<Connect2SQLite> pSqlite
		, std::string& response_payload
	)
	{
		// check for registered user_email_address
		if (pSqlite->selectFromTable_(
			"user_email_address",
			user_email_address) == 0)
			;
		else {
			response_payload += "email address is unknown, try again.";
			return;
		}
		// check for correct user_password
		if (pSqlite->selectFromTable_(
			"user_email_address",
			user_email_address,
			"user_password",
			user_password) == 0)
		{
			response_payload += "succeeded.";
			return;
		}
		else {
			response_payload += "password is incorrect.";
			return;
		}
	}
};