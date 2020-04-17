#include"table.h"

table::table(TABLE_TYPE type, string name)
{
    this->t_type = type;
    this->t_name = name;
}

table::~table()
{

}

/* 获取符号表 */
vector<symbol> &table::getTable()
{
    return this->t_table;
}

/* 获取表类型 */
TABLE_TYPE table::getType()
{
    return this->t_type;
}

/* 获取表名称 */
string table::getName()
{
    return this->t_name;
}

/* 符号表是否存在指定名称符号 */
int table::findSymbol(const string &str) const
{
    for (vector<symbol>::const_iterator it = this->t_table.begin(); it != this->t_table.end();it++)
    {
        if(str==it->s_name)
            return it - this->t_table.begin();
    }
    return -1;
}

/* 符号表中查找是否存在指定的常数 */
int table::findConst(const string &str) const
{
    for (vector<symbol>::const_iterator it = this->t_table.begin(); it != this->t_table.end();it++)
    {
        if (CONST!=it->s_mode)
            continue;
        if (str == it->s_value)
            return it - this->t_table.begin();
    }
    return -1;
}

/* 添加临时变量符号，返回序号 */
int table::addSymbol()
{
    string name = "T" + to_string(this->t_table.size());
    symbol s;
    s.s_name = name;
    s.s_mode = TEMP;
    this->t_table.push_back(s);
    return this->t_table.size() - 1;
}

/* 添加临时变量符号，返回序号 */
int table::addSymbol(const string& str)
{
    string name = "T" + to_string(this->t_table.size());
    symbol s;
    s.s_name = name;
    s.s_mode = TEMP;
    s.s_value = str;
    this->t_table.push_back(s);
    return this->t_table.size() - 1;
}

/* 添加临时变量符号，返回序号 */
int table::addSymbol(const symbol &s)
{
    if (-1 != findSymbol(s.s_name))
        return -1;
    this->t_table.push_back(s);
    return this->t_table.size() - 1;
}

/* 设定指定符号的字面量 */
void table::setSymbolValue(int pos, string &str)
{
    this->t_table[pos].s_value = str;
    return;
}

/* 根据pos获取符号 */
symbol &table::getSymbol(int pos)
{
    return this->t_table[pos];
}

/* 根据pos获取符号名称 */
string table::getSymbolName(int pos) const
{
    return this->t_table[pos].s_name;
}

/* 根据pos获取符号类型 */
SYMBOL_TYPE table::getSymbolMode(int pos) const
{
    return this->t_table[pos].s_mode;
}

/* 根据pos获取符号类型 */
VARIABLE_TYPE table::getSymbolType(int pos) const
{
    return this->t_table[pos].s_type;
}