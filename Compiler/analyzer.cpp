#include"analyzer.h"

analyzer::analyzer()
{
    /* 全局符号表 */
    this->a_table.push_back(table(GLOBAL_TABLE, "global_table"));
    this->a_sstack.push_back(0);

    /* 临时变量表 */
    this->a_table.push_back(table(TEMP_TABLE, "temp_table"));

    this->a_tprinter.open("result_temp.txt");
    if(!this->a_tprinter.is_open())
    {
        cout << "Fail to open temp file" << endl;
    }

    this->a_next = 1;
    this->a_bplevel = 0;
    this->main_no = -1;

    this->a_wcnt = 0;
    this->a_fcnt = 0;
    this->a_wnum = 0;

    this->a_generator.initTable(this->a_table);
}

analyzer::~analyzer()
{
    if(this->a_tprinter.is_open())
        this->a_tprinter.close();
    
    if(this->a_treader.is_open())
        this->a_treader.close();

    if(this->a_printer.is_open())
        this->a_printer.close();

    remove("result_temp.txt");
}

/* 创建新符号表 */
void analyzer::createTable(TABLE_TYPE t_type, const string &t_name)
{
    this->a_table.push_back(table(t_type, t_name));
    this->a_sstack.push_back(this->a_table.size() - 1);
    return;
}

/* 根据四元式打印中间代码临时文件 */
void analyzer::printQua(const qua &q)
{
    /* 不用回填 */
    if (0 == this->a_bplevel)
    {
        this->a_tprinter << q.no << " (" << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")" << endl;
    }
    /* 回填 */
    else
    {
        this->a_qstack.push_back(q);
    }
    return;
}

/* 根据临时文件打印中间代码 */
bool analyzer::printQua()
{
    this->a_printer.open("result_inter.txt");
    if (!this->a_printer.is_open())
    {
        cout << "Fail to open inter code file" << endl;
        return false;
    }

    if(this->a_tprinter.is_open())
        this->a_tprinter.close();

    this->a_treader.open("result_temp.txt");
    if(!this->a_treader.is_open())
    {
        cout << "Fail to open temp file" << endl;
        return false;
    }

    /* 跳转到main函数 */
    this->a_printer << "0 (j, -, -, " << this->main_no << ")" << endl;
    while(!this->a_treader.eof())
    {
        string str;
        getline(this->a_treader, str);
        if ("" == str)
            continue;
        this->a_printer << str << endl;
    }

    if (this->a_treader.is_open())
        this->a_treader.close();

    return true;
}

/* 查看下一个四元式序号 */
int analyzer::peekNextQno()
{
    return a_next;
}

/* 获得下一个四元式序号 */
int analyzer::getNextQno()
{
    return a_next++;
}

/* 获取变量名称 */
string analyzer::getArgName(symbolpos &pos, bool is_return)
{
    string name = this->a_table[pos.t_pos].getSymbolName(pos.s_pos);

    if (false == is_return)
    {
        if (VARIABLE == this->a_table[pos.t_pos].getSymbolMode(pos.s_pos))
        {
            name = name + "-" + this->a_table[pos.t_pos].getName() + "Var";
        }
    }
    else
    {
        int p = this->a_table[0].findSymbol(this->a_table[pos.t_pos].getName());
        symbol &s = this->a_table[0].getSymbol(p);
        name = name + "-" + s.s_name + "_paramerter" + to_string(pos.s_pos);
    }

    return name;
}

/* 执行语义分析 */
bool analyzer::startAnalyze(vector<symbolinfo> &g_infostack, const production &p)
{
    /* Const_value->num */
    if("Const_value"==p.p_left)
    {
        symbolinfo s_info = g_infostack.back();

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = s_info.s_value;

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();

        g_infostack.push_back(c_info);
    }
    /* Factor->Const_value */ 
    else if ("Factor" == p.p_left && "Const_value" == p.p_right[0])
    {
        symbolinfo s_info = g_infostack.back();
        int s_pos = this->a_table[1].addSymbol(s_info.s_value);
        string s_name = this->a_table[1].getSymbolName(s_pos);

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = s_name;
        c_info.s_pos = symbolpos(1, s_pos);

        printQua(qua(getNextQno(), ":=", s_info.s_value, "-", s_name));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);

        /* 目标代码生成 */
        this->a_generator.loadImm(s_info.s_value, c_info.s_pos);
    }
    /* Factor->( Expression ) */
    else if("Factor" == p.p_left && "(" == p.p_right[0])
    {
        symbolinfo s_info = g_infostack[g_infostack.size() - 2];
        
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = this->a_table[s_info.s_pos.t_pos].getSymbolName(s_info.s_pos.s_pos);
        c_info.s_pos = s_info.s_pos;

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Factor->identifier FTYPE */
    else if("Factor" == p.p_left && "identifier" == p.p_right[0])
    {
        symbolinfo &s_id = g_infostack[g_infostack.size() - 2];
        symbolinfo s_ftype = g_infostack.back();

        int id_pos = -1;
        int id_layer = this->a_sstack.size() - 1;
        for (; id_layer >= 0; id_layer--)
        {
            table s_table = this->a_table[this->a_sstack[id_layer]];
            id_pos = s_table.findSymbol(s_id.s_value);
            if (-1 != id_pos)
                break;
        }

        if (-1 == id_pos)
        {
            cout << "Sematic analyze error : \'" << s_id.s_value << "\' has not been defined" << endl;
            return false;
        }

        s_id.s_pos.t_pos = this->a_sstack[id_layer];
        s_id.s_pos.s_pos = id_pos;

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        /* FTYPE->$ */
        if ("" == s_ftype.s_value)
        {
            if (TEMP == this->a_table[s_id.s_pos.t_pos].getSymbolMode(s_id.s_pos.s_pos))
            {
                c_info.s_pos = symbolpos(s_id.s_pos.t_pos, s_id.s_pos.s_pos);
                c_info.s_value = s_id.s_value;
            }
            else
            {
                int s_pos = this->a_table[1].addSymbol(s_id.s_value);
                string s_name = this->a_table[1].getSymbolName(s_pos);

                printQua(qua(getNextQno(), ":=", getArgName(s_id.s_pos), "-", s_name));

                c_info.s_pos = symbolpos(1, s_pos);

                symbolpos tmp_pos = symbolpos(s_id.s_pos.t_pos, s_id.s_pos.s_pos);
                string a1_name = this->a_generator.setArg(c_info.s_pos);
                string a2_name = this->a_generator.setArg(tmp_pos);

                this->a_generator.printInst(instruction("move", a1_name, a2_name, ""));
            }
        }
        /* FTYPE->Call_func */
        else
        {
            int t_pos = 0;
            int s_pos = 0;
            t_pos = this->a_table[s_ftype.s_pos.t_pos].getSymbol(s_ftype.s_pos.s_pos).s_ftable;
            c_info.s_value = "";

            int s_pos_t = this->a_table[1].addSymbol(s_id.s_value);
            string s_name = this->a_table[1].getSymbolName(s_pos_t);

            int ft_pos_t = this->a_table[0].getSymbol(s_id.s_pos.s_pos).s_ftable;
            symbolpos r_s_pos = symbolpos(ft_pos_t, 0);
            printQua(qua(getNextQno(), ":=", getArgName(r_s_pos), "-", s_name));
            c_info.s_pos = symbolpos(1, s_pos_t);

            /* 跳转 */
            this->a_generator.clearReg();
            string f_name = this->a_table[0].getSymbolName(id_pos);
            this->a_generator.printInst(instruction("jal", f_name, "", ""));

            /* 清理栈帧 */
            this->a_generator.printInst(instruction("move", "$sp", "$fp", ""));
            this->a_generator.printInst(instruction("lw", "$fp", "($fp)", ""));
            int pnum = this->a_table[0].getSymbol(s_id.s_pos.s_pos).s_pnum;
            int sp_back = -1 - pnum;
            this->a_generator.pushStack(sp_back);

            /* 返回值 */
            string a_name = this->a_generator.setArg(c_info.s_pos);
            this->a_generator.printInst(instruction("move", a_name, "$v0", ""));
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Factor_loop->$ */
    else if ("Factor_loop" == p.p_left && "$" == p.p_right[0])
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";
        g_infostack.push_back(c_info);
    }
    /* Factor_loop->Factor_loop Factor *|Factor_loop Factor / */
    else if ("Factor_loop" == p.p_left && "Factor_loop" == p.p_right[0])
    {
        symbolinfo s_factor = g_infostack[g_infostack.size() - 2];
        symbolinfo s_factor_loop = g_infostack[g_infostack.size() - 3];

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        if ("" == s_factor_loop.s_value)
        {
            c_info.s_value = g_infostack.back().s_name;
            if (TEMP == this->a_table[s_factor.s_pos.t_pos].getSymbolMode(s_factor.s_pos.s_pos))
            {
                c_info.s_pos = s_factor.s_pos;
            }
            else
            {
                int s_pos = this->a_table[1].addSymbol(s_factor.s_value);
                string s_name = this->a_table[1].getSymbolName(s_pos);

                printQua(qua(getNextQno(), ":=", getArgName(s_factor.s_pos), "-", s_name));
                c_info.s_pos = symbolpos(1, s_pos);

                string a1_name = this->a_generator.setArg(c_info.s_pos);
                string a2_name = this->a_generator.setArg(s_factor.s_pos);
                this->a_generator.printInst(instruction("move", a1_name, a2_name, ""));
            }
        }
        else
        {
            string a1_name = getArgName(s_factor_loop.s_pos);
            string a2_name = getArgName(s_factor.s_pos);

            string t_a1_name = this->a_generator.setArg(s_factor_loop.s_pos);
            string t_a2_name = this->a_generator.setArg(s_factor.s_pos);

            c_info.s_value = s_factor_loop.s_value;
            c_info.s_pos = s_factor_loop.s_pos;

            if ("*" == s_factor_loop.s_value)
            {
                printQua(qua(getNextQno(), "*", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("mult", t_a1_name, t_a2_name, ""));
                this->a_generator.printInst(instruction("mflo", t_a1_name, "", ""));
            }
            else
            {
                printQua(qua(getNextQno(), "/", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("div", t_a1_name, t_a2_name, ""));
                this->a_generator.printInst(instruction("mflo", t_a1_name, "", ""));
            }
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Item->Factor_loop Factor */
    else if ("Item" == p.p_left)
    {
        symbolinfo s_factor = g_infostack.back();
        symbolinfo s_factor_loop = g_infostack[g_infostack.size() - 2];
        
        symbolinfo c_info;
        c_info.s_name = p.p_left;

        if ("" == s_factor_loop.s_value)
        {
            c_info.s_value = "";
            c_info.s_pos = s_factor.s_pos;
        }
        else
        {
            string a1_name = getArgName(s_factor_loop.s_pos);
            string a2_name = getArgName(s_factor.s_pos);

            string t_a1_name = this->a_generator.setArg(s_factor_loop.s_pos);
            string t_a2_name = this->a_generator.setArg(s_factor.s_pos);

            if ("*" == s_factor.s_value)
            {
                printQua(qua(getNextQno(), "*", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("mult", t_a1_name, t_a2_name, ""));
                this->a_generator.printInst(instruction("mflo", t_a1_name, "", ""));
            }
            else
            {
                printQua(qua(getNextQno(), "/", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("div", t_a1_name, t_a2_name, ""));
                this->a_generator.printInst(instruction("mflo", t_a1_name, "", ""));
            }

            c_info.s_value = s_factor_loop.s_value;
            c_info.s_pos = s_factor_loop.s_pos;
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Item_loop->$ */
    else if ("Item_loop" == p.p_left && "$" == p.p_right[0])
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";
        g_infostack.push_back(c_info);
    }
    /* Item_loop->Item_loop Item +|Item_loop Item - */
    else if ("Item_loop" == p.p_left && "Item_loop" == p.p_right[0])
    {
        symbolinfo s_item = g_infostack[g_infostack.size() - 2];
        symbolinfo s_item_loop = g_infostack[g_infostack.size() - 3];

        symbolinfo c_info;
        c_info.s_name = p.p_left;

        if ("" == s_item_loop.s_value)
        {
            c_info.s_value = g_infostack.back().s_name;
            c_info.s_pos = s_item.s_pos;
        }
        else
        {
            string a1_name = getArgName(s_item_loop.s_pos);
            string a2_name = getArgName(s_item.s_pos);

            string t_a1_name = this->a_generator.setArg(s_item_loop.s_pos);
            string t_a2_name = this->a_generator.setArg(s_item.s_pos);

            if ("+" == s_item_loop.s_value)
            {
                printQua(qua(getNextQno(), "+", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("add", t_a1_name, t_a1_name, t_a2_name));
            }
            else
            {
                printQua(qua(getNextQno(), "-", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("sub", t_a1_name, t_a1_name, t_a2_name));
            }

            c_info.s_value = g_infostack.back().s_name;
            c_info.s_pos = s_item_loop.s_pos;
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Add_expression->Item_loop Item */
    else if ("Add_expression" == p.p_left)
    {
        symbolinfo s_item = g_infostack.back();
        symbolinfo s_item_loop = g_infostack[g_infostack.size() - 2];

        symbolinfo c_info;
        c_info.s_name = p.p_left;

        if ("" == s_item_loop.s_value)
        {
            c_info.s_value = "";
            c_info.s_pos = s_item.s_pos;
        }
        else
        {
            string a1_name = getArgName(s_item_loop.s_pos);
            string a2_name = getArgName(s_item.s_pos);

            string t_a1_name = this->a_generator.setArg(s_item_loop.s_pos);
            string t_a2_name = this->a_generator.setArg(s_item.s_pos);

            if ("+" == s_item_loop.s_value)
            {
                printQua(qua(getNextQno(), "+", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("add", t_a1_name, t_a1_name, t_a2_name));
            }
            else
            {
                printQua(qua(getNextQno(), "-", a1_name, a2_name, a1_name));
                this->a_generator.printInst(instruction("sub", t_a1_name, t_a1_name, t_a2_name));
            }

            c_info.s_value = s_item_loop.s_value;
            c_info.s_pos = s_item_loop.s_pos;
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Add_expression_loop->$ */
    else if ("Add_expression_loop" == p.p_left && "$" == p.p_right[0])
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";
        g_infostack.push_back(c_info);
    }
    /* Add_expression_loop->Add_expression_loop Add_expression Relop */
    else if ("Add_expression_loop" == p.p_left && "Add_expression_loop" == p.p_right[0])
    {
        symbolinfo s_add_expression = g_infostack[g_infostack.size() - 2];
        symbolinfo s_add_expression_loop = g_infostack[g_infostack.size() - 3];

        symbolinfo c_info;
        c_info.s_name = p.p_left;

        if ("" == s_add_expression_loop.s_value)
        {
            c_info.s_value = g_infostack.back().s_value;
            c_info.s_pos = s_add_expression.s_pos;
        }
        else
        {
            string a1_name = getArgName(s_add_expression_loop.s_pos);
            string a2_name = getArgName(s_add_expression.s_pos);
            string op_name = "j" + s_add_expression_loop.s_value;

            int n_qno = getNextQno();
            printQua(qua(n_qno, op_name, a1_name, a2_name, to_string(n_qno + 3)));
            printQua(qua(getNextQno(), ":=", "0", "-", a1_name));
            printQua(qua(getNextQno(), "j", "-", "-", to_string(n_qno + 4)));
            printQua(qua(getNextQno(), ":=", "1", "-", a1_name));

            string t_a1_name = this->a_generator.setArg(s_add_expression_loop.s_pos);
            string t_a2_name = this->a_generator.setArg(s_add_expression.s_pos);
            if ("<" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("slt", t_a1_name, t_a1_name, t_a2_name));
            }
            else if ("<=" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("sle", t_a1_name, t_a1_name, t_a2_name));
            }
            else if (">" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("sgt", t_a1_name, t_a1_name, t_a2_name));
            }
            else if (">=" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("sge", t_a1_name, t_a1_name, t_a2_name));
            }
            else if ("==" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("seq", t_a1_name, t_a1_name, t_a2_name));
            }
            else if ("!=" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("snq", t_a1_name, t_a1_name, t_a2_name));
            }
            else
            {
                cout << "Unexcepted add expression loop" << endl;
                return false;
            }

            c_info.s_value = s_add_expression_loop.s_value;
            c_info.s_pos = s_add_expression_loop.s_pos;
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Relop-><|<=|>|>=|==|!= */
    else if ("Relop" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = g_infostack.back().s_value;

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Expression->Add_expression_loop Add_expression */
    else if ("Expression" == p.p_left)
    {
        symbolinfo s_add_expression = g_infostack.back();
        symbolinfo s_add_expression_loop = g_infostack[g_infostack.size() - 2];

        symbolinfo c_info;
        c_info.s_name = p.p_left;

        if ("" == s_add_expression_loop.s_value)
        {
            c_info.s_value = s_add_expression.s_value;
            c_info.s_pos = s_add_expression.s_pos;
        }
        else
        {
            string a1_name = getArgName(s_add_expression_loop.s_pos);
            string a2_name = getArgName(s_add_expression.s_pos);
            string op_name = "j" + s_add_expression_loop.s_value;

            int n_qno = getNextQno();
            printQua(qua(n_qno, op_name, a1_name, a2_name, to_string(n_qno + 3)));
            printQua(qua(getNextQno(), ":=", "0", "-", a1_name));
            printQua(qua(getNextQno(), "j", "-", "-", to_string(n_qno + 4)));
            printQua(qua(getNextQno(), ":=", "1", "-", a1_name));

            string t_a1_name = this->a_generator.setArg(s_add_expression_loop.s_pos);
            string t_a2_name = this->a_generator.setArg(s_add_expression.s_pos);
            if ("<" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("slt", t_a1_name, t_a1_name, t_a2_name));
            }
            else if ("<=" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("sle", t_a1_name, t_a1_name, t_a2_name));
            }
            else if (">" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("sgt", t_a1_name, t_a1_name, t_a2_name));
            }
            else if (">=" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("sge", t_a1_name, t_a1_name, t_a2_name));
            }
            else if ("==" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("seq", t_a1_name, t_a1_name, t_a2_name));
            }
            else if ("!=" == s_add_expression_loop.s_value)
            {
                this->a_generator.printInst(instruction("snq", t_a1_name, t_a1_name, t_a2_name));
            }
            else
            {
                cout << "Unexcepted add expression loop" << endl;
                return false;
            }

            c_info.s_value = s_add_expression_loop.s_value;
            c_info.s_pos = s_add_expression_loop.s_pos;
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Internal_variable_stmt->int identifier */
    else if ("Internal_variable_stmt" == p.p_left)
    {
        symbolinfo s_id = g_infostack.back();
        table &cur_table = this->a_table[this->a_sstack.back()];

        if (-1 != cur_table.findSymbol(s_id.s_value))
        {
            cout << "Sematic analyze error : \'" << s_id.s_value << "\' has been defined" << endl;
            return false;
        }

        int s_pos_tmp = this->a_table[0].findSymbol(s_id.s_value);
        if (-1 != s_pos_tmp && FUNCTION == this->a_table[0].getSymbolMode(s_pos_tmp))
        {
            cout << "Sematic analyze error : \'" << s_id.s_value << "\' has been defined as function" << endl;
            return false;
        }

        symbol v_symbol;
        v_symbol.s_mode = VARIABLE;
        v_symbol.s_name = s_id.s_value;
        v_symbol.s_type = INT;
        v_symbol.s_value = "";

        int s_pos = cur_table.addSymbol(v_symbol);

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = s_id.s_value;
        c_info.s_pos = symbolpos(this->a_sstack.back(), s_pos);
        g_infostack.push_back(c_info);

        this->a_generator.pushStack(1);
    }
    /* Assign_sentence->identifier = Expression ; */
    else if ("Assign_sentence" == p.p_left)
    {
        symbolinfo s_expression = g_infostack[g_infostack.size() - 2];
        symbolinfo s_id = g_infostack[g_infostack.size() - 4];

        int id_pos = -1;
        int id_layer = this->a_sstack.size() - 1;
        for (; id_layer >= 0; id_layer--)
        {
            table table_temp = this->a_table[this->a_sstack[id_layer]];
            id_pos = table_temp.findSymbol(s_id.s_value);
            if (-1 != id_pos)
                break;
        }

        if (-1 == id_pos)
        {
            cout << "Sematic analyze error : \'" << s_id.s_value << "\' has not been defined" << endl;
            return false;
        }

        symbolpos pos;
        pos.t_pos = this->a_sstack[id_layer];
        pos.s_pos = id_pos;
        
        string r_name = getArgName(pos);
        string a_name = getArgName(s_expression.s_pos);
        printQua(qua(getNextQno(), ":=", a_name, "-", r_name));

        string t_a1_name = this->a_generator.setArg(pos);
        string t_a2_name = this->a_generator.setArg(s_expression.s_pos);
        this->a_generator.printInst(instruction("move", t_a1_name, t_a2_name, ""));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";
        g_infostack.push_back(c_info);
    }
    /* Create_Function_table->$ */
    else if ("Create_Function_table" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";

        symbolinfo s_id = g_infostack.back();

        if (-1 != this->a_table[0].findSymbol(s_id.s_value))
        {
            cout << "Sematic analyze error : \'" << s_id.s_value << "\' has been defined" << endl;
            return false;
        }

        table n_table(FUNCTION_TABLE, s_id.s_value);
        this->a_table.push_back(n_table);

        symbol n_func;
        n_func.s_mode = FUNCTION;
        n_func.s_name = s_id.s_value;
        n_func.s_type = VOID;
        n_func.s_ftable = this->a_table.size() - 1;
        this->a_table[0].addSymbol(n_func);

        this->a_sstack.push_back(this->a_table.size() - 1);

        symbolinfo s_returntype = g_infostack[g_infostack.size() - 2];

        symbol n_variable;
        n_variable.s_mode = RETURN_VAR;
        n_variable.s_name = this->a_table[this->a_sstack.back()].getName() + "-return_value";
        if("int"==s_returntype.s_value)
        {
            n_variable.s_type = INT;
        }
        else if("void"==s_returntype.s_value)
        {
            n_variable.s_type = VOID;
        }
        else
        {
            cout << "Invaild return type" << endl;
            return false;
        }
        n_variable.s_value = "";

        if ("main" == s_id.s_value)
        {
            this->main_no = peekNextQno();
        }
        printQua(qua(getNextQno(), s_id.s_value, "-", "-", "-"));

        this->a_table[this->a_sstack.back()].addSymbol(n_variable);
        g_infostack.push_back(c_info);

        this->a_generator.printInst(instruction(s_id.s_value + ":", "", "", ""));
        this->a_generator.createFtrame();

        this->a_wfcnt.push_back(this->a_wnum);
        this->a_wnum = 0;
    }
    /* Exit_Function_table->$ */
    else if("Exit_Function_table" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";

        symbolinfo s_id = g_infostack[g_infostack.size() - 6];
        this->a_sstack.pop_back();

        g_infostack.push_back(c_info);

        this->a_wnum = this->a_wfcnt.back();
        this->a_wfcnt.pop_back();

        this->a_generator.clearReg();

        if("main"!=s_id.s_value)
        {
            this->a_generator.printInst(instruction("jr", "$ra", "", ""));
        }
        else
        {
            this->a_generator.printInst(instruction("syscall", "", "", ""));
        }
    }
    /* Variavle_stmt->; */
    else if("Variavle_stmt" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = ";";

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Stmt_type->Variavle_stmt */
    else if ("Stmt_type" == p.p_left && "Variavle_stmt" == p.p_right[0])
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = ";";

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Stmt->int identifier Stmt_type */
    else if ("Stmt" == p.p_left && "int" == p.p_right[0])
    {
        symbolinfo s_id = g_infostack[g_infostack.size() - 2];
        symbolinfo s_st = g_infostack.back();

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        if (";" == s_st.s_value)
        {
            int id_pos = -1;
            int id_layer = this->a_sstack.size() - 1;
            for (; id_layer >= 0; id_layer--)
            {
                table tmp_table = this->a_table[this->a_sstack[id_layer]];
                id_pos = tmp_table.findSymbol(s_id.s_value);
                if (id_pos != -1)
                    break;
            }

            if (-1 != id_pos)
            {
                cout << "Sematic analyze error : \'" << s_id.s_value << "\' has been defined" << endl;
                return false;
            }

            symbol n_variable;
            n_variable.s_mode = VARIABLE;
            n_variable.s_name = s_id.s_value;
            n_variable.s_type = INT;
            n_variable.s_value = "";

            int pos = this->a_table[this->a_sstack.back()].addSymbol(n_variable);
            this->a_generator.pushStack(1);

            c_info.s_value = s_id.s_value;
        }

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Return_expression->$ */
    else if ("Return_expression" == p.p_left && "$" == p.p_right[0])
    {
        if (VOID != this->a_table[this->a_sstack.back()].getSymbolType(0))
        {
            cout << "Unexpected return type" << endl;
            return false;
        }

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";
        g_infostack.push_back(c_info);
    }
    /* Return_expression->Expression */
    else if ("Return_expression" == p.p_left && "Expression" == p.p_right[0])
    {
        if (INT != this->a_table[this->a_sstack.back()].getSymbolType(0))
        {
            cout << "Unexpected return type" << endl;
            return false;
        }

        symbolinfo s_expression = g_infostack.back();
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "int";
        c_info.s_pos = s_expression.s_pos;

        string arg_name = this->a_generator.setArg(s_expression.s_pos);
        this->a_generator.printInst(instruction("move", "$v0", arg_name, ""));
        this->a_generator.printInst(instruction("lw", "$ra", "-4($fp)", ""));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Return_sentence->return Return_expression ; */
    else if("Return_sentence" == p.p_left)
    {
        symbolinfo s_return = g_infostack[g_infostack.size() - 2];

        symbolinfo c_info;
        c_info.s_name = p.p_left;

        table &cur_table = this->a_table[this->a_sstack.back()];
        int func_pos = this->a_table[0].findSymbol(cur_table.getName());
        symbol &cur_func = this->a_table[0].getSymbol(func_pos);

        if ("" != s_return.s_value)
        {
            symbolpos r_pos;
            r_pos.t_pos = this->a_sstack.back();
            r_pos.s_pos = 0;
            string r_name = getArgName(r_pos);
            string arg_name = getArgName(s_return.s_pos);

            printQua(qua(getNextQno(), ":=", arg_name, "-", r_name));

            c_info.s_value = "int";
            c_info.s_pos = s_return.s_pos;
        }
        else
        {
            c_info.s_value = "";
        }

        printQua(qua(getNextQno(), "reutrn", "-", "-", cur_table.getName()));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Parameter->int identifier */
    else if("Parameter" == p.p_left)
    {
        symbolinfo s_id = g_infostack.back();
        table &cur_table = this->a_table[this->a_sstack.back()];
        int t_pos = this->a_table[0].findSymbol(cur_table.getName());

        if (-1 != cur_table.findSymbol(s_id.s_value))
        {
            cout << "Sematic analyze error : a function has had the same parameter" << endl;
            return false;
        }

        int tmp_s_pos = this->a_table[0].findSymbol(s_id.s_value);
        if (-1 != tmp_s_pos && FUNCTION == this->a_table[0].getSymbolMode(tmp_s_pos))
        {
            cout << "Sematic analyze error : \'" << s_id.s_value << "\' has been defined as a function" << endl;
            return false;
        }

        symbol n_variable;
        n_variable.s_mode = VARIABLE;
        n_variable.s_name = s_id.s_value;
        n_variable.s_type = INT;
        n_variable.s_value = "";

        int s_pos = cur_table.addSymbol(n_variable);
        this->a_table[0].getSymbol(t_pos).s_pnum++;

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = s_id.s_value;
        c_info.s_pos = symbolpos(this->a_sstack.back(), s_pos);
        g_infostack.push_back(c_info);
    }
    /* Sentence_block->Sentence_block_m { Internal_stmt Sentence_string } */
    else if("Sentence_block" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = to_string(peekNextQno());

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* While_sentence_m2->$ */
    else if("While_sentence_m2" == p.p_left)
    {
        symbolinfo s_expression = g_infostack[g_infostack.size() - 2];
        string s_name = this->a_table[s_expression.s_pos.t_pos].getSymbolName(s_expression.s_pos.s_pos);

        printQua(qua(getNextQno(), "j=", s_name, "0", "---j="));
        this->a_bppos.push_back(this->a_qstack.size() - 1);

        printQua(qua(getNextQno(), "j=", "-", "-", "---j"));
        this->a_bppos.push_back(this->a_qstack.size() - 1);

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = to_string(peekNextQno());

        string t_arg_name = this->a_generator.setArg(s_expression.s_pos);
        string j_label_end = this->a_jump.back();
        this->a_jump.pop_back();
        this->a_generator.printInst(instruction("beq", t_arg_name, "$zero", j_label_end));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* While_sentence_m1->$ */
    else if("While_sentence_m1" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = to_string(peekNextQno());

        this->a_bplevel++;

        string t_name = this->a_table[this->a_sstack.back()].getName();
        int cnt = this->a_wnum++;
        string label_1 = "Label_" + t_name + "_while_begin_" + to_string(cnt);
        string label_2 = "Label_" + t_name + "_while_end_" + to_string(cnt);

        this->a_while.push_back(label_2);
        this->a_while.push_back(label_1);
        this->a_jump.push_back(label_1);
        this->a_jump.push_back(label_2);

        string w_label_begin = this->a_while.back();
        this->a_while.pop_back();

        this->a_generator.clearReg();
        this->a_generator.printInst(instruction(w_label_begin + ":", "", "", ""));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* While_sentence->while While_sentence_m1 ( Expression ) While_sentence_m2 Sentence_block */
    else if("While_sentence" == p.p_left)
    {
        symbolinfo s_while_m1 = g_infostack[g_infostack.size() - 6];
        symbolinfo s_while_m2 = g_infostack[g_infostack.size() - 2];
        symbolinfo s_block = g_infostack.back();
        printQua(qua(getNextQno(), "j", "-", "-", s_while_m1.s_value));

        /* 回填 */
        int batch_pos;
        /* true */
        batch_pos = this->a_bppos.back();
        this->a_bppos.pop_back();
        this->a_qstack[batch_pos].setQua(this->a_qstack[batch_pos].no, this->a_qstack[batch_pos].op, this->a_qstack[batch_pos].arg1, this->a_qstack[batch_pos].arg2, s_while_m2.s_value);
        /* false */
        batch_pos = this->a_bppos.back();
        this->a_bppos.pop_back();
        this->a_qstack[batch_pos].setQua(this->a_qstack[batch_pos].no, this->a_qstack[batch_pos].op, this->a_qstack[batch_pos].arg1, this->a_qstack[batch_pos].arg2, to_string(peekNextQno()));

        this->a_bplevel--;

        if (0 == this->a_bplevel)
        {
            for (auto it = this->a_qstack.begin(); it != this->a_qstack.end();it++)
            {
                printQua(*it);
            }
            int len = this->a_qstack.size();
            for (int i = 0; i < len; i++)
            {
                this->a_qstack.pop_back();
            }
        }

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";

        this->a_generator.clearReg();
        string j_label_begin = this->a_jump.back();
        this->a_jump.pop_back();
        this->a_generator.printInst(instruction("j", j_label_begin, "", ""));
        string w_label_end = this->a_while.back();
        this->a_while.pop_back();
        this->a_generator.printInst(instruction(w_label_end, ":", "", ""));
        this->a_generator.clearReg();
        // this->a_generator.printInst(instruction("", "", "", ""));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* If_sentence_m0->$ */
    else if("If_sentence_m0" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = to_string(peekNextQno());

        this->a_bplevel++;

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* If_sentence_m1->$ */
    else if("If_sentence_m1" == p.p_left)
    {
        symbolinfo s_expression = g_infostack[g_infostack.size() - 2];
        string s_name = this->a_table[s_expression.s_pos.t_pos].getSymbolName(s_expression.s_pos.s_pos);

        printQua(qua(getNextQno(), "j=", s_name, "0", "---j="));
        this->a_bppos.push_back(this->a_qstack.size() - 1);

        printQua(qua(getNextQno(), "j", "-", "-", "---j"));
        this->a_bppos.push_back(this->a_qstack.size() - 1);

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = to_string(peekNextQno());

        string t_name = this->a_table[this->a_sstack.back()].getName();
        int cnt = this->a_wnum++;
        string label_1 = "Label_" + t_name + "_if_end_" + to_string(cnt);
        string label_2 = "Label_" + t_name + "_else_end_" + to_string(cnt);
        this->a_while.push_back(label_2);
        this->a_while.push_back(label_1);
        this->a_jump.push_back(label_2);
        this->a_jump.push_back(label_1);
        string j_label_if_end = this->a_jump.back();
        this->a_jump.pop_back();
        string t_arg_name = this->a_generator.setArg(s_expression.s_pos);
        this->a_generator.clearReg();
        this->a_generator.printInst(instruction("beq", t_arg_name, "$zero", j_label_if_end));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* If_sentence_n->$ */
    else if("If_sentence_n" == p.p_left)
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;

        printQua(qua(getNextQno(), "j", "-", "-", "---j-if-n"));
        this->a_bppos.push_back(this->a_qstack.size() - 1);

        c_info.s_value = to_string(peekNextQno());

        string w_label_if_end = this->a_while.back();
        this->a_while.pop_back();
        string j_label_else_end = this->a_jump.back();
        this->a_jump.pop_back();

        this->a_generator.clearReg();
        this->a_generator.printInst(instruction("j", j_label_else_end, "", ""));
        this->a_generator.printInst(instruction(w_label_if_end, ":", "", ""));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* If_expression->$ */
    else if ("If_expression" == p.p_left && "$" == p.p_right[0])
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";

        string w_label_if_end = this->a_while.back();
        this->a_while.pop_back();
        this->a_while.pop_back();
        this->a_jump.pop_back();

        this->a_generator.clearReg();
        this->a_generator.printInst(instruction(w_label_if_end, ":", "", ""));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* If_expression->If_sentence_n else Sentence_block */
    else if ("If_expression" == p.p_left && "If_sentence_n" == p.p_right[0])
    {
        symbolinfo s_if_n = g_infostack[g_infostack.size() - 3];
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = s_if_n.s_value;

        string w_label_else_end = this->a_while.back();
        this->a_while.end();

        this->a_generator.clearReg();
        this->a_generator.printInst(instruction(w_label_else_end, "：", "", ""));

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* If_sentence->if If_sentence_m0 ( Expression ) If_sentence_m1 Sentence_block If_expression */
    else if("If_sentence" == p.p_left)
    {
        symbolinfo s_if_m1 = g_infostack[g_infostack.size() - 3];
        symbolinfo s_if_expression = g_infostack.back();

        if ("" == s_if_expression.s_value)
        {
            int batch_pos;
            /* 真出口 */
            batch_pos = this->a_bppos.back();
            this->a_bppos.pop_back();
            this->a_qstack[batch_pos].setQua(this->a_qstack[batch_pos].no, this->a_qstack[batch_pos].op, this->a_qstack[batch_pos].arg1, this->a_qstack[batch_pos].arg2, s_if_m1.s_value);
            /* 假出口 */
            batch_pos = this->a_bppos.back();
            this->a_bppos.pop_back();
            this->a_qstack[batch_pos].setQua(this->a_qstack[batch_pos].no, this->a_qstack[batch_pos].op, this->a_qstack[batch_pos].arg1, this->a_qstack[batch_pos].arg2, to_string(peekNextQno()));
        }
        else
        {
            int batch_pos;
            /* if-else */
            batch_pos = this->a_bppos.back();
            this->a_bppos.pop_back();
            this->a_qstack[batch_pos].setQua(this->a_qstack[batch_pos].no, this->a_qstack[batch_pos].op, this->a_qstack[batch_pos].arg1, this->a_qstack[batch_pos].arg2, to_string(peekNextQno()));
            /* if真出口 */
            batch_pos = this->a_bppos.back();
            this->a_bppos.pop_back();
            this->a_qstack[batch_pos].setQua(this->a_qstack[batch_pos].no, this->a_qstack[batch_pos].op, this->a_qstack[batch_pos].arg1, this->a_qstack[batch_pos].arg2, s_if_m1.s_value);
            /* if假出口 */
            batch_pos = this->a_bppos.back();
            this->a_bppos.pop_back();
            this->a_qstack[batch_pos].setQua(this->a_qstack[batch_pos].no, this->a_qstack[batch_pos].op, this->a_qstack[batch_pos].arg1, this->a_qstack[batch_pos].arg2, s_if_expression.s_value);
        }

        this->a_bplevel--;
        if (0 == this->a_bplevel)
        {
            for (auto it = this->a_qstack.begin(); it != this->a_qstack.end();it++)
            {
                printQua(*it);
            }
            int len = this->a_qstack.size();
            for (int i = 0; i < len;i++)
            {
                this->a_qstack.pop_back();
            }
        }

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Call_func_check->$ */
    else if("Call_func_check" == p.p_left)
    {
        symbolinfo s_id = g_infostack[g_infostack.size() - 2];

        int id_pos = -1;
        int id_layer = this->a_sstack.size() - 1;
        for (; id_layer >= 0;id_layer--)
        {
            table tmp_table = this->a_table[this->a_sstack[id_layer]];
            id_pos = tmp_table.findSymbol(s_id.s_value);
            if (-1 != id_pos)
                break;
        }

        if (-1 == id_pos)
        {
            cout << "Sematic analyze error : \'" << s_id.s_value << "\' has not been defined" << endl;
            return false;
        }

        if (FUNCTION != this->a_table[this->a_sstack[id_layer]].getSymbolMode(id_pos))
        {
            cout << "Sematic analyze error : " << s_id.s_value << " is not a function" << endl;
            return false;
        }

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";
        c_info.s_pos = symbolpos(this->a_sstack[id_layer], id_pos);

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Expression_loop->$ */
    else if ("Expression_loop" == p.p_left && "$" == p.p_right[0])
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "0";

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Expression_loop->Expression_loop Expression , */
    else if ("Expression_loop" == p.p_left && "Expression_loop" == p.p_right[0])
    {
        symbolinfo s_call_func_check = g_infostack[g_infostack.size() - 4];
        symbolinfo s_expression_loop = g_infostack[g_infostack.size() - 3];
        symbolinfo s_expression = g_infostack[g_infostack.size() - 2];
        
        symbol &func = this->a_table[s_call_func_check.s_pos.t_pos].getSymbol(s_call_func_check.s_pos.s_pos);
        int pnum = func.s_pnum;
        int p_pnum = stoi(s_expression_loop.s_value);
        if (p_pnum >= pnum)
        {
            cout << "Sematic analyze error : \'" << func.s_name << "\' with too many parameters" << endl;
            return false;
        }

        int t_pos = func.s_pos;
        symbolpos r_pos;
        r_pos.t_pos = t_pos;
        r_pos.s_pos = p_pnum + 1;

        string r_name = getArgName(r_pos, true);
        string arg_name = getArgName(s_expression.s_pos);

        printQua(qua(getNextQno(), ":=", arg_name, "-", r_name));

        p_pnum++;

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = to_string(p_pnum);

        string t_arg_name = this->a_generator.setArg(s_expression.s_pos);
        this->a_generator.printInst(instruction("sw", t_arg_name, "($sp)", ""));
        this->a_generator.pushStack(1);

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Actual_parameter_list->Expression_loop Expression */
    else if ("Actual_parameter_list" == p.p_left && "Expression_loop" == p.p_right[0])
    {
        symbolinfo s_call_func_check = g_infostack[g_infostack.size() - 3];
        symbolinfo s_expression_loop = g_infostack[g_infostack.size() - 2];
        symbolinfo s_expression = g_infostack.back();

        symbol &func = this->a_table[s_call_func_check.s_pos.t_pos].getSymbol(s_call_func_check.s_pos.s_pos);
        int pnum = func.s_pnum;
        int p_pnum = stoi(s_expression_loop.s_value);
        if (p_pnum >= pnum)
        {
            cout << "Sematic analyze error : \'" << func.s_name << "\' with too many parameters" << endl;
            return false;
        }

        int t_pos = func.s_pos;
        symbolpos r_pos;
        r_pos.t_pos = t_pos;
        r_pos.s_pos = p_pnum + 1;

        string r_name = getArgName(r_pos, true);
        string arg_name = getArgName(s_expression.s_pos);

        printQua(qua(getNextQno(), ":=", arg_name, "-", r_name));

        p_pnum++;

        if (p_pnum < pnum)
        {
            cout << "Sematic analyze error : \'" << func.s_name << "\' with missing parameter" << endl;
            return false;
        }

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = to_string(p_pnum);

        string t_arg_name = this->a_generator.setArg(s_expression.s_pos);
        this->a_generator.printInst(instruction("sw", t_arg_name, "($sp)", ""));
        this->a_generator.pushStack(1);

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Actual_parameter_list->$ */
    else if ("Actual_parameter_list" == p.p_left && "$" == p.p_right[0])
    {
        symbolinfo s_call_func_check = g_infostack.back();
        symbol &func = this->a_table[s_call_func_check.s_pos.t_pos].getSymbol(s_call_func_check.s_pos.s_pos);
        int pnum = func.s_pnum;
        if (0 != pnum)
        {
            cout << "Sematic analyze error : \'" << func.s_name << "\' with missing parameter" << endl;
            return false;
        }

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "0";

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    else if ("Call_func" == p.p_left)
    {
        symbolinfo s_id = g_infostack[g_infostack.size() - 5];
        symbolinfo s_call_func_check = g_infostack[g_infostack.size() - 3];

        printQua(qua(getNextQno(), "call", "-", "-", s_id.s_value));

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";
        c_info.s_pos = s_call_func_check.s_pos;

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* FTYPE->Call_func */
    else if ("FTYPE" == p.p_left && "Call_func" == p.p_right[0])
    {
        symbolinfo s_call_func = g_infostack.back();

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "call_func";
        c_info.s_pos = s_call_func.s_pos;

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* Program->Stmt_string */
    else if ("Program" == p.p_left)
    {
        if (-1 == this->main_no)
        {
            cout << "Sematic analyze error : lack of main function" << endl;
            return false;
        }

        printQua();

        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    /* 其余规约，不产生动作 */
    else
    {
        symbolinfo c_info;
        c_info.s_name = p.p_left;
        c_info.s_value = "";   

        int len = 0;
        if ("$" != p.p_right[0])
            len = p.p_right.size();

        for (int i = 0; i < len; i++)
            g_infostack.pop_back();
        g_infostack.push_back(c_info);
    }
    
    return true;
}