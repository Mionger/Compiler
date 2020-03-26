#ifndef PRODUCTION_H
#define PRODUCTION_H

#include<vector>
#include<string>
using namespace std;

struct production
{
    string          p_left;
    vector<string>  p_right;
};

#endif