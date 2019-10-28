#pragma once
#include "GroupBox.hpp"

//****************************************************************************
//*                     global
//****************************************************************************
extern const size_t BUFFER_MAX;
extern PWCHAR pszTextBuffer;

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
		// set the font of the edit control to a more typical system GUI font
		SendMessage(hWndEdtRequest
			, WM_SETFONT
			, (WPARAM)GetStockObject(DEFAULT_GUI_FONT)
			, (LPARAM)0
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
	VOID setGroupBoxText(const std::string log_message_req)
	{
		std::wstring wstring = std::wstring(log_message_req.begin()
			, log_message_req.end()
		);
		const wchar_t* pszText = wstring.c_str();
		SendMessage(hWndEdtRequest
			, WM_SETTEXT
			, (WPARAM)0
			, (LPARAM)pszText
		);
	}
};
