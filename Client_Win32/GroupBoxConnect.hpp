#pragma once
#include "GroupBox.hpp"
class GroupBoxConnect : public GroupBox {
	HWND hWndLblDomain, hWndLblHost, hWndLblPort, hWndLblHttp;
	HWND hWndEdtDomain, hWndEdtHost, hWndEdtPort, hWndEdtHttp;
public:
	HWND createGroupBox(const HINSTANCE& hInst
		, const HWND& hDlg
		, const int& resourceId
	)
	{
		HWND hWndGroupBoxConnect = GroupBox::createGroupBox(
			hInst
			, hDlg
			, resourceId
			, (PWCHAR)L"Connect"
		);
		// create groupboxconnect specific content
		// the hDlg is the parent window for the control
		hWndLblDomain = CreateWindow(L"STATIC"
			, L"Domain"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_STATIC
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndLblDomain
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
		hWndEdtDomain = CreateWindow(L"EDIT"
			, L"www.localhost.com"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_EDT_DOMAIN
			, hInst
			, NULL
		);
		// set font on edit
		SendMessage(hWndEdtDomain
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);

		hWndLblHost = CreateWindow(L"STATIC"
			, L"Host"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_STATIC
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndLblHost
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
		hWndEdtHost = CreateWindow(L"EDIT"
			, L"192.168.178.14"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_EDT_HOST
			, hInst
			, NULL
		);
		// set font on edit
		SendMessage(hWndEdtHost
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);

		hWndLblPort = CreateWindow(L"STATIC"
			, L"Port"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_STATIC
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndLblPort
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
		hWndEdtPort = CreateWindow(L"EDIT"
			, L"8080"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_EDT_PORT
			, hInst
			, NULL
		);
		// set font on edit
		SendMessage(hWndEdtPort
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);

		hWndLblHttp = CreateWindow(L"STATIC"
			, L"HTTP"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_STATIC
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndLblHttp
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
		hWndEdtHttp = CreateWindow(L"EDIT"
			, L"1.1"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_EDT_HTTP
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndEdtHttp
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
		return hWndGroupBoxConnect;
	}
	int SetGroupBox(const HWND hWnd
		, const int& x
		, const int& y
		, const int& width
		, const int& height
	)
	{
		GroupBox::SetGroupBox(hWnd, x, y, width, height);
		// the placement of controls in the groupbox is relative to
		// the x- and y-position of the groupbox
		SetWindowPos(hWndLblDomain
			, HWND_TOP
			, 5 + x, 25 + y, 40, 18
			, SWP_SHOWWINDOW);
		SetWindowPos(hWndEdtDomain
			, HWND_TOP
			, 45 + x, 25 + y, 110, 18
			, SWP_SHOWWINDOW);

		SetWindowPos(hWndLblHost
			, HWND_TOP
			, 5 + x, 45 + y, 40, 18
			, SWP_SHOWWINDOW);
		SetWindowPos(hWndEdtHost
			, HWND_TOP
			, 45 + x, 45 + y, 110, 18
			, SWP_SHOWWINDOW);

		SetWindowPos(hWndLblPort
			, HWND_TOP
			, 5 + x, 65 + y, 40, 18
			, SWP_SHOWWINDOW);
		SetWindowPos(hWndEdtPort
			, HWND_TOP
			, 45 + x, 65 + y, 110, 18
			, SWP_SHOWWINDOW);

		SetWindowPos(hWndLblHttp
			, HWND_TOP
			, 5 + x, 85 + y, 40, 18
			, SWP_SHOWWINDOW);
		SetWindowPos(hWndEdtHttp
			, HWND_TOP
			, 45 + x, 85 + y, 110, 18
			, SWP_SHOWWINDOW);
		return 0;
	}
};