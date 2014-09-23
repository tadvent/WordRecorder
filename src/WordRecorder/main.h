#ifndef __MAIN__
#define __MAIN__

const int IDB_PREV      = 1;
const int IDB_NEXT      = 2;
const int IDB_NOTREM    = 3;
const int IDB_HINT      = 4;

const int IDW_TEXT      = 10;

const int IDM_OPEN      = 50;
const int IDM_EXIT      = 51;
const int IDM_ABOUT     = 60;
const int IDM_LEARNLIST_BEGIN   = 100;
const int IDM_DELETE_BEGIN      = 400;

const wchar_t szAppName[] = L"WordRecorder";
const wchar_t szChildWnd[] = L"WR_ChildWndClass";
LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK HintTextProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

#endif