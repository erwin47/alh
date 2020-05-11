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

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch) && !(ch == '\r') && !(ch == '\n') && !(ch == '\t');
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch) && !(ch == '\r') && !(ch == '\n') && !(ch == '\t');
    }).base(), s.end());
}

// trim from both ends (in place)
static inline std::string trim(std::string s) {
    ltrim(s);
    rtrim(s);
    return s;
}

namespace item_control
{
    //take on input full name, plural or code, returns code
    std::string codename(const std::string& name);

    int weight(const std::string& item_code);
}

namespace unit_control
{
    struct UnitError
    {
        std::string type_;
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
    }

    bool is_leader(CUnit* unit);
    long structure_id(const CUnit* unit);
    bool is_struct_owner(const CUnit* unit);
    void set_structure(CUnit* unit, long struct_id, bool owns);
    
    long get_upkeep(CUnit* unit);

    void get_weights(CUnit* unit, long weights[5]);
    
    std::set<CItem> get_all_items(CUnit* unit);

    //!mask may define specific group of items, according to UNIT_PROPERTY_GROUPS settings
    std::set<CItem> get_all_items_by_mask(CUnit* unit, const char* mask);
    long get_item_amount(CUnit* unit, const std::string& short_name, bool initial = false);
    long get_item_amount_by_mask(CUnit* unit, const char* mask);
    //void modify_item_amount(CUnit* unit, const std::string& source_name, const std::string& short_name, long new_amount);

    void modify_silver(CUnit* unit, long new_amount, const std::string& reason);

    void modify_item_by_produce(CUnit* unit, const std::string& codename, long new_amount);

    void modify_item_by_reason(CUnit* unit, const std::string& codename, long new_amount, const std::string& reason);

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
        long skill_price_;
        CUnit* unit_;
    };

    struct Trader
    {
        std::shared_ptr<orders::Order> order_;
        std::string item_name_;
        long items_amount_;
        long market_price_;
        CUnit* unit_;
    };

    struct Taxers
    {
        bool is_pillaging_;
        long man_amount_;
        long land_tax_available_;
        long expected_income_;
        std::vector<CUnit*> units_;
    };

    struct ActionUnit
    {
        std::string action_;
        std::string description_;
        CUnit* unit_;
    };

    CProductMarket get_wanted(LandState& state, const std::string& item_code);
    CProductMarket get_for_sale(LandState& state, const std::string& item_code);
    void add_resource(LandState& state, const CItem& item);
    long get_resource(LandState& state, const std::string& item_code);
    void set_produced_items(LandState& state, const std::string& item_code, long amount);
    long get_produced_items(LandState& state, const std::string& item_code);

    CStruct* get_struct(CLand* land, long struct_id);
    long get_struct_weight(CLand* land, long struct_id);

    template<typename T>
    void perform_on_each_struct(CLand* land, T Pred)
    {
        for (auto& structure : land->current_state_.structures_) {
            Pred(structure);
        }
    }

    template<typename T>
    CStruct* find_first_structure_if(CLand* land, T Pred)
    {
        for (auto& structure : land->current_state_.structures_) {
            if (Pred(structure))
                return structure;
        }
        return nullptr;    
    }    

    namespace structures
    {
        void update_struct_weights(CLand* land);
        
        CStruct* add_structure(CLand* land, LandState& lstate, CStruct* structure);
        CStruct* remove_structure(CLand* land, LandState& lstate, CStruct* structure);
        void clean_structures(LandState& lstate);//except HIDDEN & SHAFTS

        bool link_shafts(CLand* from, CLand* to, long struct_id);

        void land_flags_update(CLand* land, CStruct* structure);

    }

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
    void get_land_coordinates(long land_id, int& x, int& y, int& z);
    std::string land_full_name(CLand* land);

    long get_plane_id(const char* plane_name);
    CLand* get_land(int x, int y, int z);
    CLand* get_land(long land_id);

    namespace economy 
    {
        void economy_calculations(CLand* land, CEconomy& res, std::vector<unit_control::UnitError>& errors);
    }

    void apply_land_flags(CLand* land, std::vector<unit_control::UnitError>& errors);
    void get_land_builders(CLand* land, std::vector<ActionUnit>& out, std::vector<unit_control::UnitError>& errors);
    void get_land_taxers(CLand* land, Taxers& out, std::vector<unit_control::UnitError>& errors);
    void get_land_sells(CLand* land, std::vector<Trader>& out, std::vector<unit_control::UnitError>& errors);
    void get_land_buys(CLand* land, std::vector<Trader>& out, std::vector<unit_control::UnitError>& errors);
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

    bool get_struct_attributes(const std::string& struct_type, long& capacity, long& sailPower, long& structFlag);

}

namespace struct_control
{
    void parse_struct(const std::string& line, long& id, std::string& name, 
                      std::string& type, std::vector<std::pair<std::string, long>>& substructures);

    namespace flags {
        inline bool is_shaft(CStruct* structure) {  return structure->Attr & SA_SHAFT;  }
        inline bool is_ship(CStruct* structure) {  return structure->Attr & SA_MOBILE;  }
    }

    void copy_structure(CStruct* from, CStruct* to);

    bool has_link(CStruct* structure);
    
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
