#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "CppSQLite3.h"

using namespace std;
//****************************************************************************
//*                     typedef
//****************************************************************************
// typedef for sqlite
typedef std::string
	  td_id
	, td_time_of_creation
	, td_user_email_address
	, td_user_password
	, td_file_name
	, td_owner;
typedef std::tuple<
	td_id
	, td_time_of_creation
	, td_user_email_address
	, td_user_password> tuple_user_data;
typedef std::tuple<
	td_id
	, td_time_of_creation
	, td_file_name
	, td_owner> tuple_resource_data;

//****************************************************************************
//*                     Connect2SQLite
//****************************************************************************
class Connect2SQLite
{
	const int DATA_MAX = 128;
	char* data = new char[DATA_MAX];
	char* pszErrMsg = 0;
	int* pHighestKeyValue = new int(0);
	sqlite3* db = nullptr;
	int rc = -1;
	string sql = "";
	vector<tuple_user_data> vector_user_data;
	vector<tuple_resource_data> vector_resource_data;
	static string NAME_DB;
public:
	int openDb()
	{
		// open database
		rc = sqlite3_open("http_server_async_ssl.db", &db);
		return rc;
	}
	int closeDb()
	{
		// close database
		rc = sqlite3_close(db);
		// clean up
		delete[] data;
		data = nullptr;
		delete pHighestKeyValue;
		pHighestKeyValue = nullptr;
		return rc;
	}
	int createTableUser()
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		// create table
		sql =
			"CREATE TABLE IF NOT EXISTS user_access_login_data(" \
			"id                 INT PRIMARY KEY NOT NULL," \
			"time_of_creation	TEXT NOT NULL," \
			"user_email_address TEXT NOT NULL UNIQUE," \
			"user_password      TEXT NOT NULL" \
			");";
		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	int createTableResource()
	{
		Connect2SQLite::NAME_DB = "user_space_resource_data";
		// create table
		sql =
			"CREATE TABLE IF NOT EXISTS user_space_resource_data(" \
			"id                 INT PRIMARY KEY NOT NULL," \
			"time_of_creation	TEXT NOT NULL," \
			"resource_file_name	TEXT NOT NULL UNIQUE," \
			"resource_owner	    TEXT NOT NULL" \
			");";
		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	int insertDefaultUser(const std::string current_gmt)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		sql =
			"INSERT INTO user_access_login_data(" \
			"id, time_of_creation, user_email_address, user_password) " \
			"VALUES (" \
			"1, '" 
			+ current_gmt
			+ "', 'guest@example.com', 'anonymous'" \
			");";
		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	int selectFromTable_(const string& colName
		, const string& value
	)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		// begin every execution with an empty vector
		vector_user_data.clear();

		// select from table with: user_email_address
		sql = string("SELECT * FROM user_access_login_data ")
			+ "WHERE "
			+ colName
			+ " = "
			+ "'" + value + "'"
			+ ";";
		rc = execute(sql, &vector_user_data);
		if (vector_user_data.size() == 0)
			// no data found in database
			return -1;
		return rc;
	}
	int selectFromTable_(const string& user_email_address
		, const string& user_email_address_value
		, const string& user_password
		, const string& user_password_value)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		// begin every execution with an empty vector
		vector_user_data.clear();

		// select from table: user_email_address, and user_password
		sql = string("SELECT * FROM user_access_login_data ")
			+ "WHERE "
			+ user_email_address
			+ " = "
			+ "'" + user_email_address_value + "'"
			+ " AND "
			+ user_password
			+ " = "
			+ "'" + user_password_value + "'"
			+ ";";
		rc = execute(sql, &vector_user_data);
		if (vector_user_data.size() == 0)
			// no data found in database
			return -1;
		return rc;
	}
	int selectAll_user(const HWND& hWnd)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		// begin every execution with an empty vector
		vector_user_data.clear();

		sql = "SELECT * FROM user_access_login_data;";
		rc = execute(sql, &vector_user_data);
		SendMessage(hWnd
			, IDM_DB_USER_SELECT_ALL_CALLBACK
			, (WPARAM)rc
			, (LPARAM)& vector_user_data
		);
		return rc;
	}
	int deleteRow_user(const int& id)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		sql = string("DELETE FROM user_access_login_data WHERE id=")
			+ to_string(id)
			+ ";";
		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	int deleteRow(const std::string& table_descriptor
		, const int& id
	)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		if (table_descriptor == "user")
		{
			sql = string("DELETE FROM user_access_login_data WHERE id=")
				+ to_string(id)
				+ ";";
			// no data will be returned from this SQL execution
			return execute(sql, nullptr);
		}
		Connect2SQLite::NAME_DB = "user_space_resource_data";
		if (table_descriptor == "resource")
		{
			sql = string("DELETE FROM user_space_resource_data WHERE id=")
				+ to_string(id)
				+ ";";
			// no data will be returned from this SQL execution
			return execute(sql, nullptr);
		}
	}
	int insertRegisterUser(const std::string current_gmt
		, const string& user_email_address
		, const string& user_password
	)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		sql = "SELECT max(ID) FROM user_access_login_data;";
		// no data will be returned from this SQL execution
		// set the variable that holds the highest key
		// for the callback
		rc = execute(sql, pHighestKeyValue);
		string key = to_string(++(*pHighestKeyValue));

		// insert into table
		sql = string("INSERT INTO user_access_login_data(")
			+ "id, time_of_creation, user_email_address, user_password) VALUES ("
			+ key
			+ ", '"
			+ current_gmt
			+ "', '"
			+ user_email_address
			+ "', '"
			+ user_password
			+ "');";

		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	int updateUserPassword(const string& user_email_address
		, const string& user_password
	)
	{
		Connect2SQLite::NAME_DB = "user_access_login_data";
		//sql = "UPDATE COMPANY SET SALARY = 25000.00 WHERE ID = 1;";
		sql = string("UPDATE user_access_login_data SET user_password = ")
			+ "'"
			+ user_password
			+ "'"
			+ " WHERE user_email_address = "
			+ "'"
			+ user_email_address
			+ "'"
			+ ";";
		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	/////////////////////////////////////////////////////////////////////////////
	//                     resource
	/////////////////////////////////////////////////////////////////////////////
	int insertResource(const std::string current_gmt
		, const std::string& resource_file_name
		, const std::string& resource_owner
	)
	{
		Connect2SQLite::NAME_DB = "user_space_resource_data";
		string resource_owner_from_db = "";
		// check if file exist
		sql = string("SELECT resource_owner FROM user_space_resource_data ")
			+ "WHERE resource_file_name='"
			+ resource_file_name
			+ "';";
		rc = execute(sql, &resource_owner_from_db);
		if (resource_owner_from_db.length() != 0)
		{
			// file exists, update time_of_creation
			// TOC is Time Of Creation
			updateResourceTOC(current_gmt
				, resource_file_name
				, resource_owner
			); 
		}

		// search for next key value
		sql = "SELECT max(ID) FROM user_space_resource_data;";
		// no data will be returned from this SQL execution
		// set the variable that holds the highest key
		// for the callback
		rc = execute(sql, pHighestKeyValue);
		string key = to_string(++(*pHighestKeyValue));

		// insert into table
		sql = string("INSERT INTO user_space_resource_data(")
			+ "id, time_of_creation, resource_file_name, resource_owner) VALUES ("
			+ key
			+ ", '"
			+ current_gmt
			+ "', '"
			+ resource_file_name
			+ "', '"
			+ resource_owner
			+ "');";

		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	int updateResourceTOC(const std::string current_gmt
		, const std::string& resource_file_name
		, const std::string& resource_owner
	)
	{
		sql = string("UPDATE user_space_resource_data SET time_of_creation = ")
			+ "'"
			+ current_gmt
			+ "'"
			+ " WHERE resource_file_name = "
			+ "'"
			+ resource_file_name
			+ "'"
			+ " AND resource_owner = "
			+ "'"
			+ resource_owner
			+ "';";
		// no data will be returned from this SQL execution
		return execute(sql, nullptr);
	}
	int selectAll_resource(const HWND& hWnd)
	{
		Connect2SQLite::NAME_DB = "user_space_resource_data";
		// begin every execution with an empty vector
		vector_resource_data.clear();

		sql = "SELECT * FROM user_space_resource_data;";
		rc = execute(sql, &vector_resource_data);
		SendMessage(hWnd
			, IDM_DB_RESOURCE_SELECT_ALL_CALLBACK
			, (WPARAM)rc
			, (LPARAM)&vector_resource_data
		);
		return rc;
	}
private:
	int execute(const string& sql
		, const void* data
	)
	{
		// begin every execution with an empty vector
		//vector_user_data.clear();
		int rc = sqlite3_exec(db
			, sql.c_str()
			, callback
			, (void*)data
			, &pszErrMsg
		);
		if (rc != SQLITE_OK)
			// some kind of error
			sqlite3_free(pszErrMsg);
		return rc;
	}
	// called for every row in a query result
	static int callback(void* data
		, int argc
		, char** argv
		, char** aszColName
	)
	{
		// use variable named colName, to make a comparison easier
		string colName = *aszColName;
		if (colName == "max(ID)") {
			// for an empty table the argv[0] pointer will be null
			// return the value for the highest key value
			*(static_cast<int*>(data)) = (argv[0] == NULL) ? 0 : stoi(argv[0]);
			return 0;
		}
		if (Connect2SQLite::NAME_DB == "user_access_login_data")
		{
			tuple_user_data new_tuple_user_data(argv[0]
				, argv[1]
				, argv[2]
				, argv[3]
			);
			// store the C++ tuple into the vector
			static_cast<vector<tuple_user_data>*>(data)->
				push_back(new_tuple_user_data);
		}
		if (Connect2SQLite::NAME_DB == "user_space_resource_data")
		{
			// result from query that looks for file existance
			if (argc == 1 && colName == "resource_owner")
			{
				*static_cast<string*>(data) = argv[0];
				return 0;
			}
			tuple_resource_data new_tuple_resource_data(argv[0]
				, argv[1]
				, argv[2]
				, argv[3]
			);
			// store the C++ tuple into the vector
			static_cast<vector<tuple_resource_data>*>(data)->
				push_back(new_tuple_resource_data);
		}
		return EXIT_SUCCESS;
	}
};
std::string Connect2SQLite::NAME_DB = "";
