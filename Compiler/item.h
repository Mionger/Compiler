#ifndef ITEM_H
#define ITEM_H

struct item
{
    int i_no;
    int i_pos;
    item(int i_no, int i_pos) { this->i_no = i_no, this->i_pos = i_pos; };
    bool operator==(const item &i) const;
    bool operator<(const item &i) const;
};

#endif