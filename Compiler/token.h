#ifndef TOKEN_H
#define TOKEN_H

#include<string>
using namespace std;

enum TOKEN_TYPE
{
    L_KEYWORD,      //关键字
    L_BORDER,       //分界符
    L_TYPE,         //数据类型
    L_IDENTIFIER,   //标识符
    L_OPERATOR,     //操作符
    L_CINT,         //数字常数
    L_EOF,          //结束符
    L_UNKNOWN       //未知
};

struct token
{
    TOKEN_TYPE  t_type;
    string      t_value;
};

#endif