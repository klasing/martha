//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Client_Win32.h"

//****************************************************************************
//*                     typedef
//****************************************************************************

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
			, rectClient.left, rectClient.top,
			rectClient.right, rectClient.bottom - 22
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

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// create groupbox
		hWndConnect = oGroupBoxConnect.createGroupBoxConnect(hInst
			, hDlg
			, IDC_GB_CONNECT
		);
		//hWndAccess = oGroupBox.createGroupBox(hInst, hWnd, IDC_GB_ACCESS);
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
		oGroupBoxConnect.SetGroupBoxConnect(hWndConnect, 10, 20, 100, 50);
		//oGroupBox.SetGroupBox(hWndAccess, 10, 80, 100, 50);
		//oGroupBox.SetGroupBox(hWndDownload, 120, 20, 100, 50);
		//oGroupBox.SetGroupBox(hWndUpload, 120, 80, 100, 50);
		return TRUE;
	} // eof switch

	return (INT_PTR)FALSE;
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
