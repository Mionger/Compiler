#ifndef ANALYZER_H
#define ANALYZER_H

#include<stack>
#include"tool.h"
#include"generator.h"
#include"qua.h"
#include"production.h"

class analyzer
{
private:
    vector<table> a_table;/* 符号表，每个符号表对应一个过程 */
    vector<int> a_sstack;/* 当前所处定义域的所有相关层次的符号表的序号栈 */

    ofstream a_tprinter;/* 输出临时文件 */
    fstream a_treader;/* 读取临时文件 */
    ofstream a_printer;/* 输出中间代码 */

    int a_next;/* 下一四元式序号 */
    int main_no;  /* 主函数的序号位置 */
    int a_bplevel;/* 回填层次 */
    vector<qua> a_qstack;/* 四元式栈，回填用 */
    vector<int> a_bpvalue;/* 回填的值 */
    vector<int> a_bppos;/* 回填点在回填栈中的地址 */

    vector<string> a_while;/* while,if语句写出的label */
    vector<string> a_jump;/* 跳转语句 */
    vector<int> a_wfcnt;   /* while语句数目 */
    int a_wnum;
    int a_wcnt;/* 生成跳转用的标签号 */
    int a_fcnt;/* 生成跳转用的标签号 */

    generator a_generator; /* 目标代码生成器 */

    void createTable(TABLE_TYPE t_type, const string &t_name); /* 创建新符号表 */

    void printQua(const qua &q);/* 根据四元式打印中间代码临时文件 */
    bool printQua();/* 根据临时文件打印中间代码 */

public:
    analyzer();
    ~analyzer();

    int peekNextQno();/* 查看下一个四元式序号 */
    int getNextQno();/* 获得下一个四元式序号 */

    string getArgName(symbolpos &pos, bool is_return = false); /* 获取变量名称 */

    bool startAnalyze(vector<symbolinfo> &g_infostack, const production &p);/* 执行语义分析 */
};

#endif