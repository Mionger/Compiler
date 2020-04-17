#ifndef REGISTER_H
#define REGISTER_H

#include<string>
#include"symbol.h"
using namespace std;

struct reg
{
    string          r_name;
    string          r_content;
    symbolpos       r_info;
    int             r_miss;
    bool            r_possessed;
};

#endif