#include "learnlist.h"
#include "codecvt_utf.h"
#include <fstream>
#include <boost/filesystem/fstream.hpp>
#include "strop.h"

int learnlist::curword(std::wstring &wordstr){
    // return 0 if succeed. wordstr: current word to learn
    // return -1 if no word to learn.
    if(curline >= int(size()))return -1;
    wordstr = (*this)[curline];
    return 0;
}

int learnlist::nextword(){
    // return 0 if succeed.
    // return -1 if this list has finished
    if(++curline >= int(size()))return -1;
    return 0;
}

int progress::LoadLearnLists(){
    // return 0 if succeed
    // return -1 if can't open LearnListFile
    // return -2 if can't open some wordlist, others are loaded if available

    //std::wifstream wfin((getcurdir()+LearnListFileName).c_str(), std::ios::binary);
    boost::filesystem::wifstream wfin((getcurdir()+LearnListFileName).c_str(), std::ios::binary);
    if(wfin.fail())return -1;
    if(!tvt::IsStreamUnicode(wfin))wfin.imbue(std::locale(""));

    wfin>>CurListIdx;
    wfin.get();

    std::wstring filepath;
    std::vector<std::wstring> names;
    std::vector<int> lnums;
    while(std::getline(wfin, filepath))if(!removews(filepath).empty()){
        int linenum;
        wfin>>linenum;
        wfin.get();
        names.push_back(filepath);
        lnums.push_back(linenum);
        //push_back(learnlist(filepath,linenum));
    }

    wfin.close();

    int state = 0;
    for(int i=0;i<int(names.size());++i){
        learnlist llst(names[i],lnums[i]);
        if(llst.LoadWords())state = -2;
        else push_back(llst);
    }
    //for(std::vector<learnlist>::iterator itr = begin(); itr != end(); ++itr){
    //    if(itr->LoadWords())
    //        state = -2;
    //}

    return state;
}

int progress::SaveLearnLists()const{
    // return 0 if succeed
    // return -1 if can't open LearnListFile

    //std::wofstream wfout((getcurdir()+LearnListFileName).c_str(), std::ios::binary);
    boost::filesystem::wofstream wfout((getcurdir()+LearnListFileName).c_str(), std::ios::binary);
    if(wfout.fail())return -1;
    wfout.imbue(std::locale(std::locale("C"), new tvt::codecvt_utf8));
    wfout<<tvt::utf_bom;
    wfout<<CurListIdx<<std::endl;
    for(const_iterator itr = begin(); itr != end(); ++itr){
        wfout<<itr->filepath<<std::endl;
        wfout<<itr->curline<<std::endl;
    }
    return 0;
}

int progress::NextWord(){
    // return 0 if succeed
    // if the current list has finished then delete this list and return 1, and the current
    //      word point to the first unfinished word
    // if all lists are finished, then return -1

    if(empty())return -1;
    if(CurListIdx >= int(size()))CurListIdx = 0;

    if(-1 == (*this)[CurListIdx].nextword()){
        erase(begin() + CurListIdx);
        CurListIdx = 0;
        if(empty())return -1;
        return 1;
    }
    return 0;
}

int progress::CurrentWord(std::wstring &wordstr){
    // return 0 if succeed. wordstr: the current word to learn
    // return -1 if no word. wordstr: an empty wstring ...

    if(empty())return -1;
    if(CurListIdx >= int(size()))CurListIdx = 0;
    return (*this)[CurListIdx].curword(wordstr);
}

bool progress::IsInList(std::wstring Filepath){
    std::wstring curdir = getcurdir();
    for(std::wstring::iterator itr=Filepath.begin();itr!=Filepath.end();++itr)*itr=std::tolower(*itr,std::locale(""));
    for(std::wstring::iterator itr=curdir.begin();itr!=curdir.end();++itr)*itr=std::tolower(*itr,std::locale(""));
    //std::transform(Filepath.begin(),Filepath.end(),Filepath.begin(),tolower);
    //std::transform(curdir.begin(),curdir.end(),curdir.begin(),tolower);
    if(Filepath.find(curdir) == 0)Filepath.erase(0,curdir.length());
    for(iterator itr = begin(); itr != end(); ++itr){
        if(itr->filepath == Filepath)return true;
    }
    return false;
}

int progress::AddNewList(const std::wstring &Filepath){
    // return 0 if succeed
    // return -1 if can't load data

    learnlist nllist(Filepath,0);
    if(-1 == nllist.LoadWords())return -1;

    push_back(nllist);
    progress::CurListIdx = size() - 1;
    return 0;
}

void progress::DeleteList(int DeleteIdx){
    if(DeleteIdx == progress::CurListIdx){
        CurListIdx = 0;
    }
    erase(begin() + DeleteIdx);
}