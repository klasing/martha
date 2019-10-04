#pragma once
#include "GroupBox.hpp"
class GroupBoxAccess : public GroupBox {
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
		return 0;
	}
};