#ifndef GRAMMAR_H
#define GRAMMAR_H

#include<vector>
#include<string>
using namespace std;

struct grammar
{
    string          g_left;
    vector<string>  g_right;
};

#endif