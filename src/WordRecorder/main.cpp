#include<windows.h>
#include"main.h"

int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nShowCmd){
    WNDCLASS wndclass;

    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hbrBackground  = GetSysColorBrush(COLOR_BTNFACE);
            //GetSysColorBrush(COLOR_WINDOW);
    wndclass.hCursor        = LoadCursor(NULL,IDC_ARROW);
    wndclass.hIcon          = LoadIcon(hInstance,szAppName);
    wndclass.hInstance      = hInstance;
    wndclass.lpfnWndProc    = MainWindowProc;
    wndclass.lpszClassName  = szAppName;
    wndclass.lpszMenuName   = NULL;
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;

    if(!RegisterClassW(&wndclass)){
        MessageBoxW(NULL,L"This program requires Windows NT...",szAppName,MB_ICONERROR);
        return 0;
    }

    wndclass.cbWndExtra     = sizeof(void*);
    wndclass.hIcon          = NULL;
    wndclass.lpfnWndProc    = HintTextProc;
    wndclass.lpszClassName  = szChildWnd;
    RegisterClassW(&wndclass);

    HWND hMainWnd = CreateWindowW(szAppName,szAppName,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            0,0,0,0, NULL,NULL,hInstance,0);

    ShowWindow(hMainWnd,nShowCmd);
    UpdateWindow(hMainWnd);

    MSG msg;
    while(GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}