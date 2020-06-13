#ifndef AUTONAMING_INCL
#define AUTONAMING_INCL

#include <map>
#include <string>

#include "data.h"

namespace autonaming
{
    std::string generate_unit_autoname(CLand* land, CUnit* unit);
    std::string generate__initial_unit_autoname(const std::string& race, 
                                                const std::string& skill);

}


#endif