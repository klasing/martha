//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Client_Win32.h"

//****************************************************************************
//*                     typedef
//****************************************************************************
typedef struct tagSTRUCTCLIENT {
	HWND hWnd = NULL;
	PWCHAR pszDomain = nullptr;
	PWCHAR pszHost = nullptr;
	PWCHAR pszPort = nullptr;
	int HttpVersion = 0;
	bool bConnected = false;
	bool bLoggedin = false;
	PWCHAR pszUEA = nullptr;
	PWCHAR pszUP = nullptr;
	PWCHAR pszUC = nullptr;
	std::string target = "/";
	std::string mode = "";
	std::string payload = "";
	std::string response_body;	// contains message from server
} STRUCTCLIENT, *PSTRUCTCLIENT;

//****************************************************************************
//*                     global
//****************************************************************************
const size_t BUFFER_MAX = 256;
PWCHAR pszTextBuffer = new WCHAR[BUFFER_MAX];
StatusBar oStatusBar;

//****************************************************************************
//*                     prototype
//****************************************************************************
// forward declarations of functions included in this module:
ATOM				MyRegisterClass(HINSTANCE, const PWCHAR&);
BOOL				InitInstance(HINSTANCE, const int&, const PWCHAR&, const PWCHAR&);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ConnectProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK	DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	LoginProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ForgotPasswordProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	RegisterProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DownloadProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	UploadProc(HWND, UINT, WPARAM, LPARAM);
VOID set_window_login(const HWND&
	, const HINSTANCE&
	, const BOOL&
);
VOID set_window_with_user_code(const HWND&
	, const HINSTANCE&
);
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

// http_client_async function (runs in a WIN32-Thread)
DWORD WINAPI		http_client_async(LPVOID);

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
		if (IsDialogMessage(FindWindow(NULL, L"Connect"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Login"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Forgot Password"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Register"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Download"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Upload"), &msg))
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENTWIN32));
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
LRESULT CALLBACK WndProc(HWND hWnd
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTCLIENT pStructClient = new STRUCTCLIENT;
	//static HWND hWndDlg;

	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		// create a status bar
		oStatusBar.createStatusBar(hInst, hWnd);
		//hWndDlg = CreateDialog(hInst
		//	, L"SUBWINDOW"
		//	, hWnd
		//	, DlgProc
		//);

		pStructClient->hWnd = hWnd;
		pStructClient->pszHost = new WCHAR[BUFFER_MAX];
		pStructClient->pszPort = new WCHAR[BUFFER_MAX];
		wcscpy_s(pStructClient->pszHost
			, wcslen(L"192.168.178.14") + 1
			, L"192.168.178.14"
		);
		wcscpy_s(pStructClient->pszPort
			, wcslen(L"8080") + 1
			, L"8080"
		);
		pStructClient->HttpVersion = 11;
		// testing for a connection, by sending a TRACE message
		pStructClient->mode = "trace";
		// start thread			
		DWORD dwThreadID;
		HANDLE hThread = CreateThread(NULL
			, 0						// default stack size
			, http_client_async		// thread function
			, pStructClient			// argument to thread function
			, 0						// default creation flags
			, &dwThreadID			// returns the thread identifier
		);

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	} // eof WM_NCCREATE
	case WM_SIZE:
		oStatusBar.SetStatusBar(hWnd);
		//RECT rectClient;
		//GetClientRect(hWnd, &rectClient);
		// subtract statusbar height from bottom
		//SetWindowPos(hWndDlg
		//	, HWND_TOP
		//	, rectClient.left, rectClient.top
		//	, rectClient.right, rectClient.bottom - 22
		//	, SWP_SHOWWINDOW
		//);
		pStructClient->hWnd = hWnd;
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_CONNECT:
			OutputDebugString(L"IDM_CONNECT\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, ConnectProc
				, (LPARAM)pStructClient
			);
			break;
		case IDM_LOGIN:
			OutputDebugString(L"IDM_LOGIN\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, LoginProc
				, (LPARAM)pStructClient
			);
			break;
		case IDM_FORGOTPASSWORD:
			OutputDebugString(L"IDM_FORGOTPASSWORD [WndProc]\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, ForgotPasswordProc
				, (LPARAM)pStructClient
			);
			break;
		case IDM_REGISTER:
			OutputDebugString(L"IDM_REGISTER\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, RegisterProc
				, (LPARAM)pStructClient
			);
			break;
		case IDM_DOWNLOAD:
			OutputDebugString(L"IDM_DOWNLOAD\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, DownloadProc
				, (LPARAM)pStructClient
			);
			break;
		case IDM_UPLOAD:
			OutputDebugString(L"IDM_UPLOAD\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hWnd
				, UploadProc
				, (LPARAM)pStructClient
			);
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
	case IDM_ALLDONE:
		OutputDebugString(L"IDM_ALLDONE\n");
		pStructClient = (PSTRUCTCLIENT)lParam;
		// the verb 'trace' is used for the mode when connecting
		// to the server
		if (pStructClient->mode == "trace")
			if (pStructClient->bConnected)
				oStatusBar.StatusBarSetText(0
					, L"Connected to server"
				);
			else
				oStatusBar.StatusBarSetText(0
					, L"Can not connect to server"
				);
		// the verb 'access' is used for the mode when a user logs in 
		// to the server
		if (pStructClient->mode == "access")
			if (pStructClient->bLoggedin)
			{
				StringCchPrintf(pszTextBuffer
					, BUFFER_MAX
					, L"Logged in %s"
					, pStructClient->pszUEA
					);
				oStatusBar.StatusBarSetText(1
					, pszTextBuffer
				);
			}
			else
			{
				//StringCchPrintf(pszTextBuffer
				//	, BUFFER_MAX
				//	, L"%S"
				//	, L"bla die bla"//pStructClient->response_body.c_str()
				//);
				//size_t length = strlen(pszItem) + 1;
				//PWCHAR pwChar = new WCHAR[length];
				//size_t conv;
				//mbstowcs_s(&conv
				//	, pwChar
				//	, length
				//	, pszItem
				//	, length - 1
				//);
				size_t length = strlen(pStructClient->response_body.c_str()) + 1;
				size_t conv;
				mbstowcs_s(&conv
					, pszTextBuffer
					, length
					, pStructClient->response_body.c_str()
					, length - 1
				);
				oStatusBar.StatusBarSetText(1
					, pszTextBuffer
				);
			}
		break;
	case WM_DESTROY:
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
//*                     ConnectProc
//****************************************************************************
INT_PTR CALLBACK ConnectProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTCLIENT pStructClient;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// enables communication with parent dialog
		pStructClient = (PSTRUCTCLIENT)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Connect");
		//RECT clientRect;
		//GetClientRect(pStructDlg->hWnd, &clientRect);
		//ClientToScreen(&clientRect);
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(pStructClient->hWnd, &point);
		SetWindowPos(hDlg
			, HWND_TOP
			, point.x//clientRect.left
			, point.y//clientRect.top
			, 420, 210
			, SWP_SHOWWINDOW
		);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"Domain"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 30, 65, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L"www.localhost.com"
			, 0
			, hDlg
			, IDC_EDT_DOMAIN
			, hInst
			, 140, 30, 230, 16
		);

		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"Host"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 50, 65, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L"192.168.178.14"
			, 0
			, hDlg
			, IDC_EDT_DOMAIN
			, hInst
			, 140, 50, 230, 16
		);

		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"Port"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 70, 65, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L"8080"
			, 0
			, hDlg
			, IDC_EDT_PORT
			, hInst
			, 140, 70, 230, 16
		);

		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"Http version"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 94, 65, 16
		);
		HWND hWndCbHttpVersion =
			helper_for_render_control((std::string)"combobox"
				, (PWCHAR)L""
				, CBS_DROPDOWNLIST
				, hDlg
				, IDC_EDT_HTTP_VERSION
				, hInst
				, 140, 90, 55, 18
			);
		WCHAR* http_version[2];
		http_version[0] = new WCHAR[4];
		http_version[1] = new WCHAR[4];
		wcscpy_s(http_version[0], 4, L"1.0");
		wcscpy_s(http_version[1], 4, L"1.1");
		// add content to combobox
		SendMessage(hWndCbHttpVersion
			, CB_ADDSTRING
			, (WPARAM)0
			, (LPARAM)http_version[0]
		);
		SendMessage(hWndCbHttpVersion
			, CB_ADDSTRING
			, (WPARAM)0
			, (LPARAM)http_version[1]
		);
		// set initial select, WPARAM has the index
		SendMessage(hWndCbHttpVersion
			, CB_SETCURSEL
			, (WPARAM)1
			, (LPARAM)0
		);

		HWND hWndBtnConnect =
			helper_for_render_control((std::string)"button"
				, (PWCHAR)L"Connect"
				, 0
				, hDlg
				, IDC_BTN_CONNECT
				, hInst
				, 300, 120, 70, 22
			);

		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_CONNECT:
			OutputDebugString(L"IDC_BTN_CONNECT\n");
			if (!pStructClient->bConnected)
			{
				// testing for a connection, by sending a TRACE message
				pStructClient->mode = "trace";
				// start thread			
				DWORD dwThreadID;
				HANDLE hThread = CreateThread(NULL
					, 0						// default stack size
					, http_client_async		// thread function
					, pStructClient			// argument to thread function
					, 0						// default creation flags
					, &dwThreadID			// returns the thread identifier
				);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof switch
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     DlgProc
//****************************************************************************
//INT_PTR CALLBACK DlgProc(HWND hDlg
//	, UINT uMsg
//	, WPARAM wParam
//	, LPARAM lParam
//)
//{
//	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
//	// create brush for background color
//	static COLORREF rgbWhite = RGB(0xFF, 0xFF, 0xFF);
//	static HBRUSH hBrush = CreateSolidBrush(rgbWhite);
//	static HWND hWndConnect, hWndAccess, hWndDownload, hWndUpload;
//	static GroupBox oGroupBox;
//	static GroupBoxConnect oGroupBoxConnect;
//	static GroupBoxAccess oGroupBoxAccess;
//	static PSTRUCTDLG pStructDlg = new STRUCTDLG;
//
//
//	switch (uMsg)
//	{
//	case WM_INITDIALOG:
//		// necessary for the message loop to find this dialog, when found
//		// the message pump can dispatch messages to this dialog
//		SetWindowText(hDlg, L"DlgProc");
//		// create groupbox
//		//hWndConnect = oGroupBoxConnect.createGroupBox(hInst
//		//	, hDlg
//		//	, IDC_GB_CONNECT
//		//);
//		//hWndAccess = oGroupBoxAccess.createGroupBox(hInst
//		//	, hDlg
//		//	, IDC_GB_ACCESS
//		//);
//		//hWndDownload = oGroupBox.createGroupBox(hInst, hWnd, IDC_GB_DOWNLOAD);
//		//hWndUpload = oGroupBox.createGroupBox(hInst, hWnd, IDC_GB_UPLOAD);
//		pStructDlg->hDlg = hDlg;
//		return (INT_PTR)TRUE;
//	case WM_CTLCOLORDLG:
//		// set background color
//		return (INT_PTR)hBrush;
//	case WM_CTLCOLORSTATIC: {
//		// background color for groupbox title
//		// must be the same as the background color for the dialog, 
//		// i.d. rgb is white
//		HDC hDC = (HDC)wParam;
//		// text color: rgb is black
//		SetTextColor(hDC, RGB(0x0, 0x0, 0x0));
//		SetBkColor(hDC, rgbWhite);
//		return (INT_PTR)hBrush;
//	} // eof WM_CTLCOLORSTATIC
//	case WM_SIZE:
//		//oGroupBoxConnect.SetGroupBox(hWndConnect, 10, 20, 165, 110);
//		//oGroupBoxAccess.SetGroupBox(hWndAccess, 185, 20, 100, 110);
//		//oGroupBox.SetGroupBox(hWndDownload, 120, 20, 100, 50);
//		//oGroupBox.SetGroupBox(hWndUpload, 120, 80, 100, 50);
//		return TRUE;
//	case WM_COMMAND:
//		switch (LOWORD(wParam))
//		{
//		//case IDC_BTN_LOGIN:
//		//	OutputDebugString(L"IDC_BTN_LOGIN\n");
//		//	DialogBoxParam(hInst
//		//		, L"MODALWINDOW"//MAKEINTRESOURCE(IDD_LOGINBOX)
//		//		, hDlg
//		//		, LoginProc
//		//		, (LPARAM)pStructDlg
//		//	);
//		//	break;
//		//case IDC_BTN_REGISTER:
//		//	OutputDebugString(L"IDC_BTN_REGISTER\n");
//		//	DialogBoxParam(hInst
//		//		, L"MODALWINDOW"
//		//		, hDlg
//		//		, RegisterProc
//		//		, (LPARAM)pStructDlg
//		//	);
//		//	break;
//		} // eof switch
//		break;
//	case IDM_FORGOTPASSWORD:
//		OutputDebugString(L"***IDM_FORGOTPASSWORD\n");
//		DialogBoxParam(hInst
//			, L"MODALWINDOW"//MAKEINTRESOURCE(IDD_FORGOTPASSWORDBOX)
//			, hDlg
//			, ForgotPasswordProc
//			, (LPARAM)pStructDlg
//		);
//		break;
//	} // eof switch
//
//	return (INT_PTR)FALSE;
//}

//****************************************************************************
//*                     LoginProc
//****************************************************************************
INT_PTR CALLBACK LoginProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTCLIENT pStructClient;

	switch (uMsg)
	{
	case WM_INITDIALOG: {
		// enables communication with parent dialog
		pStructClient = (PSTRUCTCLIENT)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Login");
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(pStructClient->hWnd, &point);
		SetWindowPos(hDlg
			, HWND_TOP
			, point.x
			, point.y
			, 420, 190
			, SWP_SHOWWINDOW
		);
		set_window_login(hDlg
			, hInst
			, TRUE
		);

		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND: {
		// load pStructDlg with UEA and UP data
		pStructClient->pszUEA = new WCHAR[BUFFER_MAX];	// UEA = UserEmailAddress
		pStructClient->pszUP = new WCHAR[BUFFER_MAX];	// UP = UserPassword
		GetDlgItemText(hDlg
			, IDC_EDT_UEA
			, pStructClient->pszUEA
			, BUFFER_MAX
		);
		GetDlgItemText(hDlg
			, IDC_EDT_UP
			, pStructClient->pszUP
			, BUFFER_MAX
		);
		// convert to char*
		size_t convertedChars = 0;
		size_t lenUEA = wcslen(pStructClient->pszUEA) + 1;
		size_t lenUP = wcslen(pStructClient->pszUP) + 1;
		char* user_email_address_ = new char[lenUEA];
		char* user_password_ = new char[lenUP];
		wcstombs_s(&convertedChars, user_email_address_, lenUEA, pStructClient->pszUEA, _TRUNCATE);
		wcstombs_s(&convertedChars, user_password_, lenUP, pStructClient->pszUP, _TRUNCATE);

		switch (LOWORD(wParam))
		{
		case IDC_BTN_FORGOTPASSWORD:
			OutputDebugString(L"IDC_BTN_FORGOTPASSWORD\n");
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage(pStructClient->hWnd
				, WM_COMMAND
				, (WPARAM)IDM_FORGOTPASSWORD
				, (LPARAM)0
			);
			return (INT_PTR)TRUE;
		case IDC_BTN_SUBMIT:
		{
			OutputDebugString(L"IDC_BTN_SUBMIT\n");
			pStructClient->bLoggedin = false;
			pStructClient->mode = "access";
			pStructClient->target = "/login";
			pStructClient->payload =
				std::string("user_email_address=") +
				user_email_address_ +
				"&user_password=" +
				user_password_;
			// start thread			
			DWORD dwThreadID;
			HANDLE hThread = CreateThread(NULL
				, 0						// default stack size
				, http_client_async		// thread function
				, pStructClient			// argument to thread function
				, 0						// default creation flags
				, &dwThreadID			// returns the thread identifier
			);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof IDC_BTN_SUBMIT
		case IDCANCEL:
			OutputDebugString(L"IDCANCEL\n");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof switch
		break;
	} // eof WM_COMMAND
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     set_window_login
//****************************************************************************
VOID set_window_login(const HWND& hDlg
	, const HINSTANCE& hInst
	, const BOOL& bLogin = FALSE
)
{
	helper_for_render_control((std::string)"static"
		, (PWCHAR)L"User email address"
		, 0
		, hDlg
		, IDC_STATIC
		, hInst
		, 75, 30, 95, 16
	);
	helper_for_render_control((std::string)"edit"
		, (PWCHAR)L""
		, 0
		, hDlg
		, IDC_EDT_UEA
		, hInst
		, 170, 30, 200, 16
	);

	helper_for_render_control((std::string)"static"
		, (PWCHAR)L"User password"
		, 0
		, hDlg
		, IDC_STATIC
		, hInst
		, 75, 50, 95, 16
	);
	helper_for_render_control((std::string)"edit"
		, (PWCHAR)L""
		, ES_PASSWORD
		, hDlg
		, IDC_EDT_UP
		, hInst
		, 170, 50, 200, 16
	);

	if (bLogin)
		HWND hWndBtnForgotPassword =
			helper_for_render_control((std::string)"button"
				, (PWCHAR)L"Forgot Password"
				, 0
				, hDlg
				, IDC_BTN_FORGOTPASSWORD
				, hInst
				, 170, 80, 120, 22
			);

	HWND hWndBtnSubmit =
		helper_for_render_control((std::string)"button"
			, (PWCHAR)L"Submit"
			, 0
			, hDlg
			, IDC_BTN_SUBMIT
			, hInst
			, 300, 80, 70, 22
		);

	if (bLogin)
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 110, 95, 16
		);
}

//****************************************************************************
//*                     ForgotPasswordProc
//****************************************************************************
INT_PTR CALLBACK ForgotPasswordProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTCLIENT pStructClient;
	static HWND hWnd;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// enables communication with parent dialog
		pStructClient = (PSTRUCTCLIENT)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Forgot Password");
//		set_window_with_user_code(hDlg
//			, hInst
//			, 0, 0, 410, 300
//			, SWP_NOMOVE
//		);
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(pStructClient->hWnd, &point);
		SetWindowPos(hDlg
			, HWND_TOP
			, point.x
			, point.y
			, 420, 250
			, SWP_SHOWWINDOW
		);
		set_window_with_user_code(hDlg
			, hInst
		);
		// load data for UEA and UP, set by the user in the LoginProc
		SetDlgItemText(hDlg
			, IDC_EDT_UEA
			, pStructClient->pszUEA
		);
		SetDlgItemText(hDlg
			, IDC_EDT_UP
			, pStructClient->pszUP
		);
		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_SUBMIT:
			OutputDebugString(L"IDC_BTN_SUBMIT [ForgotPasswordProc]\n");
			EnableWindow(GetDlgItem(hDlg, IDC_BTN_SUBMIT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BTN_SUBMIT_UC), TRUE);
			break;
		case IDC_BTN_SUBMIT_UC:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		case IDCANCEL:
			OutputDebugString(L"IDCANCEL\n");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof switch
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     RegisterProc
//****************************************************************************
INT_PTR CALLBACK RegisterProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTCLIENT pStructClient;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		OutputDebugString(L"WM_INITDIALOG [RegisterProc]\n");
		// enables communication with parent dialog
		pStructClient = (PSTRUCTCLIENT)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Register");
//		set_window_with_user_code(hDlg
//			, hInst
//			, 0, 0, 410, 300
//			, SWP_NOMOVE
//		);
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(pStructClient->hWnd, &point);
		SetWindowPos(hDlg
			, HWND_TOP
			, point.x
			, point.y
			, 420, 250
			, SWP_SHOWWINDOW
		);
		set_window_with_user_code(hDlg
			, hInst
		);
		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_SUBMIT:
			OutputDebugString(L"IDC_BTN_SUBMIT [RegisterProc]\n");
			EnableWindow(GetDlgItem(hDlg, IDC_BTN_SUBMIT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BTN_SUBMIT_UC), TRUE);
			break;
		case IDC_BTN_SUBMIT_UC:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		case IDCANCEL:
			OutputDebugString(L"IDCANCEL [RegisterProc]\n");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof switch
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     set_window_with_user_code
//****************************************************************************
VOID set_window_with_user_code(const HWND& hDlg
	, const HINSTANCE& hInst
)
{
	set_window_login(hDlg
		, hInst
		// no bool, no render forgot password
	);
	helper_for_render_control((std::string)"static"
		, (PWCHAR)L"Enter the code received by email"
		, 0
		, hDlg
		, IDC_STATIC
		, hInst
		, 75, 110, 180, 16
	);
	HWND hWndEdtUC =
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_UC
			, hInst
			, 170, 130, 200, 16
		);
	HWND hWndBtnSubmitUC =
		helper_for_render_control((std::string)"button"
			, (PWCHAR)L"Submit"
			, WS_DISABLED
			, hDlg
			, IDC_BTN_SUBMIT_UC
			, hInst
			, 300, 160, 70, 22
		);
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
	PWCHAR lpClassName = new WCHAR[BUFFER_MAX];
	// initialize memory
	wcscpy_s(lpClassName, BUFFER_MAX, L"");

	if (typeControl == "static")
		wcscpy_s(lpClassName, BUFFER_MAX, L"STATIC");
	if (typeControl == "edit")
		wcscpy_s(lpClassName, BUFFER_MAX, L"EDIT");
	if (typeControl == "combobox")
		wcscpy_s(lpClassName, BUFFER_MAX, L"COMBOBOX");
	if (typeControl == "button")
		wcscpy_s(lpClassName, BUFFER_MAX, L"BUTTON");

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
	// clean up
	delete[] lpClassName;

	return hWnd;
}

//****************************************************************************
//*                     DownloadProc
//****************************************************************************
INT_PTR CALLBACK DownloadProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTCLIENT pStructClient;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		OutputDebugString(L"WM_INITDIALOG [DownloadProc]\n");
		// enables communication with parent dialog
		pStructClient = (PSTRUCTCLIENT)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Download");
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(pStructClient->hWnd, &point);
		SetWindowPos(hDlg
			, HWND_TOP
			, point.x
			, point.y
			, 420, 190
			, SWP_SHOWWINDOW
		);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"File name on server"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 30, 95, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_TARGET_FILE
			, hInst
			, 170, 30, 200, 16
		);

		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"File name on client"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 50, 95, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_DESTINATION_FILE
			, hInst
			, 170, 50, 200, 16
		);
		HWND hWndBtnTransfer =
			helper_for_render_control((std::string)"button"
				, (PWCHAR)L"Transfer"
				, WS_DISABLED
				, hDlg
				, IDC_BTN_TRANSFER
				, hInst
				, 300, 80, 70, 22
			);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L""//(PWCHAR)L"Please login first."
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 110, 180, 16
		);
		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			OutputDebugString(L"IDCANCEL [DownloadProc]\n");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof switch
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

//****************************************************************************
//*                     UploadProc
//****************************************************************************
INT_PTR CALLBACK UploadProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	static PSTRUCTCLIENT pStructClient;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		OutputDebugString(L"WM_INITDIALOG [UploadProc]\n");
		// enables communication with parent dialog
		pStructClient = (PSTRUCTCLIENT)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Upload");
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(pStructClient->hWnd, &point);
		SetWindowPos(hDlg
			, HWND_TOP
			, point.x
			, point.y
			, 420, 190
			, SWP_SHOWWINDOW
		);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"File name on server"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 30, 95, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_TARGET_FILE
			, hInst
			, 170, 30, 200, 16
		);

		helper_for_render_control((std::string)"static"
			, (PWCHAR)L"File name on client"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 50, 95, 16
		);
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_DESTINATION_FILE
			, hInst
			, 170, 50, 200, 16
		);
		HWND hWndBtnTransfer =
			helper_for_render_control((std::string)"button"
				, (PWCHAR)L"Transfer"
				, WS_DISABLED
				, hDlg
				, IDC_BTN_TRANSFER
				, hInst
				, 300, 80, 70, 22
			);
		helper_for_render_control((std::string)"static"
			, (PWCHAR)L""//(PWCHAR)L"Please login first"
			, 0
			, hDlg
			, IDC_STATIC
			, hInst
			, 75, 110, 180, 16
		);
		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			OutputDebugString(L"IDCANCEL [UploadProc]\n");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} // eof switch
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//***************************************************************************
//*                    global
//***************************************************************************
const int SECONDS_BEFORE_EXPIRING = 30;

//****************************************************************************
//*                     fail
//****************************************************************************
inline void
fail(beast::error_code ec, char const* what)
{
	;// std::cerr << what << ": " << ec.message() << "\n";
}

//****************************************************************************
//*                     session
//****************************************************************************
// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
	tcp::resolver resolver_;
	beast::tcp_stream stream_;
	beast::flat_buffer buffer_; // (Must persist between reads)
	http::request<http::empty_body> req_with_empty_body_;
	http::request<http::string_body> req_with_string_body_;
	http::request<http::file_body> req_with_file_body_;
	http::response<http::string_body> res_;
	std::string mode_;
	std::string payload_;
	std::string target_;
	std::string host_;
	std::string port_;
	int version_;
	PSTRUCTCLIENT pStructClient_;
	std::shared_ptr<std::string const> doc_root_;

public:
	// Objects are constructed with a strand to
	// ensure that handlers do not execute concurrently.
	explicit
		session(net::io_context& ioc)
		: resolver_(net::make_strand(ioc))
		, stream_(net::make_strand(ioc))
	{
		OutputDebugString(L"<<constructor>> session()\n");
		doc_root_ = std::make_shared<std::string>("./user_space");
	}

	// Start the asynchronous operation
	void
		run(PSTRUCTCLIENT pStructClient
			, char const* payload
			, char const* host
			, char const* port
		)
	{
		mode_ = pStructClient->mode;
		payload_ = payload;
		target_ = pStructClient->target;
		version_ = pStructClient->HttpVersion;
		host_ = host;
		port_ = port;
		pStructClient_ = pStructClient;

		if (mode_ == "trace")
		{
			// Set up an HTTP TRACE request message
			req_with_empty_body_.version(version_);
			req_with_empty_body_.method(http::verb::trace);
			req_with_empty_body_.target(target_);
			req_with_empty_body_.set(http::field::host, host_);
			req_with_empty_body_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		}
		if (mode_ == "access")
		{
			// prepare payload
			http::string_body::value_type body;
			body = payload_;
			auto size = payload_.length();

			// Set up an HTTP POST request message
			req_with_string_body_.method(http::verb::post);
			req_with_string_body_.target(target_);
			req_with_string_body_.version(version_);
			req_with_string_body_.set(http::field::host, host_);
			req_with_string_body_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
			req_with_string_body_.set(http::field::content_type, "application/x-www-form-urlencoded");
			req_with_string_body_.content_length(size);
			req_with_string_body_.body() = body;
			req_with_string_body_.prepare_payload();
		}

		// Look up the domain name
		resolver_.async_resolve(
			host_,
			port_,
			beast::bind_front_handler(
				&session::on_resolve,
				shared_from_this()));
	}

	void
		on_resolve(
			beast::error_code ec,
			tcp::resolver::results_type results)
	{
		if (ec)
			return fail(ec, "resolve");

		// Set a timeout on the operation
		stream_.expires_after(std::chrono::seconds(SECONDS_BEFORE_EXPIRING));

		// Make the connection on the IP address we get from a lookup
		stream_.async_connect(
			results,
			beast::bind_front_handler(
				&session::on_connect,
				shared_from_this()));
	}

	void
		on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
	{
		if (ec)
		{
			return fail(ec, "connect");
			pStructClient_->bConnected = false;
		}

		// Set a timeout on the operation
		stream_.expires_after(std::chrono::seconds(SECONDS_BEFORE_EXPIRING));

		// Send the HTTP request to the remote host
		if (mode_ == "trace")
		{
			http::async_write(stream_, req_with_empty_body_,
				beast::bind_front_handler(
					&session::on_write,
					shared_from_this()));
		}
		if (mode_ == "access")
		{
			http::async_write(stream_, req_with_string_body_,
				beast::bind_front_handler(
					&session::on_write,
					shared_from_this()));
		}
	}

	void
		on_write(
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return fail(ec, "write");

		// Receive the HTTP response
		http::async_read(stream_, buffer_, res_,
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

		if (ec)
			return fail(ec, "read");

		if (mode_ == "trace")
			pStructClient_->bConnected = true;

		std::string response_body = res_.body();
		if (mode_ == "access")
		{
			if (response_body == "login: succeeded")
				pStructClient_->bLoggedin = true;
			pStructClient_->response_body = response_body;
		}
			
		// Gracefully close the socket
		stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

		// not_connected happens sometimes so don't bother reporting it.
		if (ec && ec != beast::errc::not_connected)
			return fail(ec, "shutdown");

		// If we get here then the connection is closed gracefully
	}
};

//****************************************************************************
//*                     http_client_async
//****************************************************************************
DWORD WINAPI http_client_async(LPVOID lpVoid)
{
	PSTRUCTCLIENT pStructClient = (PSTRUCTCLIENT)lpVoid;
	auto const payload = pStructClient->payload.c_str();

	size_t convertedChars = 0;
	size_t lenHost = wcslen(pStructClient->pszHost) + 1;
	size_t lenPort = wcslen(pStructClient->pszPort) + 1;
	char* host_ = new char[lenHost];
	char* port_ = new char[lenPort];
	wcstombs_s(&convertedChars, host_, lenHost, pStructClient->pszHost, _TRUNCATE);
	wcstombs_s(&convertedChars, port_, lenPort, pStructClient->pszPort, _TRUNCATE);
	auto const host = host_;
	auto const port = port_;

	// The io_context is required for all I/O
	net::io_context ioc;

	// Launch the asynchronous operation
	std::make_shared<session>(ioc)->run(pStructClient
		, payload
		, host
		, port
	);

	// Run the I/O service. The call will return when
	// the get operation is complete.
	ioc.run();

	// so, this is the way to set a pointer into a LPARAM variable!!!
	LPARAM lParam = (INT_PTR)pStructClient;
	// send message to return the connect state
	SendMessage(((PSTRUCTCLIENT)lpVoid)->hWnd,
		IDM_ALLDONE, (WPARAM)0, (LPARAM)lParam);
	return (DWORD)EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////

//// Client_Win32.cpp : Defines the entry point for the application.
////
//
//#include "framework.h"
//#include "Client_Win32.h"
//
//#define MAX_LOADSTRING 100
//
//// Global Variables:
//HINSTANCE hInst;                                // current instance
//WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
//WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
//
//// Forward declarations of functions included in this code module:
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
//
//int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
//                     _In_opt_ HINSTANCE hPrevInstance,
//                     _In_ LPWSTR    lpCmdLine,
//                     _In_ int       nCmdShow)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//
//    // TODO: Place code here.
//
//    // Initialize global strings
//    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//    LoadStringW(hInstance, IDC_CLIENTWIN32, szWindowClass, MAX_LOADSTRING);
//    MyRegisterClass(hInstance);
//
//    // Perform application initialization:
//    if (!InitInstance (hInstance, nCmdShow))
//    {
//        return FALSE;
//    }
//
//    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTWIN32));
//
//    MSG msg;
//
//    // Main message loop:
//    while (GetMessage(&msg, nullptr, 0, 0))
//    {
//        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//    }
//
//    return (int) msg.wParam;
//}
//
//
//
////
////  FUNCTION: MyRegisterClass()
////
////  PURPOSE: Registers the window class.
////
//ATOM MyRegisterClass(HINSTANCE hInstance)
//{
//    WNDCLASSEXW wcex;
//
//    wcex.cbSize = sizeof(WNDCLASSEX);
//
//    wcex.style          = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc    = WndProc;
//    wcex.cbClsExtra     = 0;
//    wcex.cbWndExtra     = 0;
//    wcex.hInstance      = hInstance;
//    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENTWIN32));
//    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
//    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
//    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENTWIN32);
//    wcex.lpszClassName  = szWindowClass;
//    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
//
//    return RegisterClassExW(&wcex);
//}
//
////
////   FUNCTION: InitInstance(HINSTANCE, int)
////
////   PURPOSE: Saves instance handle and creates main window
////
////   COMMENTS:
////
////        In this function, we save the instance handle in a global variable and
////        create and display the main program window.
////
//BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
//{
//   hInst = hInstance; // Store instance handle in our global variable
//
//   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
//
//   if (!hWnd)
//   {
//      return FALSE;
//   }
//
//   ShowWindow(hWnd, nCmdShow);
//   UpdateWindow(hWnd);
//
//   return TRUE;
//}
//
////
////  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  PURPOSE: Processes messages for the main window.
////
////  WM_COMMAND  - process the application menu
////  WM_PAINT    - Paint the main window
////  WM_DESTROY  - post a quit message and return
////
////
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    switch (message)
//    {
//    case WM_COMMAND:
//        {
//            int wmId = LOWORD(wParam);
//            // Parse the menu selections:
//            switch (wmId)
//            {
//            case IDM_ABOUT:
//                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
//                break;
//            case IDM_EXIT:
//                DestroyWindow(hWnd);
//                break;
//            default:
//                return DefWindowProc(hWnd, message, wParam, lParam);
//            }
//        }
//        break;
//    case WM_PAINT:
//        {
//            PAINTSTRUCT ps;
//            HDC hdc = BeginPaint(hWnd, &ps);
//            // TODO: Add any drawing code that uses hdc here...
//            EndPaint(hWnd, &ps);
//        }
//        break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
//
//// Message handler for about box.
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    UNREFERENCED_PARAMETER(lParam);
//    switch (message)
//    {
//    case WM_INITDIALOG:
//        return (INT_PTR)TRUE;
//
//    case WM_COMMAND:
//        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//        {
//            EndDialog(hDlg, LOWORD(wParam));
//            return (INT_PTR)TRUE;
//        }
//        break;
//    }
//    return (INT_PTR)FALSE;
//}
