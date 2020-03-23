#include<iostream>
#include<string>
#include"lexer.h"
using namespace std;

lexer mylexer;

void printHelp()
{
    cout << "Compiler by M.G. Park" << endl;
}

int main(int argc, char *argv[])
{
    switch (argc) 
    {
        case 2:
            if ("--help" == string(argv[1]))
            {
                printHelp();
            }
            return 0;
        case 3:
            if ("-l" == string(argv[1]) || "--lexer" == string(argv[1]))
            {
                if (0 != mylexer.openFiles(string(argv[2])))
                {
                    return -1;
                }
                mylexer.startScanner();
                return 0;
            }
            break;
        default:
            printHelp();
            return 0;
    }
}