#pragma once
#include "GroupBox.hpp"
class GroupBoxAccess : public GroupBox {
	HWND hWndBtnLogin, hWndBtnRegister, hWndBtnForgotPassword;
public:
	HWND createGroupBox(const HINSTANCE& hInst
		, const HWND& hDlg
		, const int& resourceId
	)
	{
		HWND hWndGroupBoxAccess = GroupBox::createGroupBox(
			hInst
			, hDlg
			, resourceId
			, (PWCHAR)L"Access"
		);
		// create groupboxaccess specific content
		// the hDlg is the parent window for the control
		hWndBtnLogin = CreateWindow(L"BUTTON"
			, L"Login"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_BTN_LOGIN
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndBtnLogin
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);

		hWndBtnRegister = CreateWindow(L"BUTTON"
			, L"Register"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_BTN_REGISTER
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndBtnRegister
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);

		hWndBtnForgotPassword = CreateWindow(L"BUTTON"
			, L"Forgot Password"
			, WS_CHILD //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_BTN_FORGOTPASSWORD
			, hInst
			, NULL
		);
		// set font on static
		SendMessage(hWndBtnForgotPassword
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);

		return hWndGroupBoxAccess;
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
		SetWindowPos(hWndBtnLogin
			, HWND_TOP
			, 5 + x, 25 + y, 70, 20
			, SWP_SHOWWINDOW);

		SetWindowPos(hWndBtnRegister
			, HWND_TOP
			, 5 + x, 50 + y, 70, 20
			, SWP_SHOWWINDOW);

		//SetWindowPos(hWndBtnForgotPassword
		//	, HWND_TOP
		//	, 5 + x, 75 + y, 90, 20
		//	, SWP_SHOWWINDOW);
		return 0;
	}
};