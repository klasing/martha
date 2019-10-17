#pragma once
#include "GroupBox.hpp"
//****************************************************************************
//*                     GroupBoxResponse
//****************************************************************************
class GroupBoxResponse : public GroupBox
{
	HWND hWndEdtResponse;
public:
	HWND createGroupBox(const HINSTANCE& hInst
		, const HWND& hDlg
		, const int& resourceId
	)
	{
		HWND hWndGroupBoxResponse = GroupBox::createGroupBox(hInst
			, hDlg
			, resourceId
			, (PWCHAR)L"Response"
		);

		hWndEdtResponse = CreateWindowEx(0
			, L"EDIT"
			, L""
			, ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_EDT_RESPONSE
			, hInst
			, NULL
		);

		return hWndGroupBoxResponse;
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
		SetWindowPos(hWndEdtResponse
			, HWND_TOP
			, x + 10
			, y + 20
			, width - 20
			, height - 30
			, SWP_SHOWWINDOW
		);

		return 0;
	}
};
