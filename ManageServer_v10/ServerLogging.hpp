#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "resource.h"

//****************************************************************************
//*                     extern global
//****************************************************************************
extern const size_t BUFFER_MAX;
extern PWCHAR pszTextBuffer;

//****************************************************************************
//*                     date_for_http_response
//****************************************************************************
inline std::string
date_for_http_response()
{
	const std::string dow[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	const std::string month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	time_t tt;
	time(&tt);
	tm t;
	localtime_s(&t, &tt);
	struct tm gmt;
	gmtime_s(&gmt, &tt);
	std::ostringstream oss;
	oss << dow[gmt.tm_wday] << ", "
		<< std::setw(2) << std::setfill('0') << gmt.tm_mday << " "
		<< month[gmt.tm_mon] << " "
		<< gmt.tm_year + 1900 << " "
		<< std::setw(2) << std::setfill('0') << gmt.tm_hour << ":"
		<< std::setw(2) << std::setfill('0') << gmt.tm_min << ":"
		<< std::setw(2) << std::setfill('0') << gmt.tm_sec << " "
		<< "GMT";
	return oss.str();
}

//****************************************************************************
//*                     typedef
//****************************************************************************
// typedef for logging
typedef std::string td_current_gmt
, td_remote_endpoint
, td_request
, td_response
, td_elapsed_time
, td_user
, td_user_agent;
typedef boost::tuples::tuple<td_current_gmt
	, td_remote_endpoint
	, td_request
	, td_response
	, td_elapsed_time
	, td_user
	, td_user_agent> tuple_logging;

//****************************************************************************
//*                     ServerLogging
//****************************************************************************
class ServerLogging
{
	friend std::string date_for_http_response();
public:
	HWND hWnd = 0;
	const boost::uintmax_t LOGGING_BYTES_MAX = 1024;// 8192; (8 kb)
	const std::string LOGGING_FILE_NAME = "logging";
	const boost::filesystem::path p{ LOGGING_FILE_NAME };
	boost::uintmax_t filesize = 0;
	boost::filesystem::ofstream ofs;
	boost::filesystem::ifstream ifs;
	std::vector<tuple_logging> vector_with_tuples;
public:
	ServerLogging() {}
	// copy constructor
	ServerLogging(ServerLogging& rhs)
	{
		this->hWnd = rhs.hWnd;
		this->filesize = rhs.filesize;
	}
	// assignment operator
	ServerLogging operator=(ServerLogging& rhs)
	{
		this->hWnd = rhs.hWnd;
		this->filesize = rhs.filesize;
	}
	void set_hwnd(const HWND& hWnd) { this->hWnd = hWnd; }
	void parse_logfile()
	{
		typedef std::string one_line_from_logging;
		one_line_from_logging str;
		tuple_logging tlin;
		// clear vector
		vector_with_tuples.clear();
		// open log for reading
		ifs.open(p);
		// read one line from log file, until eof
		while (std::getline(ifs, str))
		{
			size_t sBegin = 0, sEnd = 0;
			// 1
			// first character on a line is '('
			// start at sBegin = 1
			sBegin = sEnd + 1;
			sEnd = str.find("#", sBegin);
			std::string current_gmt = str.substr(sBegin, sEnd - sBegin);
			// 2
			sBegin = sEnd + 1;
			sEnd = str.find("#", sBegin);
			std::string remote_endpoint = str.substr(sBegin, sEnd - sBegin);
			// 3
			sBegin = sEnd + 1;
			sEnd = str.find("#", sBegin);
			std::string request = str.substr(sBegin, sEnd - sBegin);
			// 4
			sBegin = sEnd + 1;
			sEnd = str.find("#", sBegin);
			std::string response = str.substr(sBegin, sEnd - sBegin);
			// 5
			sBegin = sEnd + 1;
			sEnd = str.find("#", sBegin);
			std::string elapsed_time = str.substr(sBegin, sEnd - sBegin);
			// 6
			sBegin = sEnd + 1;
			sEnd = str.find("#", sBegin);
			std::string user = str.substr(sBegin, sEnd - sBegin);
			// 7
			// last character on a line is ')'
			sBegin = sEnd + 1;
			sEnd = str.find(")", sBegin);
			std::string user_agent = str.substr(sBegin, sEnd - sBegin);
			// create tuple from parsed log data out of file
			tlin = boost::tuples::make_tuple(
				current_gmt
				, remote_endpoint
				, request
				, response
				, elapsed_time
				, user
				, user_agent
			);
			// set tuple into vector
			vector_with_tuples.push_back(tlin);
		}
		// close log file
		ifs.close();
	}
	void show_vector_with_tuples()
	{
		parse_logfile();
		int i = 0;
		tuple_logging tlin;
		const UINT BUFFER_MAX = 256;
		PWCHAR pszText = new WCHAR[BUFFER_MAX];
		for (auto ite : vector_with_tuples)
		{
			tlin = ite;
//			StringCchPrintf(pszText
//				, BUFFER_MAX
//				, L"%2d\n\
//current GMT.......: %s\n\
//remote endpoint...: %s\n\
//request...........: %s\n\
//response..........: %s\n\
//elapsed time......: %s (ms)\n
//user..............: %s\n"
//user agent........: %s\n"
//				, ++i
//				, tlin.get<0>().c_str()
//				, tlin.get<1>().c_str()
//				, tlin.get<2>().c_str()
//				, tlin.get<3>().c_str()
//				, tlin.get<4>().c_str()
//              , tlin.get<5>().c_str()
//              , tlin.get<6>().c_str()
//			);
//			OutputDebugString(pszText);
		}
	}
	void get_filesize_logfile()
	{
		OutputDebugString(L"ServerLogging::get_filesize_logfile()\n");
		boost::system::error_code ec;
		// get file size
		filesize = boost::filesystem::file_size(p, ec);
		if (ec)
			;//error
		// send info about the logfile size, 
		// to the ManageServer module
		double d = filesize / 1000.0;
		StringCchPrintf(pszTextBuffer
			, BUFFER_MAX
			, L"Logfile size %.3f (kb)"
			, d
		);
		SendMessage(hWnd
			, IDM_MONITORING_LOGFILESIZE
			, (WPARAM)0
			, (LPARAM)pszTextBuffer
		);
		return;
	}
	void clear_logfile_if_too_big()
	{
		if (filesize > LOGGING_BYTES_MAX)
		{
			ofs.open(p);
			ofs.clear();
			ofs.close();
		}
	}
	void write_logfile(const tuple_logging& tl)
	{
		// write tuple to file
		ofs.open(p, std::ios_base::app);
		ofs << boost::tuples::set_delimiter('#') << tl << '\n';
		ofs.close();
	}
	void store_log(const std::string& remote_endpoint
		, const std::string& request
		, const std::string& response
		, const boost::timer::cpu_times& timer_elapsed
		, const std::string& user
		, const std::string& user_agent
	)
	{
		// get current gmt
		std::string current_gmt = date_for_http_response();
		std::string elapsed_time = std::to_string(
			timer_elapsed.wall / (double)1e6);
		// store in tuple
		tuple_logging tl{ current_gmt
			, remote_endpoint
			, request
			, response
			, elapsed_time
			, user
			, user_agent
		};
		// write tuple to file
		write_logfile(tl);
		get_filesize_logfile();
		// clear file if too big
		clear_logfile_if_too_big();
		// send message to server manager
		tuple_logging* ptl = &tl;
		SendMessage(hWnd
			, IDM_LOG_MSG
			, (WPARAM)0
			, (LPARAM)ptl
		);
	}
};
