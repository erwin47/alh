#ifndef DATA_CONTROL_INCL
#define DATA_CONTROL_INCL

#include "data.h"
#include <memory>

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

    std::set<CItem> get_all_items(CUnit* unit);
    long get_item_amount(CUnit* unit, const std::string& short_name);
    //void modify_item_amount(CUnit* unit, const std::string& source_name, const std::string& short_name, long new_amount);

    void modify_silver(CUnit* unit, long new_amount, const std::string& reason);

    void modify_item_from_market(CUnit* unit, const std::string& codename, long new_amount, long price);
    void modify_item_from_unit(CUnit* unit, CUnit* source, const std::string& codename, long new_amount);

    void modify_man_from_market(CUnit* unit, const std::string& codename, long new_amount, long price);
    void modify_man_from_unit(CUnit* unit, CUnit* source, const std::string& codename, long new_amount);

    std::string compose_unit_name(CUnit* unit);

    long get_max_skill_lvl(CUnit* unit, const std::string& skill);

    void order_message(CUnit* unit, const char* line, const char* descr);

    std::string get_initial_description(CUnit* unit);
    std::string get_actual_description(CUnit* unit);
}

namespace land_control
{
    struct Student
    {
        std::shared_ptr<orders::Order> order_;
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
