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
        {"SAIL", "S"},
        {"RIDI", "X"},
        {"OBSE", "X"},
        {"STEA", "X"},
        {"TACT", "B"}
        
    };

    std::map<std::string, long> skill_type_weights = {
        {"S", 7},
        {"M", 6},
        {"B", 5},
        {"A", 4},
        {"X", 3}, 
        {"W", 2}
        //{"S", 1}
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

    void add_skill(std::string& skills_line, const std::string& skill, long skill_lvl, bool increasing = false)
    {
        skills_line.push_back(' ');
        for (const char& c : skill)
            skills_line.push_back(std::tolower(c));
        skills_line += std::to_string(skill_lvl);
        if (increasing)
            skills_line.push_back('+');
    }

    long get_skill_lvl(CUnit* unit, const std::string& skill_name)
    {
        long skill_days_initial(0);
        long skill_days(0);
        if (unit->skills_initial_.find(skill_name) != unit->skills_initial_.end())
            skill_days_initial = unit->skills_initial_[skill_name];
        if (unit->skills_.find(skill_name) != unit->skills_.end())
            skill_days = unit->skills_[skill_name];

        if (skill_days_initial + 60 < skill_days)
            return skills_control::get_skill_lvl_from_days(skill_days);//for case when skill was modified by giving unit, we should use modified variant
        else
            return skills_control::get_skill_lvl_from_days(skill_days_initial);
    }

    void get_highest_skill(CUnit* unit, std::string& highest, long& highest_lvl)
    {
        std::vector<std::pair<std::string, long>> sorted_skills(unit->skills_.begin(), unit->skills_.end());
        std::sort(sorted_skills.begin(), sorted_skills.end(), [&](const std::pair<std::string, long>& item1, 
                                                                  const std::pair<std::string, long>& item2) {
                return skill_weights[item1.first] > skill_weights[item2.first];                
            });

        highest_lvl = -1;
        for (auto& pair : sorted_skills)
        {
            if (skill_type_weights[skills_map[pair.first]] > skill_type_weights[skills_map[highest]])
            {
                highest_lvl = get_skill_lvl(unit, pair.first);
                highest = pair.first;
            }
            else if (skill_type_weights[skills_map[highest]] == skill_type_weights[skills_map[pair.first]] && 
                     skills_control::get_skill_lvl_from_days(pair.second) > highest_lvl)//this check should be done with current skill, not initial
            {
                highest_lvl = get_skill_lvl(unit, pair.first);
                highest = pair.first;
            }
        }
    }

    bool check_if_skill_is_increasing(CUnit* unit, const std::string& highest_skill)
    {
        long skill_days(0);
        long skill_days_initial(0);
        if (unit->skills_initial_.find(highest_skill) != unit->skills_initial_.end())
            skill_days_initial = unit->skills_initial_[highest_skill];
        if (unit->skills_.find(highest_skill) != unit->skills_.end())
            skill_days = unit->skills_[highest_skill];

        return skill_days_initial < skill_days && 
               skill_days_initial + 60 >= skill_days;
    }

    std::string generate_skill_line(CUnit* unit, const std::string& highest_skill, long highest_lvl)
    {
        std::vector<std::pair<std::string, long>> sorted_skills(unit->skills_.begin(), unit->skills_.end());
        std::sort(sorted_skills.begin(), sorted_skills.end(), [&](const std::pair<std::string, long>& item1, 
                                                                  const std::pair<std::string, long>& item2) {
                return skill_weights[item1.first] > skill_weights[item2.first];                
            });

        std::string skills_line;
        if (highest_lvl > -1)
        {

            add_skill(skills_line, highest_skill, highest_lvl, check_if_skill_is_increasing(unit, highest_skill));
            for (const auto& skill : sorted_skills)
            {
                if (skill.first == highest_skill)
                    continue;
                
                if (skill_type_weights[skills_map[skill.first]] > skill_type_weights["X"])//full skill
                    add_skill(skills_line, skill.first, get_skill_lvl(unit, skill.first), check_if_skill_is_increasing(unit, skill.first));
                else if (skill_type_weights[skills_map[skill.first]] == skill_type_weights["X"])//first letter
                    add_skill(skills_line, std::string(1, skill.first[0]), get_skill_lvl(unit, skill.first), check_if_skill_is_increasing(unit, skill.first));
                
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
        if (struct_id >= 100)
        {
            CStruct* structure = land_control::get_struct(land, struct_id);
            autogenerated_name << structure->type_.substr(0,1) << struct_id << " ";

            if (structure->capacity_ > 0)
                structure_capacity = std::to_string(structure->occupied_capacity_)+
                    "/"+std::to_string(structure->capacity_)+" ";
        }

        if (orders::autoorders_caravan::is_caravan(unit->orders_))
        {
            if (unit_control::structure_id(unit) >= 100)
                autogenerated_name << structure_capacity << "T";//caravan in the ship
            else if (highest_skill.size() > 0)
                autogenerated_name << "T" << highest_skill[0];
            else
                autogenerated_name << "T";            
        }            
        else if (highest_skill.size() > 0)
        {
            if (unit->Flags & UNIT_FLAG_PRODUCING)
                autogenerated_name << "P";
            else if (unit_control::is_leader(unit))
                autogenerated_name << "E";
            else if (unit_control::is_struct_owner(unit) && structure_capacity.size() > 0)//structure owners have structure capacity 
                autogenerated_name << structure_capacity;
            else
                autogenerated_name << skills_map[highest_skill];
            autogenerated_name << highest_skill[0];            
        }
        else if (unit_control::flags::is_working(unit))
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
                    highest_lvl = get_skill_lvl(unit, highest_skill);
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

    std::string generate_initial_unit_autoname(const std::string& race, 
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


    /**
     * Main conception of autonaming, is that name should not reflect anything meaningful & unknown to enemy, to not let
     * deduct any additional state of faction from the name of unit. That's why it should be based on just items and races
     */
    enum class SquadTypes
    {
        LightCavalry,
        HeavyCavalry,
        LightInfantry,
        HeavyInfantry,
        Bowmen,
        Crossbowmen,
        Worker,
        Scout,
        Transport,
        Undefined
    };

    //probably have to be uploaded from settings
    std::map<std::string, std::map<SquadTypes, std::string>> race_name_generators = {
        {
            "GBLN", {
                { SquadTypes::LightCavalry, "Gustyboys" },
                { SquadTypes::HeavyCavalry, "Braet Gustyboys" },
                { SquadTypes::LightInfantry, "Doughboys" },
                { SquadTypes::HeavyInfantry, "Braet Doughboys" },
                { SquadTypes::Bowmen, "Arrowshooters" },
                { SquadTypes::Crossbowmen, "Boltshooters" },
                { SquadTypes::Worker, "Hunkies" },
                { SquadTypes::Scout, "Sneaker" },
                { SquadTypes::Transport, "Pushcarters" },
            }
        },
        {
            "GNOL", {
                { SquadTypes::LightCavalry, "War Hyenas" },
                { SquadTypes::HeavyCavalry, "Heavy War Hyenas" },
                { SquadTypes::LightInfantry, "War Hyenas" },
                { SquadTypes::HeavyInfantry, "Heavy War Hyenas" },
                { SquadTypes::Bowmen, "Distance fighters" },
                { SquadTypes::Crossbowmen, "Distance fighters" },
                { SquadTypes::Worker, "Gnoll Flock" },
                { SquadTypes::Scout, "Sneaker" },
                { SquadTypes::Transport, "Gnoll Flock" },
            }
        },
        {
            "ORC", {
                { SquadTypes::LightCavalry, "Wolfriders" },
                { SquadTypes::HeavyCavalry, "Heavy wolfriders" },
                { SquadTypes::LightInfantry, "Grunts" },
                { SquadTypes::HeavyInfantry, "Heavy Grunts" },
                { SquadTypes::Bowmen, "Orcish Archers" },
                { SquadTypes::Crossbowmen, "Orcish Boltsmen" },
                { SquadTypes::Worker, "Orcs Flock" },
                { SquadTypes::Scout, "Sneaker" },
                { SquadTypes::Transport, "Orcs Flock" },
            }
        },
        {
            "WELF", {
                { SquadTypes::LightCavalry, "Wood Rangers" },
                { SquadTypes::HeavyCavalry, "Wood Lords" },
                { SquadTypes::LightInfantry, "Wood Rangers" },
                { SquadTypes::HeavyInfantry, "Wood Lords" },
                { SquadTypes::Bowmen, "Wood Marksmen" },
                { SquadTypes::Crossbowmen, "Wood Marksmen" },
                { SquadTypes::Worker, "Tribe" },
                { SquadTypes::Scout, "Scout" },
                { SquadTypes::Transport, "Roaming Tribe" },
            }
        },
        {
            "HELF", {
                { SquadTypes::LightCavalry, "High Rangers" },
                { SquadTypes::HeavyCavalry, "High Lords" },
                { SquadTypes::LightInfantry, "High Rangers" },
                { SquadTypes::HeavyInfantry, "High Lords" },
                { SquadTypes::Bowmen, "High Marksmen" },
                { SquadTypes::Crossbowmen, "High Marksmen" },
                { SquadTypes::Worker, "Tribe" },
                { SquadTypes::Scout, "Ranger" },
                { SquadTypes::Transport, "Roaming Tribe" },
            }
        },
        {
            "DRLF", {
                { SquadTypes::LightCavalry, "Drow Rangers" },
                { SquadTypes::HeavyCavalry, "Drow Lords" },
                { SquadTypes::LightInfantry, "Drow Rangers" },
                { SquadTypes::HeavyInfantry, "Drow Lords" },
                { SquadTypes::Bowmen, "Drow Marksmen" },
                { SquadTypes::Crossbowmen, "Drow Marksmen" },
                { SquadTypes::Worker, "Tribe" },
                { SquadTypes::Scout, "Ranger" },
                { SquadTypes::Transport, "Roaming Tribe" },
            }
        },        
        {
            "IDWA", {
                { SquadTypes::LightCavalry, "Longbeards" },
                { SquadTypes::HeavyCavalry, "Storm Hird" },
                { SquadTypes::LightInfantry, "Longbeards" },
                { SquadTypes::HeavyInfantry, "Hird" },
                { SquadTypes::Bowmen, "Dwarfish Archers" },
                { SquadTypes::Crossbowmen, "Dwarfish Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Dwarf" },
                { SquadTypes::Transport, "Dwarfish Appraisers" },
            }
        },
        {
            "HDWA", {
                { SquadTypes::LightCavalry, "Longbeards" },
                { SquadTypes::HeavyCavalry, "Storm Hird" },
                { SquadTypes::LightInfantry, "Longbeards" },
                { SquadTypes::HeavyInfantry, "Hird" },
                { SquadTypes::Bowmen, "Dwarfish Archers" },
                { SquadTypes::Crossbowmen, "Dwarfish Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Dwarf" },
                { SquadTypes::Transport, "Dwarfish Appraisers" },
            }
        },
        {
            "UDWA", {
                { SquadTypes::LightCavalry, "Longbeards" },
                { SquadTypes::HeavyCavalry, "Storm Hird" },
                { SquadTypes::LightInfantry, "Longbeards" },
                { SquadTypes::HeavyInfantry, "Hird" },
                { SquadTypes::Bowmen, "Dwarfish Archers" },
                { SquadTypes::Crossbowmen, "Dwarfish Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Dwarf" },
                { SquadTypes::Transport, "Dwarfish Appraisers" },
            }
        },
        {
            "GNOM", {
                { SquadTypes::LightCavalry, "Muleriders" },
                { SquadTypes::HeavyCavalry, "Heavy muleriders" },
                { SquadTypes::LightInfantry, "Sneaky Warriors" },
                { SquadTypes::HeavyInfantry, "Sneaky Longbeards" },
                { SquadTypes::Bowmen, "Half-tall Archers" },
                { SquadTypes::Crossbowmen, "Half-tall Crossbowmen" },
                { SquadTypes::Worker, "Gnomish Mob" },
                { SquadTypes::Scout, "Gnome" },
                { SquadTypes::Transport, "Appraisers" },
            }
        },                              
        {
            "CTAU", {
                { SquadTypes::LightCavalry, "Hussars" },
                { SquadTypes::HeavyCavalry, "Heavy hussars" },
                { SquadTypes::LightInfantry, "Hussars" },
                { SquadTypes::HeavyInfantry, "Heavy hussars" },
                { SquadTypes::Bowmen, "Archers" },
                { SquadTypes::Crossbowmen, "Crossbowmen" },
                { SquadTypes::Worker, "Herd" },
                { SquadTypes::Scout, "Centaur" },
                { SquadTypes::Transport, "Roaming Herd" },
            }
        },
        {
            "HUMN", {
                { SquadTypes::LightCavalry, "Cavalry" },
                { SquadTypes::HeavyCavalry, "Heavy Cavalry" },
                { SquadTypes::LightInfantry, "Infantry" },
                { SquadTypes::HeavyInfantry, "Heavy infantry" },
                { SquadTypes::Bowmen, "Archers" },
                { SquadTypes::Crossbowmen, "Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Human" },
                { SquadTypes::Transport, "Peasants" },
            }
        },
        {
            "LIZA", {
                { SquadTypes::LightCavalry, "Oceanriders" },
                { SquadTypes::HeavyCavalry, "Heavy oceanriders" },
                { SquadTypes::LightInfantry, "Oceanfighters" },
                { SquadTypes::HeavyInfantry, "Heavy oceanfighters" },
                { SquadTypes::Bowmen, "Swamp Archers" },
                { SquadTypes::Crossbowmen, "Swamp Crossbowmen" },
                { SquadTypes::Worker, "Swamp Peasants" },
                { SquadTypes::Scout, "Lizard" },
                { SquadTypes::Transport, "Swamp Peasants" },
            }
        },
        {
            "LEAD", {
                { SquadTypes::LightInfantry, "Elite Infantry" },
                { SquadTypes::HeavyInfantry, "Iron Fist Infantry" },
                { SquadTypes::LightCavalry, "Knights" },
                { SquadTypes::HeavyCavalry, "Heavy Knights" },
                { SquadTypes::Worker, "Nobles" },
                { SquadTypes::Scout, "Noble" },
                { SquadTypes::Transport, "Nobles" },
            }
        },        
        {
            "default", {
                { SquadTypes::LightInfantry, "Infantry" },
                { SquadTypes::HeavyInfantry, "Heavy Infantry" },
                { SquadTypes::LightCavalry, "Cavalry" },
                { SquadTypes::HeavyCavalry, "Heavy Cavalry" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Scout" },
                { SquadTypes::Transport, "Carriers" },
            }
        },
    };

    std::map<std::string, long> armor_weight {
        {"CLAR", 1},
        {"LARM", 2},
        {"CARM", 3},
        {"WSHD", 3},
        {"PARM", 4},
        {"ISHD", 5},
        {"MARM", 5},
        {"ARNG", 6},
        {"MSHD", 6},
        {"AARM", 7},
        {"CLOA", 8},
    };

    std::map<std::string, long> weapon_weight {
        {"PICK", 1},
        {"HAMM", 1},
        {"AXE", 1},
        {"SPEA", 1},
        {"SWOR", 3},
        {"LANC", 4},
        {"PIKE", 4},
        {"MSWO", 5},
        {"ASWR", 6},
        {"FSWO", 10},
        {"RUNE", 11},
    };

    std::map<std::string, long> bow_weight {
        {"JAVE", 3},
        {"LBOW", 3},
        {"XBOW", 3},
        {"MXBO", 5},
        {"DBOW", 6},
    };


    long deduct_equipment_weight(long men_amount, CUnit* unit, const std::map<std::string, long>& weights) 
    {
        long res = 0;
        for (auto& item : weights)
        {
            if (unit_control::get_item_amount(unit, item.first)*2 > men_amount && item.second > res)
                res = item.second;
        }
        return res;
    }

    bool is_transport(long men_amount, CUnit* unit) 
    {
        if (unit_control::get_item_amount(unit, "WAGO") * 10 > men_amount)
            return true;
        if (unit_control::get_item_amount(unit, "MWAG") * 20 > men_amount)
            return true;
        if (unit_control::get_item_amount(unit, "HORS") +
            unit_control::get_item_amount(unit, "CAME") + 
            unit_control::get_item_amount(unit, "WING") +
            unit_control::get_item_amount(unit, "GLID")/2 >= men_amount*2)
            return true;
        return false;
    }

    bool is_heavy(long weap_weight, long arm_weight)
    {
        if (arm_weight > armor_weight["MARM"])
                return true;
        return false;
    }

    bool is_cavalry(CUnit* unit)
    {
        unit_control::MoveMode movemode = unit_control::get_move_state(unit);
        if (movemode.speed_ >= 4 && unit_control::get_current_skill_days(unit, "RIDI") > 0)
            return true;
        return false;
    }

    SquadTypes deduct_type(CUnit* unit) {
        long men_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
        if (men_amount == 1)
            return SquadTypes::Scout;

        long armor_points = deduct_equipment_weight(men_amount, unit, armor_weight);
        long weapon_points = deduct_equipment_weight(men_amount, unit, weapon_weight);
        long bow_points = deduct_equipment_weight(men_amount, unit, bow_weight);

        if (weapon_points + armor_points < 2 ||
            unit_control::get_current_skill_days(unit, "COMB") < 30)
        {
            if (bow_points >= 2 && 
                unit_control::get_current_skill_days(unit, "XBOW") >= 30)
                return SquadTypes::Crossbowmen;
            else if (bow_points >= 2 && 
                     unit_control::get_current_skill_days(unit, "LBOW") >= 30)
                return SquadTypes::Bowmen;
            else if (is_transport(men_amount, unit))
                return SquadTypes::Transport;
            else
                return SquadTypes::Worker;
        }

        if (is_heavy(weapon_points, armor_points))
        {
            if (is_cavalry(unit))
                return SquadTypes::HeavyCavalry;
            else
                return SquadTypes::HeavyInfantry;
        }
        else
        {
            if (is_cavalry(unit))
                return SquadTypes::LightCavalry;
            else
                return SquadTypes::LightInfantry;
        }
    }

    std::string generate_unit_name(CLand* land, CUnit* unit)
    {
        SquadTypes type = deduct_type(unit);
        auto races = unit_control::get_all_items_by_mask(unit, PRP_MEN);
        std::string race_name;
        long men_amount = 0;
        for (auto race : races)
        {
            if (race.amount_ > men_amount)
            {
                men_amount = race.amount_;
                race_name = race.code_name_;
            }
        }
        if (race_name_generators.find(race_name) == race_name_generators.end())
            return race_name_generators["default"][type];

        return race_name_generators[race_name][type];
    }
}