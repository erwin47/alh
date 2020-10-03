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
        UNIT_FACTION,
        UNIT_AO_NEED,
        UNIT_AO_SOURCE,
        REGION_NAME,
        REGION_LOCATION,
        REGION_SELL_AMOUNT,
        REGION_SELL_PRICE,
        REGION_WANTED_AMOUNT,
        REGION_WANTED_PRICE,
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

    template<typename T>
    inline bool curtail(T& t1)
    {
        if (t1 >= 1000) {
            t1 = t1 / 1000;
            return true;
        }
        return false;
    }
    template<>
    inline bool curtail<std::string>(std::string& t1)
    {
        return false;
    }

    std::string to_string(Operation op);

    bool evaluate_amount_filter(long amount, const std::string& filter);

    bool evaluate_unit_statement(CLand* land, CUnit* unit, const std::string& statement, std::vector<unit_control::UnitError>& errors);
    bool evaluate_land_statement(CLand* land, long& res, const std::string& statement, std::vector<unit_control::UnitError>& errors);

    const std::map<std::string, std::string>& function_descriptions();
}


#endif
