#include"operation.h"

bool operation::operator==(const operation &o) const
{
    return (this->o_operation == o.o_operation) && (this->o_no == o.o_no);
}