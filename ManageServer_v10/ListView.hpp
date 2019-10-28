#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"

//****************************************************************************
//*                     createListView
//****************************************************************************
HWND createListView(const HINSTANCE& hInst
	, const HWND& hDlg
	, const int& resourceId
)
{
	HWND hWndLV = CreateWindowEx(0
		, WC_LISTVIEW
		, NULL
		, WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS
		, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		, hDlg
		, (HMENU)resourceId
		, hInst
		, NULL
	);
	// supply gridlines and make row selection possible
	SendMessage(hWndLV
		, LVM_SETEXTENDEDLISTVIEWSTYLE
		, (WPARAM)0
		, (LPARAM)LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT
	);
	return hWndLV;
}

//****************************************************************************
//*                     addColumn
//****************************************************************************
VOID addColumn(const HINSTANCE& hInst
	, const HWND& hWndLV
	, const int nofResource // variadic: can't be a reference
	, ...
)
{
	const UINT MAX_LOADSTRING = 256;
	WCHAR pszColumnName[MAX_LOADSTRING];
	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
	va_list args;
	va_start(args, nofResource);
	for (int i = 0; i < nofResource; i++)
	{
		int resource_id = va_arg(args, int);
		LoadString(hInst, resource_id, pszColumnName, MAX_LOADSTRING);
		lvc.cx = wcslen(pszColumnName) * 8 + 10;
		lvc.cchTextMax = wcslen(pszColumnName);
		lvc.pszText = pszColumnName;
		SendMessage(hWndLV
			, LVM_INSERTCOLUMN
			, (WPARAM)i
			, (LPARAM)& lvc
		);
	}
	va_end(args);
}

//****************************************************************************
//*                     clearListView
//****************************************************************************
VOID clearListView(const HWND& hWndLV)
{
	SendMessage(hWndLV
		, LVM_DELETEALLITEMS
		, (WPARAM)0
		, (LPARAM)0
	);
}

//****************************************************************************
//*                     addListViewItem
//****************************************************************************
VOID addListViewItem(const HWND& hWndLV
	, const PWCHAR& pszItem
	, const int& iItem
	, const int& iSubItem
)
{
	LVITEM lvi = { 0 };
	UINT cchItem = wcslen(pszItem);
	lvi.mask = LVIF_TEXT;
	lvi.iItem = iItem;
	lvi.iSubItem = iSubItem;
	lvi.cchTextMax = cchItem;
	lvi.pszText = pszItem;
	// get the column width
	size_t iColumnWidth = SendMessage(hWndLV
		, LVM_GETCOLUMNWIDTH
		, (WPARAM)iSubItem
		, (LPARAM)0
	);
	// determine width column for subitem
	size_t iItemLength = wcslen(pszItem) * 8 + 10;
	if (iItemLength > iColumnWidth)
		// adjust the column width for the length of the sub item
		SendMessage(hWndLV
			, LVM_SETCOLUMNWIDTH
			, (WPARAM)iSubItem
			, (LPARAM)iItemLength
		);
	BOOL iSuccess;
	if (iSubItem == 0)
		// insert new listview row
		iSuccess = SendMessage(hWndLV
			, LVM_INSERTITEM
			, (WPARAM)0
			, (LPARAM)& lvi
		);
	else
		// set subitem
		iSuccess = SendMessage(hWndLV
			, LVM_SETITEM
			, (WPARAM)0
			, (LPARAM)& lvi
		);
}
VOID addListViewItem(const HWND& hWndLV
	, const char* pszItem
	, const int& iItem
	, const int& iSubItem
)
{
	size_t length = strlen(pszItem) + 1;
	PWCHAR pwChar = new WCHAR[length];
	size_t conv;
	mbstowcs_s(&conv
		, pwChar
		, length
		, pszItem
		, length - 1
	);
	addListViewItem(hWndLV
		, pwChar
		, iItem
		, iSubItem
	);
	// clean up
	delete[] pwChar;
}

//****************************************************************************
//*                     getListViewItem
//****************************************************************************
VOID getListViewItem(const HWND& hWndLV
	, const UINT& uItem
	, const UINT& uSubItem
	, PWCHAR& pszText
)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = uItem;
	lvi.iSubItem = uSubItem;
	lvi.cchTextMax = wcslen(pszText);
	lvi.pszText = pszText;
	SendMessage(hWndLV
		, LVM_GETITEM
		, (WPARAM)0
		, (LPARAM)& lvi
	);
}
