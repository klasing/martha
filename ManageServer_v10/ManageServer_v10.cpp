// ManageServer_v10.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ManageServer_v10.h"

#define MAX_LOADSTRING 100

//****************************************************************************
//*                     typedef
//****************************************************************************
// typedef for sqlite
typedef std::string
	  td_id
	, td_time_of_creation
	, td_user_email_address
	, td_user_password;
typedef std::tuple<
	  td_id
	, td_time_of_creation
	, td_user_email_address
	, td_user_password> tuple_user_data;
// typedef for http_server_async thread
typedef struct tagSTRUCTSERVER {
	HWND hWnd = NULL;
} STRUCTSERVER, *PSTRUCTSERVER;

// Global Variables:
//****************************************************************************
//*                     global
//****************************************************************************
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const size_t BUFFER_MAX = 256;
StatusBar oStatusBar;

// Forward declarations of functions included in this code module:
//****************************************************************************
//*                     prototype
//****************************************************************************
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
		// necessary to convey a WM_COMMAND message,
		// containing a (...) message,
		// to the concerning dialog
		//if (IsDialogMessage(FindWindow(NULL, L"Create User"), &msg))
		//	continue;
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
	switch (uMsg)
	{
	case WM_NCCREATE:
		// create a tab control
		oTabControl.createTabControl(hInst, hWnd);
		// create a status bar
		oStatusBar.createStatusBar(hInst, hWnd);
		// initialize the struct value
		pStructServer->hWnd = hWnd;
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
		case IDM_USER_CREATE:
			OutputDebugString(L"IDM_USER_CREATE\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, UserCreateProc
				, (LPARAM)pStructServer
			);
			break;
		case IDM_USER_SELECT_ALL:
			OutputDebugString(L"IDM_USER_SELECT_ALL\n");
			break;
		case IDM_RESOURCE_SELECT_ALL:
			OutputDebugString(L"IDM_RESOURCE_SELECT_ALL\n");
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
			break;
		case IDM_SERVER_START:
			OutputDebugString(L"IDM_SERVER_START\n");
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_START, MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_STOP, MF_ENABLED);
			SendMessage(oTabControl.hWndDlg[2]
				, IDM_SERVER_START
				, (WPARAM)0
				, (LPARAM)0
			);
			break;
		case IDM_SERVER_STOP:
			OutputDebugString(L"IDM_SERVER_STOP\n");
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_START, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), IDM_SERVER_STOP, MF_GRAYED);
			SendMessage(oTabControl.hWndDlg[2]
				, IDM_SERVER_STOP
				, (WPARAM)0
				, (LPARAM)0
			);
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
	static HWND hWndLV;
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
			, 4
			, IDS_LVDB_COL0, IDS_LVDB_COL1, IDS_LVDB_COL2
			, IDS_LVDB_COL3
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
		{
			MoveWindow(hWndLV
				, 0, 0
				, GET_X_LPARAM(lParam)
				, GET_Y_LPARAM(lParam)
				, TRUE
			);
		}
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
	case IDM_MONITORING_START:
		OutputDebugString(L"IDM_MONITORING_START [Tab1Proc]\n");
		oStatusBar.StatusBarSetText(0, 1, L"Monitor ON");
		break;
	case IDM_MONITORING_STOP:
		OutputDebugString(L"IDM_MONITORING_STOP [Tab1Proc]\n");
		oStatusBar.StatusBarSetText(0, 1, L"Monitor OFF");
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
//*                     date_for_http_response
//****************************************************************************
std::string
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
//*                     convert_str_to_wchar
//****************************************************************************
std::wstring
convert_str_to_wstr(const std::string& str)
{
	return std::wstring(str.begin(), str.end()).c_str();
}

//////////////////////////////////////////////////////////////////////////////
//                      boost part
//////////////////////////////////////////////////////////////////////////////
//****************************************************************************
//*                     http_server_async
//****************************************************************************
DWORD WINAPI http_server_async(LPVOID lpVoid)
{
	OutputDebugString(L"http_server_async\n");
	return (DWORD)EXIT_SUCCESS;
}