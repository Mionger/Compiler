# 词法分析器Tokenizer
## 项目信息Information
|item|info|
|:----:|:----:|  
|Language|Python|  
|Python Version|Python 3.7.2|  
|OS|win10 1903|  
|Recognizing Language|C-Like|  
|Version|0.1.0|  
## 词法分析器简介Introduction
编译过程简单来说就是把源程序转化为另一种形式的程序，就像母语不同的人想要交谈需要一个翻译，编译器就是类似翻译的地位  
程序设计语言和自然语言不一样，程序设计语言都是用符号来描述、每个特定的符号表示特定的意思，而且程序设计语言是上下文无关语言，每个特定语句表达的意思只由自身决定  
## 需求分析
### 输入
输入源程序，按照构词规则分解成一系列单词符号。单词是语言中具有独立意义的最小单位。包括关键字、标识符、运算符、界符和常量等。  
1. 关键字 : 由程序语言定义的具有固定意义的标识符，有时被称为保留字或基本字。  
2. 标识符 : 用来表示各种名字，如变量名、数组名、过程名等等，都是在代码中声明定义的。  
3. 常数   : 常数的类型一般有整型、实型、布尔型、字符型等等。 
4. 运算符 : 如+-*/等等。  
5. 界符   : 如逗号、分号、括号、/\*、\*/等等。  
### 输出
词法分析器输出的单词符号常表示成如下的二元式:  
```
                        (单词种别， 单词符号的属性值)
```
#### 单词种别
其中的单词种别通常由整数编码。关键字、运算符、界符采用一字一码；标识符一般归类为一种；常数则直接按照数据类型分种。  
#### 属性值
在翻译成目标代码时，相同的种别的不同单词符号翻译的结果是不一样的。因此用属性值来记录其附加信息。  
关键字、运算符和界符时一符一种，不需要给出属性值；标识符的属性值是存放它在符号表项的指针或者内部字符串；常数的属性值是存放它在常数表项的指针或者二进制形式。  
## 词法规则Lexical Rules
|item|info|
|:----:|:----:|  
|Keyword|int\|void\|if\|else\|while\|return|
|Identifier|\[a-zA-Z](\[a-zA-Z]\|\[0-9])* (ps:different from Keywords)|
|Numerical Value|\[0-9]([0-9])*|
|Assignment Operator|=|
|Operator|+\|-\|*\|/\|=\|==\|>\|>=\|<\|<=\|!=|
|Boundary Symbol|;|
|Delimiter|,|
|Comment Sign|/\* \*/\|//|
|Left Parenthesis|(|
|Right Parenthesis|)|
|Left Curly Bracket|{|
|Right Curly Bracket|}|
|Character|\[a-zA-Z]|
|Number|\[0-9]|
|EOF|#|
## 单词符号表Word List
|value|type|
|:----:|:----:|
|-6|hasEnd|
|-5|Illegal Charactr|
|-4|hasError|
|-3|EOF|
|-2|Error|
|0|#|
|1|int|
|1|void|
|3|if|
|4|else|
|5|while|
|6|return|
|7|Identifier|
|8|Numerical Value|
|9|=|
|10|+|
|11|-|
|12|\*|
|13|/|
|14|==|
|15|!=|
|16|<|
|17|<=|
|18|>|
|19|>=|
|20|;|
|21|,|
|22|(|
|23|)|
|24|{|
|25|}|
|26|//|
|27|/\*|
|28|\*/|
|29|Keyword or Identifier|