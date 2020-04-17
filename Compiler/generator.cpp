#include"generator.h"

generator::generator()
{
    this->c_printer.open("result.asm");
    if(!this->c_printer.is_open())
    {
        cout << "Fail to open target file" << endl;
    }
    reg t0;
    t0.r_name = "$t0";
    t0.r_content = "";
    t0.r_miss = 0;
    t0.r_possessed = false;
    this->c_register.push_back(t0);
    reg t1;
    t1.r_name = "$t1";
    t1.r_content = "";
    t1.r_miss = 0;
    t1.r_possessed = false;
    this->c_register.push_back(t1);
    reg t2;
    t2.r_name = "$t2";
    t2.r_content = "";
    t2.r_miss = 0;
    t2.r_possessed = false;
    this->c_register.push_back(t2);
    reg t3;
    t3.r_name = "$t3";
    t3.r_content = "";
    t3.r_miss = 0;
    t3.r_possessed = false;
    this->c_register.push_back(t3);
    reg t4;
    t4.r_name = "$t4";
    t4.r_content = "";
    t4.r_miss = 0;
    t4.r_possessed = false;
    this->c_register.push_back(t4);
    reg t5;
    t5.r_name = "$t5";
    t5.r_content = "";
    t5.r_miss = 0;
    t5.r_possessed = false;
    this->c_register.push_back(t5);
    reg t6;
    t6.r_name = "$t6";
    t6.r_content = "";
    t6.r_miss = 0;
    t6.r_possessed = false;
    this->c_register.push_back(t6);
    reg t7;
    t7.r_name = "$t7";
    t7.r_content = "";
    t7.r_miss = 0;
    t7.r_possessed = false;
    this->c_register.push_back(t7);
    reg t8;
    t8.r_name = "$t8";
    t8.r_content = "";
    t8.r_miss = 0;
    t8.r_possessed = false;
    this->c_register.push_back(t8);
    reg t9;
    t9.r_name = "$t9";
    t9.r_content = "";
    t9.r_miss = 0;
    t9.r_possessed = false;
    this->c_register.push_back(t9);
    init();
    this->gf_offset = GLOBAL_INIT;
}

generator::~generator()
{
    if (this->c_printer.is_open())
        this->c_printer.close();
}

/* 初始化sp,fp,gp */
void generator::init()
{
    printInst(instruction("lui", "$sp", "0x1001", ""));
    printInst(instruction("lui", "$gp", "0x1000", ""));
    printInst(instruction("j", "main", "", ""));
}

/* 初始化符号表 */
void generator::initTable(vector<table> &t)
{
    this->c_table_ptr = &t;
    return;
}

/* 打印指令 */
void generator::printInst(const instruction &i)
{
    this->c_printer << i.op << " " << i.arg1 << " " << i.arg2 << " " << i.arg3 << endl;
    return;
}

/* 声明变量、调用函数时调节指针 */
void generator::pushStack(int num)
{
    printInst(instruction("addi", "$sp", "$sp", to_string(4 * num)));
    return;
}

/* 创建栈帧 */
void generator::createFtrame()
{
    printInst(instruction("sw", "$ra", "($sp)", ""));
    printInst(instruction("sw", "$fp", "4($sp)", ""));
    printInst(instruction("addi", "$fp", "$sp", "4"));
    pushStack(2);
    return;
}

/* 设置未被使用时间 */
void generator::setTime(int r_no)
{
    int len = this->c_register.size();
    for (int i = 0; i < len;i++)
    {
        if(r_no==i)
        {
            this->c_register[i].r_miss = 0;
        }
        else
        {
            this->c_register[i].r_miss += 1;
        }
    }
}

/* 获取最久没被使用的寄存器 */
int generator::getReg()
{
    int reg_t = -1;
    int miss_t = -1;

    int reg_p = -1;
    int miss_p = -1;

    for (vector<reg>::iterator it = this->c_register.begin(); it != this->c_register.end(); it++)
    {
        if (false == it->r_possessed)
        {
            if (it->r_miss > miss_t)
            {
                reg_t = it - this->c_register.begin();
                miss_t = it->r_miss;
            }
        }
        if (it->r_miss > miss_p)
        {
            reg_p = it - this->c_register.begin();
            miss_p = it->r_miss;
        }
    }

    if (-1 != reg_t)
        return reg_t;
    else
        return reg_p;
}

/* 判断符号是否已经被加载到寄存器 */
bool generator::isLoaded(symbolpos &pos) const
{
    vector<table> &s_table = *this->c_table_ptr;
    symbol &s = s_table[pos.t_pos].getSymbol(pos.s_pos);
    if (-1 == s.s_reg)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/* 根据pos返回内存地址 */
string generator::getAddr(const symbolpos &pos) const
{
    string result = "";
    vector<table> &s_table = *this->c_table_ptr;
    /* 全局变量 */
    if (0 == pos.t_pos)
    {
        result += "($gp)";
        int gp_offset = -1;
        vector<symbol> &g_table = s_table[0].getTable();
        for (vector<symbol>::iterator it = g_table.begin(); it != g_table.end(); it++)
        {
            if(FUNCTION==it->s_mode)
            {
                continue;
            }
            gp_offset++;
            if (pos.s_pos == it - g_table.begin())
            {
                break;
            }
        }
        result = to_string(gp_offset * 4) + result;
    }
    /* 临时变量 */
    else if (1 == pos.t_pos)
    {
        result += "($gp)";
        int gp_offset = GLOBAL_INIT + pos.s_pos;
        result = to_string(gf_offset * 4) + result;
    }
    /* 局部变量、形参、返回值 */
    else
    {
        int func_pos = s_table[0].findSymbol(s_table[pos.t_pos].getName());
        symbol &func = s_table[0].getSymbol(func_pos);

        /* 返回值 */
        if (0 == pos.s_pos)
        {
            symbol return_value = s_table[pos.t_pos].getSymbol(0);
            if (INT == return_value.s_type)
            {
                result = "$v0";
            }
            else
            {
                result = "";
            }
        }
        else
        {
            int p_num = func.s_pnum;
            /* 形参 */
            if(pos.s_pos<=func.s_pnum)
            {
                result = "$(fp)";
                int fp_offset = -2 - func.s_pnum + pos.s_pos;
                result = to_string(fp_offset * 4) + result;
            }
            else
            {
                result = "$(fp)";
                int fp_offset = func.s_pnum + pos.s_pos;
                result = to_string(fp_offset * 4) + result;
            }
        }
    }

    return result;
}

/* 把符号表中指定符号加载到指定寄存器 */
void generator::loadReg(int r_no, symbolpos &pos)
{
    vector<table> &s_table = *this->c_table_ptr;
    symbol &s = s_table[pos.t_pos].getSymbol(pos.s_pos);

    /* 寄存器被占用 */
    if (true == this->c_register[r_no].r_possessed)
    {
        string sw_addr = getAddr(this->c_register[r_no].r_info);
        printInst(instruction("sw", this->c_register[r_no].r_name, sw_addr, ""));
        s_table[this->c_register[r_no].r_info.t_pos].getSymbol(this->c_register[r_no].r_info.s_pos).s_reg = -1;
    }

    printInst(instruction("lw", this->c_register[r_no].r_name, getAddr(pos), ""));

    this->c_register[r_no].r_info = pos;
    this->c_register[r_no].r_possessed = true;

    s.s_reg = r_no;

    return;
}

/* 把立即数加载到临时寄存器中 */
int generator::loadImm(const string &imm, const symbolpos &pos)
{
    vector<table> &s_table = *this->c_table_ptr;
    int r_no = getReg();

    /* 寄存器被占用 */
    if (true == this->c_register[r_no].r_possessed)
    {
        string sw_addr = getAddr(this->c_register[r_no].r_info);
        printInst(instruction("sw", this->c_register[r_no].r_name, sw_addr, ""));
        s_table[this->c_register[r_no].r_info.t_pos].getSymbol(this->c_register[r_no].r_info.s_pos).s_reg = -1;
    }

    printInst(instruction("addi", this->c_register[r_no].r_name, "$zero", imm));

    symbol &s = s_table[pos.t_pos].getSymbol(pos.s_pos);
    s.s_reg = r_no;

    this->c_register[r_no].r_info = pos;
    this->c_register[r_no].r_possessed = true;

    return r_no;
}

/* main函数结束 */
void generator::endMain()
{
    printInst(instruction("syscall", "", "", ""));
    return;
}

/* 将操作数放入寄存器 */
string generator::setArg(symbolpos pos)
{
    vector<table> &s_table = *this->c_table_ptr;
    symbol &s = s_table[pos.t_pos].getSymbol(pos.s_pos);

    string r_name = "$t";
    if (-1 == s.s_reg)
    {
        int r_no = getReg();
        loadReg(r_no, pos);
    }
    r_name += to_string(s.s_reg);

    return r_name;
}

/* 清空寄存器，写回局部变量 */
void generator::clearReg()
{
    vector<table> &s_table = *this->c_table_ptr;
    symbolpos n_pos;
    n_pos.s_pos = -1;
    n_pos.t_pos = -1;
    for (vector<reg>::iterator it = this->c_register.begin(); it != this->c_register.end(); it++)
    {
        if (false == it->r_possessed)
            continue;
        symbolpos pos = it->r_info;
        symbol &s = s_table[pos.t_pos].getSymbol(pos.s_pos);
        s.s_reg = -1;
        it->r_possessed = false;
        it->r_miss = 0;
        it->r_info = n_pos;
        
        if (TEMP == s.s_mode)
            continue;
        int r_no = it - this->c_register.begin();
        string mem = getAddr(pos);
        printInst(instruction("sw", this->c_register[r_no].r_name, mem, ""));
    }

    return;
}

/* 复位寄存器的使用情况 */
void generator::resetReg()
{
    vector<table> &s_table = *this->c_table_ptr;
    symbolpos n_pos;
    n_pos.s_pos = -1;
    n_pos.t_pos = -1;
    for (vector<reg>::iterator it = this->c_register.begin(); it != this->c_register.end(); it++)
    {
        if (false == it->r_possessed)
            continue;
        symbolpos pos = it->r_info;
        symbol &s = s_table[pos.t_pos].getSymbol(pos.s_pos);
        s.s_reg = -1;
        it->r_possessed = false;
        it->r_miss = 0;
        it->r_info = n_pos;
    }
    return;
}