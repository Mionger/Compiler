#include"lexer.h"

lexer::lexer()
{
    this->line_cnt = 1;
    this->step_cnt = 1;
    this->print_flag = true;
}

lexer::~lexer()
{
    if(this->scanner.is_open())
    {
        this->scanner.close();
    }

    if(this->printer.is_open())
    {
        this->printer.close();
    }
}

/* 是否是字母 */
int lexer::isLetter (const unsigned char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return 1;
    else
        return 0;
}

/* 是否是数字 */
int lexer::isDigit (const unsigned char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;
}

/* 是否是运算符 */
int lexer::isOperator (const unsigned char c)
{
    if ('+' == c || '-' == c || '*' == c || '/' == c)
        return 1;
    else if ('=' == c || '>' == c || '<' == c || '!' == c)
        return 2;
	else
		return 0;
}

/* 是否是空格 */
int lexer::isBlank (const unsigned char c)
{
    if (' ' == c || '\n' == c || '\t' == c || 255 == c)
		return 1;
	else
		return 0;
}

/* 获取下一个字符 */
unsigned char lexer::getNextChar()
{
    unsigned char c = this->scanner.get();
    if ('\n' == c)
        this->line_cnt++;
    return c;
}

/* 获取文法符号，不能打印 */
token lexer::getTokenNP()
{
    unsigned char c;
    string str_token = "";
    token t;

    while (!this->scanner.eof())
    {
        c = getNextChar();
        /*  处理注释 */
        if ('/' == c)
        {
            unsigned char next = getNextChar();
            /* 单行注释 */
            if ('/' == next)
            {
                while ('\n' != getNextChar() && !this->scanner.eof())
                    continue;
            }
            /* 多行注释 */
            else if ('*' == next)
            {
                while (!this->scanner.eof())
                {
                    while ('*' != getNextChar() && !this->scanner.eof())
                        continue;

                    if(this->scanner.eof())
                    {
                        t.t_type = L_UNKNOWN;
                        t.t_value = "Unexcepted token at line " + to_string(this->line_cnt);
                        return t;
                    }

                    unsigned char temp = getNextChar();
                    if ('/' == temp)
                        break;
                    //TODO:
                    else
                    {
                        t.t_type = L_UNKNOWN;
                        t.t_value = "Unexcepted token" + to_string(temp) + " at line " + to_string(this->line_cnt);
                        return t;
                    }
                }
            }
            /* 除号 */
            else
            {
                this->scanner.seekg(-1, ios::cur);
                t.t_type = L_OPERATOR;
                t.t_value = c;
                return t;
            }
        }
        else if (isDigit(c))
        {
            str_token += c;
            while(!this->scanner.eof())
            {
                unsigned char next = getNextChar();
                if(!isDigit(next))
                {
                    this->scanner.seekg(-1, ios::cur);
                    t.t_type = L_CINT;
                    t.t_value = str_token;
                    return t;
                }
                else
                {
                    str_token += next;
                }
            }
        }
        else if (isLetter(c))
        {
            str_token += c;
            while(!this->scanner.eof())
            {
                unsigned char next = getNextChar();
                if(!isDigit(next)&&!isLetter(next))
                {
                    this->scanner.seekg(-1, ios::cur);

                    if (this->KEYWORDS.find(str_token) != this->KEYWORDS.end())
                    {
                        t.t_type = L_KEYWORD;
                    }
                    else if(this->TYPES.find(str_token)!=this->TYPES.end())
                    {
                        t.t_type = L_TYPE;
                    }
                    else
                    {
                        t.t_type = L_IDENTIFIER;
                    }

                    t.t_value = str_token;
                    return t;
                }
                else
                {
                    str_token += next;
                }
            }
        }
        else if (this->BORDERS.find(c) != this->BORDERS.end())
        {
            t.t_type = L_BORDER;
            t.t_value = c;
            return t;
        }
        else if (1 == isOperator(c))
        {
            t.t_type = L_OPERATOR;
            t.t_value = c;
            return t;
        }
        else if (2 == isOperator(c))
        {
            str_token += c;
            unsigned char next = getNextChar();

            if ('=' == next)
            {
                str_token += next;
                t.t_type = L_OPERATOR;
                t.t_value = str_token;
                return t;
            }
            else
            {
                this->scanner.seekg(-1, ios::cur);
                if ('!' == c)
                {
                    t.t_type = L_UNKNOWN;
                    t.t_value = "Unexcepted token" + str_token + " at line " + to_string(this->line_cnt);
                    return t;
                }
                else
                {
                    t.t_type = L_OPERATOR;
                    t.t_value = str_token;
                    return t;
                }
            }
        }
        else if (isBlank(c))
        {
            continue;
        }
        else if ('#' == c)
        {
            t.t_type = L_EOF;
            t.t_value = c;
            return t;
        }
        else
        {
            t.t_type = L_UNKNOWN;
            t.t_value = "Unexcepted token" + to_string(c) + " at line " + to_string(this->line_cnt);
            return t;
        }
    }

    t.t_type = L_EOF;
    t.t_value = "#";
    return t;
}

/* 打印符号信息 */
void lexer::printToken (token t)
{
    string type;
    switch (t.t_type)
    {
    case L_KEYWORD:
        type = "KEYWORD";
        break;
    case L_BORDER:
        type = "BORDER";
        break;
    case L_TYPE:
        type = "TYPE";
        break;
    case L_IDENTIFIER:
        type = "IDENTIFIER";
        break;
    case L_OPERATOR:
        type = "OPERATOR";
        break;
    case L_CINT:
        type = "CINT";
        break;
    case L_EOF:
        type = "EOF";
        break;
    default:
        type = "UNKNOWN";
        break;
    }

    this->printer << "step " << this->step_cnt << "\ttype : " << type << "\t\t\t\tvalue : " << t.t_value << endl;
    this->step_cnt++;
    return;
}

/* 打开需要的文件 */
int lexer::openFiles(const string src_file)
{
    this->scanner.open(src_file);
    if(!this->scanner.is_open())
    {
        cout << "Fail to open the source file" << endl;
        return -1;
    }

    if(this->print_flag)
    {
        this->printer.open("result_lexer.txt");
        if(!this->scanner.is_open())
        {
            cout << "Fail to open the lexer result file" << endl;
            return -2;
        }
    }

    return 0;
}

/* 获取文法符号，可以打印 */
token lexer::getTokenP()
{
    token t = getTokenNP();
    if (L_EOF == t.t_type)
    {
        t.t_name = "#";
    }
    else if(L_CINT==t.t_type)
    {
        t.t_name = "num";
    }
    else if (L_IDENTIFIER == t.t_type)
    {
        t.t_name = "identifier";
    }
    else if (L_TYPE == t.t_type || L_KEYWORD == t.t_type || L_BORDER == t.t_type || L_OPERATOR == t.t_type)
    {
        t.t_name = t.t_value;
    }
    else
    {
        t.t_name = "";
    }

    if (this->print_flag)
        printToken(t);
    return t;
}

/* 开始词法分析 */
void lexer::startScanner()
{
    while(true)
    {
        token t = getTokenP();
        if (L_UNKNOWN == t.t_type || L_EOF == t.t_type)
        {
            return;
        }
    }
    return;
}