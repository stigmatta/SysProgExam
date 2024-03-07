#include "ProgressDlg.h"
#include "MainDlg.h"
#include <unordered_map>
#include <unordered_set>
#include <sstream>
using namespace std;
namespace fs = std::filesystem;

CProgressDlg* CProgressDlg::ptr = nullptr;
HWND CProgressDlg::hProgressDlg = nullptr;
HWND g_hProgressBar = nullptr;
HANDLE hThread;
bool g_PauseFlag = false;
bool g_StopFlag = false;
INT folderNumber=0;

void WriteReport();

CProgressDlg::CProgressDlg() {
    ptr = this;
}

void CProgressDlg::Cls_OnClose(HWND hwnd)
{
    DestroyWindow(hwnd);
    hProgressDlg= NULL;
}

int HowManyTextFiles() {
    int nFiles = 0;
    fs::path currentDir = fs::current_path();
    for (const auto& entry : fs::directory_iterator(currentDir)) {
        if (entry.path().extension() == ".txt") {
            nFiles++;
        }
    }
    return nFiles;
}


bool sortByValue(const pair<string, int>& a, const pair<string, int>& b) {
    return a.second > b.second;
}

void TopWordsList(HWND hListTop, const vector<pair<string, int>>& wordVector) {
    SendMessage(hListTop, LB_RESETCONTENT, 0, 0);
    for (const auto& pair : wordVector) {
        string word = pair.first;
        int count = pair.second;
        string item = word + " - " + to_string(count);

        SendMessageA(hListTop,LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
    }
}

DWORD WINAPI FindWordsAndPlaceInDir(LPVOID lpParam) {
    unordered_set<string> originalWords; 
    unordered_map<string, int> wordCount;
    HWND hProgress = g_hProgressBar;
    TCHAR* buff = reinterpret_cast<TCHAR*>(lpParam);

    int strSize = WideCharToMultiByte(CP_UTF8, 0, buff, -1, NULL, 0, NULL, NULL);
    string buffString(strSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, buff, -1, &buffString[0], strSize, NULL, NULL);

    istringstream iss(buffString);
    string word;
    while (iss >> word) {
        originalWords.insert(word);
    }

    fs::path currentDir = fs::current_path();
    fs::path foundDir;
    if (!folderNumber) {
        foundDir = fs::current_path() / "Found";
        folderNumber++;
    }
    else {
        foundDir = fs::current_path() / ("Found(" + to_string(folderNumber) + ")");
        folderNumber++;
    }

    if (fs::exists(foundDir)) {
        SendMessage(hProgress, PBM_SETPOS, 100, 0);
        return 0;
    }

    fs::create_directory(foundDir);

    for (const auto& entry : fs::directory_iterator(currentDir)) {
        if (g_PauseFlag) {
            while (g_PauseFlag) {
                Sleep(100);
            }
        }
        else if (g_StopFlag) {
            return 0;
        }

        if (entry.path().extension() == ".txt" && entry.path().filename().string().find("encrypted") == string::npos) {
            ifstream file(entry.path());
            string word;
            bool wordFound = false;
            while (file >> word) {
                if (originalWords.find(word) != originalWords.end()) {
                    wordCount[word]++;
                }

                if (buffString.find(word) != std::string::npos) {
                    wordFound = true;
                    break;
                }
            }

            if (wordFound) {
                fs::copy(entry.path(), foundDir / entry.path().filename());
            }

            file.close();
            Sleep(1500);

            SendMessage(hProgress, PBM_STEPIT, 0, 0);
        }
    }
    vector<pair<string, int>> sortedWordVector(wordCount.begin(), wordCount.end());
    sort(sortedWordVector.begin(), sortedWordVector.end(), sortByValue);
    HWND hListTop = GetDlgItem(CProgressDlg::hProgressDlg, IDC_LISTTOP);

    TopWordsList(hListTop, sortedWordVector);

    Sleep(1500);
    MessageBox(CProgressDlg::hProgressDlg, L"Отчет сформирован", L"Отчет", MB_OK);
    ShowWindow(hProgress, SW_HIDE);
    HWND hStatic = GetDlgItem(CProgressDlg::hProgressDlg, IDC_STATIC);
    ShowWindow(hStatic, SW_HIDE);
    ShowWindow(hListTop, SW_SHOW);
    WriteReport();
    return 0;
}





BOOL CProgressDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
    HWND hProgress = GetDlgItem(hwnd, IDC_PROGRESS1);
    int nFiles = HowManyTextFiles();
    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, nFiles));
    SendMessage(hProgress, PBM_SETSTEP, 1, 0);

    hProgressDlg = hwnd;

    g_hProgressBar = hProgress;

    if (CMainDlg::buffer[0] == '\0') {
        hThread = CreateThread(nullptr, 0, FindWordsAndPlaceInDir, (LPVOID)CMainDlg::fileContent, 0, nullptr);
    }
    else {
        hThread = CreateThread(nullptr, 0, FindWordsAndPlaceInDir, (LPVOID)CMainDlg::buffer, 0, nullptr);
    }
    CloseHandle(hThread);

    return TRUE;
}



void CProgressDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    HWND hPause = GetDlgItem(hwnd, IDPAUSE);
    TCHAR buttonText[50];
    GetWindowText(hPause, buttonText, 50); 

    if (id == IDPAUSE && _tcscmp(buttonText, TEXT("Пауза")) == 0) {
        g_PauseFlag = !g_PauseFlag;
        SuspendThread(hThread);
        SetWindowText(hPause, TEXT("Возобновить")); 
    }
    else if (id == IDPAUSE && _tcscmp(buttonText, TEXT("Возобновить")) == 0) {
        g_PauseFlag = !g_PauseFlag;
        ResumeThread(hThread);
        SetWindowText(hPause, TEXT("Пауза"));
    }
    if (id == IDSTOP) {
        g_StopFlag = true; 
        CloseWindow(hwnd);
    }
}

void WriteReport() {
    ofstream reportFile("report.txt");
    fs::path directoryPath = fs::current_path()/"Found";
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".txt" && entry.path().filename().string().find("encrypted") == string::npos)
        {
            reportFile << "Путь к файлу: " << entry.path() << endl;
            reportFile << "Размер файла: " << fs::file_size(entry.path()) << " байт" << endl<<endl<<endl;
        }
    }


    reportFile.close();
}

CProgressDlg::~CProgressDlg() {}


BOOL CALLBACK CProgressDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
        HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
    }
    return FALSE;
}
