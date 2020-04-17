#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
using namespace std;

struct instruction
{
    string op;
    string arg1;
    string arg2;
    string arg3;
    instruction();
    instruction(const string &op, const string &arg1, const string &arg2, const string &arg3);
    void setInst(const string &op, const string & arg1, const string & arg2, const string & arg3);
};

#endif