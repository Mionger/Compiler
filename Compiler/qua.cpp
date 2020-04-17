#include"qua.h"

qua::qua(int no, const string &op, const string &a1, const string &a2, const string &r)
{
    this->no = no;
    this->op = op;
    this->arg1 = a1;
    this->arg2 = a2;
    this->result = r;
}

void qua::setQua(int no, const string &op, const string &a1, const string &a2, const string &r)
{
    this->no = no;
    this->op = op;
    this->arg1 = a1;
    this->arg2 = a2;
    this->result = r;
}