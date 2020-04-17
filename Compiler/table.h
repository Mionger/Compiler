#ifndef TABLE_H
#define TABLE_H

#include<vector>
#include"symbol.h"

enum TABLE_TYPE 
{ 
    CONST_TABLE,        //常量表
    TEMP_TABLE,         //临时变量表
    GLOBAL_TABLE,       //全局变量表
    FUNCTION_TABLE,     //函数表
    WHILE_TABLE,        //循环体
    IF_TABLE            //选择分支
};

class table
{
private:
    vector<symbol>  t_table;
    TABLE_TYPE      t_type;
    string          t_name;

public:
    table(TABLE_TYPE type, string name = "");
    ~table();
    
    vector<symbol> &getTable();/* 获取符号表 */
    TABLE_TYPE getType(); /* 获取表类型 */
    string getName();/* 获取表名称 */

    int findSymbol(const string &str) const;/* 符号表是否存在指定名称符号 */
    int findConst(const string &str) const;/* 符号表中查找是否存在指定的常数 */

    int addSymbol();/* 添加临时变量符号，返回序号 */
    int addSymbol(const string& str);/* 添加临时变量符号，返回序号 */
    int addSymbol(const symbol &s);/* 添加临时变量符号，返回序号 */

    void setSymbolValue(int pos, string &str);/* 设定指定符号的字面量 */

    symbol &getSymbol(int pos);/* 根据pos获取符号 */
    string getSymbolName(int pos) const;     /* 根据pos获取符号名称 */
    SYMBOL_TYPE getSymbolMode(int pos) const;/* 根据pos获取符号模式 */
    VARIABLE_TYPE getSymbolType(int pos) const;/* 根据pos获取符号类型 */
};

#endif