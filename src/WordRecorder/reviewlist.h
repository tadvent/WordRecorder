#ifndef _REVIEWLIST_H_
#define _REVIEWLIST_H_

#include "wordlist.h"
#include <ctime>
#include <cstdlib>

const wchar_t SaveDataFileName[] = L"save.dat";

struct wordinfo {
    std::wstring wordstr;   // word string
    int fileno;             // file num of this word
    int lineno;             // line num in the file
    int nth;                // next is the nth time to review
    time_t tLast;           // the time of last review

    wordinfo():fileno(0),lineno(0),nth(0),tLast(0){}
    wordinfo(const std::wstring &WordStr, int FileNum, int LineNum, int Nth, time_t LastReview)
        :wordstr(WordStr), fileno(FileNum), lineno(LineNum), nth(Nth), tLast(LastReview){}
    int timesincelast()const;   // in secs
};

class reviewlist {  // keep and operate on all wordsinfo that are in review process
public:
    std::vector<int> timeinterval;      // load timeintervals from option obj
    std::vector<std::wstring> wlfilenames;  // keep the filenames of wordlist

    std::vector<wordinfo> notontime;    // three wordinfo vectors are for data process
    std::vector<wordinfo> toreview;
    std::vector<wordinfo> timexceed;

    reviewlist(){srand((unsigned)time(NULL));}

    void clear(void){
        wlfilenames.clear();
        toreview.clear();
        timexceed.clear();
        notontime.clear();
    }

    // load data from SaveDataFile to fill the three wordinfo vectors.
    // after that, allwords can remove the words and keep filename only...
    // return 0 if succeed
    // return -1 if can't open SaveDataFile
    // return -2 if can't open or read wordlist file, the file name is returned by FailFileName
    // nMode = 0 : load SaveDataFile from beginning ...
    // nMode = 1 : Retry last failed wordlist file ...
    // nMode = 2 : Ignore failed file and go on next ...
    // nMode = 3 : Abort reading data and close all files ...
    int LoadData(int nMode, std::wstring &FailFileName);

    // return 0 if succeed
    // return -1 if can't create SaveDataFile ...
    // return -2 if can't create backup file ...
    int SaveData(void)const;

    // return 0 if needn't reviewing any more
    // return 1 if not the time to review
    // return 2 if time to review
    // return 3 if review time has exceed
    // return -1 if invalid nth ...
    int CheckWord(const wordinfo &wi, int &dev)const;
    // dev = interval[nth] - CurInterval;

    // insert and insort the target vector
    void insertword(const wordinfo &wi);

    // return the next word to review
    // nMode = 1 if remember previous word
    // nMode = 2 if can't remember previous word
    // if no word to review, returned wordinfo.wordstr is empty
    wordinfo nextword(int nMode);

    void addword(const std::wstring &wordstr, const std::wstring &filepath, int linenum);

    void insert_sort(std::vector<wordinfo> &wdvec, const wordinfo &wi, int widev);
};

#endif