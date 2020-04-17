#ifndef QUA_H
#define QUA_H

#include<string>
using namespace std;

struct qua
{
    int     no;
    string  op;
    string  arg1;
    string  arg2;
    string  result;

    qua(int n, const string &o, const string &a1, const string &a2, const string &r);
    void setQua(int n, const string &o, const string &a1, const string &a2, const string &r);
};

#endif