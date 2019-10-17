#pragma once
#include "GroupBox.hpp"
//****************************************************************************
//*                     GroupBoxRequest
//****************************************************************************
class GroupBoxRequest : public GroupBox
{
	HWND hWndEdtRequest;
public:
	HWND createGroupBox(const HINSTANCE& hInst
		, const HWND& hDlg
		, const int& resourceId
	)
	{
		HWND hWndGroupBoxRequest = GroupBox::createGroupBox(
			hInst
			, hDlg
			, resourceId
			, (PWCHAR)L"Request"
		);
		hWndEdtRequest = CreateWindowEx(0
			, L"EDIT"
			, L""
			, ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE //| WS_BORDER
			, 0, 0, 0, 0
			, hDlg
			, (HMENU)IDC_EDT_REQUEST
			, hInst
			, NULL
		);
		return hWndGroupBoxRequest;
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
		SetWindowPos(hWndEdtRequest
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