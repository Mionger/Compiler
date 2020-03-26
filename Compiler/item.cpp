#include"item.h"

bool item::operator==(const item & i) const 
{
    return (this->i_no == i.i_no) && (this->i_pos == i.i_pos);
}

bool item::operator<(const item & i) const
{
    return this->i_no < i.i_no || (this->i_no == i.i_no && this->i_pos < i.i_pos);
}