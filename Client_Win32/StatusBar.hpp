#pragma once
//****************************************************************************
//*                     StatusBar
//****************************************************************************
class StatusBar
{
	static const UINT cParts = 3;
	HWND hWndStatus = NULL;
public:
	VOID createStatusBar(const HINSTANCE& hInst
		, const HWND& hWnd
	)
	{
		// create a statusbar
		hWndStatus = CreateWindowEx(0
			, STATUSCLASSNAME
			, (PCTSTR)NULL
			, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE
			, 0, 0, 0, 0
			, hWnd
			, (HMENU)IDC_STATUSBAR
			, hInst
			, NULL
		);
		// set the font of the statusbar to a more typical system GUI font
		SendMessage(hWndStatus
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
	}
	int SetStatusBar(const HWND& hWnd)
	{
		// get the client coordinates of the parent window's client area
		RECT rectClient;
		GetClientRect(hWnd, &rectClient);
		// allocate an array for holding the right edge coordinates
		HLOCAL hloc = LocalAlloc(LHND, sizeof(int) * StatusBar::cParts);
		// error warning C28183, hloc could be zero, ...
		// return without showing statusbar
		if (hloc == 0)
			return -1;
		PINT paParts = (PINT)LocalLock(hloc);
		// calculate the right edge coordinates for each part,
		// and copy the coordinates to the array
		int nWidth = rectClient.right / StatusBar::cParts;
		int rightEdge = nWidth;
		// error warning C6011: dereferencing NULL pointer paParts
		// return without showing statusbar
		if (!paParts)
			return -2;
		for (int i = 0; i < StatusBar::cParts; i++)
		{
			paParts[i] = rightEdge;
			rightEdge += nWidth;
		}
		// tell the statusbar to create the window parts
		SendMessage(hWndStatus
			, SB_SETPARTS
			, (WPARAM)StatusBar::cParts
			, (LPARAM)paParts
		);
		// this solves the problem, stated at the start of the
		// function named: WndProc, in the module ManageServer
		//SendMessage(hWndStatus
		//	, SB_SETTEXT
		//	, (WPARAM)0
		//	, (LPARAM)pTabItem[0]->pszTextParts[0]
		//);
		// free the array
		LocalUnlock(hloc);
		LocalFree(hloc);
		// give the statusbar a theoretical size
		MoveWindow(hWndStatus, rectClient.left, rectClient.top,
			rectClient.right, rectClient.bottom, TRUE);
		ShowWindow(hWndStatus, SW_SHOWNORMAL);
		return 0;
	}
	VOID StatusBarSetText(const int& part
		, const wchar_t* pszText
	)
	{
		SendMessage(hWndStatus
			, SB_SETTEXT
			, (WPARAM)part
			, (LPARAM)pszText
		);
	}
};