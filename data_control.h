#ifndef DATA_CONTROL_INCL
#define DATA_CONTROL_INCL

#include "data.h"
#include <memory>
#include <algorithm>

#ifdef _MSC_VER 
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

namespace unit_control
{
    struct UnitError
    {
        CUnit* unit_;
        std::string message_;
    };
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

    //!mask may define specific group of items, according to UNIT_PROPERTY_GROUPS settings
    std::set<CItem> get_all_items_by_mask(CUnit* unit, const char* mask);
    long get_item_amount(CUnit* unit, const std::string& short_name, bool initial = false);
    long get_item_amount_by_mask(CUnit* unit, const char* mask);
    //void modify_item_amount(CUnit* unit, const std::string& source_name, const std::string& short_name, long new_amount);

    void modify_silver(CUnit* unit, long new_amount, const std::string& reason);

    void modify_item_by_produce(CUnit* unit, const std::string& codename, long new_amount);

    void modify_item_from_market(CUnit* unit, const std::string& codename, long new_amount, long price);
    void modify_item_from_unit(CUnit* unit, CUnit* source, const std::string& codename, long new_amount);

    void modify_man_from_market(CUnit* unit, const std::string& codename, long new_amount, long price);
    void modify_man_from_unit(CUnit* unit, CUnit* source, const std::string& codename, long new_amount);

    std::string compose_unit_name(CUnit* unit);
    std::string compose_unit_number(long number);

    long get_max_skill_lvl(CUnit* unit, const std::string& skill);
    long get_current_skill_days(CUnit* unit, const std::string& skill);

    void order_message(CUnit* unit, const char* line, const char* descr);

    std::string get_initial_description(CUnit* unit);
    std::string get_actual_description(CUnit* unit);

    bool init_caravan(CUnit* unit);
    void clean_caravan(CUnit* unit);
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

    CProductMarket get_wanted(CLand* land, const std::string& item_code);
    CProductMarket get_for_sale(CLand* land, const std::string& item_code);
    void add_resource(CLand* land, const CItem& item);
    long get_resource(CLand* land, const std::string& item_code);
    void set_produced_items(CLand* land, const std::string& item_code, long amount);
    long get_produced_items(CLand* land, const std::string& item_code);

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
    CUnit* find_first_unit_if(CLand* land, T Pred)
    {
        for (size_t i = 0; i < land->Units.Count(); i++)
        {
            CUnit* unit = (CUnit*)land->Units.At(i);
            if (Pred(unit))
                return unit;
        }
        return nullptr;    
    }    

    template<typename T>
    void perform_on_each_unit(CLand* land, T Pred)
    {
        for (size_t i = 0; i < land->UnitsSeq.Count(); i++)
        {
            CUnit* unit = (CUnit*)land->UnitsSeq.At(i);
            Pred(unit);
        }       
    }

    long get_land_id(const char* land);

    long get_plane_id(const char* plane_name);
    CLand* get_land(int x, int y, int z);
    CLand* get_land(long land_id);

    std::unordered_map<long, Student> get_land_students(CLand* land, std::vector<unit_control::UnitError>& errors);
    void update_students_by_land_teachers(CLand* land, std::unordered_map<long, Student>& students, std::vector<unit_control::UnitError>& errors);

}

namespace game_control
{
    struct NameAndAmount
    {
        std::string name_;
        double amount_;
    };

    template<typename T>
    T convert_to(const std::string& str);

    std::string get_gpapp_config(const char* section, const char* key);

    template<typename T>
    std::vector<T> get_game_config(const char* section, const char* key)
    {
        std::vector<T> ret;
        std::string value_string = get_gpapp_config(section, key);
        const char* beg = value_string.c_str();
        const char* end = value_string.c_str() + value_string.size();
        const char* runner = beg;
        while(beg < end)
        {
            while (beg < end && !isalpha(*beg) && !isdigit(*beg))
                ++beg;

            if (beg == end)
                break;
                
            runner = beg;
            while (runner < end && *runner != ',')
                ++runner;

            ret.push_back(convert_to<T>(std::string(beg, runner)));
            ++runner;
            beg = runner;
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
