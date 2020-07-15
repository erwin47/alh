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
        if (unit_control::get_item_amount_by_mask(unit, PRP_MEN) == 0)
            return "000 $c";

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
        //war categories
        LightCavalry,
        HeavyCavalry,
        LightInfantry,
        HeavyInfantry,
        
        HeavyWarrior,
        Warrior,
        Guardsmen,

        WeakEquipment,
        StrongEquipment,        

        Bowmen,
        Crossbowmen,

        AmountHuge,
        AmountSmall,

        //peaceful categories
        Worker,
        Scout,
        Transport,

        //ships
        Sailors,

        Undefined
    };

    struct namingParameters
    {
        uint64_t is_guard_:1;
        uint64_t is_avoid_:1;
        uint64_t is_behind_:1;
        uint64_t is_ship_owner_:1;
        uint64_t is_big_:1;
        uint64_t is_small_:1;
        uint64_t is_one_:1;
        uint64_t is_heavy_armored_:1;
        uint64_t is_light_armored_:1;
        uint64_t is_heavy_shield_:1;
        uint64_t is_light_shield_:1;
        uint64_t is_heavy_weaponed_:1;
        uint64_t is_light_weaponed_:1;
        uint64_t is_archer_:1;
        uint64_t is_crossbowmen_:1;
        uint64_t is_flying_:1;
        uint64_t is_riding_:1;
        uint64_t is_transporting_:1;
        uint64_t reserve_:17;
    };


    //probably have to be uploaded from settings
    std::map<std::string, std::map<SquadTypes, std::string>> race_name_generators = {
        {
            "GBLN", {
                { SquadTypes::Bowmen, "Arrowshooters" },
                { SquadTypes::Crossbowmen, "Boltshooters" },
                { SquadTypes::Worker, "Hunkies" },
                { SquadTypes::Scout, "Sneaker" },
                { SquadTypes::Transport, "Pushcarters" },

                { SquadTypes::Guardsmen, "Gartt" },
                { SquadTypes::HeavyWarrior, "Gustyboiz" },
                { SquadTypes::Warrior, "Doughboiz" },
                { SquadTypes::WeakEquipment, "Braet" },
                { SquadTypes::StrongEquipment, "Ourmd" },                 
                { SquadTypes::AmountHuge, "Horde" },
                { SquadTypes::AmountSmall, "Bunch" },

                { SquadTypes::Sailors, "Gobsailors" }
            }
        },
        {
            "GNOL", {
                { SquadTypes::Bowmen, "Gnoll Bowmen" },
                { SquadTypes::Crossbowmen, "Gnoll Boltsmen" },
                { SquadTypes::Worker, "Gnolls" },
                { SquadTypes::Scout, "Gnoll" },
                { SquadTypes::Transport, "Gnolls Cartpushers" },

                { SquadTypes::Guardsmen, "Watchdogs" },
                { SquadTypes::HeavyWarrior, "Yeenoghu Fangs" },
                { SquadTypes::Warrior, "Hyenas" },
                { SquadTypes::WeakEquipment, "Paw" },
                { SquadTypes::StrongEquipment, "Spotted" },                 
                { SquadTypes::AmountHuge, "Flock" },
                { SquadTypes::AmountSmall, "Group" },

                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "ORC", {
                { SquadTypes::Bowmen, "Orcish Archers" },
                { SquadTypes::Crossbowmen, "Orcish Boltsmen" },
                { SquadTypes::Worker, "Peons" },
                { SquadTypes::Scout, "Peon" },
                { SquadTypes::Transport, "Roaming Peons" },

                { SquadTypes::Guardsmen, "Gruntguard" },
                { SquadTypes::HeavyWarrior, "Grunts" },
                { SquadTypes::Warrior, "Skulls" },
                { SquadTypes::WeakEquipment, "Ragged" },
                { SquadTypes::StrongEquipment, "Ironclad" },
                { SquadTypes::AmountHuge, "Horde" },
                { SquadTypes::AmountSmall, "Pack" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "WELF", {
                { SquadTypes::Bowmen, "Wood Marksmen" },
                { SquadTypes::Crossbowmen, "Wood Marksmen" },
                { SquadTypes::Worker, "Wood Tribe" },
                { SquadTypes::Scout, "Wood Elf" },
                { SquadTypes::Transport, "Roaming Tribe" },

                { SquadTypes::Guardsmen, "Elfish Guard" },
                { SquadTypes::HeavyWarrior, "Wood Lords" },
                { SquadTypes::Warrior, "Wood Rangers" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Shining" },                 
                { SquadTypes::AmountHuge, "Regiment" },
                { SquadTypes::AmountSmall, "Detachment" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "HELF", {
                { SquadTypes::Bowmen, "High Marksmen" },
                { SquadTypes::Crossbowmen, "High Marksmen" },
                { SquadTypes::Worker, "High Tribe" },
                { SquadTypes::Scout, "High Elf" },
                { SquadTypes::Transport, "Roaming Tribe" },

                { SquadTypes::Guardsmen, "Elfish Guard" },
                { SquadTypes::HeavyWarrior, "High Lords" },
                { SquadTypes::Warrior, "High Rangers" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Shining" },                 
                { SquadTypes::AmountHuge, "Regiment" },
                { SquadTypes::AmountSmall, "Detachment" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "DRLF", {
                { SquadTypes::Bowmen, "Drow Marksmen" },
                { SquadTypes::Crossbowmen, "Drow Marksmen" },
                { SquadTypes::Worker, "Drow Tribe" },
                { SquadTypes::Scout, "Drow Elf" },
                { SquadTypes::Transport, "Roaming Tribe" },

                { SquadTypes::Guardsmen, "Elfish Guard" },
                { SquadTypes::HeavyWarrior, "Drow Lords" },
                { SquadTypes::Warrior, "Drow Rangers" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Shining" },                 
                { SquadTypes::AmountHuge, "Regiment" },
                { SquadTypes::AmountSmall, "Detachment" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },        
        {
            "IDWA", {
                { SquadTypes::Bowmen, "Dwarfish Archers" },
                { SquadTypes::Crossbowmen, "Dwarfish Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Ice Dwarf" },
                { SquadTypes::Transport, "Appraisers" },

                { SquadTypes::Guardsmen, "Dwarfish Guard" },
                { SquadTypes::HeavyWarrior, "Longbeard Lords" },
                { SquadTypes::Warrior, "Longbeards" },
                { SquadTypes::WeakEquipment, "Nimble" },
                { SquadTypes::StrongEquipment, "Ice" },
                { SquadTypes::AmountHuge, "Hird" },
                { SquadTypes::AmountSmall, "Squad" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "HDWA", {
                { SquadTypes::Bowmen, "Dwarfish Archers" },
                { SquadTypes::Crossbowmen, "Dwarfish Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Hill Dwarf" },
                { SquadTypes::Transport, "Appraisers" },

                { SquadTypes::Guardsmen, "Dwarfish Guard" },
                { SquadTypes::HeavyWarrior, "Longbeard Lords" },
                { SquadTypes::Warrior, "Longbeards" },
                { SquadTypes::WeakEquipment, "Nimble" },
                { SquadTypes::StrongEquipment, "Steel" },                 
                { SquadTypes::AmountHuge, "Hird" },
                { SquadTypes::AmountSmall, "Squad" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "UDWA", {
                { SquadTypes::Bowmen, "Dwarfish Archers" },
                { SquadTypes::Crossbowmen, "Dwarfish Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Under Dwarf" },
                { SquadTypes::Transport, "Appraisers" },

                { SquadTypes::Guardsmen, "Dwarfish Guard" },
                { SquadTypes::HeavyWarrior, "Longbeard Lords" },
                { SquadTypes::Warrior, "Longbeards" },
                { SquadTypes::WeakEquipment, "Nimble" },
                { SquadTypes::StrongEquipment, "Mithril" },
                { SquadTypes::AmountHuge, "Hird" },
                { SquadTypes::AmountSmall, "Squad" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "GNOM", {
                { SquadTypes::Bowmen, "Half-tall Archers" },
                { SquadTypes::Crossbowmen, "Half-tall Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Gnome" },
                { SquadTypes::Transport, "Gnome Appraisers" },

                { SquadTypes::Guardsmen, "Gnomish Guard" },
                { SquadTypes::HeavyWarrior, "Shortbeard Lords" },
                { SquadTypes::Warrior, "Shortbeards" },
                { SquadTypes::WeakEquipment, "Nimble" },
                { SquadTypes::StrongEquipment, "Iron" },
                { SquadTypes::AmountHuge, "Mob" },
                { SquadTypes::AmountSmall, "Group" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },                              
        {
            "CTAU", {
                { SquadTypes::Bowmen, "Archers" },
                { SquadTypes::Crossbowmen, "Crossbowmen" },
                { SquadTypes::Worker, "Centaurs" },
                { SquadTypes::Scout, "Centaur" },
                { SquadTypes::Transport, "Roaming Herd" },

                { SquadTypes::Guardsmen, "Guard" },
                { SquadTypes::HeavyWarrior, "Hussars" },
                { SquadTypes::Warrior, "Hussars" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Heavy" },
                { SquadTypes::AmountHuge, "Herd" },
                { SquadTypes::AmountSmall, "Group" },
                { SquadTypes::Sailors, "Heavy Sailors" }
            }
        },
        {
            "HUMN", {
                { SquadTypes::Bowmen, "Archers" },
                { SquadTypes::Crossbowmen, "Crossbowmen" },
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Human" },
                { SquadTypes::Transport, "Peasants" },

                { SquadTypes::Guardsmen, "Guard" },
                { SquadTypes::HeavyWarrior, "Infantry" },
                { SquadTypes::Warrior, "Warriors" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Heavy" },
                { SquadTypes::AmountHuge, "Regiment" },
                { SquadTypes::AmountSmall, "Detachment" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
        {
            "LIZA", {
                { SquadTypes::Bowmen, "Swamp Archers" },
                { SquadTypes::Crossbowmen, "Swamp Crossbowmen" },
                { SquadTypes::Worker, "Swamp Peasants" },
                { SquadTypes::Scout, "Lizard" },
                { SquadTypes::Transport, "Swamp Peasants" },

                { SquadTypes::Guardsmen, "Oceanguard" },
                { SquadTypes::HeavyWarrior, "White Sharks" },
                { SquadTypes::Warrior, "Sharks" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Sharp-Toothed" },
                { SquadTypes::AmountHuge, "Swarm" },
                { SquadTypes::AmountSmall, "Pack" },
                { SquadTypes::Sailors, "Woodwalkers" }
            }
        },
        {
            "LEAD", {
                { SquadTypes::Bowmen, "Archers" },
                { SquadTypes::Crossbowmen, "Crossbowmen" },
                { SquadTypes::Worker, "Nobles" },
                { SquadTypes::Scout, "Noble" },
                { SquadTypes::Transport, "Nobles" },

                { SquadTypes::Guardsmen, "Knightguard" },
                { SquadTypes::HeavyWarrior, "Heavy Knights" },
                { SquadTypes::Warrior, "Knights" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Shining" },
                { SquadTypes::AmountHuge, "Regiment" },
                { SquadTypes::AmountSmall, "Detachment" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },        
        {
            "default", {
                { SquadTypes::Bowmen, "Archers" },
                { SquadTypes::Crossbowmen, "Crossbowmen" },              
                { SquadTypes::Worker, "Peasants" },
                { SquadTypes::Scout, "Scout" },
                { SquadTypes::Transport, "Carriers" },

                { SquadTypes::Guardsmen, "Guard" },
                { SquadTypes::HeavyWarrior, "Infantry" },
                { SquadTypes::Warrior, "Warrior" },
                { SquadTypes::WeakEquipment, "Light" },
                { SquadTypes::StrongEquipment, "Heavy" },
                { SquadTypes::AmountHuge, "Regiment" },
                { SquadTypes::AmountSmall, "Detachment" },
                { SquadTypes::Sailors, "Sailors" }
            }
        },
    };

    std::map<std::string, long> shield_weight {
        {"WSHD", 3},
        {"ISHD", 4},
        {"MSHD", 5},
    };

    std::map<std::string, long> armor_weight {
        {"CLAR", 1},
        {"LARM", 2},
        {"CARM", 3},
        {"PARM", 4},
        {"MARM", 5},
        {"ARNG", 6},
        {"AARM", 7},
        {"CLOA", 8},
    };

    std::map<std::string, long> weapon_weight {
        {"PICK", 1},
        {"HAMM", 1},
        {"AXE", 1},
        {"SPEA", 1},
        {"SWOR", 3},
        {"JAVE", 3},
        {"LANC", 4},
        {"PIKE", 4},
        {"MSWO", 5},
        {"ASWR", 6},
        {"FSWO", 10},
        {"RUNE", 11},
    };

    std::map<std::string, long> bow_weight {
        {"LBOW", 4},
        {"DBOW", 6},
    };

    std::map<std::string, long> xbow_weight {
        {"XBOW", 3},
        {"MXBO", 5},
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

    namingParameters get_naming_parameters(CUnit* unit)
    {
        namingParameters ret;
        memset(&ret, 0, sizeof(namingParameters));

        long men_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
        if (men_amount >= 100)
            ret.is_big_ = 1;
        else if (men_amount == 1)
            ret.is_one_ = 1;
        else if (men_amount < 10)
            ret.is_small_ = 1;

        if (unit_control::flags::is_avoid(unit))
            ret.is_avoid_ = 1;
        if (unit_control::flags::is_behind(unit))
            ret.is_behind_ = 1;
        if (unit_control::flags::is_guard(unit))
            ret.is_guard_ = 1;
        if (unit_control::is_struct_owner(unit) && unit_control::structure_id(unit) > 100)
            ret.is_ship_owner_ = 1;
        if (is_transport(men_amount, unit))
            ret.is_transporting_ = 1;
        
        unit_control::MoveMode movemode = unit_control::get_move_state(unit);
        if (movemode.speed_ == 4)
            ret.is_riding_ = 1;
        if (movemode.speed_ == 6)
            ret.is_flying_ = 1;
        
        long bow_score = deduct_equipment_weight(men_amount, unit, bow_weight);
        long xbow_score = deduct_equipment_weight(men_amount, unit, xbow_weight);
        long weap_score = deduct_equipment_weight(men_amount, unit, weapon_weight);
        long armor_score = deduct_equipment_weight(men_amount, unit, armor_weight);
        long shield_score = deduct_equipment_weight(men_amount, unit, shield_weight);
        if (bow_score > 0)
            ret.is_archer_ = 1;
        if (xbow_score > 0)
            ret.is_crossbowmen_ = 1;

        if (weap_score >= weapon_weight["MSWO"])
            ret.is_heavy_weaponed_ = 1;
        else if (weap_score >= weapon_weight["SWOR"])
            ret.is_light_weaponed_ = 1;

        if (armor_score >= armor_weight["MARM"])
            ret.is_heavy_armored_ = 1;
        else if (armor_score >= armor_weight["CARM"])
            ret.is_light_armored_ = 1;

        if (shield_score >= shield_weight["MSHD"])
            ret.is_heavy_shield_ = 1;
        else if (shield_score >= shield_weight["ISHD"])
            ret.is_light_shield_ = 1;

        return ret;
    }

    std::string name_generator(const std::string& race, namingParameters params)
    {
        std::string ret, adjective, noun, sizer;

        if (params.is_ship_owner_) {
            if (params.is_one_)
                noun = "Captain";
            else
                noun = race_name_generators[race][SquadTypes::Sailors];
        } else if (params.is_avoid_) {
            if (params.is_transporting_)
                noun = race_name_generators[race][SquadTypes::Transport];
            else if (params.is_one_)
                noun = race_name_generators[race][SquadTypes::Scout];
            else
                noun = race_name_generators[race][SquadTypes::Worker];            
        } else {
            if (params.is_guard_) {
                noun = race_name_generators[race][SquadTypes::Guardsmen];
                
                if (params.is_heavy_weaponed_ + params.is_heavy_shield_)
                    adjective = race_name_generators[race][SquadTypes::StrongEquipment];
                else if (params.is_light_weaponed_ + params.is_light_shield_ == 0)
                    adjective = race_name_generators[race][SquadTypes::WeakEquipment];
                
            } else if (params.is_crossbowmen_) {
                adjective = "";
                noun = race_name_generators[race][SquadTypes::Crossbowmen];
            } else if (params.is_archer_) {
                adjective = "";
                noun = race_name_generators[race][SquadTypes::Bowmen];
            } else if (params.is_one_) {
                adjective = "";
                noun = race_name_generators[race][SquadTypes::Scout];

            } else if (params.is_heavy_armored_) {
                noun = race_name_generators[race][SquadTypes::HeavyWarrior];

                if (params.is_heavy_weaponed_ + params.is_heavy_shield_)
                    adjective = race_name_generators[race][SquadTypes::StrongEquipment];
                else if (params.is_light_weaponed_ + params.is_light_shield_ == 0)
                    adjective = race_name_generators[race][SquadTypes::WeakEquipment];

            } else if (params.is_heavy_armored_) {
                noun = race_name_generators[race][SquadTypes::Warrior];

                if (params.is_heavy_weaponed_ + params.is_heavy_shield_)
                    adjective = race_name_generators[race][SquadTypes::StrongEquipment];
                else if (params.is_light_weaponed_ + params.is_light_shield_ == 0)
                    adjective = race_name_generators[race][SquadTypes::WeakEquipment];

            } else {
                adjective = "";
                noun = race_name_generators[race][SquadTypes::Worker];
            }

            if (params.is_small_) {
                sizer = race_name_generators[race][SquadTypes::AmountSmall];
            } else if (params.is_big_) {
                sizer = race_name_generators[race][SquadTypes::AmountHuge];
            }
        }

        if (adjective.size() > 0)
            ret = adjective + " ";
        ret += noun;
        if (sizer.size() > 0)
            ret += " " + sizer;
        return ret;
    }

    SquadTypes deduct_type(CUnit* unit) {
        long men_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
        if (men_amount == 1)
            return SquadTypes::Scout;

        long armor_points = deduct_equipment_weight(men_amount, unit, armor_weight);
        long weapon_points = deduct_equipment_weight(men_amount, unit, weapon_weight);
        long bow_points = deduct_equipment_weight(men_amount, unit, bow_weight);
        long xbow_points = deduct_equipment_weight(men_amount, unit, xbow_weight);

        if (weapon_points + armor_points < 2 ||
            unit_control::get_current_skill_days(unit, "COMB") < 30)
        {
            if (xbow_points >= 2)
                return SquadTypes::Crossbowmen;
            else if (bow_points >= 2)
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
        namingParameters params = get_naming_parameters(unit);

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
            race_name = "default";
            
        return name_generator(race_name, params);
        /*SquadTypes type = deduct_type(unit);
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

        return race_name_generators[race_name][type];*/
    }
}