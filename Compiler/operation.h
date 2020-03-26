#ifndef OPERATION_H
#define OPERATION_H

enum OPERATION_TYPE
{
    CONCLUDE,    //规约
    MOVE,        //移进
    ACCEPT,      //接受
    NONTERMINAL, //压入非终结符
    ERROR        //错误
};

struct operation
{
	OPERATION_TYPE  o_operation;
	int             o_no;
	bool operator==(const operation &o) const;
};

#endif