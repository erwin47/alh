#ifndef DATA_CONTROL_INCL
#define DATA_CONTROL_INCL

#include "data.h"

namespace unit_control
{
    namespace flags
    {
        bool is_behind(CUnit* unit);
        bool is_guard(CUnit* unit);
        bool is_hold(CUnit* unit);
        bool is_noaid(CUnit* unit);
        bool is_avoid(CUnit* unit);
        bool is_nocross(CUnit* unit);
        bool is_sharing(CUnit* unit);
        bool is_reveal(CUnit* unit, std::string flag);
        bool is_spoils(CUnit* unit, std::string flag);
        bool is_consume(CUnit* unit, std::string flag);
        bool is_leader(CUnit* unit);
    }

    std::set<CItem>& get_items(CUnit* unit);
    long get_item_amount(CUnit* unit, const std::string& short_name);
    void modify_item_amount(CUnit* unit, const std::string& source_name, const std::string& short_name, long new_amount);

    std::string get_initial_description(CUnit* unit);
    std::string get_actual_description(CUnit* unit);

}

namespace land_control
{
    template<typename T>
    void get_units_if(CLand* land, std::vector<CUnit*>& units, T Pred)
    {
        for (size_t i = 0; i < land->Units.Count(); i++)
        {
            CUnit* unit = (CUnit*)land->Units.At(i);
            if (Pred(unit))
                units.push_back(unit);
        }       
    }
}

#endif
