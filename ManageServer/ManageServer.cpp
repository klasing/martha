//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "ManageServer.h"

//****************************************************************************
//*                     typedef
//****************************************************************************
// typedef for sqlite
typedef std::string td_id
, td_user_email_address
, td_user_password;
typedef std::tuple<td_id
	, td_user_email_address
	, td_user_password> tuple_user_data;
// typedef for logging
typedef std::string td_current_gmt
, td_remote_endpoint
, td_request
, td_response
, td_elapsed_time;
typedef boost::tuples::tuple<td_current_gmt
	, td_remote_endpoint
	, td_request
	, td_response
	, td_elapsed_time> tuple_logging;
// typedef for http_server_async thread
typedef struct tagSTRUCTSERVER {
	PVOID pServerLogging = nullptr;
	PVOID pSqlite = nullptr;
} STRUCTSERVER, * PSTRUCTSERVER;

//****************************************************************************
//*                     global
//****************************************************************************
const size_t BUFFER_MAX = 256;
PWCHAR pszTextBuffer = new WCHAR[BUFFER_MAX];
StatusBar oStatusBar;
Connect2SQLite oSqlite;

//****************************************************************************
//*                     prototype
//****************************************************************************
// forward declarations of functions included in this module:
ATOM				MyRegisterClass(HINSTANCE, const PWCHAR&);
BOOL				InitInstance(HINSTANCE, const int&, const PWCHAR&, const PWCHAR&);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
// http_server_async function (runs in a WIN32-Thread)
DWORD WINAPI		http_server_async(LPVOID);

//****************************************************************************
//*                     wWinMain
//****************************************************************************
int WINAPI
wWinMain(_In_ HINSTANCE hInstance
	, _In_opt_ HINSTANCE hPrevInstance
	, _In_ PWSTR pCmdLine
	, _In_ int nCmdShow
)
{
	// TODO: place code here
	INITCOMMONCONTROLSEX ic;
	ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ic.dwICC = ICC_STANDARD_CLASSES
		| ICC_TAB_CLASSES
		| ICC_LISTVIEW_CLASSES
		| ICC_BAR_CLASSES;
	BOOL bSuccessInit = InitCommonControlsEx(&ic);

	const UINT MAX_LOADSTRING = 256;
	WCHAR szWindowClass[MAX_LOADSTRING];
	WCHAR szTitle[MAX_LOADSTRING];

	// initialize global string
	LoadString(hInstance, IDS_WINDOWCLASS, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	MyRegisterClass(hInstance, szWindowClass);

	// perform application initialization
	if (!InitInstance(hInstance
		, nCmdShow
		, szWindowClass
		, szTitle
	))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance
		, MAKEINTRESOURCE(IDC_ACCELERATOR)
	);

	// main message loop:
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		// necessary to convey a WM_COMMAND message,
		// containing a (...) message,
		// to the concerning dialog
		if (IsDialogMessage(FindWindow(NULL, L"Tab0Proc"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Tab1Proc"), &msg))
			continue;
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

//****************************************************************************
//*                     MyRegisterClass
//****************************************************************************
ATOM
MyRegisterClass(HINSTANCE hInstance
	, const PWCHAR& pszWindowClass
)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MANAGESERVER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MENU);
	wcex.lpszClassName = pszWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//****************************************************************************
//*                     InitInstance
//****************************************************************************
BOOL
InitInstance(HINSTANCE hInstance
	, const int& nCmdShow
	, const PWCHAR& pszWindowClass
	, const PWCHAR& pszTitle
)
{
	HWND hWnd = CreateWindowEx(0
		, pszWindowClass
		, pszTitle
		, WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0
		, nullptr
		, nullptr
		, hInstance
		, nullptr
	);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//****************************************************************************
//*                     WndProc
//****************************************************************************
// TODO: solve the statusbar initialization problem
//       the text: "Database OPEN" is not shown initially
// SOLVED
LRESULT CALLBACK WndProc(HWND hWnd
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static TabControl oTabControl;
	static ServerLogging oServerLogging;
	static PSTRUCTSERVER pStructServer = new STRUCTSERVER;

	switch (uMsg)
	{
	case WM_NCCREATE:
		// create a tab control
		oTabControl.createTabControl(hInst, hWnd);
		// create a status bar
		oStatusBar.createStatusBar(hInst, hWnd);
		// open database
		oSqlite.openDb();
		// initialize structure for asynchronous http server
		pStructServer->pServerLogging = &oServerLogging;
		pStructServer->pSqlite = &oSqlite;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		oTabControl.showTabItems(lParam);
		oStatusBar.SetStatusBar(hWnd);
		return 0;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_TABCONTROL)
			return oTabControl.onWmNotify(lParam
				, oStatusBar
			);
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_DB_SELECT_ALL:
			// sqlite will call IDM_DB_SELECT_ALL_CALLBACK in hWndDlg[0]
			oSqlite.selectAll(oTabControl.hWndDlg[0]);
			break;
		case IDM_MONITORING_START:
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_START, MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_STOP, MF_ENABLED);
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_START
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_MONITORING_STOP:
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_START, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_STOP, MF_GRAYED);
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_STOP
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_MONITORING_CLEAR:
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_CLEAR
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_SERVER_START:
		{
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_START, MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_STOP, MF_ENABLED);
			// the dialog stored in the oTabControl instance
			// will receive log messages from the ServerLogging instance
			// (mind the message pump)
			oServerLogging.hWnd = oTabControl.hWndDlg[1];
			SendMessage(oTabControl.hWndDlg[2]
				, IDM_SERVER_START
				, (WPARAM)0
				, (LPARAM)0
			);
			// start thread			
			DWORD dwThreadID;
			HANDLE hThread = CreateThread(NULL
				, 0						// default stack size
				, http_server_async		// thread function
				, pStructServer			// argument to thread function
				, 0						// default creation flags
				, &dwThreadID			// returns the thread identifier
			);
			break;
		} // eof IDM_SERVER_START
		case IDM_SERVER_STOP:
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_START, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_STOP, MF_GRAYED);
			SendMessage(oTabControl.hWndDlg[2]
				, IDM_SERVER_STOP
				, (WPARAM)0
				, (LPARAM)0
			);
			// raise a signal to kill the server
			raise(SIGABRT);
			break;
		case IDM_ABOUT:
			DialogBox(hInst
				, MAKEINTRESOURCE(IDD_ABOUTBOX)
				, hWnd
				, About
			);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		} // eof switch
		break;
	case WM_DESTROY:
		// close database
		oSqlite.closeDb();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	} // eof swtich

	return 0;
}

//****************************************************************************
//*                     About
//****************************************************************************
INT_PTR CALLBACK About(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     Tab0Proc
//****************************************************************************
// Message handler for the "Database" tab item
INT_PTR CALLBACK Tab0Proc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static HWND hWndLV;
	static HMENU popup_menu;
	static int selection_mark = -1;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Tab0Proc");
		// create listview
		hWndLV = createListView(hInst, hDlg, IDC_LVDB);
		// add column to listview
		addColumn(hInst
			, hWndLV
			, 3
			, IDS_LVDB_COL0, IDS_LVDB_COL1, IDS_LVDB_COL2
		);
		popup_menu = CreatePopupMenu();
		AppendMenu(popup_menu
			, MF_STRING
			, IDM_DB_DELETE_USER
			, L"&Delete"
		);
		SetMenu(hWndLV
			, popup_menu
		);
		return (INT_PTR)TRUE;
	case WM_SIZE:
		MoveWindow(hWndLV
			, 0, 0
			, GET_X_LPARAM(lParam)
			, GET_Y_LPARAM(lParam)
			, TRUE
		);
		return 0;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_RCLICK:
		{
			selection_mark = SendMessage(hWndLV
				, LVM_GETSELECTIONMARK
				, (WPARAM)0
				, (LPARAM)0
			);
			if (selection_mark == -1)
				return 0;
			// show popup menu
			POINT ptCursorPos;
			GetCursorPos(&ptCursorPos);
			TrackPopupMenu(popup_menu
				, TPM_RIGHTBUTTON
				, ptCursorPos.x
				, ptCursorPos.y
				, 0
				// the window that receives the IDM_DB_DELETE_USER message
				// via a WM_COMMAND message
				, hDlg
				, NULL
			);
			return 0;
		} // eof NM_RCLICK
		} // eof switch
		// the return value is ignored
		return 0;
	case IDM_DB_SELECT_ALL_CALLBACK:
	{
		// clear listview
		clearListView(hWndLV);
		size_t iItem = 0;
		std::vector<tuple_user_data>* pvud =
			(std::vector<tuple_user_data>*)lParam;
		for (auto it = pvud->begin(); it < pvud->end(); ++it)
		{
			tuple_user_data tud = *it;
			addListViewItem(hWndLV
				, const_cast<char*>(std::get<0>(tud).c_str())
				, iItem, 0);
			addListViewItem(hWndLV
				, const_cast<char*>(std::get<1>(tud).c_str())
				, iItem, 1);
			addListViewItem(hWndLV
				, const_cast<char*>(std::get<2>(tud).c_str())
				, iItem, 2);
			++iItem;
		}
		break;
	} // eof IDM_DB_SELECT_ALL_CALLBACK
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_DB_DELETE_USER:
		{
			if (selection_mark == -1)
				break;
			PWCHAR pszText = new WCHAR[64];
			// find the primary key ID for the user that is going to be deleted
			getListViewItem(hWndLV
				, selection_mark
				, 0
				, pszText
			);
			// delete the user from the database,
			// with the found primary key ID
			oSqlite.deleteRow(_wtoi(pszText));
			// delete selected listview item
			SendMessage(hWndLV
				, LVM_DELETEITEM
				, (WPARAM)selection_mark
				, (LPARAM)0
			);
			// clean up
			delete[] pszText;
			break;
		} // eof IDM_DB_DELETE_USER
		} // eof switch
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     Tab1Proc
//****************************************************************************
// Message handler for the "Logging" tab item
INT_PTR CALLBACK Tab1Proc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static HWND hWndLV;
	static BOOL bMonitor = TRUE;
	static UINT iItem = 0;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Tab1Proc");
		// create listview
		hWndLV = createListView(hInst, hDlg, IDC_LVDB);
		// add column to listview
		addColumn(hInst
			, hWndLV
			, 5
			, IDS_LVLOG_COL0, IDS_LVLOG_COL1, IDS_LVLOG_COL2
			, IDS_LVLOG_COL3, IDS_LVLOG_COL4
		);
		return (INT_PTR)TRUE;
	case WM_SIZE:
		MoveWindow(hWndLV
			, 0, 0
			, GET_X_LPARAM(lParam)
			, GET_Y_LPARAM(lParam)
			, TRUE
		);
		return 0;
	case IDM_MONITORING_CLEAR:
		clearListView(hWndLV);
		iItem = 0;
		break;
	case IDM_MONITORING_LOGFILESIZE:
		oStatusBar.StatusBarSetText(1, 5, (PWCHAR)lParam);
		break;
	case IDM_MONITORING_START:
		oStatusBar.StatusBarSetText(1, 0, L"Monitor ON");
		bMonitor = TRUE;
		break;
	case IDM_MONITORING_STOP:
		oStatusBar.StatusBarSetText(1, 0, L"Monitor OFF");
		bMonitor = FALSE;
		break;
	case IDM_LOG_MSG:
		if (bMonitor)
		{
			tuple_logging* ptl = (tuple_logging*)lParam;
			addListViewItem(hWndLV
				, ptl->get<0>().c_str()
				, iItem, 0);
			addListViewItem(hWndLV
				, ptl->get<1>().c_str()
				, iItem, 1);
			addListViewItem(hWndLV
				, ptl->get<2>().c_str()
				, iItem, 2);
			addListViewItem(hWndLV
				, ptl->get<3>().c_str()
				, iItem, 3);
			addListViewItem(hWndLV
				, ptl->get<4>().c_str()
				, iItem, 4);
			++iItem;
		}
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     Tab2Proc
//****************************************************************************
// Message handler for the "Server" tab item
INT_PTR CALLBACK Tab2Proc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Tab2Proc");
		return (INT_PTR)TRUE;
	case IDM_SERVER_START:
		oStatusBar.StatusBarSetText(2, 0, L"Server ALIVE");
		break;
	case IDM_SERVER_STOP:
		oStatusBar.StatusBarSetText(2, 0, L"Server DEAD");
		break;
	} // eof switch
	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     generate_random_string
//****************************************************************************
std::string
generate_random_string()
{
	const int RANDOM_CHARACTER_MAX = 16;
	std::string random_string = "";
	std::mt19937 eng(static_cast<unsigned long>(time(nullptr)));
	uniform_int_distribution<int> dist(65, 90);
	for (int i = 0; i < RANDOM_CHARACTER_MAX; i++)
		random_string += dist(eng);
	return random_string;
}

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//****************************************************************************
//*                     global
//****************************************************************************
const int SECONDS_BEFORE_EXPIRING = 300;

//****************************************************************************
//*                     mime_type
//****************************************************************************
// Return a reasonable mime type based on the extension of a file.
beast::string_view
mime_type(beast::string_view path)
{
	using beast::iequals;
	auto const ext = [&path]
	{
		auto const pos = path.rfind(".");
		if (pos == beast::string_view::npos)
			return beast::string_view{};
		return path.substr(pos);
	}();
	if (iequals(ext, ".htm"))  return "text/html";
	if (iequals(ext, ".html")) return "text/html";
	if (iequals(ext, ".php"))  return "text/html";
	if (iequals(ext, ".css"))  return "text/css";
	if (iequals(ext, ".txt"))  return "text/plain";
	if (iequals(ext, ".js"))   return "application/javascript";
	if (iequals(ext, ".json")) return "application/json";
	if (iequals(ext, ".xml"))  return "application/xml";
	if (iequals(ext, ".swf"))  return "application/x-shockwave-flash";
	if (iequals(ext, ".flv"))  return "video/x-flv";
	if (iequals(ext, ".png"))  return "image/png";
	if (iequals(ext, ".jpe"))  return "image/jpeg";
	if (iequals(ext, ".jpeg")) return "image/jpeg";
	if (iequals(ext, ".jpg"))  return "image/jpeg";
	if (iequals(ext, ".gif"))  return "image/gif";
	if (iequals(ext, ".bmp"))  return "image/bmp";
	if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
	if (iequals(ext, ".tiff")) return "image/tiff";
	if (iequals(ext, ".tif"))  return "image/tiff";
	if (iequals(ext, ".svg"))  return "image/svg+xml";
	if (iequals(ext, ".svgz")) return "image/svg+xml";
	return "application/text";
}

//****************************************************************************
//*                     path_cat
//****************************************************************************
// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
	beast::string_view base,
	beast::string_view path)
{
	if (base.empty())
		return std::string(path);
	std::string result(base);
#ifdef BOOST_MSVC
	char constexpr path_separator = '\\';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
	for (auto& c : result)
		if (c == '/')
			c = path_separator;
#else
	char constexpr path_separator = '/';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
#endif
	return result;
}

//****************************************************************************
//*                     filter_email_password_code
//****************************************************************************
template<class Body, class Allocator>
void filter_email_password_code(
	http::request<Body, http::basic_fields<Allocator>> req
	, std::string& user_email_address
	, std::string& user_password
	, std::string& user_code
	, const std::string& user_agent
)
{
	std::string payload = req.body();
	// if user agent not equals Boost.Beast/248
	// then it is a browser request
	// replace %40 for @ in the user_email_address
	// (when user_agent is not Boost.Beast/248, i.d. a browser)
	size_t pos = payload.find("%40", 0);
	if (pos != std::string::npos)
		payload = payload.replace(pos, 3, "@");

	size_t iBegin = 0, iEnd = 0, iLength = 0, iTemp;
	// filter user_email_address
	iTemp = iEnd;
	iBegin = payload.find_first_of("=", iTemp);
	iEnd = (payload.find("&", iBegin) != std::string::npos) ?
		(payload.find("&", iBegin) - 1) :
		payload.length() - 1;
	iLength = iEnd - iBegin;
	iBegin++;
	user_email_address = payload.substr(iBegin, iLength);

	// filter user_password
	iTemp = iEnd;
	iBegin = payload.find_first_of("=", iTemp);
	iEnd = (payload.find("&", iBegin) != std::string::npos) ?
		(payload.find("&", iBegin) - 1) :
		payload.length() - 1;
	iLength = iEnd - iBegin;
	iBegin++;
	user_password = payload.substr(iBegin, iLength);

	// filter user_code
	iTemp = iEnd;
	iBegin = payload.find_first_of("=", iTemp);
	if (iBegin == std::string::npos)
		// no user code available
		return;
	iEnd = (payload.find("&", iBegin) != std::string::npos) ?
		(payload.find("&", iBegin) - 1) :
		payload.length() - 1;
	iLength = iEnd - iBegin;
	iBegin++;
	user_code = payload.substr(iBegin, iLength);
}
//****************************************************************************
//*                     write_message_to_string
//****************************************************************************
// used in handling a TRACE request
template <class Message, class DynamicBuffer>
auto
write_message_to_string(Message& message
	, DynamicBuffer& dynamic_buffer
) -> void
{
	beast::ostream(dynamic_buffer) << message;
}

//****************************************************************************
//*                     filter_filename_payload_from_form_submit
//****************************************************************************
// browser form submit for a file upload 
// delivered within a POST message 
auto
filter_filename_payload_from_form_submit(
	const std::string req_body,
	std::string& file_name,
	std::string& payload
) -> void
{
	int sbegin = 0;
	int send = 0;
	std::string string_to_search_for = "";
	// filter file_name
	// look for: filename="
	string_to_search_for = "filename=\"";
	sbegin = req_body.find(string_to_search_for);
	sbegin += string_to_search_for.length();
	// look for ", starting at the sbegin position
	send = req_body.find("\"", sbegin);
	file_name = req_body.substr(sbegin, send - sbegin);

	// the original payload has additional lines of data
	// which have to be removed

	//------WebKitFormBoundarygQus3BLIAghh6JYQ
	//Content-Disposition: form-data; name="file"; filename="bla.txt"
	//Content-Type: text/plain
	//
	//bla bla
	//------WebKitFormBoundarygQus3BLIAghh6JYQ--

	// copy the request body into the payload, the payload
	// will be searched and edited
	payload = req_body;
	// look for: ------WebKitFormBoundary
	string_to_search_for = "------WebKitFormBoundary";
	send = payload.find(string_to_search_for);
	// remove everything up to the send position
	payload.erase(0, send);
	// look for newline, four times, using sbegin as an offset
	sbegin = 0;
	string_to_search_for = "\n";
	for (int i = 0; i < 4; i++) {
		send = payload.find(string_to_search_for, sbegin);
		sbegin = send + 1;
	}
	// what remains is one newline at the beginning of the payload
	// string, remove this newline character
	payload.erase(0, 1);
	// remove everything up to the sbegin position
	payload.erase(0, send);
	// start at the end of the payload string and
	// look -backwards- for the first newline from the
	// last character -which is a newline- minus one
	send = payload.length() - 2;
	sbegin = payload.rfind(string_to_search_for, send);
	payload.erase(sbegin, send);
}

//****************************************************************************
//*                     save_to_disk
//****************************************************************************
auto
save_to_disk(
	const std::string& file_name,
	const std::string& payload
) -> void
{
	// Store the received file on disk
	std::string file_name_on_server =
		std::string("user_space/") +
		file_name;
	boost::filesystem::path p{ file_name_on_server };
	boost::filesystem::ofstream ofs{ p };
	ofs << payload;
}

//****************************************************************************
//*                     filter_start_line
//****************************************************************************
auto
filter_start_line(const std::string& req_message
) -> std::string
{
	// return the first line of a request message
	return req_message.substr(0, req_message.find('\r'));
}

//****************************************************************************
//*                     store_new_log
//****************************************************************************
template <class T>
auto
store_new_log(std::shared_ptr<ServerLogging> pServerLogging
	, std::shared_ptr<std::string const> pRemoteEndpoint
	, std::string requestLogMsg
	, T res
	, std::shared_ptr<boost::timer::cpu_timer> pTimer
) -> void
{
	// turn the response message into a string
	auto buff = beast::flat_buffer();
	write_message_to_string(res, buff);
	std::string responseLogMsg = filter_start_line(
		beast::buffers_to_string(buff.data()));

	// stop timer
	pTimer->stop();
	pServerLogging->store_log(*pRemoteEndpoint
		, requestLogMsg
		, responseLogMsg
		, pTimer->elapsed()
	);
}

// overloading
// a boost::beast::http::response<file_body> type response can't be
// templated like in the first store_new_log() function, it will
// become a deleted function, according to the compiler
auto
store_new_log(std::shared_ptr<ServerLogging> pServerLogging
	, std::shared_ptr<std::string const> pRemoteEndpoint
	, std::string requestLogMsg
	, std::string responseLogMsg
	, std::shared_ptr<boost::timer::cpu_timer> pTimer
) -> void
{
	// stop timer
	pTimer->stop();
	pServerLogging->store_log(*pRemoteEndpoint
		, requestLogMsg
		, responseLogMsg
		, pTimer->elapsed()
	);
}

//****************************************************************************
//*                     handle_request
//****************************************************************************
// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
	class Body, class Allocator,
	class Send>
	void
	handle_request(beast::string_view doc_root
		, std::shared_ptr<Connect2SQLite> pSqlite
		, std::shared_ptr<HandlerForRegister> pHandlerForRegister
		, std::shared_ptr<HandlerForResetPassword> pHandlerForResetPassword
		, std::shared_ptr<ServerLogging> pServerLogging
		, std::shared_ptr<std::string const> pRemoteEndpoint
		, std::shared_ptr<boost::timer::cpu_timer> pResponseTimer
		, http::request<Body, http::basic_fields<Allocator>>&& req
		, Send&& send
	)
{
	// Returns a bad request response
	auto const bad_request =
		[&, req, pServerLogging, pRemoteEndpoint, pResponseTimer]
	(beast::string_view why)
	{
		http::response<http::string_body> res{ http::status::bad_request, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = std::string(why);
		res.prepare_payload();

		// turn the request message into a string
		auto buff = beast::flat_buffer();
		write_message_to_string(req, buff);
		std::string req_message = beast::buffers_to_string(buff.data());
		std::string requestLogMsg = filter_start_line(req_message);
		store_new_log(pServerLogging
			, pRemoteEndpoint
			, requestLogMsg
			, res
			, pResponseTimer
		);

		return res;
	};

	// Returns a not found response
	auto const not_found =
		[&, req, pServerLogging, pRemoteEndpoint, pResponseTimer]
	(beast::string_view target)
	{
		http::response<http::string_body> res{ http::status::not_found, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "The resource '" + std::string(target) + "' was not found.";
		res.prepare_payload();

		// turn the request message into a string
		auto buff = beast::flat_buffer();
		write_message_to_string(req, buff);
		std::string req_message = beast::buffers_to_string(buff.data());
		std::string requestLogMsg = filter_start_line(req_message);
		store_new_log(pServerLogging
			, pRemoteEndpoint
			, requestLogMsg
			, res
			, pResponseTimer
		);

		return res;
	};

	// Returns a server error response
	auto const server_error =
		[&, req, pServerLogging, pRemoteEndpoint, pResponseTimer]
	(beast::string_view what)
	{
		http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "An error occurred: '" + std::string(what) + "'";
		res.prepare_payload();

		// turn the request message into a string
		auto buff = beast::flat_buffer();
		write_message_to_string(req, buff);
		std::string req_message = beast::buffers_to_string(buff.data());
		std::string requestLogMsg = filter_start_line(req_message);
		store_new_log(pServerLogging
			, pRemoteEndpoint
			, requestLogMsg
			, res
			, pResponseTimer
		);

		return res;
	};

	// application user_agent:
	const std::string APP_VALUE_USER_AGENT = "Boost.Beast/248";
	// Google Chrome browser user_agent:
	const std::string CHROME_VALUE_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko)";

	// turn the request into a string
	auto buff = beast::flat_buffer();
	write_message_to_string(req, buff);
	std::string req_message = beast::buffers_to_string(buff.data());
	std::string requestLogMsg = filter_start_line(req_message);

	// Make sure we can handle the method
	if (req.method() != http::verb::connect &&
		req.method() != http::verb::delete_ &&
		req.method() != http::verb::get &&
		req.method() != http::verb::head &&
		req.method() != http::verb::options &&
		req.method() != http::verb::post &&
		req.method() != http::verb::put &&
		req.method() != http::verb::trace)
		return send(bad_request("Unknown HTTP-method"));

	// Respond to a CONNECT request
	if (req.method() == http::verb::connect) {
		OutputDebugString(L"-> CONNECT message received\n");
		// not implemented yet
	}

	// Respond to a DELETE request
	if (req.method() == http::verb::delete_) {
		OutputDebugString(L"-> DELETE message received\n");
		// not implemented yet
	}

	// Respond to a GET request
	if (req.method() == http::verb::get) {
		OutputDebugString(L"-> GET message received\n");
		// a GET request can mean three things
		// 1) a download request for a file from an app.
		// 2) a download request for a file within the user space
		//    from a browser
		// 3) a download request for a file within the server space
		//    from a browser (a file that renders the browser's layout)
		std::string user_agent =
			static_cast<std::string>(req[http::field::user_agent]);
		if (user_agent == APP_VALUE_USER_AGENT)
		{
			// its a download request for a file from an app. (1)
			std::string path = path_cat(doc_root, req.target());
			// Request path must be absolute and not contain "..".
			if (req.target().empty() ||
				req.target()[0] != '/' ||
				req.target().find("..") != beast::string_view::npos)
				return send(bad_request("Illegal request-target"));
			if (req.target().back() == '/')
				path.append("index.html");
			// Attempt to open the file
			beast::error_code ec;
			http::file_body::value_type body;
			body.open(path.c_str(), beast::file_mode::scan, ec);
			// Handle the case where the file doesn't exist
			if (ec == beast::errc::no_such_file_or_directory)
				return send(not_found(req.target()));
			// Handle an unknown error
			if (ec)
				return send(server_error(ec.message()));
			// Cache the size since we need it after the move
			auto const size = body.size();
			http::response<http::file_body> res{
				std::piecewise_construct,
				std::make_tuple(std::move(body)),
				std::make_tuple(http::status::ok, req.version()) };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, mime_type(path));
			res.content_length(size);
			res.keep_alive(req.keep_alive());

			// turn the response message into a string
			auto buff_res = beast::flat_buffer();
			write_message_to_string(res, buff_res);
			std::string responseLogmsg = filter_start_line(
				beast::buffers_to_string(buff_res.data()));
			store_new_log(pServerLogging
				, pRemoteEndpoint
				, requestLogMsg
				, responseLogmsg
				, pResponseTimer
			);
			return send(std::move(res));
		}
		// cutoff user_agent string to a point where it matches
		// the const value for a browser, there is no differentiation
		// anymore between a Chrome and an Edge browser
		user_agent = user_agent.substr(0, user_agent.find(" Chrome/"));
		if (user_agent == CHROME_VALUE_USER_AGENT)
		{
			// if the req.target contains the substring:
			// '/server_space/user_space/'
			// then a file from within the user_space has to be
			// downloaded
			std::string path = "";
			std::string s = req.target().to_string();
			std::string t = "/server_space";
			if (s.find("user_space") < std::string::npos)
			{
				s = s.substr(t.length(), s.length() - t.length());
				path = path_cat(doc_root, s);
			}
			else
				// its a request from a browser for a rendering file
				// inside the server_space
				path = path_cat(doc_root, req.target());

			// Request path must be absolute and not contain "..".
			if (req.target().empty() ||
				req.target()[0] != '/' ||
				req.target().find("..") != beast::string_view::npos)
				return send(bad_request("Illegal request-target"));
			if (req.target().back() == '/')
				path.append("index.html");

			// Attempt to open the file
			beast::error_code ec;
			http::file_body::value_type body;
			body.open(path.c_str(), beast::file_mode::scan, ec);
			// Handle the case where the file doesn't exist
			if (ec == beast::errc::no_such_file_or_directory)
				return send(not_found(req.target()));
			// Handle an unknown error
			if (ec)
				return send(server_error(ec.message()));
			// Cache the size since we need it after the move
			auto const size = body.size();
			http::response<http::file_body> res{
				std::piecewise_construct,
				std::make_tuple(std::move(body)),
				std::make_tuple(http::status::ok, req.version()) };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, mime_type(path));
			res.content_length(size);
			res.keep_alive(req.keep_alive());

			// turn the response message into a string
			auto buff_res = beast::flat_buffer();
			write_message_to_string(res, buff_res);
			std::string responseLogmsg = filter_start_line(
				beast::buffers_to_string(buff_res.data()));
			store_new_log(pServerLogging
				, pRemoteEndpoint
				, requestLogMsg
				, responseLogmsg
				, pResponseTimer
			);
			return send(std::move(res));
		}
	}

	// Respond to a HEAD request
	if (req.method() == http::verb::head) {
		OutputDebugString(L"-> HEAD message received\n");
		// send a response without a payload
		std::string path = path_cat(doc_root, req.target());
		http::response<http::empty_body> res{ http::status::ok, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.keep_alive(req.keep_alive());
		store_new_log(pServerLogging
			, pRemoteEndpoint
			, requestLogMsg
			, res
			, pResponseTimer
		);
		return send(std::move(res));
	}

	// Respond to a OPTIONS request
	if (req.method() == http::verb::options) {
		OutputDebugString(L"-> OPTIONS message received\n");
		// not implemented yet
	}

	// Respond to a POST request
	if (req.method() == http::verb::post) {
		OutputDebugString(L"-> POST message received\n");
		// a POST request can mean two things
		// 1) an access to the server request, from an app. or a browser
		// 2) a file upload from a browser into the user space

		std::string user_agent =
			static_cast<std::string>(req[http::field::user_agent]);
		std::string target =
			static_cast<std::string>(req.target());
		// check if the POST message is concerning a login
		if (target == "/login"
			|| target == "/register"
			|| target == "/register_confirm"
			|| target == "/reset_password"
			|| target == "/reset_password_confirm"
			)
		{
			std::string user_email_address = "";
			std::string user_password = "";
			std::string user_code = "";
			// filter the user_email_address, user_password, and
			// the user_code from the request
			filter_email_password_code(req
				, user_email_address
				, user_password
				, user_code
				, user_agent
			);
			std::string response_payload = target + ": ";
			// remove forward slash from response_payload
			response_payload.erase(0, 1);
			// check which handler suits the target
			if (target == "/login")
			{
				HandlerForLogin handlerForLogin;
				handlerForLogin.handle_login(
					user_email_address
					, user_password
					, pSqlite
					, response_payload
				);
			}
			if (target == "/register")
			{
				pHandlerForRegister->handle_register(
					user_email_address
					, user_password
					, pSqlite
					, response_payload
				);
			}
			if (target == "/register_confirm")
			{
				pHandlerForRegister->handle_register_confirm(
					user_email_address
					, user_password
					, user_code
					, pSqlite
					, response_payload
				);
			}
			if (target == "/reset_password")
			{
				pHandlerForResetPassword->handle_reset_password(
					user_email_address
					, user_password
					, pSqlite
					, response_payload
				);
			}
			if (target == "/reset_password_confirm")
			{
				pHandlerForResetPassword->handle_reset_password_confirm(
					user_email_address
					, user_password
					, user_code
					, pSqlite
					, response_payload
				);
			}
			// prepare a response message
			http::response<http::string_body> res{ http::status::ok, req.version() };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, "text/html");
			res.keep_alive(req.keep_alive());
			res.content_length(response_payload.length());
			res.body() = response_payload;
			res.prepare_payload();
			store_new_log(pServerLogging
				, pRemoteEndpoint
				, requestLogMsg
				, res
				, pResponseTimer
			);
			return send(std::move(res));
		}
		else
		{
			// assume its a request from a browser
			std::string file_name = "";
			std::string payload = "";
			filter_filename_payload_from_form_submit(
				static_cast<std::string>(req.body()),
				file_name,
				payload
			);
			save_to_disk(file_name, payload);
			// prepare a response message
			http::response<http::string_body> res{
				// no_content = 204, this is necessary to stop the
				// frame-center.html from disappearing in the browser
				http::status::no_content, req.version()
			};
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.keep_alive(req.keep_alive());
			store_new_log(pServerLogging
				, pRemoteEndpoint
				, requestLogMsg
				, res
				, pResponseTimer
			);
			return send(std::move(res));
		}
	}

	// Respond to a PUT request
	if (req.method() == http::verb::put) {
		OutputDebugString(L"-> PUT message received\n");
		// remove all the \r-characters (return) from the req.body()
		boost::erase_all(req.body(), "\r");
		std::string file_name = static_cast<std::string>(req.target());
		std::string request_payload = req.body();
		save_to_disk(file_name, request_payload);
		// prepare a response message
		http::response<http::string_body> res{
			http::status::ok, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.keep_alive(req.keep_alive());
		store_new_log(pServerLogging
			, pRemoteEndpoint
			, requestLogMsg
			, res
			, pResponseTimer
		);
		return send(std::move(res));
	}

	// Respond to a TRACE request
	if (req.method() == http::verb::trace) {
		OutputDebugString(L"-> TRACE message received\n");
		// the request is echoed back to the client,
		// inside the payload of the response
		// in this application a non-standard comment is added
		http::string_body::value_type body;
		body = std::string("server is alive\n")
			+ req_message;
		// prepare a response message
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.set(http::field::date, beast::string_view(date_for_http_response()));
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, beast::string_view("message/http"));
		res.keep_alive(false);
		res.content_length(body.size());
		res.body() = std::move(body);
		res.prepare_payload();
		store_new_log(pServerLogging
			, pRemoteEndpoint
			, requestLogMsg
			, res
			, pResponseTimer
		);
		// send the response message
		return send(std::move(res));
	}
}

//****************************************************************************
//*                     fail
//****************************************************************************
// Report a failure
void
fail(beast::error_code ec, char const* what)
{
	;// std::cerr << what << ": " << ec.message() << "\n";
}

//****************************************************************************
//*                     session
//****************************************************************************
// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
	// This is the C++11 equivalent of a generic lambda.
	// The function object is used to send an HTTP message.
	struct send_lambda
	{
		session& self_;

		explicit
			send_lambda(session& self)
			: self_(self)
		{
		}

		template<bool isRequest, class Body, class Fields>
		void
			operator()(http::message<isRequest, Body, Fields>&& msg) const
		{
			// The lifetime of the message has to extend
			// for the duration of the async operation so
			// we use a shared_ptr to manage it.
			auto sp = std::make_shared<
				http::message<isRequest, Body, Fields>>(std::move(msg));

			// Store a type-erased version of the shared
			// pointer in the class to keep it alive.
			self_.res_ = sp;

			// Write the response
			http::async_write(
				self_.stream_,
				*sp,
				beast::bind_front_handler(
					&session::on_write,
					self_.shared_from_this(),
					sp->need_eof()));
		}
	};

	beast::tcp_stream stream_;
	beast::flat_buffer buffer_;
	std::shared_ptr<std::string const> doc_root_;
	std::shared_ptr<Connect2SQLite> pSqlite_;
	std::shared_ptr<HandlerForRegister> pHandlerForRegister_;
	std::shared_ptr<HandlerForResetPassword> pHandlerForResetPassword_;
	std::shared_ptr<ServerLogging> pServerLogging_;
	std::shared_ptr<std::string const> pRemoteEndpoint_;
	std::shared_ptr<boost::timer::cpu_timer> pResponseTimer_;
	http::request<http::string_body> req_;
	std::shared_ptr<void> res_;
	send_lambda lambda_;

public:
	// Take ownership of the stream
	session(tcp::socket&& socket
		, std::shared_ptr<std::string const> const& doc_root
		, std::shared_ptr<Connect2SQLite> const& pSqlite
		, std::shared_ptr<HandlerForRegister> const& pHandlerForRegister
		, std::shared_ptr<HandlerForResetPassword> const& pHandlerForResetPassword
		, std::shared_ptr<ServerLogging> const& pServerLogging
		, std::shared_ptr<std::string const> const& pRemoteEndpoint
		, std::shared_ptr<boost::timer::cpu_timer> const& pResponseTimer
	)
		: stream_(std::move(socket))
		, doc_root_(doc_root)
		, pSqlite_(pSqlite)
		, pHandlerForRegister_(pHandlerForRegister)
		, pHandlerForResetPassword_(pHandlerForResetPassword)
		, pServerLogging_(pServerLogging)
		, pRemoteEndpoint_(pRemoteEndpoint)
		, pResponseTimer_(pResponseTimer)
		, lambda_(*this)
	{
	}

	// Start the asynchronous operation
	void
		run()
	{
		do_read();
	}

	void
		do_read()
	{
		// Make the request empty before reading,
		// otherwise the operation behavior is undefined.
		req_ = {};

		// Set the timeout.
		stream_.expires_after(std::chrono::seconds(30));

		// Read a request
		http::async_read(stream_, buffer_, req_,
			beast::bind_front_handler(
				&session::on_read,
				shared_from_this()));
	}

	void
		on_read(
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// This means they closed the connection
		if (ec == http::error::end_of_stream)
			return do_close();

		if (ec)
			return fail(ec, "read");

		// Send the response
		handle_request(*doc_root_
			, pSqlite_
			, pHandlerForRegister_
			, pHandlerForResetPassword_
			, pServerLogging_
			, pRemoteEndpoint_
			, pResponseTimer_
			, std::move(req_)
			, lambda_);
	}

	void
		on_write(
			bool close,
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return fail(ec, "write");

		if (close)
		{
			// This means we should close the connection, usually because
			// the response indicated the "Connection: close" semantic.
			return do_close();
		}

		// We're done with the response so delete it
		res_ = nullptr;

		// Read another request
		do_read();
	}

	void
		do_close()
	{
		// Send a TCP shutdown
		beast::error_code ec;
		stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

		// At this point the connection is closed gracefully
	}
};

//****************************************************************************
//*                     listener
//****************************************************************************
// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
	net::io_context& ioc_;
	tcp::acceptor acceptor_;
	std::shared_ptr<std::string const> doc_root_;
	std::shared_ptr<Connect2SQLite> pSqlite_;
	std::shared_ptr<HandlerForRegister> pHandlerForRegister_;
	std::shared_ptr<HandlerForResetPassword> pHandlerForResetPassword_;
	std::shared_ptr<ServerLogging> pServerLogging_;
	std::shared_ptr<boost::timer::cpu_timer> pTimer_;

public:
	listener(net::io_context& ioc
		, tcp::endpoint endpoint
		, std::shared_ptr<std::string const> const& doc_root
		, std::shared_ptr<Connect2SQLite> pSqlite
		, std::shared_ptr<ServerLogging> pServerLogging
		, std::shared_ptr<boost::timer::cpu_timer> pTimer
	)
		: ioc_(ioc)
		, acceptor_(net::make_strand(ioc))
		, doc_root_(doc_root)
		, pSqlite_(pSqlite)
		, pServerLogging_(pServerLogging)
		, pTimer_(pTimer)
	{
		OutputDebugString(L"<<constructor>> listener()\n");
		HandlerForRegister handlerForRegister;
		pHandlerForRegister_ =
			std::make_shared<HandlerForRegister>(handlerForRegister);
		HandlerForResetPassword handlerForResetPassword;
		pHandlerForResetPassword_ =
			std::make_shared<HandlerForResetPassword>(handlerForResetPassword);

		beast::error_code ec;

		// Open the acceptor
		acceptor_.open(endpoint.protocol(), ec);
		if (ec)
		{
			fail(ec, "open");
			return;
		}

		// Allow address reuse
		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec)
		{
			fail(ec, "set_option");
			return;
		}

		// Bind to the server address
		acceptor_.bind(endpoint, ec);
		if (ec)
		{
			fail(ec, "bind");
			return;
		}

		// Start listening for connections
		acceptor_.listen(
			net::socket_base::max_listen_connections, ec);
		if (ec)
		{
			fail(ec, "listen");
			return;
		}
	}

	// Start accepting incoming connections
	void
		run()
	{
		pTimer_->stop();
		pServerLogging_->store_log("0.0.0.0"
			, "start server"
			, "server alive"
			, pTimer_->elapsed()
		);
		do_accept();
	}

private:
	void
		do_accept()
	{
		// The new connection gets its own strand
		acceptor_.async_accept(
			net::make_strand(ioc_),
			beast::bind_front_handler(
				&listener::on_accept,
				shared_from_this()));
	}

	void
		on_accept(beast::error_code ec, tcp::socket socket)
	{
		if (ec)
		{
			fail(ec, "accept");
		}
		else
		{
			// start timing
			boost::timer::cpu_timer response_timer;

			// get remote endpoint
			std::string remote_endpoint = boost::lexical_cast<std::string>
				(socket.remote_endpoint());

			// construct pointers
			std::shared_ptr<boost::timer::cpu_timer> pResponseTimer =
				std::make_shared<boost::timer::cpu_timer>(response_timer);
			std::shared_ptr<std::string> pRemoteEndpoint =
				std::make_shared<std::string>(remote_endpoint);

			// create the session and run it
			std::make_shared<session>(std::move(socket)
				, doc_root_
				, pSqlite_
				, pHandlerForRegister_
				, pHandlerForResetPassword_
				, pServerLogging_
				, pRemoteEndpoint
				, pResponseTimer
				)->run();
		}

		// Accept another connection
		do_accept();
	}
};

//****************************************************************************
//*                     http_server_async
//****************************************************************************
DWORD WINAPI http_server_async(LPVOID lpVoid)
{
	auto const address = net::ip::make_address("0.0.0.0");
	auto const port = static_cast<unsigned short>(8080);
	auto const doc_root = std::make_shared<std::string>(".");
	auto const threads = 1;
	ServerLogging* pServerLogging =
		(ServerLogging*)((PSTRUCTSERVER)lpVoid)->pServerLogging;
	Connect2SQLite* pSqlite =
		(Connect2SQLite*)((PSTRUCTSERVER)lpVoid)->pSqlite;
	// start timing
	boost::timer::cpu_timer timer;
	// the database object is now created in the ManageServer module
	// and the instance is conveyed via a pointer
	pSqlite->createTable();
	// will create an error message when the default user already exists
	// SQL error: UNIQUE constraint failed: 
	// user_access_login_data.user_email_address
	pSqlite->insertDefaultUser();
	// stop timer
	timer.stop();
	pServerLogging->store_log("0.0.0.0"
		, "sqlite3 db startup"
		, "sqlite3 db started"
		, timer.elapsed()
	);

	// The io_context is required for all I/O
	net::io_context ioc{ threads };

	// create pointers for the sake of the 
	// boost::beast asynchronous http server
	// 1) shared pointer to Connect2SQLite instance
	auto const pSqlite_ =
		std::make_shared<Connect2SQLite>(*pSqlite);
	// 2) shared pointer to ServerLogging instance
	auto const pServerLogging_ =
		std::make_shared<ServerLogging>(*pServerLogging);
	// 3) shared pointer to timer instance 
	auto pTimer =
		std::make_shared<boost::timer::cpu_timer>(timer);
	pTimer->start();
	// Create and launch a listening port
	std::make_shared<listener>(ioc
		, tcp::endpoint{ address, port }
		, doc_root
		, pSqlite_
		, pServerLogging_
		, pTimer)->run();

	// Capture SIGABRT to perform a clean shutdown
	net::signal_set signals(ioc, SIGABRT);
	signals.async_wait(
		[&](beast::error_code const&, int)
	{
		// Stop the `io_context`. This will cause `run()`
		// to return immediately, eventually destroying the
		// `io_context` and all of the sockets in it.
		ioc.stop();
	});

	// Run the I/O service on the requested number of threads
	std::vector<std::thread> v;
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
		v.emplace_back(
			[&ioc]
	{
		ioc.run();
	});
	ioc.run();

	// (if we get here, it means we got a SIGABRT)
	// block until all threads exit
	timer.start();
	for (auto& t : v)
		t.join();
	timer.stop();
	pServerLogging->store_log("0.0.0.0"
		, "stop server"
		, "server dead"
		, timer.elapsed()
	);

	return (DWORD)EXIT_SUCCESS;
}
