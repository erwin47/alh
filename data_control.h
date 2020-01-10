#ifndef DATA_CONTROL_INCL
#define DATA_CONTROL_INCL

#include "data.h"
#include <memory>
#include <algorithm>

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

    void get_weights(CUnit* unit, long weights[5]);
    
    std::set<CItem> get_all_items(CUnit* unit);
    long get_item_amount(CUnit* unit, const std::string& short_name);
    //void modify_item_amount(CUnit* unit, const std::string& source_name, const std::string& short_name, long new_amount);

    void modify_silver(CUnit* unit, long new_amount, const std::string& reason);

    void modify_item_from_market(CUnit* unit, const std::string& codename, long new_amount, long price);
    void modify_item_from_unit(CUnit* unit, CUnit* source, const std::string& codename, long new_amount);

    void modify_man_from_market(CUnit* unit, const std::string& codename, long new_amount, long price);
    void modify_man_from_unit(CUnit* unit, CUnit* source, const std::string& codename, long new_amount);

    std::string compose_unit_name(CUnit* unit);
    std::string compose_unit_number(long number);

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

    template<typename T>
    void perform_on_each_unit(CLand* land, T Pred)
    {
        for (size_t i = 0; i < land->Units.Count(); i++)
        {
            CUnit* unit = (CUnit*)land->Units.At(i);
            Pred(unit);
        }       
    }

    std::unordered_map<long, Student> get_land_students(CLand* land);
    void update_students_by_land_teachers(CLand* land, std::unordered_map<long, Student>& students);

}

namespace game_control
{
    template<typename T>
    T convert_to(const std::string& str);

    std::string get_gpapp_config(const char* section, const char* key);

    template<typename T>
    std::vector<T> get_game_config(const char* section, const char* key)
    {
        std::vector<T> ret;
        std::string value_string = get_gpapp_config(section, key);
        for(auto it_beg = std::begin(value_string), it_end = std::end(value_string), it_run = std::find(it_beg, it_end, ','); 
                 it_beg != it_end;
                 std::next(it_run), it_beg = it_run, it_run = std::find(it_beg, it_end, ','))
        {
            ret.push_back(convert_to<T>(std::string(it_beg, it_run)));
        }
        return ret;
    }

    template<typename T>
    T get_game_config_val(const char* section, const char* key)
    {
        return convert_to<T>(get_gpapp_config(section, key));
    }

}

/*namespace json_control
{
    template<typename T>
    T extract_type(Json::Value* ptr);

    template<typename T>
    bool check_type(const Json::Value& ptr);

    //implementation just for string. Attempt to call it with any other type would lead to compile time error
    template<>
    std::string extract_type(const Json::Value& ptr) {  return ptr->AsString();  }
    template<>
    bool check_type<std::string>(const Json::Value& ptr) {  return ptr->IsString();  };

    template<typename T>
    std::vector<T> extract_array(const Json::Value& value)
    {
        std::vector<T> ret;
        if (!value.isArray()) {
            dmcproxy_log_write(log_level, "key : %s in '%s' isn't of array type, but %d instead", valueName.c_str(),
                valueName.c_str(), value.type());
            return ret;
        }
        ret.reserve(value.size());
        for (auto it = value.begin(), it_end = value.end(); it != it_end; ++it)
        {
            if (!check_type<T>(*it)) {
                dmcproxy_log_write(log_level, "value : '%s' isn't a string inside <%s> as expected, type %d",
                    it.key().asString().c_str(), valueName.c_str(), it.key().type());
                ret.clear();
                return false;
            }
            ret.push_back(extract_type<T>(*it));
        }
        dmcproxy_assert_op(ret.size(), ==, value.size());
        return ret;
    }
}
*/
#endif
