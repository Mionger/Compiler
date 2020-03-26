#include"tool.h"

vector<string> split(const string str, const string pattern)
{
    vector<string> result;

    if ("" == str)
        return result;

    string strs = str + pattern;
    size_t pos = strs.find(pattern);
    while (string::npos != pos)
    {
        string temp = strs.substr(0, pos);
        result.push_back(temp);
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }

    return result;
}