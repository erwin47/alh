#include "ah_control.h"
#include "data_control.h"
#include <unordered_set>
#include <numeric>

namespace game_control
{
    template<>
    std::string convert_to<std::string>(const std::string& str)
    {
        return str;
    }

    template<>
    NameAndAmount convert_to(const std::string& str)
    {//TYPE X    
        size_t separator = str.find(' ');
        if (separator == std::string::npos)
            return {str, 0};
        
        return {str.substr(0, str.find(' ')), atof(&str[str.find(' ')+1])};
    }

    template<>
    long convert_to<long>(const std::string& str)
    {
        return atol(str.c_str());
    }

    template<>
    double convert_to<double>(const std::string& str)
    {
        return atof(str.c_str());
    }

    template<typename T>
    std::string convert_from(const T& t1)
    {
        return std::to_string(t1);
    }

    template<>
    std::string convert_from<std::string>(const std::string& str)
    {
        return str;
    }



    std::string get_gpapp_config(const char* section, const char* key)
    {
        return gpApp->GetConfig(section, key);
    }

    std::vector<std::pair<std::string, std::string>> get_all_configuration(const char* section)
    {
        std::vector<std::pair<std::string, std::string>> ret;
        const char  * szName;
        const char  * szValue;      
        int sectidx = gpApp->GetSectionFirst(section, szName, szValue);
        while (sectidx >= 0)
        {
            std::pair<std::string, std::string> cur_pair = {std::string(szName), std::string(szValue)};
            ret.emplace_back(cur_pair);
            sectidx = gpApp->GetSectionNext(sectidx, section, szName, szValue);
        }
        return ret;
    }

    template<typename T>
    bool doCompare(T v1, T v2, eCompareOp CompareOp)
    {
        switch (CompareOp)
        {
            case GT: return v1 > v2;
            case GE: return v1 >= v2;
            case EQ: return v1 == v2;
            case LE: return v1 <= v2;
            case LT: return v1 < v2;
            case NE: return v1 != v2;
            default: return false;
        }
    }

    template<>
    bool doCompare(std::string v1, std::string v2, eCompareOp CompareOp)
    {
        switch (CompareOp)
        {
            case GT: return stricmp(v1.c_str(), v2.c_str()) > 0;
            case GE: return stricmp(v1.c_str(), v2.c_str()) >= 0;
            case EQ: return stricmp(v1.c_str(), v2.c_str()) == 0;
            case LE: return stricmp(v1.c_str(), v2.c_str()) <= 0;
            case LT: return stricmp(v1.c_str(), v2.c_str()) < 0;
            case NE: return stricmp(v1.c_str(), v2.c_str()) != 0;
            default: return false;
        }
    }

    bool get_struct_attributes(const std::string& struct_type, long& capacity, long& sailPower, long& structFlag, SHIP_TRAVEL& travel, long& speed)
    {
        std::string codename, name, plural_name;
        if (!gpApp->ResolveAliasItems(struct_type, codename, name, plural_name))
            name = struct_type;

        long fileno = gpApp->GetConfigFileNo(SZ_SECT_STRUCTS);
        std::string struct_params = gpApp->config_[fileno].get_case_insensitive<std::string>(SZ_SECT_STRUCTS, name.c_str(), "");
        if (struct_params.size() == 0)
        {
            std::string name_without_s = name.substr(0, name.size()-1);
            struct_params = gpApp->config_[fileno].get_case_insensitive<std::string>(SZ_SECT_STRUCTS, name_without_s.c_str(), "");
            if (struct_params.size() == 0)
                return false;
        }

        std::vector<std::string> struct_parameters = convert_to_vector<std::string>(struct_params);
        for (const auto& param : struct_parameters)
        {
            if      (param == SZ_ATTR_STRUCT_MOBILE)    structFlag |= SA_MOBILE;
            else if (param == SZ_ATTR_STRUCT_HIDDEN)    structFlag |= SA_HIDDEN ;
            else if (param == SZ_ATTR_STRUCT_SHAFT)     structFlag |= SA_SHAFT  ;
            else if (param == SZ_ATTR_STRUCT_GATE)      structFlag |= SA_GATE   ;
            else if (param == SZ_ATTR_STRUCT_ROAD_N)    structFlag |= SA_ROAD_N ;
            else if (param == SZ_ATTR_STRUCT_ROAD_NE)   structFlag |= SA_ROAD_NE;
            else if (param == SZ_ATTR_STRUCT_ROAD_SE)   structFlag |= SA_ROAD_SE;
            else if (param == SZ_ATTR_STRUCT_ROAD_S)    structFlag |= SA_ROAD_S ;
            else if (param == SZ_ATTR_STRUCT_ROAD_SW)   structFlag |= SA_ROAD_SW;
            else if (param == SZ_ATTR_STRUCT_ROAD_NW)   structFlag |= SA_ROAD_NW;
            else
            {
                // Two-token attributes, MaxLoad & MinSailingPower & travel type & speed.
                size_t separator = param.find(' ');
                if (separator == std::string::npos)
                {
                    //TODO: print out the wrong setting
                    continue;
                }

                std::string key = param.substr(0, separator);
                if (key == SZ_ATTR_STRUCT_MAX_LOAD)
                    capacity += atol(param.substr(separator+1).c_str());
                if (key == SZ_ATTR_STRUCT_MIN_SAIL)
                    sailPower += atol(param.substr(separator+1).c_str());
                if (key == SZ_ATTR_STRUCT_TRAVEL) {
                    std::string traveling = param.substr(separator+1);
                    if (stricmp(traveling.c_str(), "FLY") == 0 && travel != SHIP_TRAVEL::SAIL)
                        travel = SHIP_TRAVEL::FLY;
                    else
                        travel = SHIP_TRAVEL::SAIL;
                }
                if (key == SZ_ATTR_STRUCT_USUAL_SPEED) {
                    speed = atol(param.substr(separator+1).c_str());
                }
            }
        }

        if (0 == stricmp(name.c_str(), STRUCT_GATE))
            structFlag |= SA_GATE; // to compensate for legacy missing gate flag in the config
        return true;        
    }

    std::unordered_map<std::string, long> create_upkeep_map()
    {
        std::unordered_map<std::string, long> ret;
        std::vector<std::pair<std::string, std::string>> specific_upkeep = game_control::get_all_configuration(SZ_SECT_SPECIFIC_UPKEEP);
        for (const auto& upkeep : specific_upkeep)
            ret[upkeep.first] = convert_to<long>(upkeep.second);
        return ret;
    }

    std::unordered_set<std::string> vector_to(const std::vector<std::string>& vec)
    {
        return std::unordered_set<std::string>(vec.begin(), vec.end());
    }

    long get_item_upkeep(const std::string& item) 
    {
        static std::unordered_map<std::string, long> cache = create_upkeep_map();
        if (cache.find(item) != cache.end())
            return cache[item];

        static std::unordered_set<std::string> leaders = vector_to(game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, PRP_MEN_LEADER));
        static std::unordered_set<std::string> men = vector_to(game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, PRP_MEN));
        if (leaders.find(item) != leaders.end())
            cache[item] = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_LEADER);
        else if (men.find(item) != men.end())
            cache[item] = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_PEASANT);
        else
            cache[item] = 0;

        return cache[item];
    }

    long get_study_cost(const std::string& skill) 
    {
        static std::unordered_map<std::string, long> cache;
        if (cache.find(skill) == cache.end()) {
            long fileno = gpApp->GetConfigFileNo(SZ_SECT_STUDY_COST);
            cache.insert({skill, gpApp->config_[fileno].get_case_insensitive<long>(SZ_SECT_STUDY_COST, skill.c_str(), -1)});
        }

        return cache[skill];
    }

    namespace specific
    {
        // returns men collection which are not leaders
        std::set<std::string> men_peasants() {
            std::set<std::string> ret;
            std::vector<std::string> men_items = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, PRP_MEN);
            std::vector<std::string> leader_items = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, PRP_MEN_LEADER);
            for (const auto& race : men_items)
            {
                bool found(false);
                for (const auto& lead : leader_items) 
                {
                    if (stricmp(lead.c_str(), race.c_str()) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    ret.insert(race);
            }
            return ret;
        }

        CUnit* create_scout(CUnit* parent)
        {
            CLand* land = land_control::get_land(parent->LandId);
            if (land == nullptr)
                return nullptr;

            std::string race_to_buy;
            long race_price(-1);

            static std::set<std::string> peasants = men_peasants();
            for (const auto& sale : land->current_state_.for_sale_)
            {
                if (peasants.find(sale.first) != peasants.end())
                {
                    race_to_buy = sale.first;
                    race_price = sale.second.price_;
                    break;
                }
            }
            if (race_to_buy.size() == 0) //no race to buy
                return nullptr;

            if (race_price > unit_control::get_item_amount(parent, PRP_SILVER)) //not enough silver
                return nullptr;

            int newUnitId = land->GetNextNewUnitNo();
            CUnit* new_unit = gpApp->m_pAtlantis->SplitUnit(parent, newUnitId);

            if (game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_AUTONAMING)) {
                new_unit->Orders << "@name unit \"\"" << EOL_SCR;
                new_unit->Orders << "@;; $c" << EOL_SCR;
            } else {
                new_unit->Orders << "@name unit \"Scout\"" << EOL_SCR;
            }

            new_unit->Orders << "buy 1 " << race_to_buy.c_str() << EOL_SCR;

            auto order = orders::control::compose_give_order(new_unit, race_price, "SILV", "");
            orders::control::add_order_to_unit(order, parent);
            return new_unit;
        }
    }    

}