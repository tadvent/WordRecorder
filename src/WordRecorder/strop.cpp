#include"strop.h"
#include<windows.h>

std::wstring &removews(std::wstring &wstr){
    std::wstring::iterator itr;
    itr = wstr.begin();
    while(itr != wstr.end() && isws(*itr))++itr;
    wstr.erase(wstr.begin(),itr);

    itr = wstr.end();
    while(itr != wstr.begin() && isws(*(itr-1)))--itr;
    wstr.erase(itr,wstr.end());
    return wstr;
}

std::wstring getcurdir(){
    wchar_t buffer[MAX_PATH];

    GetModuleFileNameW(NULL,buffer,MAX_PATH);
    std::wstring abspath(buffer);

    //std::wstring::iterator itr = abspath.end();
    //while(itr!=abspath.begin() && *(itr-1) != L'\\')--itr;
    //abspath.erase(itr,abspath.end());   // abspath: "F:\xxx\xxx\curdir\"
    abspath.erase(abspath.find_last_of(L'\\') + 1);
    return abspath;
}
