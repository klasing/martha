#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"

//****************************************************************************
//*                     prototype
//****************************************************************************
INT_PTR CALLBACK Tab0Proc(HWND, UINT, WPARAM, LPARAM); // "Database" tab item
INT_PTR CALLBACK Tab1Proc(HWND, UINT, WPARAM, LPARAM); // "Logging" tab item
INT_PTR CALLBACK Tab2Proc(HWND, UINT, WPARAM, LPARAM); // "Server" tab item

//****************************************************************************
//*                     TabControl
//****************************************************************************
class TabControl
{
public:
	static const DWORD OFFSET_TABHDR = 25;
	static const UINT MAX_TAB = 16;
	HWND hWndDlg[MAX_TAB] = { 0 };
	HWND hWndTab = NULL;
	UINT iPage = 0;
	UINT nofDlg = 0;
public:
	VOID createTabControl(const HINSTANCE& hInst
		, const HWND& hWnd
	)
	{
		// create tab control
		hWndTab = CreateWindowEx(0
			, WC_TABCONTROL
			, L""
			, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE
			, 0, 0, 0, 0
			, hWnd
			, (HMENU)IDC_TABCONTROL
			, hInst
			, NULL
		);
		// set the font of the tabs to a more typical system GUI font
		SendMessage(hWndTab
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
		);
		// add tab
		TCITEM tie;
		tie.mask = TCIF_TEXT;
		tie.pszText = (LPWSTR)L"Database";
		TabCtrl_InsertItem(hWndTab, 0, &tie);
		tie.pszText = (LPWSTR)L"Logging";
		TabCtrl_InsertItem(hWndTab, 1, &tie);
		tie.pszText = (LPWSTR)L"Server";
		TabCtrl_InsertItem(hWndTab, 2, &tie);
		// create dialog for the "Database" tab item
		hWndDlg[0] = CreateDialog(hInst, L"SUBWINDOW", hWnd, Tab0Proc);
		// create dialog for the "Logging" tab item
		hWndDlg[1] = CreateDialog(hInst, L"SUBWINDOW", hWnd, Tab1Proc);
		// create dialog for the "Server" tab item
		hWndDlg[2] = CreateDialog(hInst, L"SUBWINDOW", hWnd, Tab2Proc);
		// when this variable changes the static const named; cTabItem
		// in the module: StatusBar.hpp must be updated accordingly
		nofDlg = sizeof(hWndDlg) / sizeof(hWndDlg[0]);
	}
	VOID showTabItems(const LPARAM& lParam)
	{
		// show the tab items
		SetWindowPos(hWndTab
			, HWND_TOP
			, 0, 0
			, GET_X_LPARAM(lParam)
			, GET_Y_LPARAM(lParam)
			, SWP_SHOWWINDOW
		);
		for (unsigned i = 0; i < nofDlg; i++)
		{
			if (i == iPage)
				// show window
				SetWindowPos(hWndDlg[i]
					, HWND_TOP
					, 0, OFFSET_TABHDR
					, GET_X_LPARAM(lParam)
					, GET_Y_LPARAM(lParam) - 45
					, SWP_SHOWWINDOW
				);
			else
				// hide window
				SetWindowPos(hWndDlg[i]
					, HWND_TOP
					, 0, OFFSET_TABHDR
					, GET_X_LPARAM(lParam)
					, GET_Y_LPARAM(lParam) - 45
					, SWP_HIDEWINDOW
				);
		}
	}
	BOOL onWmNotify(const LPARAM& lParam
		, StatusBar& oStatusBar
	)
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case TCN_SELCHANGING:
			// return FALSE to allow the selection to change
			return FALSE;
		case TCN_SELCHANGE:
		{
			// respond to a tab item change
			iPage = TabCtrl_GetCurSel(hWndTab);
			for (unsigned i = 0; i < nofDlg; i++)
			{
				if (i == iPage)
				{
					// show window
					ShowWindow(hWndDlg[i], SW_SHOW);
					oStatusBar.StatusBarSetText(i);
				}
				else
					// hide window
					ShowWindow(hWndDlg[i], SW_HIDE);
			}
			break;
		} // eof TCN_SELCHANGE
		} // eof switch
		// the return value is ignored
		return TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////////

//#pragma once
//#include "framework.h"
//#include "resource.h"
//#include "StatusBar.hpp"
////****************************************************************************
////*                     prototype
////****************************************************************************
//INT_PTR CALLBACK Tab0Proc(HWND, UINT, WPARAM, LPARAM); // "Database" tab item
//INT_PTR CALLBACK Tab1Proc(HWND, UINT, WPARAM, LPARAM); // "Logging" tab item
//INT_PTR CALLBACK Tab2Proc(HWND, UINT, WPARAM, LPARAM); // "Server" tab item
////****************************************************************************
////*                     TabControl
////****************************************************************************
//class TabControl {
//public:
//	static const DWORD OFFSET_TABHDR = 25;
//	static const UINT MAX_TAB = 16;
//	HWND hWndDlg[MAX_TAB];
//	HWND hWndTab = NULL;
//	UINT iPage = 0;
//	UINT nofDlg = 0;
//public:
//	void createTabControl(const HINSTANCE& hInst
//		, const HWND& hWnd
//	)
//	{
//		// create tab control
//		hWndTab = CreateWindowExW(0
//			, WC_TABCONTROL
//			, L""
//			, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE
//			, 0, 0, 0, 0
//			, hWnd
//			, (HMENU)IDC_TABCONTROL
//			, hInst
//			, NULL
//		);
//		// set the font of the tabs to a more typical system GUI font
//		SendMessage(hWndTab
//			, WM_SETFONT
//			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
//			, (LPARAM)0
//		);
//		// add tab
//		TCITEM tie;
//		tie.mask = TCIF_TEXT;
//		tie.pszText = (LPWSTR)L"Database";
//		TabCtrl_InsertItem(hWndTab, 0, &tie);
//		tie.pszText = (LPWSTR)L"Logging";
//		TabCtrl_InsertItem(hWndTab, 1, &tie);
//		tie.pszText = (LPWSTR)L"Server";
//		TabCtrl_InsertItem(hWndTab, 2, &tie);
//		// create dialog for the "Database" tab item
//		hWndDlg[0] = CreateDialog(hInst, L"SUBWINDOW", hWnd, Tab0Proc);
//		// create dialog for the "Logging" tab item
//		hWndDlg[1] = CreateDialog(hInst, L"SUBWINDOW", hWnd, Tab1Proc);
//		// create dialog for the "Server" tab item
//		hWndDlg[2] = CreateDialog(hInst, L"SUBWINDOW", hWnd, Tab2Proc);
//		// when this variable changes the static const named; cTabItem
//		// in the module: StatusBar.hpp must be updated accordingly
//		nofDlg = sizeof(hWndDlg) / sizeof(hWndDlg[0]);
//	}
//	void showTabItems(const LPARAM& lParam)
//	{
//		// show the tab items
//		SetWindowPos(hWndTab
//			, HWND_TOP
//			, 0, 0
//			, GET_X_LPARAM(lParam)
//			, GET_Y_LPARAM(lParam)
//			, SWP_SHOWWINDOW
//		);
//		for (unsigned i = 0; i < nofDlg; i++)
//		{
//			if (i == iPage)
//				// show window
//				SetWindowPos(hWndDlg[i]
//					, HWND_TOP
//					, 0, OFFSET_TABHDR
//					, GET_X_LPARAM(lParam)
//					, GET_Y_LPARAM(lParam) - 45
//					, SWP_SHOWWINDOW
//				);
//			else
//				// hide window
//				SetWindowPos(hWndDlg[i]
//					, HWND_TOP
//					, 0, OFFSET_TABHDR
//					, GET_X_LPARAM(lParam)
//					, GET_Y_LPARAM(lParam) - 45
//					, SWP_HIDEWINDOW
//				);
//		}
//	}
//	BOOL onWmNotify(const LPARAM& lParam
//		, StatusBar& oStatusBar
//	)
//	{
//		switch (((LPNMHDR)lParam)->code)
//		{
//		case TCN_SELCHANGING:
//			// return FALSE to allow the selection to change
//			return FALSE;
//		case TCN_SELCHANGE:
//		{
//			// respond to a tab item change
//			iPage = TabCtrl_GetCurSel(hWndTab);
//			for (unsigned i = 0; i < nofDlg; i++)
//			{
//				if (i == iPage)
//				{
//					// show window
//					ShowWindow(hWndDlg[i], SW_SHOW);
//					oStatusBar.StatusBarSetText(i, 0);
//				}
//				else
//					// hide window
//					ShowWindow(hWndDlg[i], SW_HIDE);
//			}
//			break;
//		} // eof TCN_SELCHANGE
//		} // eof switch
//		// the return value is ignored
//		return TRUE;
//	}
//};