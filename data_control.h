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

    long get_max_skill_lvl(CUnit* unit, const std::string& skill);

    void order_message(CUnit* unit, const char* line, const char* descr);

    std::string get_initial_description(CUnit* unit);
    std::string get_actual_description(CUnit* unit);
}

namespace land_control
{
    struct Student
    {
        std::string studying_skill_;
        long man_amount_;
        long max_days_;
        long cur_days_;
        long days_of_teaching_;
        CUnit* unit_;
    };

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

    std::unordered_map<long, Student> get_land_students(CLand* land);
    void update_students_by_land_teachers(CLand* land, std::unordered_map<long, Student>& students);

}

#endif
