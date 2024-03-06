#pragma once
#include "header.h"

class CProgressDlg
{
public:
    CProgressDlg();
    ~CProgressDlg();

    static BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static CProgressDlg* ptr;

    BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void Cls_OnClose(HWND hwnd);

    static HWND hProgressDlg;
    HWND hProgressBar,hListTop,hStatic;
};
