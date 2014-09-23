#ifndef _LEARNLIST_H_
#define _LEARNLIST_H_

#include"wordlist.h"

const wchar_t LearnListFileName[] = L"learnlist.dat";

class learnlist : public wordlist {
public:
    int curline;

    learnlist(const std::wstring &FilePath, int CurrentLine)
        : wordlist(FilePath), curline(CurrentLine){}

    // return 0 if succeed. wordstr: current word to learn
    // return -1 if no word to learn.
    int curword(std::wstring &wordstr);

    // return 0 if succeed.
    // return -1 if this list has finished
    int nextword(void);
};

class progress : public std::vector<learnlist> {
public:
    int CurListIdx;

    // return 0 if succeed
    // return -1 if can't open LearnListFile
    // return -2 if can't open some wordlist, others are loaded if available
    int LoadLearnLists();

    // return 0 if succeed
    // return -1 if can't open LearnListFile
    int SaveLearnLists()const;

    // return 0 if succeed
    // return 1 if the current list has finished, delete this list, and the current
    //      word point to the first unfinished word
    // return -1 if all lists are finished
    int NextWord(void);

    // return 0 if succeed. wordstr: the current word to learn
    // return -1 if no word. wordstr: an empty wstring ...
    int CurrentWord(std::wstring &wordstr);

    // check if the filepath has already in list
    bool IsInList(std::wstring Filepath);

    // return 0 if succeed
    // return -1 if can't load data
    int AddNewList(const std::wstring &Filepath);

    //Delete progress[DeleteIdx] list
    void DeleteList(int DeleteIdx);
};

#endif