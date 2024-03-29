// ManageServer_v10.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ManageServer_v10.h"

#define MAX_LOADSTRING 100

//****************************************************************************
//*                     typedef
//****************************************************************************
typedef struct tagSTRUCTSERVER {
	HWND hWnd = NULL;
	std::shared_ptr<ServerLogging> pServerLogging = nullptr;
	std::shared_ptr<Connect2SQLite> pSqlite = nullptr;
	std::shared_ptr<HandlerForRegister> pHandlerForRegister = nullptr;
	std::shared_ptr<HandlerForResetPassword> pHandlerForResetPassword = nullptr;
	std::shared_ptr<boost::timer::cpu_timer> pTimer = nullptr;
	std::shared_ptr<boost::timer::cpu_timer> pResponseTimer = nullptr;
	std::shared_ptr<std::string> remote_endpoint = nullptr;
} STRUCTSERVER, * PSTRUCTSERVER;

//****************************************************************************
//*                     global
//****************************************************************************
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const size_t BUFFER_MAX = 256;
PWCHAR pszTextBuffer = new WCHAR[BUFFER_MAX];
StatusBar oStatusBar;
Connect2SQLite oSqlite;

//****************************************************************************
//*                     prototype
//****************************************************************************
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	UserCreateProc(HWND, UINT, WPARAM, LPARAM);
HWND& helper_for_render_control(std::string
	, const PWCHAR&
	, const DWORD&
	, const HWND&
	, const int&
	, const HINSTANCE&
	, const int&
	, const int&
	, const int&
	, const int&
);
std::string date_for_http_response();
std::wstring convert_str_to_wstr(const std::string&);
// http_server_async function (runs in a WIN32-Thread)
DWORD WINAPI http_server_async(LPVOID);

//****************************************************************************
//*                     wWinMain        
//****************************************************************************
int APIENTRY
wWinMain(_In_ HINSTANCE hInstance
	, _In_opt_ HINSTANCE hPrevInstance
	, _In_ LPWSTR lpCmdLine
	, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	INITCOMMONCONTROLSEX ic;
	ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ic.dwICC = ICC_STANDARD_CLASSES
		| ICC_TAB_CLASSES
		| ICC_LISTVIEW_CLASSES
		| ICC_BAR_CLASSES;
	BOOL bSuccessInit = InitCommonControlsEx(&ic);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MANAGESERVERV10, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MANAGESERVERV10));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//****************************************************************************
//*                     MyRegisterClass        
//****************************************************************************
ATOM
MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MANAGESERVERV10));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MANAGESERVERV10);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
//****************************************************************************
//*                     InitInstance        
//****************************************************************************
BOOL
InitInstance(HINSTANCE hInstance
	, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
//****************************************************************************
//*                     WndProc        
//****************************************************************************
LRESULT CALLBACK
WndProc(HWND hWnd
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTSERVER pStructServer = new STRUCTSERVER;
	static TabControl oTabControl;
	static ServerLogging oServerLogging;
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		// create a tab control
		oTabControl.createTabControl(hInst, hWnd);
		// create a status bar
		oStatusBar.createStatusBar(hInst, hWnd);
		// start timing, on opening the database
		boost::timer::cpu_timer timer;
		// open database
		oSqlite.openDb();
		// create table, if non existent
		oSqlite.createTableUser();
		oSqlite.createTableResource();
		// will create an error message when the default user already exists
		// SQL error: UNIQUE constraint failed: 
		// user_access_login_data.user_email_address
		oSqlite.insertDefaultUser(date_for_http_response());
		// stop timer
		timer.stop();
		oServerLogging.set_hwnd(oTabControl.hWndDlg[1]);
		oServerLogging.store_log("0.0.0.0"
			, "sqlite3 db startup"
			, "sqlite3 db started"
			, timer.elapsed()
			, ""
			, ""
			, "" // req_message, used only by http_async_server
			, "" // res_message, used only by http_async_server
		);
		// initialize structure for asynchronous http server
		pStructServer->hWnd = hWnd;
		pStructServer->pServerLogging = 
			std::make_shared<ServerLogging>(oServerLogging);
		pStructServer->pSqlite =
			std::make_shared<Connect2SQLite>(oSqlite);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	} // eof WM_NCCREATE
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
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			OutputDebugString(L"IDM_ABOUT\n");
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			OutputDebugString(L"IDM_EXIT\n");
			DestroyWindow(hWnd);
			break;
		case IDM_DB_USER_CREATE:
			OutputDebugString(L"IDM_DB_USER_CREATE\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, UserCreateProc
				, (LPARAM)pStructServer
			);
			break;
		case IDM_DB_USER_SELECT_ALL:
			OutputDebugString(L"IDM_DB_USER_SELECT_ALL\n");
			SendMessage(oTabControl.hWndDlg[0]
				, IDM_DB_USER_SELECT_ALL
				, (WPARAM)0
				, (LPARAM)0
			);
			// sqlite will call IDM_DB_USER_SELECT_ALL_CALLBACK in hWndDlg[0]
			oSqlite.selectAll_user(oTabControl.hWndDlg[0]);
			break;
		case IDM_DB_RESOURCE_SELECT_ALL:
			OutputDebugString(L"IDM_DB_RESOURCE_SELECT_ALL\n");
			SendMessage(oTabControl.hWndDlg[0]
				, IDM_DB_RESOURCE_SELECT_ALL
				, (WPARAM)0
				, (LPARAM)0
			);
			// sqlite will call IDM_DB_RESOURCE_SELECT_ALL_CALLBACK in hWndDlg[0]
			oSqlite.selectAll_resource(oTabControl.hWndDlg[0]);
			break;
		case IDM_MONITORING_START:
			OutputDebugString(L"IDM_MONITORINGING_START\n");
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_START, MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_STOP, MF_ENABLED);
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_START
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_MONITORING_STOP:
			OutputDebugString(L"IDM_MONITORING_STOP\n");
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_START, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_STOP, MF_GRAYED);
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_STOP
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_MONITORING_LIST:
			OutputDebugString(L"IDM_MONITORING_LIST\n");
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_LIST, MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_VERBOSE, MF_ENABLED);
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_LIST
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_MONITORING_VERBOSE:
			OutputDebugString(L"IDM_MONITORING_VERBOSE\n");
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_LIST, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), IDM_MONITORING_VERBOSE, MF_GRAYED);
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_VERBOSE
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_MONITORING_CLEAR:
			OutputDebugString(L"IDM_MONITORING_CLEAR\n");
			SendMessage(oTabControl.hWndDlg[1]
				, IDM_MONITORING_CLEAR
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_SERVER_START:
		{
			OutputDebugString(L"IDM_SERVER_START\n");
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_START, MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_STOP, MF_ENABLED);
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
			OutputDebugString(L"IDM_SERVER_STOP\n");
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
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
//****************************************************************************
//*                     About       
//****************************************************************************
INT_PTR CALLBACK
About(HWND hDlg
	, UINT message
	, WPARAM wParam
	, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
//****************************************************************************
//*                     Tab0Proc
//****************************************************************************
// Message handler for the "Database" tab item
INT_PTR CALLBACK 
Tab0Proc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static HWND hWndLV_user; 
	static HWND hWndLV_resource;
	static HMENU popup_menu_user;
	static HMENU popup_menu_resource;
	static int selection_mark_user = -1;
	static int selection_mark_resource = -1;
	static std::string mode = "user";
	switch (uMsg)
	{
	case WM_INITDIALOG:
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Tab0Proc");
		// create listview
		hWndLV_user = createListView(hInst, hDlg, IDC_LVDB_USER);
		// add column to listview
		addColumn(hInst
			, hWndLV_user
			, 4
			, IDS_LVDB_USER_COL0, IDS_LVDB_USER_COL1, IDS_LVDB_USER_COL2
			, IDS_LVDB_USER_COL3
		);
		popup_menu_user = CreatePopupMenu();
		AppendMenu(popup_menu_user
			, MF_STRING
			, IDM_DB_DELETE_USER
			, L"&Delete"
		);
		SetMenu(hWndLV_user
			, popup_menu_user
		);
		// create listview
		hWndLV_resource = createListView(hInst, hDlg, IDC_LVDB_RESOURCE);
		// add column to listview
		addColumn(hInst
			, hWndLV_resource
			, 4
			, IDS_LVDB_RESOURCE_COL0, IDS_LVDB_RESOURCE_COL1, IDS_LVDB_RESOURCE_COL2
			, IDS_LVDB_RESOURCE_COL3
		);
		popup_menu_resource = CreatePopupMenu();
		AppendMenu(popup_menu_resource
			, MF_STRING
			, IDM_DB_DELETE_RESOURCE
			, L"&Delete"
		);
		return (INT_PTR)TRUE;
	case WM_SIZE:
		if (mode == "user")
		{
			MoveWindow(hWndLV_user
				, 0, 0
				, GET_X_LPARAM(lParam)
				, GET_Y_LPARAM(lParam)
				, TRUE
			);
			// annoying GUI hell, size and position of the listview resource 
			// has to be determined, but the window is NOT to be shown
			// in the Tab1Proc a similar case is done differently,
			// to keep the GUI design challenging
			SetWindowPos(hWndLV_resource
				, HWND_TOP
				, 0, 0
				, GET_X_LPARAM(lParam)
				, GET_Y_LPARAM(lParam)
				, SWP_HIDEWINDOW
			);
		}
		if (mode == "resource")
			MoveWindow(hWndLV_resource
				, 0, 0
				, GET_X_LPARAM(lParam)
				, GET_Y_LPARAM(lParam)
				, TRUE
			);
		return 0;
	case WM_NOTIFY:
	{
		LPNMHDR lpNmHdr = (LPNMHDR)lParam;
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_RCLICK:
		{
			switch (lpNmHdr->idFrom)
			{
			case IDC_LVDB_USER:
			{
				selection_mark_user = SendMessage(hWndLV_user
					, LVM_GETSELECTIONMARK
					, (WPARAM)0
					, (LPARAM)0
				);
				if (selection_mark_user == -1)
					return 0;
				// show popup menu
				POINT ptCursorPos;
				GetCursorPos(&ptCursorPos);
				TrackPopupMenu(popup_menu_user
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
			} // eof IDC_LVDB_USER
			case IDC_LVDB_RESOURCE:
			{
				OutputDebugString(L"NM_RCLICK from IDC_LVDB_RESOURCE\n");
				selection_mark_resource = SendMessage(hWndLV_resource
					, LVM_GETSELECTIONMARK
					, (WPARAM)0
					, (LPARAM)0
				);
				if (selection_mark_resource == -1)
					return 0;
				// show popup menu
				POINT ptCursorPos;
				GetCursorPos(&ptCursorPos);
				TrackPopupMenu(popup_menu_resource
					, TPM_RIGHTBUTTON
					, ptCursorPos.x
					, ptCursorPos.y
					, 0
					// the window that receives the IDM_DB_DELETE_RESOURCE message
					// via a WM_COMMAND message
					, hDlg
					, NULL
				);
				return 0;
			} // eof IDC_LVDB_RESOURCE
			} // eof switch
		} // eof NM_RCLICK
		} // eof switch
		// the return value is ignored
		return 0;
	} // eof WM_NOTIFY
	case IDM_DB_USER_SELECT_ALL:
		OutputDebugString(L"IDM_DB_USER_SELECT_ALL [Tab0Proc]\n");
		mode = "user";
		// hide the non-relevant window
		ShowWindow(hWndLV_resource, SW_HIDE);
		// and then showing
		ShowWindow(hWndLV_user, SW_SHOW);
		break;
	case IDM_DB_USER_SELECT_ALL_CALLBACK:
	{
		OutputDebugString(L"IDM_DB_USER_SELECT_ALL_CALLBACK [Tab0Proc]\n");
		// clear listview
		clearListView(hWndLV_user);
		size_t iItem = 0;
		std::vector<tuple_user_data>* pvud =
			(std::vector<tuple_user_data>*)lParam;
		for (auto it = pvud->begin(); it < pvud->end(); ++it)
		{
			tuple_user_data tud = *it;
			addListViewItem(hWndLV_user
				, const_cast<char*>(std::get<0>(tud).c_str())
				, iItem, 0);
			addListViewItem(hWndLV_user
				, const_cast<char*>(std::get<1>(tud).c_str())
				, iItem, 1);
			addListViewItem(hWndLV_user
				, const_cast<char*>(std::get<2>(tud).c_str())
				, iItem, 2);
			addListViewItem(hWndLV_user
				, const_cast<char*>(std::get<3>(tud).c_str())
				, iItem, 3);
			++iItem;
		}
		break;
	} // eof IDM_DB_USER_SELECT_ALL_CALLBACK
	case IDM_DB_RESOURCE_SELECT_ALL:
		OutputDebugString(L"IDM_DB_RESOURCE_SELECT_ALL [Tab0Proc]\n");
		mode = "resource";
		// hide the non-relevant window
		ShowWindow(hWndLV_user, SW_HIDE);
		// and then showing
		ShowWindow(hWndLV_resource, SW_SHOW);
		break;
	case IDM_DB_RESOURCE_SELECT_ALL_CALLBACK:
	{
		OutputDebugString(L"IDM_DB_RESOURCE_SELECT_ALL_CALLBACK [Tab0Proc]\n");
		// clear listview
		clearListView(hWndLV_resource);
		size_t iItem = 0;
		std::vector<tuple_resource_data>* pvrd =
			(std::vector<tuple_resource_data>*)lParam;
		for (auto it = pvrd->begin(); it < pvrd->end(); ++it)
		{
			tuple_resource_data trd = *it;
			addListViewItem(hWndLV_resource
				, const_cast<char*>(std::get<0>(trd).c_str())
				, iItem, 0);
			addListViewItem(hWndLV_resource
				, const_cast<char*>(std::get<1>(trd).c_str())
				, iItem, 1);
			addListViewItem(hWndLV_resource
				, const_cast<char*>(std::get<2>(trd).c_str())
				, iItem, 2);
			addListViewItem(hWndLV_resource
				, const_cast<char*>(std::get<3>(trd).c_str())
				, iItem, 3);
			++iItem;
		}
		break;
	} // eof IDM_DB_USER_SELECT_ALL_CALLBACK
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_DB_DELETE_USER:
		{
			OutputDebugString(L"IDM_DB_DELETE_USER [Tab0Proc]\n");
			if (selection_mark_user == -1)
				break;
			PWCHAR pszText = new WCHAR[64];
			// find the primary key ID for the user that is going to be deleted
			getListViewItem(hWndLV_user
				, selection_mark_user
				, 0
				, pszText
			);
			// delete the user from the database,
			// with the found primary key ID
			oSqlite.deleteRow("user", _wtoi(pszText));
			// delete selected listview item
			SendMessage(hWndLV_user
				, LVM_DELETEITEM
				, (WPARAM)selection_mark_user
				, (LPARAM)0
			);
			// clean up
			delete[] pszText;
			break;
		} // eof IDM_DB_DELETE_USER
		case IDM_DB_DELETE_RESOURCE:
		{
			OutputDebugString(L"IDM_DB_DELETE_RESOURCE [Tab0Proc]\n");
			if (selection_mark_resource == -1)
				break;
			PWCHAR pszText = new WCHAR[64];
			// find the primary key ID for the user that is going to be deleted
			getListViewItem(hWndLV_resource
				, selection_mark_resource
				, 0
				, pszText
			);
			// delete the user from the database,
			// with the found primary key ID
			oSqlite.deleteRow("resource", _wtoi(pszText));
			// delete selected listview item
			SendMessage(hWndLV_resource
				, LVM_DELETEITEM
				, (WPARAM)selection_mark_resource
				, (LPARAM)0
			);
			// clean up
			delete[] pszText;
			break;
		} // eof IDM_DB_DELETE_RESOURCE
		} // eof switch
		break;
	} // eof switch
	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     Tab1Proc
//****************************************************************************
// Message handler for the "Logging" tab item
INT_PTR CALLBACK 
Tab1Proc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static HWND hWndLV;
	static GroupBoxRequest groupBoxRequest;
	static GroupBoxResponse groupBoxResponse;
	static HWND hWndGroupBoxRequest, hWndGroupBoxResponse;
	static std::string mode = "list";
	static BOOL bMonitor = TRUE;
	static UINT iItem = 0;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Tab1Proc");
		// create listview
		hWndLV = createListView(hInst, hDlg, IDC_LVLOG);
		// add column to listview
		addColumn(hInst
			, hWndLV
			, 7
			, IDS_LVLOG_COL0, IDS_LVLOG_COL1, IDS_LVLOG_COL2
			, IDS_LVLOG_COL3, IDS_LVLOG_COL4, IDS_LVLOG_COL5
			, IDS_LVLOG_COL6
		);
		// create render for verbose
		hWndGroupBoxRequest = groupBoxRequest.createGroupBox(hInst
			, hDlg
			, IDC_GB_REQUEST
		);
		hWndGroupBoxResponse = groupBoxResponse.createGroupBox(hInst
			, hDlg
			, IDC_GB_RESPONSE
		);
		return (INT_PTR)TRUE;
	case WM_SIZE:
		if (mode == "list")
			MoveWindow(hWndLV
				, 0, 0
				, GET_X_LPARAM(lParam)
				, GET_Y_LPARAM(lParam)
				, TRUE
			);
		if (mode == "verbose")
		{
			RECT clientRect;
			GetClientRect(hDlg, &clientRect);
			int width_groupbox =
				clientRect.right / 2 - 20;
			int height_groupbox =
				clientRect.bottom - 20;
			groupBoxRequest.SetGroupBox(hWndGroupBoxRequest
				, 10
				, 10
				, width_groupbox
				, height_groupbox
			);
			groupBoxResponse.SetGroupBox(hWndGroupBoxResponse
				, clientRect.right / 2 + 10
				, 10
				, width_groupbox
				, height_groupbox
			);
		}
		return 0;
	case IDM_MONITORING_CLEAR:
		OutputDebugString(L"IDM_MONITORING_CLEAR [Tab1Proc]\n");
		clearListView(hWndLV);
		iItem = 0;
		break;
	case IDM_MONITORING_LOGFILESIZE:
		OutputDebugString(L"IDM_MONITORING_LOGFILESIZE [Tab1Proc]\n");
		oStatusBar.StatusBarSetText(0, 5, (PWCHAR)lParam);
		break;
	case IDM_MONITORING_START:
		OutputDebugString(L"IDM_MONITORING_START [Tab1Proc]\n");
		oStatusBar.StatusBarSetText(0, 1, L"Monitor ON");
		bMonitor = TRUE;
		break;
	case IDM_MONITORING_STOP:
		OutputDebugString(L"IDM_MONITORING_STOP [Tab1Proc]\n");
		oStatusBar.StatusBarSetText(0, 1, L"Monitor OFF");
		bMonitor = FALSE;
		break;
	case IDM_MONITORING_LIST:
	{
		OutputDebugString(L"IDM_MONITORING_LIST [Tab1Proc]\n");
		mode = "list";
		// hide the non-relevant window
		ShowWindow(hWndGroupBoxRequest, SW_HIDE);
		ShowWindow(hWndGroupBoxResponse, SW_HIDE);
		// show the relevant window
		// by recalculating size and position
		RECT clientRect;
		GetClientRect(hDlg, &clientRect);
		MoveWindow(hWndLV
			, 0, 0
			, clientRect.right
			, clientRect.bottom
			, TRUE
		);
		// and then showing
		ShowWindow(hWndLV, SW_SHOW);
		break;
	} // eof IDM_MONITORING_LIST
	case IDM_MONITORING_VERBOSE:
	{
		OutputDebugString(L"IDM_MONITORING_VERBOSE [Tab1Proc]\n");
		mode = "verbose";
		// hide the non-relevant window
		ShowWindow(hWndLV, SW_HIDE);
		// show the relevant window
		// by recalculating size and position
		RECT clientRect;
		GetClientRect(hDlg, &clientRect);
		int width_groupbox =
			clientRect.right / 2 - 20;
		int height_groupbox =
			clientRect.bottom - 20;
		// and then showing
		groupBoxRequest.SetGroupBox(hWndGroupBoxRequest
			, 10
			, 10
			, width_groupbox
			, height_groupbox
		);
		groupBoxResponse.SetGroupBox(hWndGroupBoxResponse
			, clientRect.right / 2 + 10
			, 10
			, width_groupbox
			, height_groupbox
		);
		break;
	} // eof IDM_MONITORING_VERBOSE
	case IDM_LOG_MSG_VERBOSE:
	{
		OutputDebugString(L"IDM_LOG_MSG_VERBOSE [Tab1Proc]\n");
		tuple_logging_verbose* ptlv =
			(tuple_logging_verbose*)lParam;
		groupBoxRequest.setGroupBoxText(
			ptlv->get<0>().c_str()
		);
		groupBoxResponse.setGroupBoxText(
			ptlv->get<1>().c_str()
		);
		break;
	} // eof IDM_LOG_MSG_VERBOSE
	case IDM_LOG_MSG:
		OutputDebugString(L"IDM_LOG_MSG [Tab1Proc]\n");
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
			addListViewItem(hWndLV
				, ptl->get<5>().c_str()
				, iItem, 5);
			addListViewItem(hWndLV
				, ptl->get<6>().c_str()
				, iItem, 6);
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
INT_PTR CALLBACK 
Tab2Proc(HWND hDlg
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
		OutputDebugString(L"IDM_SERVER_START [Tab2Proc]\n");
		oStatusBar.StatusBarSetText(0, 2, L"Server ALIVE");
		break;
	case IDM_SERVER_STOP:
		OutputDebugString(L"IDM_SERVER_STOP [Tab2Proc]\n");
		oStatusBar.StatusBarSetText(0, 2, L"Server DEAD");
		break;
	} // eof switch
	return (INT_PTR)FALSE;
}
//****************************************************************************
//*                     UserCreateProc
//****************************************************************************
INT_PTR CALLBACK 
UserCreateProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTSERVER pStructServer;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// enables communication with parent dialog
		pStructServer = (PSTRUCTSERVER)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Create User");
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(pStructServer->hWnd, &point);
		SetWindowPos(hDlg
			, HWND_TOP
			, point.x
			, point.y
			, 420, 190
			, SWP_SHOWWINDOW
		);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"Created"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 30, 95, 16
		);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"User email address"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 50, 95, 16
		);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"Password"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 70, 95, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_TOC
			, hInst
			, 170, 30, 200, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_UEA // UEA is User Email Address
			, hInst
			, 170, 50, 200, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_UP // UP is User Password
			, hInst
			, 170, 70, 200, 16
		);
		HWND hWndBtnSubmit =
			helper_for_render_control((std::string)"button"
				, (PWCHAR)L"Submit"
				, 0
				, hDlg
				, IDC_BTN_USER_CREATE_SUBMIT
				, hInst
				, 300, 100, 70, 22
			);
		// set the time of creation
		SendMessage(GetDlgItem(hDlg, IDC_EDT_TOC)
			, WM_SETTEXT
			, (WPARAM)0
			, (LPARAM)convert_str_to_wstr(date_for_http_response()).c_str());
		// set the time of creation edit control to read only
		SendMessage(GetDlgItem(hDlg, IDC_EDT_TOC)
			, EM_SETREADONLY
			, (WPARAM)TRUE
			, (LPARAM)0);
		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_USER_CREATE_SUBMIT:
			OutputDebugString(L"IDC_BTN_USER_CREATE_SUBMIT [UserCreateProc]\n");
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof switch
		break;
	} // eof switch
	return (INT_PTR)FALSE;
}
//****************************************************************************
//*                     helper_for_render_control
//****************************************************************************
HWND& helper_for_render_control(std::string typeControl
	, const PWCHAR& lpWindowName
	, const DWORD& dwStyle
	, const HWND& hWndParent
	, const int& resourceId
	, const HINSTANCE& hInst
	, const int& x
	, const int& y
	, const int& cx
	, const int& cy
)
{
	// change case to uppercase
	std::transform(typeControl.begin()
		, typeControl.end()
		, typeControl.begin()
		, ::toupper);
	// convert to wide string
	std::wstring wTypeControl = std::wstring(typeControl.begin()
		, typeControl.end());
	// assign the std::wstring to a wchar_t*,  which is PWCHAR in WIN-32
	// a value of type const wchar_t* cannot be used to initialize an entity of PWCHAR
	// therefore a const_cast<PWCHAR>(var.c_str()) is necessary
	PWCHAR lpClassName = const_cast<PWCHAR>(wTypeControl.c_str());
	HWND hWnd = CreateWindow(lpClassName
		, lpWindowName
		, WS_CHILD | dwStyle //| WS_BORDER
		, 0, 0, 0, 0
		, hWndParent
		, (HMENU)resourceId
		, hInst
		, NULL
	);
	SendMessage(hWnd
		, WM_SETFONT
		, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
		, (LPARAM)0
	);
	SetWindowPos(hWnd
		, HWND_TOP
		, x, y, cx, cy
		, SWP_SHOWWINDOW);
	return hWnd;
}
//****************************************************************************
//*                     convert_str_to_wchar
//****************************************************************************
std::wstring
convert_str_to_wstr(const std::string& str)
{
	return std::wstring(str.begin(), str.end()).c_str();
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

//////////////////////////////////////////////////////////////////////////////
//                      boost part
//////////////////////////////////////////////////////////////////////////////
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//****************************************************************************
//*                     global
//****************************************************************************
const int SECONDS_BEFORE_EXPIRING = 300;
//****************************************************************************
//*                     filter_email_password_code
//****************************************************************************
template<class Body, class Allocator>
void filter_email_password_code(
	http::request<Body, http::basic_fields<Allocator>> req
	, std::string& user_email_address
	, std::string& user_password
	, std::string& user_code
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
//*                     filter_filename_payload_from_form_submit
//****************************************************************************
// browser form submit for a file upload 
// delivered within a POST message 
auto
filter_filename_payload_from_form_submit(const std::string req_body
	, std::string& user_email_address
	, std::string& file_name
	, std::string& payload
) -> void
{
	OutputDebugStringA(req_body.c_str());
	// copy the request body into the payload, the payload
	// will be searched and edited
	payload = req_body;

	int sbegin = 0;
	int send = 0;
	std::string string_to_search_for = "";
	// filter user_email_address
	string_to_search_for = "name=\"user_email_address\"";
	sbegin = payload.find(string_to_search_for);
	sbegin += string_to_search_for.length();
	send = sbegin;
	// look for return, two times, using sbegin as an offset
	string_to_search_for = "\r";
	for (int i = 0; i < 2; i++) {
		send = payload.find(string_to_search_for, sbegin);
		sbegin = send + 1;
	}
	// remove everything up to the send position,
	// plus 2 for a remaining \r\n
	send += 2;
	payload.erase(0, send);
	// now look for another return, this will be the end
	// of the user_email_address value
	send = payload.find(string_to_search_for, 0);
	user_email_address = payload.substr(0, send);

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
	//payload = req_body;
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
save_to_disk(std::shared_ptr<Connect2SQLite> pSqlite
	, const std::string& file_name
	, const std::string& payload
	, const std::string& user
) -> void
{
	// Store the received file on disk
	std::string file_name_on_server =
		std::string("user_space/") +
		file_name;
	boost::filesystem::path p{ file_name_on_server };
	boost::filesystem::ofstream ofs{ p };
	ofs << payload;
	// insert resource meta-data into the database
	pSqlite->insertResource(date_for_http_response()
		, file_name
		, user
	);
}
//****************************************************************************
//*                     output_buffer
//****************************************************************************
class output_buffer : public std::streambuf
{
	static const unsigned BUFFER_MAX = 8192; // 8k buffer
	char* pBuffer = new char[BUFFER_MAX];
	unsigned iBuffer = 0;
public:
	output_buffer()
	{
		clear_and_reset_buffer();
	}
	void clear_and_reset_buffer()
	{
		for (int i = 0; i < BUFFER_MAX; ++i)
			pBuffer[i] = '\0';
		iBuffer = 0;
 	}
	char* get_buffer() const
	{
		return pBuffer;
	}
protected:
	virtual int_type overflow(int_type c)
	{
		if (c != EOF && iBuffer < BUFFER_MAX - 2)
			if (iBuffer == BUFFER_MAX - 2)
				pBuffer[iBuffer++] = '\n';
			else
				pBuffer[iBuffer++] = c;
		return c;
	}
};
//****************************************************************************
//*                     output_stream
//****************************************************************************
class output_stream : public std::ostream
{
	output_buffer ob;
public:
	output_stream() : std::ostream(&ob) {}
	char* get_buffer()
	{
		return ob.get_buffer();
	}
	void clear_and_reset_buffer()
	{
		ob.clear_and_reset_buffer();
	}
};
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
//*                     handle_request
//****************************************************************************
// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<class Body, class Allocator, class Send>
void
handle_request(beast::string_view doc_root
	, std::shared_ptr<STRUCTSERVER> pStructServer
	, http::request<Body, http::basic_fields<Allocator>>&& req
	, Send&& send
)
{
	// application user_agent:
	const std::string APP_VALUE_USER_AGENT = "Boost.Beast/248";
	// not sure if browsers will keep this string constant over time...
	// Google Chrome browser user_agent:
	const std::string CHROME_VALUE_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko)";

	// stream the request into a buffer
	output_stream os;
	os << req << std::endl;
	std::string req_message = os.get_buffer();
	OutputDebugStringA(req_message.c_str());

	auto filter_start_line = [](const std::string& message)
	{
		// return the first line of a request message
		return message.substr(0, message.find('\r'));
	};

	// get the start-line
	std::string requestLogMessage =
		filter_start_line(req_message);
	// get the user_agent
	std::string user_agent =
		static_cast<std::string>(req[http::field::user_agent]);
	// get the user
	std::string user = "";
	if (user_agent == APP_VALUE_USER_AGENT)
		user = static_cast<std::string>(req[http::field::from]);
	user_agent = user_agent.substr(0, user_agent.find(" Chrome/"));
	if (user_agent == CHROME_VALUE_USER_AGENT)
	{
		user = static_cast<std::string>(req[http::field::from]);
		if (user == "")
		{
			user = static_cast<std::string>(req[http::field::cookie]);
			size_t sbegin = user.find("=", 0);
			sbegin++;
			user = user.substr(sbegin, user.length() - sbegin);
		}
	}

	// get the target
	std::string target =
		static_cast<std::string>(req.target());

	auto send_response = [&](auto status
		, const std::string& content_type
		, const std::string& response_payload)
	{
		// prepare a response message
		http::response<http::string_body> res;
		res.result(status);
		res.version(11);
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::date, beast::string_view(date_for_http_response()));
		res.set(http::field::content_type, content_type);
		res.keep_alive(req.keep_alive());
		res.content_length(response_payload.length());
		res.body() = response_payload;
		res.prepare_payload();
		// stream the response into a std::string, and filter
		// the response start-line for logging
		os.clear_and_reset_buffer();
		os << res << std::endl;
		std::string res_message = os.get_buffer();
		OutputDebugStringA(res_message.c_str());
		std::string responseLogMessage =
			filter_start_line(res_message);
		// stop the timer and log the HTTP request/response message
		pStructServer->pResponseTimer->stop();
		pStructServer->pServerLogging->store_log(
			*pStructServer->remote_endpoint
			, requestLogMessage
			, responseLogMessage
			, pStructServer->pResponseTimer->elapsed()
			, user
			, user_agent
			, req_message
			, res_message
		);
		// send the response message
		return send(std::move(res));
	};

	auto const send_error = [&](
		http::response<http::string_body> res)
	{
		// stream the response into a std::string, and filter
		// the response start-line for logging
		os.clear_and_reset_buffer();
		os << res << std::endl;
		std::string res_message = os.get_buffer();
		OutputDebugStringA(res_message.c_str());
		std::string responseLogMessage =
			filter_start_line(res_message);
		// stop the timer and log the HTTP request/response message
		pStructServer->pResponseTimer->stop();
		pStructServer->pServerLogging->store_log(
			*pStructServer->remote_endpoint
			, requestLogMessage
			, responseLogMessage
			, pStructServer->pResponseTimer->elapsed()
			, user
			, user_agent
			, req_message
			, res_message
		);
		return send(std::move(res));
	};

	// Returns a bad request response
	auto const bad_request = [&, req](beast::string_view why)
	{
		// prepare a response message
		http::response<http::string_body> res;
		res.result(http::status::bad_request);
		res.version(11);
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::date, beast::string_view(date_for_http_response()));
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = std::string(why);
		res.prepare_payload();
		return send_error(res);
	};

	// Returns a not found response
	auto const not_found = [&, req](beast::string_view target)
	{
		// prepare a response message
		http::response<http::string_body> res;
		res.result(http::status::not_found);
		res.version(11);
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::date, beast::string_view(date_for_http_response()));
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "the resource '" + std::string(target) + "' was not found";
		res.prepare_payload();
		return send_error(res);
	};

	// Returns a server error response
	auto const server_error = [&, req](beast::string_view what)
	{
		// prepare a response message
		http::response<http::string_body> res;
		res.result(http::status::internal_server_error);
		res.version(11);
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::date, beast::string_view(date_for_http_response()));
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "an error occurred: '" + std::string(what) + "'";
		res.prepare_payload();
		return send_error(res);
	};

	// Make sure we can handle the method
	if (req.method() != http::verb::connect &&
		req.method() != http::verb::delete_ &&
		req.method() != http::verb::get &&
		req.method() != http::verb::head &&
		req.method() != http::verb::options &&
		req.method() != http::verb::post &&
		req.method() != http::verb::put &&
		req.method() != http::verb::trace)
		return bad_request("unknown HTTP-method");

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
		if (user_agent == APP_VALUE_USER_AGENT)
		{
			OutputDebugString(L"-> from an application\n");
			std::string path = path_cat(doc_root, req.target());
			// Request path must be absolute and not contain "..".
			if (req.target().empty() ||
				req.target()[0] != '/' ||
				req.target().find("..") != beast::string_view::npos)
				return bad_request("Illegal request-target");
			if (req.target().back() == '/')
				path.append("index.html");
			// Attempt to open the file
			beast::error_code ec;
			http::file_body::value_type body;
			body.open(path.c_str(), beast::file_mode::scan, ec);
			// Handle the case where the file doesn't exist
			if (ec == beast::errc::no_such_file_or_directory)
				return not_found(req.target());
			// Handle an unknown error
			if (ec)
				return server_error(ec.message());
			// Cache the size since we need it after the move
			auto const size = body.size();
			http::response<http::file_body> res{
				std::piecewise_construct,
				std::make_tuple(std::move(body)),
				std::make_tuple(http::status::ok, req.version()) };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::date, beast::string_view(date_for_http_response()));
			res.set(http::field::content_type, mime_type(path));
			res.content_length(size);
			res.keep_alive(req.keep_alive());
			// stream the response into a std::string, and filter
			// the response start-line for logging
			os.clear_and_reset_buffer();
			os << res << std::endl;
			std::string res_message = os.get_buffer();
			OutputDebugStringA(res_message.c_str());
			std::string responseLogMessage =
				filter_start_line(res_message);
			// stop the timer and log the HTTP request/response message
			pStructServer->pResponseTimer->stop();
			pStructServer->pServerLogging->store_log(
				*pStructServer->remote_endpoint
				, requestLogMessage
				, responseLogMessage
				, pStructServer->pResponseTimer->elapsed()
				, user
				, user_agent
				, req_message
				, res_message
			);
			return send(std::move(res));
		}
		user_agent = user_agent.substr(0, user_agent.find(" Chrome/"));
		if (user_agent == CHROME_VALUE_USER_AGENT)
		{
			OutputDebugString(L"-> from a browser\n");
			std::string s = req.target().to_string();
			if (s.find("user_space") == std::string::npos)
			{
				OutputDebugString(L"-> for rendering the browser layout\n");
				std::string path = path_cat(doc_root, req.target());
				// Request path must be absolute and not contain "..".
				if (req.target().empty() ||
					req.target()[0] != '/' ||
					req.target().find("..") != beast::string_view::npos)
					return bad_request("Illegal request-target");
				if (req.target().back() == '/')
					path.append("index.html");
				// Attempt to open the file
				beast::error_code ec;
				http::file_body::value_type body;
				body.open(path.c_str(), beast::file_mode::scan, ec);
				// Handle the case where the file doesn't exist
				if (ec == beast::errc::no_such_file_or_directory)
					return not_found(req.target());
				// Handle an unknown error
				if (ec)
					return server_error(ec.message());
				// Cache the size since we need it after the move
				auto const size = body.size();
				http::response<http::file_body> res{
					std::piecewise_construct,
					std::make_tuple(std::move(body)),
					std::make_tuple(http::status::ok, req.version()) };
				res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
				res.set(http::field::date, beast::string_view(date_for_http_response()));
				res.set(http::field::content_type, mime_type(path));
				res.content_length(size);
				res.keep_alive(req.keep_alive());
				// stream the response into a std::string, and filter
				// the response start-line for logging
				os.clear_and_reset_buffer();
				os << res << std::endl;
				std::string res_message = os.get_buffer();
				OutputDebugStringA(res_message.c_str());
				std::string responseLogMessage =
					filter_start_line(res_message);
				// stop the timer and log the HTTP request/response message
				pStructServer->pResponseTimer->stop();
				pStructServer->pServerLogging->store_log(
					*pStructServer->remote_endpoint
					, requestLogMessage
					, responseLogMessage
					, pStructServer->pResponseTimer->elapsed()
					, user
					, user_agent
					, req_message
					, res_message
				);
				return send(std::move(res));
			}
			if (s.find("user_space") < std::string::npos)
			{
				OutputDebugString(L"-> for a browser file download request\n");
				std::string path = "";
				std::string s = req.target().to_string();
				std::string t = "/server_space";
				if (s.find("user_space") < std::string::npos)
				{
					s = s.substr(t.length(), s.length() - t.length());
					path = path_cat(doc_root, s);
				}
				// Request path must be absolute and not contain "..".
				if (req.target().empty() ||
					req.target()[0] != '/' ||
					req.target().find("..") != beast::string_view::npos)
					return bad_request("Illegal request-target");
				if (req.target().back() == '/')
					path.append("index.html");
				// Attempt to open the file
				beast::error_code ec;
				http::file_body::value_type body;
				body.open(path.c_str(), beast::file_mode::scan, ec);
				// Handle the case where the file doesn't exist
				if (ec == beast::errc::no_such_file_or_directory)
					return not_found(req.target());
				// Handle an unknown error
				if (ec)
					return server_error(ec.message());
				// Cache the size since we need it after the move
				auto const size = body.size();
				http::response<http::file_body> res{
					std::piecewise_construct,
					std::make_tuple(std::move(body)),
					std::make_tuple(http::status::ok, req.version()) };
				res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
				res.set(http::field::date, beast::string_view(date_for_http_response()));
				res.set(http::field::content_type, mime_type(path));
				res.content_length(size);
				res.keep_alive(req.keep_alive());
				// stream the response into a std::string, and filter
				// the response start-line for logging
				os.clear_and_reset_buffer();
				os << res << std::endl;
				std::string res_message = os.get_buffer();
				OutputDebugStringA(res_message.c_str());
				std::string responseLogMessage =
					filter_start_line(res_message);
				// stop the timer and log the HTTP request/response message
				pStructServer->pResponseTimer->stop();
				pStructServer->pServerLogging->store_log(
					*pStructServer->remote_endpoint
					, requestLogMessage
					, responseLogMessage
					, pStructServer->pResponseTimer->elapsed()
					, user
					, user_agent
					, req_message
					, res_message
				);
				return send(std::move(res));
			}
		}
	}

	// Respond to a HEAD request
	if (req.method() == http::verb::head) {
		OutputDebugString(L"-> HEAD message received\n");
		// not implemented yet
	}

	// Respond to a OPTIONS request
	if (req.method() == http::verb::options) {
		OutputDebugString(L"-> OPTIONS message received\n");
		// not implemented yet
	}

	// Respond to a POST request
	if (req.method() == http::verb::post) {
		OutputDebugString(L"-> POST message received\n");
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
					, pStructServer->pSqlite
					, response_payload
				);
			}
			if (target == "/register")
			{
				pStructServer->pHandlerForRegister->handle_register(
					user_email_address
					, user_password
					, pStructServer->pSqlite
					, response_payload
				);
			}
			if (target == "/register_confirm")
			{
				pStructServer->pHandlerForRegister->handle_register_confirm(
					user_email_address
					, user_password
					, user_code
					, pStructServer->pSqlite
					, response_payload
				);
			}
			if (target == "/reset_password")
			{
				pStructServer->pHandlerForResetPassword->handle_reset_password(
					user_email_address
					, user_password
					, pStructServer->pSqlite
					, response_payload
				);
			}
			if (target == "/reset_password_confirm")
			{
				pStructServer->pHandlerForResetPassword->handle_reset_password_confirm(
					user_email_address
					, user_password
					, user_code
					, pStructServer->pSqlite
					, response_payload
				);
			}
			// send the response message
			return send_response(http::status::ok
				, "text/html"
				, response_payload
			);
		}
		std::string user = "";
		std::string file_name = "";
		std::string payload = "";
		filter_filename_payload_from_form_submit(
			static_cast<std::string>(req.body())
			, user
			, file_name
			, payload
		);
		save_to_disk(pStructServer->pSqlite, file_name, payload, user);
		// send the response message
		return send_response(http::status::no_content
			, "text/html" // mandatory!!!
			, ""
		);
	}

	// Respond to a PUT request
	if (req.method() == http::verb::put) {
		OutputDebugString(L"-> PUT message received\n");
		// remove all the \r-characters (return) from the req.body()
		boost::erase_all(req.body(), "\r");
		std::string file_name = static_cast<std::string>(req.target());
		std::string request_payload = req.body();
		save_to_disk(pStructServer->pSqlite, file_name, request_payload, user);
		// send the response message
		return send_response(http::status::ok
			, "message/http"
			, ""
		);
	}

	// Respond to a TRACE request
	if (req.method() == http::verb::trace) {
		OutputDebugString(L"-> TRACE message received\n");
		// the request is echoed back to the client,
		// inside the payload of the response
		// in this application a non-standard comment is added
		http::string_body::value_type response_payload;
		response_payload = 
			std::string("server is alive\r\n") + req_message;
		// send the response message
		return send_response(http::status::ok
			, "message/http"
			, response_payload
		);
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
	const size_t sizeInWords = std::strlen(what) + 1;
	size_t* pReturnValue = new size_t();
	size_t count = _TRUNCATE;
	wchar_t* wcsWhat = new wchar_t[sizeInWords];
	errno_t err = mbstowcs_s(pReturnValue
		, wcsWhat
		, sizeInWords
		, what
		, count
	);
	OutputDebugString(wcsWhat);
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
		{}
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
			http::async_write(self_.stream_
				, *sp
				, beast::bind_front_handler(&session::on_write
					, self_.shared_from_this()
					, sp->need_eof())
			);
		}
	};
	beast::tcp_stream stream_;
	beast::flat_buffer buffer_;
	std::shared_ptr<std::string const> doc_root_;
	http::request<http::string_body> req_;
	std::shared_ptr<STRUCTSERVER> pStructServer_;
	std::shared_ptr<void> res_;
	send_lambda lambda_;
public:
	// Take ownership of the stream
	session(tcp::socket&& socket
		, std::shared_ptr<std::string const> const& doc_root
		, std::shared_ptr<STRUCTSERVER> const& pStructServer
	)
		: stream_(std::move(socket))
		, doc_root_(doc_root)
		, pStructServer_(pStructServer)
		, lambda_(*this)
	{}
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
		stream_.expires_after(std::chrono::seconds(SECONDS_BEFORE_EXPIRING));
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
			, pStructServer_
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
	std::shared_ptr<STRUCTSERVER> pStructServer_;
public:
	listener(net::io_context& ioc
		, tcp::endpoint endpoint
		, std::shared_ptr<std::string const> const& doc_root
		, std::shared_ptr<STRUCTSERVER> pStructServer

	)
		: ioc_(ioc)
		, acceptor_(net::make_strand(ioc))
		, doc_root_(doc_root)
		, pStructServer_(pStructServer)
	{
		OutputDebugString(L"<<constructor>> listener()\n");
		HandlerForRegister handlerForRegister;
		pStructServer_->pHandlerForRegister =
			std::make_shared<HandlerForRegister>(handlerForRegister);
		HandlerForResetPassword handlerForResetPassword;
		pStructServer_->pHandlerForResetPassword =
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
		// the server is running, so the timer can be stopped
		// and a message can be logged
		pStructServer_->pTimer->stop();
		pStructServer_->pServerLogging->store_log("0.0.0.0"
			, "start server"
			, "server alive"
			, pStructServer_->pTimer->elapsed()
			, ""
			, ""
			, "" // req_message, used only for client to server messages
			, "" // res_message, used only for client to server messages
		);
		do_accept();
	}
private:
	void
		do_accept()
	{
		// The new connection gets its own strand
		acceptor_.async_accept(net::make_strand(ioc_)
			, beast::bind_front_handler(
				&listener::on_accept
				, shared_from_this())
		);
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
			// start timing, on a new connection
			// the timer, times the duration for the server to respond
			boost::timer::cpu_timer response_timer;
			// get remote endpoint
			std::string remote_endpoint = boost::lexical_cast<std::string>
				(socket.remote_endpoint());
			// construct pointers
			pStructServer_->pResponseTimer =
				std::make_shared<boost::timer::cpu_timer>(response_timer);
			pStructServer_->remote_endpoint =
				std::make_shared<std::string>(remote_endpoint);
			// create the session and run it
			std::make_shared<session>(std::move(socket)
				, doc_root_
				, pStructServer_
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
	OutputDebugString(L"http_server_async\n");
	auto pStructServer = std::make_shared<STRUCTSERVER>(
		*((PSTRUCTSERVER)lpVoid)
	);

	//ServerLogging* pServerLogging =
	//	(ServerLogging*)((PSTRUCTSERVER)lpVoid)->pServerLogging;

	auto const address = net::ip::make_address("0.0.0.0");
	auto const port = static_cast<unsigned short>(8080);
	auto const doc_root = std::make_shared<std::string>(".");
	auto const threads = 1;

	// start timing, on server startup
	boost::timer::cpu_timer timer;
	// create pointer for the sake of the 
	pStructServer->pTimer = 
		std::make_shared<boost::timer::cpu_timer>(timer);

	// The io_context is required for all I/O
	net::io_context ioc{ threads };
	// Create and launch a listening port
	std::make_shared<listener>(ioc
		, tcp::endpoint{ address, port }
		, doc_root
		, pStructServer)->run();

	// Capture SIGABRT to perform a clean shutdown
	net::signal_set signals(ioc, SIGABRT);
	signals.async_wait(
		[&](beast::error_code const&, int)
		{
			// Stop the `io_context`. This will cause `run()`
			// to return immediately, eventually destroying the
			// `io_context` and all of the sockets in it.
			ioc.stop();
		}
	);

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
	// start timing, on server shutdown
	timer.start();
	for (auto& t : v)
		t.join();
	timer.stop();
	pStructServer->pServerLogging->store_log("0.0.0.0"
		, "stop server"
		, "server dead"
		, timer.elapsed()
		, ""
		, ""
		, "" // req_message, used only for client to server messages
		, "" // res_message, used only for client to server messages
	);

	return (DWORD)EXIT_SUCCESS;
}