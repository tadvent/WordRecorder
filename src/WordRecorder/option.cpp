#include "option.h"
#include <fstream>
#include <sstream>
#include <locale>
#include <boost/filesystem/fstream.hpp>
#include "codecvt_utf.h"
#include "strop.h"

wchar_t deftimestr[] = L"5m\r\n30m\r\n6h\r\n1d\r\n2d\r\n5d\r\n15d\r\n30d\r\n";

int deftimevalues[] = {
    5*60, 30*60, 6*3600, 1*24*3600, 2*24*3600, 5*24*3600, 15*24*3600, 30*24*3600
};

int option::LoadOptionsFromFile(){
    // return 0 if succeed;
    // return -1 if can't open file.
    // return -2 if invalid data.

    //std::wifstream wfin(::OptionFileName,std::ios::binary);
    boost::filesystem::wifstream wfin(::OptionFileName,std::ios::binary);
    if(wfin.fail()){
        timeinterval.assign(deftimevalues, deftimevalues
            + sizeof(deftimevalues)/sizeof(deftimevalues[0]));
        return -1;
    }

    timeinterval.clear();
    if(!tvt::IsStreamUnicode(wfin))wfin.imbue(std::locale(""));

    int secs;
    std::wstring line;
    std::wstringstream wsstr;
    while(std::getline(wfin,line)){
        int num;
        wchar_t unit;

        secs = 0;
        wsstr.clear();
        wsstr<<line;
        while(wsstr>>num>>unit){
            switch(unit){
                case L'y':
                    secs += num * 365 * 24 * 3600;
                    break;
                case L'd':
                    secs += num * 24 * 3600;
                    break;
                case L'h':
                    secs += num * 3600;
                    break;
                case L'm':
                    secs += num * 60;
                    break;
                case L's':
                    secs += num;
                    break;
                default:
                    timeinterval.assign(deftimevalues, deftimevalues 
                        + sizeof(deftimevalues)/sizeof(deftimevalues[0]));
                    return -2;
            }
        }

        timeinterval.push_back(secs);
    }

    wfin.close();
    return 0;
}

int option::CreateNewOptionFile(){
    // return 0 if succeed;
    // return -1 if can't create file.

    //std::wofstream wfout((getcurdir()+::OptionFileName).c_str(),std::ios::binary);
    boost::filesystem::wofstream wfout((getcurdir()+::OptionFileName).c_str(),std::ios::binary);
    if(wfout.fail())return -1;

    wfout.imbue(std::locale(""));
    wfout<<deftimestr;
    //wfout.write(deftimestr, sizeof(deftimestr)/sizeof(deftimestr[0]));

    wfout.close();
    return 0;
}