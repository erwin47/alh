#ifndef AUTOLOGIC_INCL
#define AUTOLOGIC_INCL

#include <string>
#include "data_control.h"

namespace autologic
{
    enum class Command  
    {
        NONE,
        UNIT_ITEM,
        UNIT_SKILL,
        UNIT_SPEED,
        REGION_LOCATION,
        REGION_SELL,
        REGION_WANTED,
        REGION_RESOURCE
    };

    enum class Operation 
    {
        NONE,
        MORE,
        LESS,
        EQUAL,
        MORE_OR_EQUAL,
        LESS_OR_EQUAL,
        NOT_EQUAL
    };

    std::string to_string(Operation op);
    bool evaluate_statement(CLand* land, CUnit* unit, const std::string& statement, std::vector<unit_control::UnitError>& errors);
}


#endif
