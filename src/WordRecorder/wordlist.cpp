#include"wordlist.h"
#include"strop.h"
#include<fstream>
#include<boost/filesystem/fstream.hpp>
#include"codecvt_utf.h"

int wordlist::LoadWords(){
    // return 0 if succeed
    // return -1 if can't file
    //std::wifstream wfin;
    boost::filesystem::wifstream wfin;

    if(removews(filepath)[1] != L':')
        wfin.open((getcurdir()+filepath).c_str(),std::ios::binary);
    else wfin.open(filepath.c_str(),std::ios::binary);
    if(wfin.fail())return -1;
    if(!tvt::IsStreamUnicode(wfin))wfin.imbue(std::locale(""));

    clear();

    std::wstring wstr;
    while(std::getline(wfin,wstr)){  // do not skip empty lines to make line num unchange...
        std::string::size_type pos = wstr.find(L"||");
        if(std::string::npos != pos){
            std::wstring newstr;
            pos += 2;
            while((pos = wstr.find(L"||", pos)) == std::string::npos){
                pos = wstr.size();
                if(!std::getline(wfin,newstr)){
                    wstr += L"||";
                    break;
                }
                wstr += newstr;
            }
            wstr.erase(pos + 2);
        }
        push_back(removews(wstr));
    }

    wfin.close();
    return 0;
}