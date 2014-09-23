#include "reviewlist.h"
#include "codecvt_utf.h"
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <boost/filesystem/fstream.hpp>
#include "strop.h"

void reviewlist::insert_sort(std::vector<wordinfo> &wdvec, const wordinfo &wi, int widev){
    int curdev;

    wdvec.resize(wdvec.size()+1);
    std::vector<wordinfo>::iterator itr = wdvec.begin() + wdvec.size() - 1;
    while(itr != wdvec.begin()){
        CheckWord(*(itr-1), curdev);
        if(curdev <= widev)break;
        *itr = *(itr-1);
        --itr;
    }
    *itr = wi;
}

int wordinfo::timesincelast()const{
    return static_cast<int>(time(NULL) - tLast);
}

int reviewlist::LoadData(int nMode, std::wstring &FailFileName){
    // load data from SaveDataFile to fill the three wordinfo vectors.
    // return 0 if succeed
    // return -1 if can't open or read SaveDataFile
    // return -2 if can't open or read wordlist file, the file name is returned by FailFileName

    // nMode = 0 : load SaveDataFile from beginning ...
    // nMode = 1 : Retry last failed wordlist file ...
    // nMode = 2 : Ignore failed file and go on next ...
    // nMode = 3 : Abort reading data and close all files ...

    //static std::wifstream savefile;
    static boost::filesystem::wifstream savefile;
    static wordlist wdlst;

    bool reading = true;
    int linenum, nth;
    time_t tlast;

    switch(nMode){
        case 0:
            clear();
            savefile.open((getcurdir()+SaveDataFileName).c_str(), std::ios::binary);
            if(savefile.fail())return -1;
            if(!tvt::IsStreamUnicode(savefile))savefile.imbue(std::locale(""));

            if(!std::getline(savefile,wdlst.filepath)){
                savefile.close();
                return -1;
            }
            removews(wdlst.filepath);
            break;

        case 1:
            break;

        case 2:
            while(savefile>>linenum && linenum != -1)
                savefile>>nth>>tlast;
            if(std::getline(savefile,wdlst.filepath)){
                removews(wdlst.filepath);
                break;
            }
            // fall through

        case 3:
            reading = false;
            break;
    }

    for(;reading;){
        if(-1 == wdlst.LoadWords()){
            FailFileName = wdlst.filepath;
            return -2;
        }
        while(savefile>>linenum && linenum != -1){
            savefile>>nth>>tlast;
            if(linenum < 0 || linenum >= int(wdlst.size()))continue;
            insertword(wordinfo(wdlst[linenum],wlfilenames.size(),linenum,nth,tlast));
        }
        savefile.get();
        wlfilenames.push_back(wdlst.filepath);
        if(!std::getline(savefile,wdlst.filepath))break;
        removews(wdlst.filepath);
    }

    wdlst.clear();
    savefile.close();
    return 0;
}

int reviewlist::SaveData()const{
    // return 0 if succeed
    // return -1 if can't create SaveDataFile ...
    // return -2 if can't create backup file ...

    //std::wifstream savefile((getcurdir()+SaveDataFileName).c_str(), std::ios::binary);
    //boost::filesystem::wifstream savefile((getcurdir()+SaveDataFileName).c_str(), std::ios::binary);
    //if(!savefile.fail()){
    //    if(!tvt::IsStreamUnicode(savefile))savefile.imbue(std::locale(std::locale::classic(),"",std::locale::ctype));
    //    std::wofstream backfile((getcurdir()+std::wstring(SaveDataFileName) + std::wstring(L".bak")).c_str(),
    //            std::ios::binary);
    //    if(backfile.fail()){
    //        savefile.close();
    //        return -2;
    //    }
    //    backfile.imbue(std::locale(std::locale("C"), new tvt::codecvt_utf8));
    //    backfile<<tvt::utf_bom;
    //    std::wstring wstr;
    //    while(std::getline(savefile,wstr))
    //        backfile<<wstr;
    //    savefile.close();
    //    backfile.close();
    //}

    //std::wofstream save((getcurdir()+SaveDataFileName).c_str(), std::ios::binary);
    boost::filesystem::wofstream save((getcurdir()+SaveDataFileName).c_str(), std::ios::binary);
    if(save.fail())return -1;
    std::locale loc(std::locale("C"), new tvt::codecvt_utf8);
    //loc.combine< std::num_put<wchar_t> >(std::locale("C"));
    save.imbue(loc);
    save<<tvt::utf_bom;

    // ...
    for(int filenum = 0; filenum < (int)wlfilenames.size(); ++filenum){
        bool notfound = true;
        for(std::vector<wordinfo>::const_iterator itr = notontime.begin();
            notfound && itr != notontime.end(); ++itr)if(filenum == itr->fileno){
                notfound = false;
        }
        for(std::vector<wordinfo>::const_iterator itr = toreview.begin();
            notfound && itr != toreview.end(); ++itr)if(filenum == itr->fileno){
                notfound = false;
        }
        for(std::vector<wordinfo>::const_iterator itr = timexceed.begin();
            notfound && itr != timexceed.end(); ++itr)if(filenum == itr->fileno){
                notfound = false;
        }

        if(notfound)continue;
        save<<wlfilenames[filenum]<<std::endl;
        const int bufsize = 14;
        wchar_t buffer[bufsize];

        for(std::vector<wordinfo>::const_iterator itr = notontime.begin();
            itr != notontime.end(); ++itr)if(filenum == itr->fileno){
                _itow(itr->lineno,buffer,10);
                save<<buffer<<L' ';
                _itow(itr->nth,buffer,10);
                save<<buffer<<L' ';
                _i64tow(itr->tLast,buffer,10);
                save<<buffer<<L'\n';
        }
        for(std::vector<wordinfo>::const_iterator itr = toreview.begin();
            itr != toreview.end(); ++itr)if(filenum == itr->fileno){
                _itow(itr->lineno,buffer,10);
                save<<buffer<<L' ';
                _itow(itr->nth,buffer,10);
                save<<buffer<<L' ';
                _i64tow(itr->tLast,buffer,10);
                save<<buffer<<L'\n';
        }
        for(std::vector<wordinfo>::const_iterator itr = timexceed.begin();
            itr != timexceed.end(); ++itr)if(filenum == itr->fileno){
                _itow(itr->lineno,buffer,10);
                save<<buffer<<L' ';
                _itow(itr->nth,buffer,10);
                save<<buffer<<L' ';
                _i64tow(itr->tLast,buffer,10);
                save<<buffer<<L'\n';
        }
        save<<L"-1"<<L'\n';
    }
    save.close();
    return 0;
}

int reviewlist::CheckWord(const wordinfo &wi, int &dev)const{
    // return 0 if needn't reviewing any more
    // return 1 if not the time to review
    // return 2 if time to review
    // return 3 if review time has exceed
    // return -1 if invalid nth ...
    // dev = interval[nth] - CurInterval if return 1,2,3
    // smaller dev shoud be earlier to review

    if(wi.nth <0)return -1;
    if(wi.nth >= int(timeinterval.size()))return 0;

    int itv = wi.timesincelast();
    dev = timeinterval[wi.nth] - itv;
    if(itv < timeinterval[wi.nth])return 1;

    if(wi.nth + 1 >= int(timeinterval.size()))return 2;
    if(itv < timeinterval[wi.nth + 1])return 2;
    return 3;
}

void reviewlist::insertword(const wordinfo &wi){
    int widev;
    //std::vector<wordinfo>::iterator itr;

    switch(CheckWord(wi,widev)){
        case 1:
            insert_sort(notontime,wi,widev);
            break;
        case 2:
            insert_sort(toreview,wi,widev);
            break;
        case 3:
            insert_sort(timexceed,wi,widev);
            break;
    }
}

wordinfo reviewlist::nextword(int nMode){
    // return the next word to review
    // nMode = 1 if remember previous word
    // nMode = 2 if can't remember previous word
    // if no word to review, returned wordinfo.wordstr is empty

    static int last = 0;
    // last = 0 means no last word
    // last = 1 means last is toreview word
    // last = 2 means last is timexceed word

    wordinfo lastword;

    switch(last){
        case 1:
            lastword = toreview[0];
            toreview.erase(toreview.begin());
            break;
        case 2:
            lastword = timexceed[0];
            timexceed.erase(timexceed.begin());
            break;
    }

    for(;!toreview.empty();){
        int dev;
        if(2 == CheckWord(*toreview.begin(),dev))break;
        insert_sort(timexceed, *toreview.begin(), dev);
        toreview.erase(toreview.begin());
    }
    for(;!notontime.empty();){
        int rst, dev;
        rst = CheckWord(*notontime.begin(),dev);
        if(1 == rst)break;
        switch(rst){
            case 2:
                insert_sort(toreview, *notontime.begin(), dev);
                break;
            case 3:
                insert_sort(timexceed, *notontime.begin(), dev);
                break;
        }
        notontime.erase(notontime.begin());
    }

    // process lastword and insert it if need
    switch(last){
        case 1: // toreview
            switch(nMode){
                case 1: // remember
                    lastword.nth++;
                    break;
                case 2: // can't remember
                    //lastword.nth/=2;
                    lastword.nth = lastword.nth * 2 / 3;
                    break;
            }
            lastword.tLast = time(NULL)
                + int(double(2*rand() - RAND_MAX)/RAND_MAX * timeinterval[lastword.nth] * 0.2);
            break;
        case 2: // timexceed
            switch(nMode){
                case 1: // remember
                    //if(lastword.nth>0)lastword.nth--;
                    if(lastword.nth == 0) ++lastword.nth;
                    break;
                case 2: // can't remember
                    // lastword.nth = 0;
                    lastword.nth = lastword.nth / 3;
                    break;
            }
            lastword.tLast = time(NULL)
                + int(double(2*rand() - RAND_MAX)/RAND_MAX * timeinterval[lastword.nth] * 0.2);
            break;
    }
    if(last && lastword.nth < (int)timeinterval.size())
        insertword(lastword);

    // set last value and return wordinfo
    if(!toreview.empty()){
        last = 1;
        return toreview[0];
    }
    if(!timexceed.empty()){
        last = 2;
        return timexceed[0];
    }
    last = 0;
    lastword.wordstr.clear();
    return lastword;
}

void reviewlist::addword(const std::wstring &wordstr, const std::wstring &filepath, int linenum){
    wordinfo wi;

    wi.fileno = std::find(wlfilenames.begin(), wlfilenames.end(), filepath) - wlfilenames.begin();
    if(wlfilenames.size() == wi.fileno){
        wlfilenames.push_back(filepath);
    }

    wi.lineno = linenum;
    wi.nth = 0;
    wi.wordstr = wordstr;
    wi.tLast = time(NULL);

    insertword(wi);
}