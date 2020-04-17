#ifndef GENERATOR_H
#define GENERATOR_H

#include<iostream>
#include<fstream>
#include"table.h"
#include"register.h"
#include"instruction.h"
using namespace std;

#define GLOBAL_INIT 1024

class generator
{
private:
    ofstream c_printer;

    vector<table> *c_table_ptr;
    vector<reg> c_register;

    int gf_offset;


public:
    generator();
    ~generator();

    void init();/* 初始化sp,fp,gp */
    void initTable(vector<table> &t); /* 初始化符号表 */

    void printInst(const instruction &i); /* 打印指令 */

    void pushStack(int num);/* 声明变量、调用函数时调节指针 */
    void createFtrame();/* 创建栈帧 */

    void setTime(int r_no);/* 设置未被使用时间 */
    int getReg();          /* 获取最久没被使用的寄存器 */
    bool isLoaded(symbolpos &pos) const;/* 判断符号是否已经被加载到寄存器 */
    string getAddr(const symbolpos &pos) const; /* 根据pos返回内存地址 */
    void loadReg(int r_no, symbolpos &pos); /* 把符号表中指定符号加载到指定寄存器 */
    int loadImm(const string &imm, const symbolpos &pos);/* 把立即数加载到临时寄存器中 */
    void endMain();/* main函数结束 */
    string setArg(symbolpos pos); /* 将操作数放入寄存器 */
    void clearReg();/* 清空寄存器，写回局部变量 */
    void resetReg();/* 复位寄存器的使用情况 */
};

#endif