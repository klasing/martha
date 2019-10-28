#pragma once
//****************************************************************************
//*                     GroupBox
//****************************************************************************
class GroupBox {
public:
	HWND createGroupBox(const HINSTANCE& hInst
		, const HWND& hWnd
		, const int& resourceId
		, const PWCHAR& pszTitle
	)
	{
		// create groupbox
		HWND hWndGB = CreateWindow(L"BUTTON"
			, pszTitle
			, WS_CHILD | BS_GROUPBOX
			, 0, 0, 0, 0
			, hWnd
			, (HMENU)resourceId
			, hInst
			, NULL
		);
		// set font on groupbox
		SendMessage(hWndGB
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
		return hWndGB;
	}
	int SetGroupBox(const HWND hWnd
		, const int& x
		, const int& y
		, const int& width
		, const int& height
	)
	{
		SetWindowPos(hWnd
			, HWND_TOP
			, x, y, width, height
			, SWP_SHOWWINDOW);
		return 0;
	}
};