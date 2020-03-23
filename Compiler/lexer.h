#ifndef LEXER_H
#define LEXER_H


#include<iostream>
#include<fstream>
#include<set>
#include<string>
#include"token.h"
using namespace std;

class lexer
{
private:
    set<string> TYPES       = {"int", "void"};
    set<string> KEYWORDS    = {"if", "else","while","return"};
    set<string> OPERATORS   = { "+", "-", "*", "/", "=", "==", ">", ">=", "<", "<=", "!="};
    set<char>   BORDERS     = {',', ';', '(', ')', '{', '}'};

    ifstream scanner;
    ofstream printer;

    unsigned int    line_cnt;
    unsigned int    step_cnt;
    bool            print_flag;

    int isLetter(const unsigned char c);    /* 是否是字母 */
    int isDigit(const unsigned char c);     /* 是否是数字 */
    int isOperator(const unsigned char c);  /* 是否是运算符 */
    int isBlank(const unsigned char c);     /* 是否是空格 */
    unsigned char getNextChar();            /* 获取下一个字符 */
    void printToken(token t);               /* 打印符号信息 */
    token getTokenNP();                     /* 获取文法符号，不能打印 */

public:
    lexer();
    ~lexer();
    int openFiles(const string src_file);   /* 打开需要的文件 */
    token getTokenP();                      /* 获取文法符号，可以打印 */
    void startScanner();                    /* 开始词法分析 */
};

#endif