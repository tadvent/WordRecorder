#include<windows.h>
#include<commdlg.h>
#include<sstream>
#include"main.h"
#include"option.h"
#include"learnlist.h"
#include"reviewlist.h"
#include"strop.h"

extern HMENU hMenu;

static OPENFILENAME ofn;
static LOGFONT lf;

void InitOpenFileName(HWND hwnd){
    static wchar_t szFilter[] = /*L"WordRecord Format 1 Files (*.wr1)\0*.wr1\0"*/ \
                                L"Text Files (*.txt)\0*.txt\0" \
                                L"All Files (*.*)\0*.*\0\0";
    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner       = hwnd;
    ofn.hInstance       = NULL;
    ofn.lpstrFilter     = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter  = 0;
    ofn.nFilterIndex    = 0;
    ofn.lpstrFile       = NULL; // set this value later
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = NULL; // set this value later, set to the learns.curlistidx path
    ofn.lpstrTitle      = NULL;
    ofn.Flags           = OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST;
    ofn.nFileOffset     = 0;
    ofn.nFileExtension  = 0;
    ofn.lpstrDefExt     = NULL;
    ofn.lCustData       = 0L;
    ofn.lpfnHook        = NULL;
    ofn.lpTemplateName  = NULL;
}

void InitLogFont(){
    lf.lfWidth          = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight         = FW_NORMAL;
    lf.lfItalic         = 0;
    lf.lfUnderline      = 0;
    lf.lfStrikeOut      = 0;
    lf.lfCharSet        = DEFAULT_CHARSET;
    lf.lfOutPrecision   = 0;
    lf.lfClipPrecision  = 0;
    lf.lfQuality        = 0;
    lf.lfPitchAndFamily = 0;
}

static int CkDismode(const wordinfo &wi, progress &learns, std::wstring &nw){
    // return 1 if review word is valid
    // return 2 if new word to display
    // return 0 if no word to review or learn

    if(!wi.wordstr.empty())return 1;
    if(0 == learns.CurrentWord(nw))return 2;
    return 0;
}

std::wstring PathToFile(const std::wstring &filepath){
    std::wstring filename(filepath);
    std::wstring::iterator itr = filename.end();
    while(itr!=filename.begin() && *(itr-1) != L'\\')--itr;
    filename.erase(filename.begin(),itr);
    return filename;
}

std::wstring PathToAbspath(const std::wstring &filepath){
    std::wstring abspath(filepath);
    std::wstring::iterator itr = abspath.end();
    while(itr!=abspath.begin() && *(itr-1) != L'\\')--itr;
    abspath.erase(itr,abspath.end());
    return abspath;
}

static void SetListMenu(HMENU &hmenu, const progress &learns, int offset){
    int cnt = GetMenuItemCount(hmenu);
    while(cnt != learns.size()){
        if(cnt > int(learns.size())){
            DeleteMenu(hmenu, --cnt, MF_BYPOSITION);
        } else {
            AppendMenu(hmenu, MF_STRING, ++cnt, L"NewItem");
        }
    }

    for(cnt=0; cnt<int(learns.size()); ++cnt){
        ModifyMenu(hmenu, cnt, MF_STRING|MF_BYPOSITION, offset + cnt,
                PathToFile(learns[cnt].filepath).c_str());
    }
    if(offset == IDM_LEARNLIST_BEGIN)
        CheckMenuItem(hmenu, learns.CurListIdx, MF_BYPOSITION|MF_CHECKED);
}

int HasHintPart(std::wstring &word, std::wstring &name, std::wstring &hint){
    // return 0 if has no hint part. word do not change
    // return 1 if has a hint part. word became the word part,
    //     name is the button name and hint is the hinttext
    std::string::size_type pos, end;
    std::wstringstream wsstrm;
    hint.clear();
    if((pos = word.find(L"||")) == std::string::npos)return 0;
    pos += 2;
    if((end = word.find(L"||", pos)) != std::string::npos){   // find 2nd "||"
        wsstrm<<word.substr(pos, end - pos);
    } else {    // can't find 2nd "||"
        wsstrm<<word.substr(pos);
    }
    removews(word.erase(pos - 2));

    wsstrm>>name;
    wsstrm.get();
    std::wstring wstr;
    while(std::getline(wsstrm, wstr))hint += wstr;
    return 1;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
    static HINSTANCE hInst;
    static HWND hPrev, hNext, hNotRem, hHint, hText;
    static int cxClient,cyClient,cxChar,cyChar;
    static RECT rectWord, rectMode, rectRemain, rectIdx/*, rectHint*/;
    static HMENU hMenu, hMenuFile, hMenuLearn, hMenuDelete, hMenuHelp;
    static HBRUSH hTextBack;
    static HFONT hFontWord, hFontMode, hFontRemain, hFontIdx, hFontButton;

    static option op;
    static progress learns;
    static reviewlist revs;
    static int dismode;  // 1 means review word, 2 means new word, 0 means no word on display ...
    static wordinfo wi; // store the review word ...
    static std::wstring nw; // store the new word to learn ...

    static std::vector<std::wstring> prevwords;
    static int pwidx = 0;
    static wchar_t szFilePath[MAX_PATH] = {0};
    static wchar_t buffer[20];
    static bool bShowHint = false;
    std::wstring wstr, wsMode, wsRemain, wsIdx, wsButtonName, wsFilePath;
    static std::wstring wsHint;
    const COLORREF black = RGB(0,0,0);
    const COLORREF red   = RGB(255,0,0);
    const COLORREF green = RGB(60,185,60);
    const COLORREF blue  = RGB(0,0,255);
    COLORREF color;
    PAINTSTRUCT ps;
    HDC hdc;

    int identifier, linenum;

    switch(message){
        case WM_CREATE:
            hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            cxChar = LOWORD(GetDialogBaseUnits());
            cyChar = HIWORD(GetDialogBaseUnits());

            InitLogFont();

            MoveWindow(hwnd,(GetSystemMetrics(SM_CXSCREEN)-50*cxChar)/2,
                            (3*GetSystemMetrics(SM_CYSCREEN)/4-15*cyChar)/2,
                            50*cxChar,15*cyChar,TRUE);

            hPrev = CreateWindowW(L"button",L"Previous",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    0,0,0,0, hwnd, HMENU(IDB_PREV), hInst, 0);
            hNext = CreateWindowW(L"button",L"Next",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    0,0,0,0, hwnd, HMENU(IDB_NEXT), hInst, 0);
            hNotRem = CreateWindowW(L"button",L"Forgot",WS_CHILD | BS_PUSHBUTTON,
                    0,0,0,0, hwnd, HMENU(IDB_NOTREM), hInst, 0);
            hHint = CreateWindowW(L"button", L"Hint", WS_CHILD | BS_PUSHBUTTON,
                    0,0,0,0, hwnd, HMENU(IDB_HINT), hInst, 0);
            hText = CreateWindowW(szChildWnd, NULL, WS_CHILDWINDOW | WS_VSCROLL,
                    0,0,0,0, hwnd, HMENU(IDW_TEXT), hInst, 0);

            lstrcpy(lf.lfFaceName, L"Lucida Console");
            lf.lfHeight = 12;
            hFontRemain = CreateFontIndirectW(&lf);
            hFontIdx = hFontRemain;
            lstrcpy(lf.lfFaceName, L"Lucida Sans Unicode");
            lf.lfHeight = 17;
            hFontMode = CreateFontIndirectW(&lf);
            lstrcpy(lf.lfFaceName, L"Arial");
            lf.lfHeight = 28;
            hFontWord = CreateFontIndirectW(&lf);

            SetWindowLong(hText, 0, LONG(&wsHint));

            InitOpenFileName(hwnd);

            if(op.LoadOptionsFromFile()){
                if(-1 == op.CreateNewOptionFile()){
                    MessageBox(hwnd,L"Can't create config file...", szAppName, MB_ICONERROR);
                }
            } else {
                revs.timeinterval = op.timeinterval;
            }
            if(-2 == learns.LoadLearnLists()){
                MessageBox(hwnd,L"Some new Wordlist files load error...",
                        szAppName, MB_ICONEXCLAMATION);
            }
            {
                int nMode = 0;
                while(-2 == revs.LoadData(nMode,wstr)){
                    switch(MessageBox(hwnd, (std::wstring(L"Can't read file ") + wstr).c_str(),
                            szAppName, MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION)){
                        case IDABORT:
                            nMode = 3;
                            break;
                        case IDRETRY:
                            nMode = 1;
                            break;
                        case IGNORE:
                            nMode = 2;
                            break;
                    }
                }
            }

            hMenu       = CreateMenu();
            hMenuFile   = CreatePopupMenu();
            hMenuLearn  = CreatePopupMenu();
            hMenuDelete = CreatePopupMenu();
            hMenuHelp   = CreatePopupMenu();
            AppendMenuW(hMenu,MF_POPUP|MF_STRING,(UINT_PTR)hMenuFile,L"File");
            AppendMenuW(hMenu,MF_POPUP|MF_STRING,(UINT_PTR)hMenuLearn,L"Lists");
            AppendMenuW(hMenu,MF_POPUP|MF_STRING,(UINT_PTR)hMenuDelete,L"Delete");
            AppendMenuW(hMenu,MF_POPUP|MF_STRING,(UINT_PTR)hMenuHelp,L"Help");

            AppendMenuW(hMenuFile,MF_STRING,IDM_OPEN,L"Open New List...");
            AppendMenuW(hMenuFile,MF_SEPARATOR,NULL,NULL);
            AppendMenuW(hMenuFile,MF_STRING,IDM_EXIT,L"Exit");

            AppendMenuW(hMenuHelp,MF_STRING,IDM_ABOUT,L"About");

            SetListMenu(hMenuLearn, learns, IDM_LEARNLIST_BEGIN);
            SetListMenu(hMenuDelete, learns, IDM_DELETE_BEGIN);
            SetMenu(hwnd,hMenu);

            wi = revs.nextword(0);
            dismode = CkDismode(wi,learns,nw);

            hTextBack = GetSysColorBrush(COLOR_WINDOW);
            break;

        case WM_GETMINMAXINFO:
            LPMINMAXINFO(lParam)->ptMinTrackSize.x = 45 * cxChar;
            LPMINMAXINFO(lParam)->ptMinTrackSize.y = 12 * cyChar;
            return 0;

        case WM_SIZE:
            cxClient = LOWORD(lParam);
            cyClient = HIWORD(lParam);
            MoveWindow(hPrev,   2*cxChar,               cyClient-17*cyChar/8,
                                9*cxChar,               13*cyChar/8,TRUE);
            MoveWindow(hNext,   cxClient-11*cxChar,     cyClient-17*cyChar/8,
                                9*cxChar,               13*cyChar/8,TRUE);
            MoveWindow(hNotRem, cxClient-21*cxChar,     cyClient-17*cyChar/8,
                                9*cxChar,               13*cyChar/8,TRUE);
            MoveWindow(hHint,   12*cxChar,              cyClient-17*cyChar/8,
                                9*cxChar,               13*cyChar/8,TRUE);

            rectMode.left       = (cxClient - 12*cxChar)/2;
            rectMode.top        = cyChar/8;
            rectMode.right      = (cxClient + 12*cxChar)/2;
            rectMode.bottom     = 9*cyChar/8;

            rectRemain.left     = 2*cxChar;
            rectRemain.top      = 3*cyChar/8;
            rectRemain.right    = 16*cxChar;
            rectRemain.bottom   = 9*cyChar/8;

            rectIdx.left        = cxClient - 17*cxChar;
            rectIdx.top         = 3*cyChar/8;
            rectIdx.right       = cxClient - 2*cxChar;
            rectIdx.bottom      = 9*cyChar/8;

            rectWord.left   = 2*cxChar;
            rectWord.top    = 9*cyChar/8;
            rectWord.right  = cxClient - 2*cxChar;
            rectWord.bottom = min(cyClient - 11*cyChar/4, 25*cyChar/8);
            
            MoveWindow(hText, 2*cxChar, rectWord.bottom + cyChar/2,
                    cxClient - 4*cxChar, cyClient - 25*cyChar/8 - rectWord.bottom, TRUE);

            //rectHint.left   = 2*cxChar;
            //rectHint.top    = rectWord.bottom + cyChar/2;
            //rectHint.right  = cxClient - 2*cxChar;
            //rectHint.bottom = cyClient - 21*cyChar/8;
            break;

        case WM_SETFOCUS:
            SetFocus(hText);
            return 0;

        case WM_COMMAND:
            identifier = LOWORD(wParam);
            if(identifier >= IDM_LEARNLIST_BEGIN && identifier < IDM_LEARNLIST_BEGIN + 100){
                int idx = identifier - IDM_LEARNLIST_BEGIN;
                if(idx != learns.CurListIdx){
                    if(dismode == 2 && int(prevwords.size()) <= pwidx){
                        bShowHint = false;
                        SendMessage(hText,WM_VSCROLL,SB_TOP,0);
                    }
                    CheckMenuItem(hMenuLearn, learns.CurListIdx, MF_BYPOSITION|MF_UNCHECKED);
                    CheckMenuItem(hMenuLearn, idx, MF_BYPOSITION|MF_CHECKED);
                    learns.CurListIdx = idx;
                    DrawMenuBar(hwnd);
                }
            }
            if(identifier >= IDM_DELETE_BEGIN && identifier < IDM_DELETE_BEGIN + 100){
                int idx = identifier - IDM_DELETE_BEGIN;
                learns.DeleteList(idx);
                SetListMenu(hMenuLearn, learns, IDM_LEARNLIST_BEGIN);
                SetListMenu(hMenuDelete, learns, IDM_DELETE_BEGIN);
                DrawMenuBar(hwnd);
            }
            switch(identifier){
                case IDB_PREV:
                    if(!(pwidx--))pwidx = 0;
                    else {
                        bShowHint = false;
                        SendMessage(hText,WM_VSCROLL,SB_TOP,0);
                    }
                    break;
                case IDB_NEXT:
                    bShowHint = false;
                    SendMessage(hText,WM_VSCROLL,SB_TOP,0);
                    if(int(prevwords.size()) > pwidx){
                        ++pwidx;
                    } else {
                        switch(dismode){
                            case 1: // review word
                                prevwords.push_back(wi.wordstr);
                                pwidx = (int)prevwords.size();
                                wi = revs.nextword(1);
                                break;
                            case 2: // new word
                                prevwords.push_back(nw);
                                pwidx = (int)prevwords.size();
                                revs.addword(nw,learns[learns.CurListIdx].filepath,
                                    learns[learns.CurListIdx].curline);

                                if(learns.NextWord()){
                                    SetListMenu(hMenuLearn,learns, IDM_LEARNLIST_BEGIN);
                                    SetListMenu(hMenuDelete, learns, IDM_DELETE_BEGIN);
                                    DrawMenuBar(hwnd);
                                }
                                // fall through
                            case 0:
                                wi = revs.nextword(0);
                                break;
                        }
                    }
                    break;
                case IDB_NOTREM:
                    bShowHint = false;
                    SendMessage(hText,WM_VSCROLL,SB_TOP,0);
                    if(int(int(prevwords.size()) > pwidx)){
                        pwidx = (int)prevwords.size();
                    } else {
                        switch(dismode){
                            case 1: // review word
                                prevwords.push_back(wi.wordstr);
                                pwidx = (int)prevwords.size();
                                wi = revs.nextword(2);
                                break;
                            case 2: // new word
                                // this button not use for new word
                                if(learns.NextWord()){
                                    SetListMenu(hMenuLearn,learns, IDM_LEARNLIST_BEGIN);
                                    SetListMenu(hMenuDelete, learns, IDM_DELETE_BEGIN);
                                    DrawMenuBar(hwnd);
                                }
                                wi = revs.nextword(0);
                                break;
                            case 0:
                                break;
                        }
                    }
                    break;
                case IDB_HINT:
                    bShowHint = !bShowHint;
                    break;
                case IDM_OPEN:
                    ofn.lpstrFile  = szFilePath;
                    if(!learns.empty()){
                        ofn.lpstrInitialDir = PathToAbspath(learns[learns.CurListIdx].filepath).c_str();
                    }
                    if(GetOpenFileName(&ofn)){
                        wstr = szFilePath;
                        if(learns.IsInList(wstr)){
                            MessageBox(hwnd,L"Already in Learn list ...",szAppName,MB_ICONINFORMATION);
                        } else if(-1 == learns.AddNewList(wstr)){
                            MessageBox(hwnd,L"Can't read file ...",szAppName,MB_ICONEXCLAMATION);
                        } else {
                            SetListMenu(hMenuLearn, learns, IDM_LEARNLIST_BEGIN);
                            SetListMenu(hMenuDelete, learns, IDM_DELETE_BEGIN);
                            DrawMenuBar(hwnd);
                        }
                    }
                    break;
                case IDM_EXIT:
                    SendMessage(hwnd, WM_CLOSE, NULL, NULL);
                    return 0;

                case IDM_ABOUT:
                    MessageBox(hwnd,L"WordRecorder ver. 0.0.2\n\nby tadvent ^_^\n\ndantvt@gmail.com\nhttp://dantvt.spaces.live.com/",szAppName,MB_ICONINFORMATION);
                    return 0;
            }
            dismode = CkDismode(wi,learns,nw);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case WM_PAINT:
            color = black;
            if(pwidx < 0)pwidx = 0;
            if(int(prevwords.size()) > pwidx){
                color = green;
                wsMode = L"Look Back";

                if(revs.toreview.size()+revs.timexceed.size() > 0){
                    wsprintf(buffer,L"Remain: %d",revs.toreview.size() + revs.timexceed.size());
                    wsRemain = buffer;
                } else wsRemain.clear();
                wsprintf(buffer,L"%d/%d", pwidx+1, prevwords.size());
                wsIdx = buffer;
                wstr = prevwords[pwidx];
                SetWindowTextW(hNotRem, L"JumpOut");
                ShowWindow(hNotRem,SW_SHOW);
                ShowWindow(hHint,SW_HIDE);
            } else {
                switch(dismode){
                    case 1: // review word
                        color = blue;
                        wsMode = L"Review";
                        wsprintf(buffer,L"Remain: %d",revs.toreview.size() + revs.timexceed.size());
                        wsRemain = buffer;
                        wsprintf(buffer,L"Times: %d/%d", wi.nth+1, revs.timeinterval.size());
                        wsIdx = buffer;
                        wstr = wi.wordstr;
                        wsFilePath = revs.wlfilenames[wi.fileno];
                        linenum = wi.lineno;
                        SetWindowTextW(hNotRem, L"Forgot");
                        ShowWindow(hNotRem,SW_SHOW);
                        ShowWindow(hHint,SW_SHOW);
                        break;
                    case 2: // new word
                        color = red;
                        wsMode = L"New";
                        wsRemain.clear();
                        wsIdx.clear();
                        wstr = nw;
                        wsFilePath = learns[learns.CurListIdx].filepath;
                        linenum = learns[learns.CurListIdx].curline;
                        SetWindowTextW(hNotRem, L"Ignore");
                        ShowWindow(hNotRem,SW_SHOW);
                        ShowWindow(hHint,SW_SHOW);
                        break;
                    case 0:
                        wstr = L"No Word to Display ...";
                        ShowWindow(hNotRem,SW_HIDE);
                        ShowWindow(hHint,SW_HIDE);
                        break;
                }
            }

            if(HasHintPart(wstr,wsButtonName,wsHint))
                ShowWindow(hHint,SW_SHOW);
            else wsButtonName = L"FilePath";
            SetWindowTextW(hHint, wsButtonName.c_str());
            if(bShowHint){
                if(int(prevwords.size()) <= pwidx){
                    _itow(linenum+1,buffer,10);
                    wsHint += std::wstring(L"\n______________________\nFilePath: ") + wsFilePath
                        + std::wstring(L"\nword no. ") + std::wstring(buffer);
                }
                SendMessage(hText,WM_APP+0,NULL,NULL);
                ShowWindow(hText,SW_SHOW);
            }
            else ShowWindow(hText,SW_HIDE);
            //if(HasHintPart(wstr,wsButtonName,wsHint)){
            //    SetWindowTextW(hHint, wsButtonName.c_str());
            //    ShowWindow(hHint,SW_SHOW);
            //    if(bShowHint){
            //        SendMessage(hText,WM_APP+0,NULL,NULL);
            //        ShowWindow(hText,SW_SHOW);
            //    }
            //    else ShowWindow(hText,SW_HIDE);
            //}
            //else {
            //    ShowWindow(hHint,SW_HIDE);
            //    ShowWindow(hText,SW_HIDE);
            //}

            hdc = BeginPaint(hwnd,&ps);
            //SetBkColor(hdc,COLORREF(GetSysColor(COLOR_BTNFACE)));
            SetBkMode(hdc,TRANSPARENT);
            SelectObject(hdc, HGDIOBJ(hTextBack));
            //SelectObject(hdc, HGDIOBJ(GetStockObject(NULL_PEN)));
            Rectangle(hdc,rectWord.left,rectWord.top,rectWord.right,rectWord.bottom);
            SetTextColor(hdc,color);
            SelectObject(hdc,HGDIOBJ(hFontMode));
            DrawTextW(hdc, wsMode.c_str(), wsMode.length(), &rectMode,
                    DT_CENTER/*|DT_SINGLELINE|DT_VCENTER*/);
            SetTextColor(hdc,RGB(0,0,0));
            SelectObject(hdc,HGDIOBJ(hFontRemain));
            DrawTextW(hdc, wsRemain.c_str(), wsRemain.length(), &rectRemain,
                    DT_LEFT/*|DT_SINGLELINE|DT_VCENTER*/);
            SelectObject(hdc,HGDIOBJ(hFontIdx));
            DrawTextW(hdc, wsIdx.c_str(), wsIdx.length(), &rectIdx,
                    DT_RIGHT/*|DT_SINGLELINE|DT_VCENTER*/);
            SelectObject(hdc,HGDIOBJ(hFontWord));
            DrawTextW(hdc, wstr.c_str(), wstr.length(), &rectWord,
                    DT_CENTER|DT_SINGLELINE|DT_VCENTER);
            EndPaint(hwnd,&ps);
            SetFocus(hText);
            break;

        case WM_CLOSE:
            if(-1 == learns.SaveLearnLists()){
                MessageBox(hwnd,L"Can't save new word list file ...", szAppName, MB_ICONERROR);
            }
            switch(revs.SaveData()){
                case -1:
                    MessageBox(hwnd,L"Can't write to save file ...",szAppName,MB_ICONERROR);
                    break;
                case -2:
                    MessageBox(hwnd,L"Can't create backup save file ...",szAppName,
                        MB_ICONEXCLAMATION);
                    break;
            }
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd,message,wParam,lParam);
}


LRESULT CALLBACK HintTextProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
    static int cxClient, cyClient, cxScroll, cxChar, cyChar;
    static RECT rect;
    static SCROLLINFO si;
    static HFONT hFontText;
    static std::wstring *pwstr;
    static bool fNotInit = false;
    static int iDeltaPerLine = 40, iAccumDelta = 0;
    int iPos;
    HDC hdc;
    RECT rectTmp;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;

    switch(message){
        case WM_CREATE:
            //
            cxScroll = GetSystemMetrics(SM_CXVSCROLL);
            hdc = GetDC(hwnd);
            GetTextMetrics(hdc,&tm);
            cxChar = tm.tmAveCharWidth;
            cyChar = tm.tmHeight + tm.tmExternalLeading;
            ReleaseDC(hwnd,hdc);
            lf.lfHeight = 20;
            //lstrcpy(lf.lfFaceName, L"Î¢ÈíÑÅºÚ");
            lstrcpy(lf.lfFaceName, L"ËÎÌå");
            hFontText = CreateFontIndirectW(&lf);
            break;

        case WM_SIZE:
            cxClient = LOWORD(lParam);
            cyClient = HIWORD(lParam);

        case WM_APP + 0:
            pwstr = (std::wstring*)(GetWindowLong(hwnd,0));

            rect.left = 0;
            rect.top  = 0;
            rect.right = cxClient;

            if(fNotInit){
                hdc = GetDC(hwnd);
                SelectObject(hdc,HGDIOBJ(hFontText));
                rect.bottom = DrawTextW(hdc,pwstr->c_str(),pwstr->length(), &rect,
                        DT_CALCRECT|DT_WORDBREAK|DT_LEFT);
                if(rect.bottom > cyClient){
                    rect.right = cxClient - cxScroll;
                    rect.bottom = DrawTextW(hdc,pwstr->c_str(),pwstr->length(), &rect,
                            DT_CALCRECT|DT_WORDBREAK|DT_LEFT);
                }
                ReleaseDC(hwnd,hdc);
            } else  fNotInit = true;

            si.cbSize   = sizeof(SCROLLINFO);
            si.fMask    = SIF_RANGE | SIF_PAGE;
            //if(message == WM_APP){
            //    si.nPos     = si.nMin;
            //    si.fMask   |= SIF_POS;
            //}
            si.nMin     = 0;
            si.nMax     = rect.bottom;
            si.nPage    = cyClient;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

            break;

        case WM_VSCROLL:
            si.cbSize   = sizeof(SCROLLINFO);
            si.fMask    = SIF_ALL;
            GetScrollInfo(hwnd, SB_VERT, &si);
            iPos = si.nPos;
            switch(LOWORD(wParam)){
                case SB_TOP:
                    si.nPos = si.nMin;
                    break;
                case SB_BOTTOM:
                    si.nPos = si.nMax;
                    break;
                case SB_LINEUP:
                    si.nPos -= cyChar;
                    break;
                case SB_LINEDOWN:
                    si.nPos += cyChar;
                    break;
                case SB_PAGEUP:
                    si.nPos -= si.nPage;
                    break;
                case SB_PAGEDOWN:
                    si.nPos += si.nPage;
                    break;
                case SB_THUMBTRACK:
                    si.nPos = si.nTrackPos;
                    break;
            }
            si.fMask = SIF_POS;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            GetScrollInfo(hwnd, SB_VERT, &si);
            if(si.nPos != iPos){
                ScrollWindow(hwnd,0,iPos-si.nPos,NULL,NULL);
                UpdateWindow(hwnd);
            }
            break;

        case WM_MOUSEWHEEL:
            iAccumDelta += short(HIWORD(wParam));
            while(iAccumDelta >= iDeltaPerLine){
                SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
                iAccumDelta -= iDeltaPerLine;
            }
            while(iAccumDelta <= -iDeltaPerLine){
                SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
                iAccumDelta += iDeltaPerLine;
            }
            return 0;

        case WM_PAINT:
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_POS;
            GetScrollInfo(hwnd,SB_VERT,&si);

            rectTmp.left = rect.left;
            rectTmp.right = rect.right;
            rectTmp.top = rect.top - si.nPos;
            rectTmp.bottom = cyClient;

            hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc,TRANSPARENT);
            SelectObject(hdc,HGDIOBJ(hFontText));
            DrawTextW(hdc, pwstr->c_str(), pwstr->length(), &rectTmp,
                    DT_LEFT|DT_WORDBREAK|DT_NOPREFIX);
            EndPaint(hwnd,&ps);
            break;
    }
    return DefWindowProcW(hwnd,message,wParam,lParam);
}