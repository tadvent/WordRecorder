#ifndef _STROP_H_
#define _STROP_H_

#include<string>

inline bool isws(wchar_t wch){
    return ( L' ' == wch || L'\t' == wch || L'\n' == wch || L'\r' == wch );
}

std::wstring &removews(std::wstring &wstr);

std::wstring getcurdir(void);

#endif