#ifndef PARSER_H
#define PARSER_H

#include<map>
#include<stack>
#include"tool.h"
#include"lexer.h"
#include"grammar.h"
#include"production.h"
#include"item.h"
#include"operation.h"
#include"analyzer.h"

class parser
{
private:
    bool print_flag;

    vector<grammar>                     p_grammar;
    vector<production>                  p_production;
    set<string>                         p_symbol;
    map<string, set<string> >           p_first;
    map<string, set<string> >           p_follow;
    set<item>                           p_item;
    vector<set<item> >                  p_normal;
    map<pair<int, string>,operation>    p_table;
    stack<int>                          p_state_stack;
    stack<string>                       p_symbol_stack;
    vector<symbolinfo>                  p_ginfo_stack;
    
    int isNonTerminal(const string symbol);/* 判断是否是非终结符 */
    set<string> getFirst(const vector<string> production);/* 获取产生式的first集 */

    int genProduction(); /* 读取grammer.txt中的文法，生成产生式 */
    void genGrammar(); /* 根据产生式，生成拓广文法 */
    void genFirst();/* 根据产生式，生成first集 */
    void genFollow();/* 根据产生式，生成follow集 */
    void genSymbol();/* 根据first，生成文法符号集 */
    void genItem();/* 根据产生式，产生所有项目 */
    set<item> genClosure(const item i);/* 生成某个项目的闭包 */
    set<item> genClosure(const set<item> c);/* 生成某个闭包的闭包 */
    bool genNormal();/* 根据产生式，生成项目规范族集 */

    bool printProduction();/* 打印产生式 */
    bool printGrammar();/* 打印拓广文法 */
    bool printFirst();/* 打印first集合 */
    bool printFollow();/* 打印follow集合 */
    bool printSymbol();/* 打印文法符号 */
    bool printItem();/* 打印项目 */
    bool printFamily();/* 打印项目规范族 */
    bool printGoto();/* 打印移进规约表 */

public:
    parser();
    ~parser();

    analyzer                            p_analyzer;
    lexer                               p_lexer;

    bool buildGrammar();/* 根据文法文件，建立文法 */
    bool startParser(const string src_file);/* 开始语法分析（同步进行语义分析和中间代码生成） */
};

#endif