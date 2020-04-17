#include"instruction.h"

instruction::instruction()
{

}

instruction::instruction(const string &op, const string &arg1, const string &arg2, const string &arg3)
{
    this->op = op;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->arg3 = arg3;
}

void instruction::setInst(const string &op, const string &arg1, const string &arg2, const string &arg3)
{
    this->op = op;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->arg3 = arg3;
}