#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"

//****************************************************************************
//*                     StatusBar
//****************************************************************************
class StatusBar
{
	typedef struct tagPARTS
	{
		static const size_t TEXT_MAX = 128;
		static const UINT cParts = 6;
		PWCHAR pszTextParts[cParts];
	} PARTS, * PPARTS;
	// corresponds to the nof tabitems defined
	// holded in the variable named: nofDlg,
	// in the module: TabControl.hpp
	static const UINT cTabItem = 3;
	PPARTS pTabItem[cTabItem] = { 0 };
	HWND hWndStatus = NULL;
	UINT iPage = 0;
public:
	StatusBar()
	{
		// initialize pTabItem, and initialize the structs
		// where pTabItem is pointing to
		for (int i = 0; i < cTabItem; ++i)
		{
			// every tabitem has PARTS::cParts statusbar parts
			pTabItem[i] = new PARTS;
			for (int j = 0; j < PARTS::cParts; ++j)
			{
				// create pointers for the statusbar parts
				pTabItem[i]->pszTextParts[j] = new WCHAR[PARTS::TEXT_MAX];
				// set initially empty strings for the statusbar parts
				StringCchCopy(pTabItem[i]->pszTextParts[j]
					, PARTS::TEXT_MAX
					, L""
				);
			}
		}
	}
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
		// set the default values
		StringCchCopy(pTabItem[0]->pszTextParts[0]
			, PARTS::TEXT_MAX
			, L"Database OPEN"
		);
		StringCchCopy(pTabItem[1]->pszTextParts[0]
			, PARTS::TEXT_MAX
			, L"Monitor ON"
		);
		StringCchCopy(pTabItem[1]->pszTextParts[5]
			, PARTS::TEXT_MAX
			, L"Logfile size 0 (kb)"
		);
		StringCchCopy(pTabItem[2]->pszTextParts[0]
			, PARTS::TEXT_MAX
			, L"Server DEAD"
		);
	}
	int SetStatusBar(const HWND& hWnd)
	{
		// get the client coordinates of the parent window's client area
		RECT rectClient;
		GetClientRect(hWnd, &rectClient);
		// allocate an array for holding the right edge coordinates
		HLOCAL hloc = LocalAlloc(LHND, sizeof(int) * PARTS::cParts);
		// error warning C28183, hloc could be zero, ...
		// return without showing statusbar
		if (hloc == 0)
			return -1;
		PINT paParts = (PINT)LocalLock(hloc);
		// calculate the right edge coordinates for each part,
		// and copy the coordinates to the array
		int nWidth = rectClient.right / PARTS::cParts;
		int rightEdge = nWidth;
		// error warning C6011: dereferencing NULL pointer paParts
		// return without showing statusbar
		if (!paParts)
			return -2;
		for (int i = 0; i < PARTS::cParts; i++)
		{
			paParts[i] = rightEdge;
			rightEdge += nWidth;
		}
		// tell the statusbar to create the window parts
		SendMessage(hWndStatus
			, SB_SETPARTS
			, (WPARAM)PARTS::cParts
			, (LPARAM)paParts
		);
		// this solves the problem, stated at the start of the
		// function named: WndProc, in the module ManageServer
		SendMessage(hWndStatus
			, SB_SETTEXT
			, (WPARAM)0
			, (LPARAM)pTabItem[0]->pszTextParts[0]
		);
		// free the array
		LocalUnlock(hloc);
		LocalFree(hloc);
		// give the statusbar a theoretical size
		MoveWindow(hWndStatus, rectClient.left, rectClient.top,
			rectClient.right, rectClient.bottom, TRUE);
		ShowWindow(hWndStatus, SW_SHOWNORMAL);
		return 0;
	}
	// place statusbar text from all parts,
	// for a given tab item, on screen
	VOID StatusBarSetText(const int& iTabItem)
	{
		// keep track of the current page
		this->iPage = iTabItem;
		for (int i = 0; i < PARTS::cParts; ++i)
		{
			SendMessage(hWndStatus
				, SB_SETTEXT
				, (WPARAM)i
				, (LPARAM)pTabItem[iTabItem]->pszTextParts[i]
			);

		}
	}
	// set the buffered statusbar text when the user changes from 
	// tabitem
	VOID StatusBarSetText(const int& iTabItem
		, const int& part
	)
	{
		// keep track of the current page
		this->iPage = iTabItem;
		// adjust the status text for the current page
		SendMessage(hWndStatus
			, SB_SETTEXT
			, (WPARAM)part
			, (LPARAM)pTabItem[iTabItem]->pszTextParts[part]
		);
	}
	// buffer new statusbar text, and only show the new text
	// when the appropriate page is visible
	VOID StatusBarSetText(const int& iTabItem
		, const int& part
		, const wchar_t* pszText
	)
	{
		StringCchCopy(pTabItem[iTabItem]->pszTextParts[part]
			, PARTS::TEXT_MAX
			, pszText
		);
		// change the status text only when the appropriate page is active
		if (iTabItem == iPage)
			SendMessage(hWndStatus
				, SB_SETTEXT
				, (WPARAM)part
				, (LPARAM)pTabItem[iTabItem]->pszTextParts[part]
			);
	}
};

//////////////////////////////////////////////////////////////////////////////

//#pragma once
//#include "framework.h"
//#include "resource.h"
////****************************************************************************
////*                     StatusBar
////****************************************************************************
//class StatusBar {
//	typedef struct tagPARTS
//	{
//		static const size_t TEXT_MAX = 128;
//		static const UINT cParts = 6;
//		PWCHAR pszTextParts[cParts];
//	} PARTS, * PPARTS;
//	// corresponds to the nof tabitems defined
//	// holded in the variable named: nofDlg,
//	// in the module: TabControl.hpp
//	static const UINT cTabItem = 3;
//	PPARTS pTabItem[cTabItem] = { 0 };
//	HWND hWndStatus = NULL;
//	UINT iPage = 0;
//public:
//	StatusBar()
//	{
//		// initialize pTabItem, and initialize the structs
//		// where pTabItem is pointing to
//		for (int i = 0; i < cTabItem; ++i)
//		{
//			// every tabitem has three statusbar parts
//			pTabItem[i] = new PARTS;
//			for (int j = 0; j < PARTS::cParts; ++j)
//			{
//				// create pointers for the statusbar parts
//				pTabItem[i]->pszTextParts[j] = new WCHAR[PARTS::TEXT_MAX];
//				// set initially empty strings for the statusbar parts
//				StringCchCopy(pTabItem[i]->pszTextParts[j]
//					, PARTS::TEXT_MAX
//					, L""
//				);
//			}
//		}
//	}
//	void createStatusBar(const HINSTANCE& hInst
//		, const HWND& hWnd
//	)
//	{
//		// create a statusbar
//		hWndStatus = CreateWindowEx(0
//			, STATUSCLASSNAME
//			, (PCTSTR)NULL
//			, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE
//			, 0, 0, 0, 0
//			, hWnd
//			, (HMENU)IDC_STATUSBAR
//			, hInst
//			, NULL
//		);
//		// set the font of the statusbar to a more typical system GUI font
//		SendMessage(hWndStatus
//			, WM_SETFONT
//			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
//			, (LPARAM)0
//		);
//		// set the default values
//		StringCchCopy(pTabItem[0]->pszTextParts[0]
//			, PARTS::TEXT_MAX
//			, L"Database OPEN"
//		);
//		StringCchCopy(pTabItem[1]->pszTextParts[0]
//			, PARTS::TEXT_MAX
//			, L"Monitor ON"
//		);
//		StringCchCopy(pTabItem[2]->pszTextParts[0]
//			, PARTS::TEXT_MAX
//			, L"Server DEAD"
//		);
//	}
//	int SetStatusBar(const HWND& hWnd)
//	{
//		// get the client coordinates of the parent window's client area
//		RECT rectClient;
//		GetClientRect(hWnd, &rectClient);
//		// allocate an array for holding the right edge coordinates
//		HLOCAL hloc = LocalAlloc(LHND, sizeof(int) * PARTS::cParts);
//		// error warning C28183, hloc could be zero, ...
//		// return without showing statusbar
//		if (hloc == 0)
//			return -1;
//		PINT paParts = (PINT)LocalLock(hloc);
//		// calculate the right edge coordinates for each part,
//		// and copy the coordinates to the array
//		int nWidth = rectClient.right / PARTS::cParts;
//		int rightEdge = nWidth;
//		// error warning C6011: dereferencing NULL pointer paParts
//		// return without showing statusbar
//		if (!paParts)
//			return -2;
//		for (int i = 0; i < PARTS::cParts; i++)
//		{
//			paParts[i] = rightEdge;
//			rightEdge += nWidth;
//		}
//		// tell the statusbar to create the window parts
//		SendMessage(hWndStatus
//			, SB_SETPARTS
//			, (WPARAM)PARTS::cParts
//			, (LPARAM)paParts
//		);
//		// free the array
//		LocalUnlock(hloc);
//		LocalFree(hloc);
//		// give the statusbar a theoretical size
//		MoveWindow(hWndStatus, rectClient.left, rectClient.top,
//			rectClient.right, rectClient.bottom, TRUE);
//		ShowWindow(hWndStatus, SW_SHOWNORMAL);
//		return 0;
//	}
//	// place statusbar text from all parts,
//	// for a given tab item, on screen
//	VOID StatusBarSetText(const int& iTabItem)
//	{
//		for (int i = 0; i < PARTS::cParts; ++i)
//		{
//			SendMessage(hWndStatus
//				, SB_SETTEXT
//				, (WPARAM)i
//				, (LPARAM)pTabItem[iTabItem]->pszTextParts[i]
//			);
//
//		}
//	}
//	// set the buffered statusbar text when the user changes from 
//	// tabitem
//	VOID StatusBarSetText(const int& iTabItem
//		, const int& part
//	)
//	{
//		// keep track of the current page
//		this->iPage = iTabItem;
//		// adjust the status text for the current page
//		SendMessage(hWndStatus
//			, SB_SETTEXT
//			, (WPARAM)part
//			, (LPARAM)pTabItem[iTabItem]->pszTextParts[part]
//		);
//	}
//	// buffer new statusbar text, and only show the new text
//	// when the appropriate page is visible
//	VOID StatusBarSetText(const int& iTabItem
//		, const int& part
//		, const wchar_t* pszText
//	)
//	{
//		StringCchCopy(pTabItem[iTabItem]->pszTextParts[part]
//			, PARTS::TEXT_MAX
//			, pszText
//		);
//		// change the status text only when the appropriate page is active
//		if (iTabItem == iPage)
//			SendMessage(hWndStatus
//				, SB_SETTEXT
//				, (WPARAM)part
//				, (LPARAM)pTabItem[iTabItem]->pszTextParts[part]
//			);
//	}
//};