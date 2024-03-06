#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <regex>
#include <fstream>

#include "MainDlg.h"
#include "ProgressDlg.h"
using namespace std;
namespace fs = std::filesystem;

TCHAR CMainDlg::buffer[1024] = { 0 };
TCHAR CMainDlg::fileContent[1024] = { 0 };
CMainDlg* CMainDlg::ptr = NULL;
INT folderEncNumber = 0;

CMainDlg::CMainDlg(void)
{
	ptr = this;
}

CMainDlg::~CMainDlg(void)
{
}

void CMainDlg::Cls_OnClose(HWND hwnd)
{
    EndDialog(hwnd, IDCANCEL);
}


BOOL CMainDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
	hEdit1 = GetDlgItem(hwnd, IDC_EDIT1);
	hButtonFile = GetDlgItem(hwnd, IDC_BUTTONFILE);
	
	return TRUE;
}

void EncryptString(const wstring& str) {
    wregex wordRegex(L"\\b\\w+\\b");
    wstring encryptedStr = regex_replace(str, wordRegex, L"*******");
    fs::path currentDir = fs::current_path();
    fs::path filePath;
    if (!folderEncNumber) {
        filePath = currentDir / "encrypted.txt";
        folderEncNumber++;
    }
    else {
        filePath = currentDir / ("encrypted" + to_string(folderEncNumber) + ".txt");
        folderEncNumber++;
    }

    wofstream outFile(filePath);
    if (outFile.is_open()) {
        outFile << encryptedStr;
        outFile.close();
    }
}

void CMainDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

  

    if (id == IDC_BUTTONFILE) {
        TCHAR FullPath[MAX_PATH] = { 0 };
        string fileContentStr;
        OPENFILENAME open = { sizeof(OPENFILENAME) };
        open.hwndOwner = hwnd;
        open.lpstrFilter = TEXT("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0");
        open.lpstrFile = FullPath;
        open.nMaxFile = MAX_PATH;
        open.lpstrInitialDir = TEXT("C:\\");
        open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
        if (GetOpenFileName(&open)) {
            ifstream file(FullPath);
            if (file.is_open()) {
                string line;
                while (getline(file, line)) {
                    fileContentStr += line + '\n';
                }
                file.close();
            }

            regex_replace(fileContentStr,  regex("\\s+"), " ");
            fileContentStr = std::regex_replace(fileContentStr, regex("^\\s+|\\s+$"), "");
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, &fileContentStr[0], fileContentStr.size(), nullptr, 0);
            MultiByteToWideChar(CP_UTF8, 0, &fileContentStr[0], fileContentStr.size(), &fileContent[0], size_needed);

        }
    }

    else if (id == IDC_BUTTON1) {


        if (fileContent[0] == '\0') {
            GetDlgItemText(hwnd, IDC_EDIT1, buffer, 1024);
            if (buffer[0] == '\0') {
                MessageBeep(0);
                MessageBox(hwnd, L"Предоставьте список запрещенных слов", L"Ошибка", MB_CANCELTRYCONTINUE);
            }
            else {
                wstring bufferStr = buffer;
                bufferStr = regex_replace(bufferStr, wregex(L"\\s+"), L" ");

                bufferStr = regex_replace(bufferStr, wregex(L"^\\s+|\\s+$"), L"");

                wcsncpy(buffer, bufferStr.c_str(), sizeof(buffer) / sizeof(buffer[0]));
                buffer[sizeof(buffer) / sizeof(buffer[0]) - 1] = L'\0'; 
                EncryptString(bufferStr);

                CProgressDlg::hProgressDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hwnd, (DLGPROC)CProgressDlg::DlgProc);
                ShowWindow(CProgressDlg::hProgressDlg, SW_RESTORE);
                return;
            }
        }
        else {
            EncryptString(fileContent);
            CProgressDlg::hProgressDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hwnd, (DLGPROC)CProgressDlg::DlgProc);
            ShowWindow(CProgressDlg::hProgressDlg, SW_RESTORE);
            return;
        }
    }
}



BOOL CALLBACK CMainDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
};