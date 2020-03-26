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
        if (!isNonTerminal(*it_1))
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
                    if(!isNonTerminal(*it) && "$"!=(*it))
                        this->p_symbol.insert(*it);
                break;
            }

            right.push_back(g_str.substr(0, r_pos));
            vector<string> p_split = split(g_str.substr(0, r_pos), " ");
            p_t.p_left = left;
            p_t.p_right = p_split;
            this->p_production.push_back(p_t);
            for (vector<string>::iterator it = p_split.begin(); it != p_split.end(); it++)
                if (!isNonTerminal(*it) && "$" != (*it))
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
        set<string> first;
        set<string> follow;
        this->p_first[it->p_left] = first;
        this->p_follow[it->p_left] = follow;
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
                if(!isNonTerminal(*it_2))
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
                if(!isNonTerminal(*it_2))
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
            return;
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

        if(cur.i_pos==this->p_production[cur.i_no].p_right.size())
            continue;

        string cur_symbol = this->p_production[cur.i_no].p_right[cur.i_pos];

        if (!isNonTerminal(cur_symbol))
            continue;

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
        
        string cur_symbol = this->p_production[cur.i_no].p_right[cur.i_pos];

        if (!isNonTerminal(cur_symbol))
            continue;

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

        int cur_state = find(this->p_normal.begin(), this->p_normal.end(), normal) - this->p_normal.begin();
        for (set<item>::iterator it_1 = normal.begin(); it_1 != normal.end(); it_1++)
        {
            if (-1 == it_1->i_pos || this->p_production[it_1->i_no].p_right.size() == it_1->i_pos)
            {
                string symbol = this->p_production[it_1->i_no].p_left;
                for (set<string>::iterator it_2 = this->p_follow[symbol].begin(); it_2 != this->p_follow[symbol].end(); it_2++)
                {
                    if (this->p_table.find(make_pair(cur_state, *it_2)) == this->p_table.end())
                    {
                        this->p_table[make_pair(cur_state, *it_2)] = {CONCLUDE, it_1->i_no};
                    }
                    else
                    {
                        if (!(this->p_table[make_pair(cur_state, *it_2)] == operation{CONCLUDE, it_1->i_no}))
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
                    if (-1 == it_2->i_pos || this->p_production[it_2->i_pos].p_right.size() == it_2->i_pos)
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
                    this->p_table[make_pair(cur_state, right_symbol)] = {MOVE, next_state};
                }
                else
                {
                    if (!(this->p_table[make_pair(cur_state, right_symbol)] == operation{MOVE, next_state}))
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
    this->p_table[{cur_st, "#"}] = {ACCEPT, cur_st};

    return true;
}

/* 根据文法文件，建立文法 */
bool parser::buildGrammar()
{
    genProduction();
    genGrammar();
    genFirst();
    genFollow();
    genSymbol();
    genItem();
    return genNormal();
}