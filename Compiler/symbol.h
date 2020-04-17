#ifndef SYMBOL_H
#define SYMBOL_H

#include<string>
using namespace std;

enum SYMBOL_TYPE 
{
    FUNCTION,       //函数
    VARIABLE,       //变量
    CONST,          //常量
    TEMP,           //临时变量
    RETURN_VAR      //返回变量
};

enum VARIABLE_TYPE 
{ 
    INT, 
    VOID 
};

struct symbol
{
    SYMBOL_TYPE     s_mode;     /* 符号类型 */
    string          s_name;     /* 符号名 */
    string          s_value;    /* 常量值 */

    VARIABLE_TYPE   s_type;     /* 变量类型 */

    int     s_pnum;             /* 函数参数数量 */
    int     s_address;          /* 函数入口地址 */
    int     s_pos;              /* 函数表在符号表中位置 */
    int     s_ftable;           /* 形参表在函数表中的序号 */

    int     s_offset = -1;      /* 偏移地址 */
    int     s_reg = -1;         /* 寄存器号 */
};

struct symbolpos
{
    int t_pos;
    int s_pos;
    symbolpos()
    {
        ;
    }
    symbolpos(int t, int s) 
    { 
        this->t_pos = t;
        this->s_pos = s;
    };
};

struct symbolinfo
{
    string s_name;/* 对应语法符号名称 */
    string s_value;/* 对应文本属性值 */
    symbolpos s_pos;/* 对应位置 */
    string op;/* 表示是加减乘除 */
    // symbolinfo()
    // {
    //     ;
    // }
    // symbolinfo(string name,string value,symbolpos pos,string o)
    // {
    //     this->s_name = name;
    //     this->s_value = value;
    //     this->s_pos = pos;
    //     this->op = o;
    // };
};

#endif