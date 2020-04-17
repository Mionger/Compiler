#include"parser.h"

parser::parser()
{
    this->print_flag = true;
}

parser::~parser()
{

}

/* 判断是否是非终结符 */
int parser::isNonTerminal(const string symbol)
{
    if (0 == symbol.length())
        return -1;

    if (symbol[0] >= 'A' && symbol[0] <= 'Z')
        return 1;
    return -1;
}

/* 获取产生式的first集 */
set<string> parser::getFirst(const vector<string> p)
{
    set<string> first;
    for (vector<string>::const_iterator it_1 = p.begin(); it_1 != p.end(); it_1++)
    {
        if (-1 == isNonTerminal(*it_1))
        {
            first.insert(*it_1);
            break;
        }

        for (set<string>::iterator it_2 = this->p_first[*it_1].begin(); it_2 != this->p_first[*it_1].end(); it_2++)
        {
            if ("$" != (*it_2) && first.find(*it_2) == first.end())
                first.insert(*it_2);
        }

        if (this->p_first[*it_1].find("$") == this->p_first[*it_1].end())
            break;

        if (it_1 + 1 == p.end() && first.find("$") == first.end())
            first.insert("$");
    }

    return first;
}

/* 读取grammer.txt中的文法，生成产生式 */
int parser::genProduction()
{
    string g_name = "grammar.txt";
    ifstream g_scanner;
    g_scanner.open(g_name);
    if(!g_scanner.is_open())
    {
        cout << "Fail to open grammer file" << endl;
        return -1;
    }

    string g_str;
    while(!g_scanner.eof())
    {
        
        getline(g_scanner, g_str);
        if (0 == g_str.length())
            continue;

        string left;
        vector<string> right;

        /* 产生式左端 */
        size_t l_pos = g_str.find_first_of("->");
        if (l_pos < 0)
        {
            cout << "Some grammer misses \"->\"" << endl;
            g_scanner.close();
            return -2;
        }
        left = g_str.substr(0, l_pos);
        
        /* 产生式右端 */
        g_str = g_str.substr(l_pos + 2);
        while(true)
        {
            production p_t;
            size_t r_pos = g_str.find_first_of("|");
            if (string::npos == r_pos)
            {
                right.push_back(g_str);
                vector<string> p_split = split(g_str, " ");
                p_t.p_left = left;
                p_t.p_right = p_split;
                this->p_production.push_back(p_t);
                for (vector<string>::iterator it = p_split.begin(); it != p_split.end(); it++)
                    if (-1 == isNonTerminal(*it) && "$" != (*it))
                        this->p_symbol.insert(*it);
                break;
            }

            right.push_back(g_str.substr(0, r_pos));
            vector<string> p_split = split(g_str.substr(0, r_pos), " ");
            p_t.p_left = left;
            p_t.p_right = p_split;
            this->p_production.push_back(p_t);
            for (vector<string>::iterator it = p_split.begin(); it != p_split.end(); it++)
                if (-1 == isNonTerminal(*it) && "$" != (*it))
                    this->p_symbol.insert(*it);

            g_str = g_str.substr(r_pos + 1);
        }

        grammar g_t;
        g_t.g_left = left;
        g_t.g_right = right;
        this->p_grammar.push_back(g_t);
    }

    g_scanner.close();
    return 0;
}

/* 根据产生式，生成拓广文法 */
void parser::genGrammar()
{
    string left = "Start";
    vector<string> right;
    right.push_back(this->p_production[0].p_left);
    production p_t;
    p_t.p_left = left;
    p_t.p_right = right;
    this->p_production.insert(this->p_production.begin(), p_t);
    return;
}

/* 根据产生式，生成first集 */
void parser::genFirst()
{
    for (vector<production>::iterator it = this->p_production.begin(); this->p_production.end() != it; it++)
    {
        if(this->p_first.find(it->p_left)==this->p_first.end())
        {
            set<string> first;
            set<string> follow;
            this->p_first[it->p_left] = first;
            this->p_follow[it->p_left] = follow;
        }      
        if ("$" == it->p_right[0])
        {
            if(this->p_first[it->p_left].find("$") == this->p_first[it->p_left].end())
                this->p_first[it->p_left].insert("$");
        }
    }

    while(true)
    {
        bool change_flag = false;

        for (vector<production>::iterator it_1 = this->p_production.begin(); this->p_production.end() != it_1; it_1++)
        {
            for (vector<string>::iterator it_2 = it_1->p_right.begin(); it_1->p_right.end() != it_2; it_2++)
            {
                /* 空 */
                if ("$" == (*it_2))
                    break;

                /* 终结符 */
                if (-1 == isNonTerminal(*it_2))
                {
                    if(this->p_first[it_1->p_left].find(*it_2)==this->p_first[it_1->p_left].end())
                    {
                        this->p_first[it_1->p_left].insert(*it_2);
                        change_flag = true;
                    }
                    break;
                }

                /* 产生式右部符号对应的first */
                for (set<string>::iterator it_3 = this->p_first[*it_2].begin(); this->p_first[*it_2].end() != it_3; it_3++)
                {
                    if("$"!=(*it_3) && this->p_first[it_1->p_left].find(*it_3)==this->p_first[it_1->p_left].end())
                    {
                        this->p_first[it_1->p_left].insert(*it_3);
                        change_flag = true;
                    }
                }

                if (this->p_first[*it_2].find("$") == this->p_first[*it_2].end())
                    break;

                if (it_2 + 1 == it_1->p_right.end() && this->p_first[it_1->p_left].find("$") == this->p_first[it_1->p_left].end())
                {
                    this->p_first[it_1->p_left].insert("$");
                    change_flag = true;
                }
            }
        }

        if (!change_flag)
            break;
    }

    return;
}

/* 根据产生式，生成follow集 */
void parser::genFollow()
{
    this->p_follow[this->p_production[0].p_left].insert("#");

    while(true)
    {
        bool change_flag = false;

        for (vector<production>::iterator it_1 = this->p_production.begin(); it_1 != this->p_production.end(); it_1++)
        {
            for (vector<string>::iterator it_2 = it_1->p_right.begin(); it_2 != it_1->p_right.end(); it_2++)
            {
                if(-1 == isNonTerminal(*it_2))
                    continue;

                set<string> first = getFirst(vector<string>(it_2 + 1, it_1->p_right.end()));
                for (set<string>::iterator it = first.begin(); it != first.end(); it++)
                {
                    if ("$" != (*it) && this->p_follow[*it_2].find(*it) == this->p_follow[*it_2].end())
                    {
                        this->p_follow[*it_2].insert(*it);
                        change_flag = true;
                    }
                }

                if (first.empty() || first.find("$") != first.end())
                {
                    for (set<string>::iterator it = this->p_follow[it_1->p_left].begin(); it != this->p_follow[it_1->p_left].end(); it++)
                    {
                        if (this->p_follow[*it_2].find(*it) == this->p_follow[*it_2].end())
                        {
                            this->p_follow[*it_2].insert(*it);
                            change_flag = true;
                        }
                    }
                }
            }
        }

        if (!change_flag)
            break;
    }

    return;
}

/* 根据first，生成文法符号集 */
void parser::genSymbol()
{
    for (map<string, set<string> >::iterator it = this->p_first.begin(); it != this->p_first.end(); it++)
    {
        this->p_symbol.insert(it->first);
    }
    this->p_symbol.insert("#");
    return;
}

/* 根据产生式，产生所有项目 */
void parser::genItem()
{
    for (vector<production>::iterator it = this->p_production.begin(); it != this->p_production.end(); it++)
    {
        if ("$" == it->p_right[0])
        {
            this->p_item.insert(item(it - p_production.begin(), -1));
            continue;
        }
        int len = it->p_right.size();
        for (int i = 0; i <= len; i++)
        {
            this->p_item.insert(item(it - p_production.begin(), i));
        }
    }

    return;
}

/* 生成某个项目的闭包 */
set<item> parser::genClosure(const item i)
{
    stack<item> i_stack;
    i_stack.push(i);

    set<item> i_set;
    while (!i_stack.empty())
    {
        item cur = i_stack.top();
        i_stack.pop();

        i_set.insert(cur);

        if (-1 == cur.i_pos)
            continue;

        if (cur.i_pos == this->p_production[cur.i_no].p_right.size())
            continue;

        if (-1 == isNonTerminal(this->p_production[cur.i_no].p_right[cur.i_pos]))
            continue;

        string cur_symbol = this->p_production[cur.i_no].p_right[cur.i_pos];
        for (set<item>::iterator it = this->p_item.begin(); it != this->p_item.end(); it++)
        {
            if (this->p_production[it->i_no].p_left == cur_symbol && it->i_pos <= 0)
            {
                if (i_set.find(*it) == i_set.end())
                    i_stack.push(*it);
            }
        }
    }

    return i_set;
}

/* 生成某个闭包的闭包 */
set<item> parser::genClosure(const set<item> c)
{
    stack<item> i_stack;
    for (set<item>::iterator it = c.begin(); it != c.end(); it++)
    {
        i_stack.push(*it);
    }

    set<item> i_set;
    while (!i_stack.empty())
    {
        item cur = i_stack.top();
        i_stack.pop();

        i_set.insert(cur);

        if (-1 == cur.i_pos)
            continue;

        if(cur.i_pos==this->p_production[cur.i_no].p_right.size())
            continue;

        if (-1 == isNonTerminal(this->p_production[cur.i_no].p_right[cur.i_pos]))
            continue;

        string cur_symbol = this->p_production[cur.i_no].p_right[cur.i_pos];
        for (set<item>::iterator it = this->p_item.begin(); it != this->p_item.end(); it++)
        {
            if (this->p_production[it->i_no].p_left == cur_symbol && it->i_pos <= 0)
            {
                if (i_set.find(*it) == i_set.end())
                    i_stack.push(*it);
            }
        }
    }

    return i_set;
}

/* 根据产生式，生成项目规范族集 */
bool parser::genNormal()
{
    stack<set<item> > n_stack;
    n_stack.push(genClosure(*(this->p_item.begin())));
    this->p_normal.push_back(n_stack.top());

    while (!n_stack.empty())
    {
        set<item> normal = n_stack.top();
        n_stack.pop();

        /* 当前规范族序号 */
        int cur_state = find(this->p_normal.begin(), this->p_normal.end(), normal) - this->p_normal.begin();
        for (set<item>::iterator it_1 = normal.begin(); it_1 != normal.end(); it_1++)
        {
            /* 需要规约项目 */
            if (-1 == it_1->i_pos || this->p_production[it_1->i_no].p_right.size() == it_1->i_pos)
            {
                string symbol = this->p_production[it_1->i_no].p_left;
                for (set<string>::iterator it_2 = this->p_follow[symbol].begin(); it_2 != this->p_follow[symbol].end(); it_2++)
                {
                    if (this->p_table.find(make_pair(cur_state, *it_2)) == this->p_table.end())
                    {
                        operation op_tmp;
                        op_tmp.o_operation = CONCLUDE;
                        op_tmp.o_no = it_1->i_no;
                        this->p_table[make_pair(cur_state, *it_2)] = op_tmp;
                    }
                    else
                    {
                        operation op_tmp;
                        op_tmp.o_operation = CONCLUDE;
                        op_tmp.o_no = it_1->i_no;
                        if (!(this->p_table[make_pair(cur_state, *it_2)] == op_tmp))
                        {
                            cout << "Is not SLR grammar" << endl;
                            return false;
                        }
                    }     
                }
            }
            else
            {
                string right_symbol = this->p_production[it_1->i_no].p_right[it_1->i_pos];
                set<item> items;
                for (set<item>::iterator it_2 = normal.begin(); it_2 != normal.end(); it_2++)
                {
                    if (-1 == it_2->i_pos || this->p_production[it_2->i_no].p_right.size() == it_2->i_pos)
                        continue;
                    if(this->p_production[it_1->i_no].p_right[it_1->i_pos]==this->p_production[it_2->i_no].p_right[it_2->i_pos])
                        items.insert(item(it_2->i_no, it_2->i_pos + 1));
                }

                set<item> next_normal = genClosure(items);

                if(find(this->p_normal.begin(),this->p_normal.end(),next_normal)==this->p_normal.end())
                {
                    this->p_normal.push_back(next_normal);
                    n_stack.push(next_normal);
                }
                int next_state = find(this->p_normal.begin(), this->p_normal.end(), next_normal) - this->p_normal.begin();

                if (this->p_table.find(make_pair(cur_state, right_symbol)) == this->p_table.end())
                {
                    operation op_tmp;
                    op_tmp.o_operation = MOVE;
                    op_tmp.o_no = next_state;
                    this->p_table[make_pair(cur_state, right_symbol)] = op_tmp;
                }
                else
                {
                    operation op_tmp;
                    op_tmp.o_operation = MOVE;
                    op_tmp.o_no = next_state;
                    if (!(this->p_table[make_pair(cur_state, right_symbol)] == op_tmp))
                    {
                        cout << "Is not SLR grammar" << endl;
                        return false;
                    }
                }
            }
        }
    }

    int cur_st = -1;
    for (vector<set<item> >::iterator it_1 = this->p_normal.begin(); it_1 != p_normal.end(); it_1++)
    {
        for (set<item>::iterator it_2 = it_1->begin(); it_2 != it_1->end(); it_2++)
        {
            item temp(it_2->i_no, it_2->i_pos);
            if (item(0, 1) == temp)
            {
                cur_st = it_1 - this->p_normal.begin();
                break;
            }
        }

        if (cur_st >= 0)
            break;
    }

    // set<item> i;
    // i.insert(item(0, 1));
    operation op_tmp;
    op_tmp.o_operation = ACCEPT;
    op_tmp.o_no = cur_st;
    this->p_table[pair<int, string>(cur_st, "#")] = op_tmp;

    return true;
}

/* 打印产生式 */
bool parser::printProduction()
{
    ofstream printer_production;
    printer_production.open("result_production.txt");

    if(!printer_production.is_open())
    {
        cout << "Fail to open production output file" << endl;
        return false;
    }

    for (auto it_1 = this->p_production.begin(); it_1 != this->p_production.end();it_1++)
    {
        /* 产生式左部 */
        printer_production << (*it_1).p_left << "->";
        /* 产生式右部 */
        for (auto it_2 = (it_1->p_right).begin(); it_2 != (it_1->p_right).end();it_2++)
        {
            printer_production << (*it_2) << ' ';
        }
        printer_production << endl;
    }

    printer_production.close();
    return true;
}

/* 打印拓广文法 */
bool parser::printGrammar()
{
    ofstream printer_grammar;
    printer_grammar.open("result_grammar.txt");

    if(!printer_grammar.is_open())
    {
        cout << "Fail to open grammar output file" << endl;
        return false;
    }

    for (auto it_1 = this->p_production.begin(); it_1 != this->p_production.end();it_1++)
    {
        /* 产生式左部 */
        printer_grammar << (*it_1).p_left << "->";
        /* 产生式右部 */
        for (auto it_2 = (it_1->p_right).begin(); it_2 != (it_1->p_right).end();it_2++)
        {
            printer_grammar << (*it_2) << ' ';
        }
        printer_grammar << endl;
    }

    printer_grammar.close();
    return true;
}

/* 打印first集合 */
bool parser::printFirst()
{
    ofstream printer_first;
    printer_first.open("result_first.txt");

    if(!printer_first.is_open())
    {
        cout << "Fail to open first output file" << endl;
        return false;
    }

    for (auto it_1 = this->p_first.begin(); it_1 != this->p_first.end();it_1++)
    {
        /* 文法符号 */
        printer_first << (*it_1).first << ":";
        /* first集合 */
        for (auto it_2 = (it_1->second).begin(); it_2 != (it_1->second).end(); it_2++)
        {
            printer_first << (*it_2) << ' ';
        }
        printer_first << endl;
    }

    printer_first.close();
    return true;
}

/* 打印follow集合 */
bool parser::printFollow()
{
    ofstream printer_follow;
    printer_follow.open("result_follow.txt");

    if(!printer_follow.is_open())
    {
        cout << "Fail to open follow output file" << endl;
        return false;
    }

    for (auto it_1 = this->p_follow.begin(); it_1 != this->p_follow.end();it_1++)
    {
        printer_follow << (*it_1).first << ":";
        for(auto it_2=it_1->second.begin();it_2!=it_1->second.end();it_2++)
        {
            printer_follow << (*it_2) << ' ';
        }
        printer_follow << endl;
    }

    printer_follow.close();
    return true;
}

/* 打印文法符号 */
bool parser::printSymbol()
{
    ofstream printer_symbol;
    printer_symbol.open("result_symbol.txt");

    if(!printer_symbol.is_open())
    {
        cout << "Fail to open symbol output file" << endl;
        return false;
    }

    for (auto it = this->p_symbol.begin(); it != this->p_symbol.end();it++)
    {
        printer_symbol << *it << endl;
    }

    printer_symbol.close();
    return true;
}

/* 打印项目 */
bool parser::printItem()
{
    ofstream printer_item;
    printer_item.open("result_item.txt");

    if(!printer_item.is_open())
    {
        cout << "Fail to open item output file" << endl;
        return false;
    }

    for (auto it_1 = this->p_item.begin(); it_1 != this->p_item.end();it_1++)
    {
        printer_item << this->p_production[it_1->i_no].p_left << "->";
        if (-1 == it_1->i_pos)
        {
            printer_item << "." << endl;
            continue;
        }

        int len = this->p_production[it_1->i_no].p_right.size();
        for (auto it_2 = this->p_production[it_1->i_no].p_right.begin(); it_2 != this->p_production[it_1->i_no].p_right.end();it_2++)
        {
            if (it_1->i_pos == (it_2 - this->p_production[it_1->i_no].p_right.begin()))
            {
                printer_item << ".";
            }
            printer_item << *it_2 << ' ';

            if (it_1->i_pos == len && this->p_production[it_1->i_no].p_right.end() - it_2 == 1)
            {
                printer_item << ".";
            }
        }

        printer_item << endl;
    }

    printer_item.close();
    return true;
}

/* 打印项目规范族 */
bool parser::printFamily()
{
    ofstream printer_family;
    printer_family.open("result_family.txt");

    if(!printer_family.is_open())
    {
        cout << "Fail to open family output file" << endl;
        return false;
    }

    for (auto it_1 = this->p_normal.begin(); it_1 != this->p_normal.end(); it_1++)
    {
        printer_family << "规范族" << it_1 - this->p_normal.begin() << ":" << endl;
        for (auto it_2 = it_1->begin(); it_2 != it_1->end();it_2++)
        {
            printer_family << this->p_production[it_2->i_no].p_left << "->";

            if (-1 == it_2->i_pos)
            {
                printer_family << "." << endl;
                continue;
            }

            int len = this->p_production[it_2->i_no].p_right.size();
            for (auto it_3 = this->p_production[it_2->i_no].p_right.begin(); it_3 != this->p_production[it_2->i_no].p_right.end();it_3++)
            {
                if (it_2->i_pos == (it_3 - this->p_production[it_2->i_no].p_right.begin()))
                {
                    printer_family << ".";
                }
                printer_family << *it_3 << ' ';

                if (it_2->i_pos == len && (this->p_production[it_2->i_no].p_right.end() - it_3 == 1))
                {
                    printer_family << ".";
                }
            }
            printer_family << endl;
        }
        printer_family << endl;
    }

    return true;
}

/* 打印移进规约表 */
bool parser::printGoto()
{
    ofstream printer_goto;
    printer_goto.open("result_goto.csv");

    if(!printer_goto.is_open())
    {
        cout << "Fail to open goto table output file" << endl;
        return false;
    }

    printer_goto << "  ";
    for (auto it = this->p_symbol.begin(); it != this->p_symbol.end(); it++)
    {
        if (-1 != isNonTerminal(*it))
        {
            continue;
        }

        if ("," == *it)
            printer_goto << "," << ".";
        else
            printer_goto << "," << *it;
    }

    for (auto it = this->p_symbol.begin(); it != this->p_symbol.end(); it++)
    {
        if (-1 == isNonTerminal(*it))
            continue;
        printer_goto << "," << *it;
    }
    printer_goto << endl;

    for (unsigned int st = 0; st < this->p_normal.size();st++)
    {
        printer_goto << "state" << st;
        for (auto it = this->p_symbol.begin(); it != this->p_symbol.end();it++)
        {
            if (-1 != isNonTerminal(*it))
                continue;

            if (this->p_table.find(make_pair(st, *it)) == this->p_table.end())
                printer_goto << ",error";
            else
            {
                int next_st = this->p_table[make_pair(st, *it)].o_no;
                int op = this->p_table[make_pair(st, *it)].o_operation;
                if (MOVE == op)
                {
                    printer_goto << ",s" << next_st;
                }
                else if (CONCLUDE == op)
                {
                    printer_goto << ",r" << next_st;
                }
                else if (ACCEPT == op)
                {
                    printer_goto << ",acc";
                }
                else
                {
                    printer_goto << ",???";
                }     
            }
        }
        for (auto it = this->p_symbol.begin(); it != this->p_symbol.end(); it++)
        {
            if (-1 == isNonTerminal(*it))
                continue;
            
            if (this->p_table.find(make_pair(st, *it)) == this->p_table.end())
                printer_goto << ",error";
            else
            {
                int next_st = this->p_table[make_pair(st, *it)].o_no;
                int op = this->p_table[make_pair(st, *it)].o_operation;
                if (MOVE == op)
                {
                    printer_goto << ",s" << next_st;
                }
                else if (CONCLUDE == op)
                {
                    printer_goto << ",r" << next_st;
                }
                else if (ACCEPT == op)
                {
                    printer_goto << ",acc";
                }
                else
                {
                    printer_goto << ",???";
                }     
            }
        }

        printer_goto << endl;
    }

    return true;
}

/* 根据文法文件，建立文法 */
bool parser::buildGrammar()
{
    genProduction();
    if(!printProduction())
    {
        return false;
    }
    genGrammar();
    if (!printGrammar())
    {
        return false;
    }
    genFirst();
    if (!printFirst())
    {
        return false;
    }
    genFollow();
    if (!printFollow())
    {
        return false;
    }
    genSymbol();
    if (!printSymbol())
    {
        return false;
    }
    genItem();
    if (!printItem())
    {
        return false;
    }

    if (!genNormal())
    {
        return false;
    }
    if(!printFamily())
    {
        return false;
    }
    if(!printGoto())
    {
        return false;
    }
    return true;
}

/* 开始语法分析（同步进行语义分析和中间代码生成） */
bool parser::startParser(const string src_file)
{
    /* 初始化符号栈和状态栈 */
    this->p_state_stack.push(0);
    this->p_symbol_stack.push("#");

    // if (0 != this->p_lexer.openFiles(src_file))
    // {
    //     return false;
    // }

    /* 初始化文法符号属性（语义分析栈） */
    symbolinfo c_info;
    c_info.s_name = "Program";
    this->p_ginfo_stack.push_back(c_info);

    int step = 0;
    while(true)
    {
        token t = this->p_lexer.getTokenP();
        string t_str = t.t_name;
        if (L_UNKNOWN == t.t_type) //EOF?
        {
            return false;
        }

        while (true)
        {
            int cur_state = this->p_state_stack.top();
            /* 错误 */
            if (this->p_table.find(make_pair(cur_state, t_str)) == this->p_table.end())
            {
                cout << "Syntactic error" << endl;
                return false;
            }
            /* 移进 */
            else if (MOVE == this->p_table[make_pair(cur_state, t_str)].o_operation)
            {
                this->p_state_stack.push(this->p_table[make_pair(cur_state, t_str)].o_no);
                this->p_symbol_stack.push(t_str);
                step++;
                symbolinfo c_info;
                c_info.s_name = t.t_name;
                c_info.s_value = t.t_value;
                this->p_ginfo_stack.push_back(c_info);
                break;
            }
            /* 规约 */
            else if (CONCLUDE == this->p_table[make_pair(cur_state, t_str)].o_operation)
            {
                int p_no = this->p_table[make_pair(cur_state, t_str)].o_no;
                
                /* 计算产生式右边长度 */
                int p_len;
                if ("$" == this->p_production[p_no].p_right[0])
                {
                    p_len = 0;
                }
                else
                {
                    p_len = this->p_production[p_no].p_right.size();
                }

                for (int i = 0; i < p_len;i++)
                {
                    this->p_state_stack.pop();
                    this->p_symbol_stack.pop();
                }

                this->p_symbol_stack.push(this->p_production[p_no].p_left);
                if (this->p_table.find(make_pair(this->p_state_stack.top(), this->p_production[p_no].p_left)) == this->p_table.end())
                {
                    cout << "Current state has no goto" << endl;
                    return false;
                }

                this->p_state_stack.push(this->p_table[make_pair(this->p_state_stack.top(), this->p_production[p_no].p_left)].o_no);
                if (!this->p_analyzer.startAnalyze(this->p_ginfo_stack, this->p_production[p_no]))
                {
                    return false;
                }
            }
            /* 接受 */
            else if (ACCEPT == this->p_table[make_pair(cur_state, t_str)].o_operation)
            {
                step++;
                cout << "Syntactic analysis done" << endl;
                return true;
            }
            /* 程序错误 */
            else
            {
                cout << "Something wrong with parser" << endl;
                return false;
            }

            step++;
        }
    }

    return true;
}