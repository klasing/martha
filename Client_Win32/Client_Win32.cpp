//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Client_Win32.h"

//****************************************************************************
//*                     typedef
//****************************************************************************
typedef struct tagSTRUCTDLG {
	HWND hDlg = NULL;
	PWCHAR pszUEA = nullptr;	
	PWCHAR pszUP = nullptr;
	PWCHAR pszUC = nullptr;
} STRUCTDLG, *PSTRUCTDLG;

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
INT_PTR CALLBACK	DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	LoginProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ForgotPasswordProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	RegisterProc(HWND, UINT, WPARAM, LPARAM);
VOID set_window_login(const HWND&
	, const HINSTANCE&
	, const BOOL&
);
VOID set_window_with_user_code(const HWND&
	, const HINSTANCE&
	, const int&
	, const int&
	, const int&
	, const int&
	, const UINT&
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
		if (IsDialogMessage(FindWindow(NULL, L"DlgProc"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Login"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Forgot Password"), &msg)
			|| IsDialogMessage(FindWindow(NULL, L"Register"), &msg))
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
	static HWND hWndDlg;

	switch (uMsg)
	{
	case WM_NCCREATE:
		// create a status bar
		oStatusBar.createStatusBar(hInst, hWnd);
		hWndDlg = CreateDialog(hInst
			, L"SUBWINDOW"
			, hWnd
			, DlgProc
		);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		oStatusBar.SetStatusBar(hWnd);
		RECT rectClient;
		GetClientRect(hWnd, &rectClient);
		// subtract statusbar height from bottom
		SetWindowPos(hWndDlg
			, HWND_TOP
			, rectClient.left, rectClient.top
			, rectClient.right, rectClient.bottom - 22
			, SWP_SHOWWINDOW
		);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
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
//*                     DlgProc
//****************************************************************************
INT_PTR CALLBACK DlgProc(HWND hDlg
	, UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
)
{
	static HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	// create brush for background color
	static COLORREF rgbWhite = RGB(0xFF, 0xFF, 0xFF);
	static HBRUSH hBrush = CreateSolidBrush(rgbWhite);
	static HWND hWndConnect, hWndAccess, hWndDownload, hWndUpload;
	static GroupBox oGroupBox;
	static GroupBoxConnect oGroupBoxConnect;
	static GroupBoxAccess oGroupBoxAccess;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"DlgProc");
		// create groupbox
		hWndConnect = oGroupBoxConnect.createGroupBox(hInst
			, hDlg
			, IDC_GB_CONNECT
		);
		hWndAccess = oGroupBoxAccess.createGroupBox(hInst
			, hDlg
			, IDC_GB_ACCESS
		);
		//hWndDownload = oGroupBox.createGroupBox(hInst, hWnd, IDC_GB_DOWNLOAD);
		//hWndUpload = oGroupBox.createGroupBox(hInst, hWnd, IDC_GB_UPLOAD);
		return (INT_PTR)TRUE;
	case WM_CTLCOLORDLG:
		// set background color
		return (INT_PTR)hBrush;
	case WM_CTLCOLORSTATIC: {
		// background color for groupbox title
		// must be the same as the background color for the dialog, 
		// i.d. rgb is white
		HDC hDC = (HDC)wParam;
		// text color: rgb is black
		SetTextColor(hDC, RGB(0x0, 0x0, 0x0));
		SetBkColor(hDC, rgbWhite);
		return (INT_PTR)hBrush;
	} // eof WM_CTLCOLORSTATIC
	case WM_SIZE:
		oGroupBoxConnect.SetGroupBox(hWndConnect, 10, 20, 165, 110);
		oGroupBoxAccess.SetGroupBox(hWndAccess, 185, 20, 100, 110);
		//oGroupBox.SetGroupBox(hWndDownload, 120, 20, 100, 50);
		//oGroupBox.SetGroupBox(hWndUpload, 120, 80, 100, 50);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_LOGIN:
			OutputDebugString(L"IDC_BTN_LOGIN\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"//MAKEINTRESOURCE(IDD_LOGINBOX)
				, hDlg
				, LoginProc
				, (LPARAM)hDlg
			);
			break;
		case IDC_BTN_REGISTER:
			OutputDebugString(L"IDC_BTN_REGISTER\n");
			DialogBoxParam(hInst
				, L"MODALWINDOW"
				, hDlg
				, RegisterProc
				, (LPARAM)hDlg
			);
			break;
		} // eof switch
		break;
	case IDM_FORGOTPASSWORD:
		OutputDebugString(L"***IDM_FORGOTPASSWORD\n");
		DialogBoxParam(hInst
			, L"MODALWINDOW"//MAKEINTRESOURCE(IDD_FORGOTPASSWORDBOX)
			, hDlg
			, ForgotPasswordProc
			, (LPARAM)hDlg
		);
		break;
	} // eof switch

	return (INT_PTR)FALSE;
}

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
	static HWND hDlgParent;

	switch (uMsg)
	{
	case WM_INITDIALOG: {
		// enables communication with parent dialog
		hDlgParent = (HWND)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Login");
		set_window_login(hDlg
			, hInst
			, TRUE
		);
		return (INT_PTR)TRUE;
	} // eof WM_INITDIALOG
	case WM_COMMAND: {
		const int BUFF_MAX = 128;
		PWCHAR pszBuffUEA = new WCHAR[BUFF_MAX];	// UEA = UserEmailAddress
		PWCHAR pszBuffUP = new WCHAR[BUFF_MAX];		// UP = UserPassword
		GetDlgItemText(hDlg
			, IDC_EDT_UEA
			, pszBuffUEA
			, BUFF_MAX
		);
		GetDlgItemText(hDlg
			, IDC_EDT_UP
			, pszBuffUP
			, BUFF_MAX
		);
		switch (LOWORD(wParam))
		{
		case IDC_BTN_FORGOTPASSWORD:
			OutputDebugString(L"IDC_BTN_FORGOTPASSWORD\n");
			SendMessage(hDlgParent, IDM_FORGOTPASSWORD, (WPARAM)0, (LPARAM)0);
			EndDialog(hDlg, LOWORD(wParam));
			//DialogBoxParam(hInst
			//	, L"MODALWINDOW"//MAKEINTRESOURCE(IDD_FORGOTPASSWORDBOX)
			//	, hDlg
			//	, ForgotPasswordProc
			//	, (LPARAM)hDlg
			//);
			return (INT_PTR)TRUE;
		case IDC_BTN_SUBMIT:
			OutputDebugString(L"IDC_BTN_SUBMIT\n");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
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
	, const BOOL& bShowForgotPassword = FALSE
)
{
	helper_for_render_control((std::string)"static"
		, (PWCHAR)L"User Email Address"
		, 0
		, hDlg
		, IDC_STATIC
		, hInst
		, 75, 30, 95, 16
	);
	HWND hWndEdtUEA =
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_UEA
			, hInst
			, 170, 30, 200, 16
		);
	helper_for_render_control((std::string)"static"
		, (PWCHAR)L"User Password"
		, 0
		, hDlg
		, IDC_STATIC
		, hInst
		, 75, 55, 95, 16
	);
	HWND hWndEdtUP =
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, ES_PASSWORD
			, hDlg
			, IDC_EDT_UP
			, hInst
			, 170, 55, 200, 16
		);
	// the forgotpassword button is shown only for the login 
	if (bShowForgotPassword)
	{
		HWND hWndBtnForgotPassword =
			helper_for_render_control((std::string)"button"
				, (PWCHAR)L"Forgot Password"
				, 0
				, hDlg
				, IDC_BTN_FORGOTPASSWORD
				, hInst
				, 170, 80, 120, 22
			);
	}
	HWND hWndBtnSubmit =
		helper_for_render_control((std::string)"button"
			, (PWCHAR)L"Submit"
			, 0
			, hDlg
			, IDC_BTN_SUBMIT
			, hInst
			, 300, 80, 70, 22
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
	static HWND hWnd, hDlgParent;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// enables communication with parent dialog
		hDlgParent = (HWND)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Forgot Password");
		set_window_with_user_code(hDlg
			, hInst
			, 0, 0, 410, 300
			, SWP_NOMOVE
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
	static HWND hDlgParent;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		OutputDebugString(L"WM_INITDIALOG [RegisterProc]\n");
		// enables communication with parent dialog
		hDlgParent = (HWND)lParam;
		// necessary for the message loop to find this dialog, when found
		// the message pump can dispatch messages to this dialog
		SetWindowText(hDlg, L"Register");
		set_window_with_user_code(hDlg
			, hInst
			, 0, 0, 410, 300
			, SWP_NOMOVE
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
	, const int& x
	, const int& y
	, const int& cx
	, const int& cy
	, const UINT& uFlags
)
{
	SetWindowPos(hDlg
		, HWND_TOP
		, x, y, cx, cy
		, SWP_SHOWWINDOW | uFlags
	);
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
		, 75, 110, 200, 16
	);
	HWND hWndEdtUC =
		helper_for_render_control((std::string)"edit"
			, (PWCHAR)L""
			, 0
			, hDlg
			, IDC_EDT_UP
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
			, 300, 155, 70, 22
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

	if (typeControl == "edit")
		wcscpy_s(lpClassName, BUFFER_MAX, L"EDIT");
	if (typeControl == "button")
		wcscpy_s(lpClassName, BUFFER_MAX, L"BUTTON");
	if (typeControl == "static")
		wcscpy_s(lpClassName, BUFFER_MAX, L"STATIC");

	HWND hWnd = CreateWindow(lpClassName
		, lpWindowName
		, WS_CHILD | dwStyle//| WS_BORDER
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
