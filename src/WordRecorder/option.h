#ifndef _OPTION_H_
#define _OPTION_H_

#include<vector>
//#include<windows.h>

const wchar_t OptionFileName[] = L"config.ini";

struct option{
    std::vector<int> timeinterval;  // in Seconds, timeinterval[0] can be used

    // return 0 if succeed;
    // return -1 if can't open file.
    // return -2 if invalid data.
    // either case timeinterval load the default values.
    int LoadOptionsFromFile(void);

    // return 0 if succeed;
    // return -1 if can't create file.
    int CreateNewOptionFile(void);
};

#endif