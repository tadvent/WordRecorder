#ifndef _WORDLIST_H_
#define _WORDLIST_H_

#include<vector>
#include<string>
#include<functional>
#include<algorithm>
#include<locale>
#include"strop.h"

class wordlist : public std::vector<std::wstring> {
public:
    std::wstring filepath;

    wordlist(){}
    wordlist(const std::wstring &FilePath):filepath(FilePath){
        std::locale loc("");
        std::wstring abspath = getcurdir();
        for(std::wstring::iterator itr=filepath.begin();itr!=filepath.end();++itr)*itr=std::tolower(*itr,loc);
        for(std::wstring::iterator itr=abspath.begin();itr!=abspath.end();++itr)*itr=std::tolower(*itr,loc);
        //std::transform(filepath.begin(),filepath.end(),filepath.begin(),std::bind2nd(std::ptr_fun(std::tolower),std::locale("")));
        //std::transform(abspath.begin(),abspath.end(),abspath.begin(),std::bind2nd(std::ptr_fun(std::tolower),std::locale("")));
        if(filepath.find(abspath) == 0){    // subdirctory
            filepath.erase(0,abspath.length());
        }
    }

    // return 0 if succeed
    // return -1 if can't open file
    int LoadWords(void);
};

#endif