#pragma once
#include "header.h"

class CMainDlg
{
public:
	CMainDlg(void);
	~CMainDlg(void);

	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static CMainDlg* ptr;

	void Cls_OnClose(HWND hwnd);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	HWND hEdit1, hButtonFile;
	static TCHAR buffer[1024];
	static TCHAR fileContent[1024];
};
