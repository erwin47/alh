#include "autonaming.h"

#include "data_control.h"
#include "ah_control.h"
#include <sstream>

namespace autonaming
{
    std::map<std::string, std::string> skills_map = {
        {"MANI", "A"},
        {"COMB", "A"},
        {"ENDU", "X"},
        {"HEAL", "A"},
        {"LBOW", "A"},
        {"XBOW", "A"},
        {"ARMO", "M"},
        {"BUIL", "M"},
        {"CARP", "M"},
        {"FARM", "M"},
        {"FISH", "M"},
        {"GCUT", "M"},
        {"HERB", "M"},
        {"MINI", "M"},
        {"HORS", "M"},
        {"HUNT", "M"},
        {"LUMB", "M"},
        {"WEAP", "M"},
        {"QUAM", "M"},
        {"QUAR", "M"},
        {"SHIP", "M"},
        {"ENTE", "W"},
        {"SAIL", "SAIL"},
        {"RIDI", "X"},
        {"OBSE", "X"},
        {"STEA", "X"},
        {"TACT", "B"}
        
    };

    std::map<std::string, long> skill_type_weights = {
        {"SAIL", 7},
        {"M", 6},
        {"B", 5},
        {"A", 4},
        {"X", 3}, 
        {"W", 2},
        {"S", 1}
    };

    std::map<std::string, long> skill_weights = {
        {"TACT", 100},
        {"MANI", 80},
        {"QUAM", 60},
        {"MINI", 46},            
        {"ARMO", 45},
        {"BUIL", 44},
        {"CARP", 43},
        {"FARM", 42},
        {"FISH", 41},
        {"GCUT", 40},
        {"HERB", 39},
        {"HORS", 38},
        {"HUNT", 37},
        {"LUMB", 36},
        {"WEAP", 35},
        {"QUAR", 34},
        {"SHIP", 33},
        {"SAIL", 20},
        {"COMB", 15},
        {"ENDU", 14},
        {"LBOW", 13},
        {"XBOW", 12},
        {"HEAL", 11},
        {"OBSE", 10},
        {"STEA", 9},
        {"RIDI", 8},
        {"ENTE", 5},
    };

    void add_skill(std::string& skills_line, const std::string& skill, long skill_lvl)
    {
        skills_line.push_back(' ');
        for (const char& c : skill)
            skills_line.push_back(std::tolower(c));
        skills_line += std::to_string(skill_lvl);        
    }

    void get_highest_skill(CUnit* unit, std::string& highest, long& highest_lvl)
    {
        std::vector<std::pair<std::string, long>> sorted_skills(unit->skills_initial_.begin(), unit->skills_initial_.end());
        std::sort(sorted_skills.begin(), sorted_skills.end(), [&](const std::pair<std::string, long>& item1, 
                                                                  const std::pair<std::string, long>& item2) {
                return skill_weights[item1.first] > skill_weights[item2.first];                
            });

        highest_lvl = -1;
        for (auto& pair : sorted_skills)
        {
            if (skill_type_weights[skills_map[pair.first]] > skill_type_weights[skills_map[highest]])
            {
                highest_lvl = skills_control::get_skill_lvl_from_days(pair.second);
                highest = pair.first;
            }
            else if (skill_type_weights[skills_map[highest]] == skill_type_weights[skills_map[pair.first]] && 
                     skills_control::get_skill_lvl_from_days(pair.second > highest_lvl))
            {
                highest_lvl = skills_control::get_skill_lvl_from_days(pair.second);
                highest = pair.first;
            }
        }
    }

    std::string generate_skill_line(CUnit* unit, const std::string& highest_skill, long highest_lvl)
    {
        std::vector<std::pair<std::string, long>> sorted_skills(unit->skills_initial_.begin(), unit->skills_initial_.end());
        std::sort(sorted_skills.begin(), sorted_skills.end(), [&](const std::pair<std::string, long>& item1, 
                                                                  const std::pair<std::string, long>& item2) {
                return skill_weights[item1.first] > skill_weights[item2.first];                
            });

        std::string skills_line;
        if (highest_lvl > -1)
        {
            add_skill(skills_line, highest_skill, highest_lvl);
            for (const auto& skill : sorted_skills)
            {
                if (skill.first == highest_skill)
                    continue;
                
                if (skill_type_weights[skills_map[skill.first]] > skill_type_weights["X"])//full skill
                    add_skill(skills_line, skill.first, skills_control::get_skill_lvl_from_days(skill.second));
                else if (skill_type_weights[skills_map[skill.first]] == skill_type_weights["X"])//first letter
                    add_skill(skills_line, std::string(1, skill.first[0]), skills_control::get_skill_lvl_from_days(skill.second));
                
                //ignore skills with weight below X
            }
        } 
        return skills_line;                
    }

    std::string extract_highest_skill_from_name(const std::string& existing_name)
    {
        size_t pos = existing_name.find(']');
        if (pos == std::string::npos)
            return "";

        while (pos < existing_name.size() && !std::isalpha(existing_name[pos]))
            ++pos;

        size_t pos_check = pos;
        while(pos_check < existing_name.size() && std::isalpha(existing_name[pos_check]))
            ++pos_check;

        if (pos == existing_name.size() || pos_check - pos != 4)
            return "";
        return existing_name.substr(pos, 4);
    }

    std::string get_type(CLand* land, CUnit* unit, const std::string& highest_skill)
    {
        std::stringstream autogenerated_name;
        std::string structure_capacity;

        autogenerated_name << "[";
        long struct_id = unit_control::structure_id(unit);
        if (struct_id > 0)
        {
            CStruct* structure = land_control::get_struct(land, struct_id);
            autogenerated_name << structure->type_.substr(0,3) << struct_id;
            autogenerated_name << " ";

            if (structure->capacity_ > 0)
                structure_capacity = std::to_string(structure->capacity_);
        }

        if (orders::autoorders_caravan::is_caravan(unit->orders_))
            autogenerated_name << "T";
        else if (highest_skill.size() > 0)
        {
            if (unit->Flags & UNIT_FLAG_PRODUCING)
                autogenerated_name << "P";
            else if (unit_control::is_leader(unit))
                autogenerated_name << "E";
            else if (unit_control::structure_id(unit) >= 100 &&
                     stricmp(highest_skill.c_str(), "SAIL") == 0)
                autogenerated_name << structure_capacity;//sailors inside a fleet has fleet's capacity
            else 
            {
                if (unit_control::is_struct_owner(unit))//structure owners always have structure capacity
                    autogenerated_name << structure_capacity << " ";
                autogenerated_name << skills_map[highest_skill];
            }
            autogenerated_name << highest_skill[0];            
        }
        else if (unit->Flags & UNIT_FLAG_WORKING)
            autogenerated_name << "W";
        else if (unit_control::is_leader(unit))
            autogenerated_name << "LEAD";
        else
        {//get max populated race
            std::string chosen_race;
            long pop(-1);
            for (auto& race : unit->men_)
            {
                if (race.amount_ > pop)
                {
                    pop = race.amount_;
                    chosen_race = race.code_name_;
                }
            }
            autogenerated_name << chosen_race;
        }
        autogenerated_name << "]";
        return autogenerated_name.str();
    }

    std::string generate_unit_autoname(CLand* land, CUnit* unit)
    {
        std::string type;
        std::string skills_line;
        std::string highest_skill;
        long highest_lvl(-1);
        if (orders::control::has_orders_with_type(orders::Type::O_COMMENT_AUTONAME, unit->orders_));
        {
            auto name_orders = orders::control::retrieve_orders_by_type(orders::Type::O_COMMENT_AUTONAME, unit->orders_);
            if (name_orders.size() > 0)
            {
                highest_skill = extract_highest_skill_from_name(name_orders[0]->comment_);
                if (highest_skill.size() > 0)
                {
                    highest_lvl = skills_control::get_skill_lvl_from_days(unit_control::get_current_skill_days(unit, highest_skill));
                    if (highest_lvl > 0)
                    {
                        type = get_type(land, unit, highest_skill);
                        skills_line = generate_skill_line(unit, highest_skill, highest_lvl);
                        return type + skills_line + " $c";
                    }                  
                }
            }
        }
        get_highest_skill(unit, highest_skill, highest_lvl);
        type = get_type(land, unit, highest_skill);
        skills_line = generate_skill_line(unit, highest_skill, highest_lvl);
        return type + skills_line + " $c";
    }

    std::string generate__initial_unit_autoname(const std::string& race, 
                                                const std::string& skill)
    {
        std::stringstream res;
        res << "[";
        if (skill.size() > 0)
        {
            if (race == "LEAD" || race == "HERO")
            {
                res << "E";
            }                
            else
            {
                res << skills_map[skill];
            }                
            std::string lc_skill(skill);
            std::transform(lc_skill.begin(), lc_skill.end(), lc_skill.begin(), [](char c) {
                return std::tolower(c);
            });
            res << char(std::toupper(skill[0])) << "] " << lc_skill;
        }
        else
        {
            res << race << "]";
        }     
        res << " $c";
        return res.str();
    }
}