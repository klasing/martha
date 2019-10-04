#pragma once
#include "GroupBox.hpp"
class GroupBoxConnect : public GroupBox {
public:
	HWND hWndLblDomain;
public:
	HWND createGroupBoxConnect(const HINSTANCE& hInst
		, const HWND& hDlg
		, const int& resourceId
	)
	{
		HWND hWndGroupBoxConnect = GroupBox::createGroupBox(
			hInst
			, hDlg
			, resourceId
		);
		// create groupboxconnect specific content
		// the hDlg is the parent window for the control
		hWndLblDomain = CreateWindow(L"STATIC"
			, L"Domain"
			, WS_CHILD | WS_BORDER
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
		return hWndGroupBoxConnect;
	}
	int SetGroupBoxConnect(const HWND hWnd
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
			, 5 + x, 15 + y, 40, 15
			, SWP_SHOWWINDOW);
		return 0;
	}
};