
#include "consts.h"
#include "data_control.h"
#include <algorithm>
#include <sstream>
#include <cctype> //for std::tolower
#include "ah_control.h"

namespace item_control
{
    std::string codename(const std::string& name)
    {
        std::string code, long_name, plural_name;
        if (gpApp->ResolveAliasItems(name, code, long_name, plural_name))
            return code;
        return name;
    }

    int weight(const std::string& item_code) 
    {
        std::vector<long> vars = game_control::get_game_config<long>(SZ_SECT_WEIGHT_MOVE, item_code.c_str());
        return vars[0];
    }

    CItem get_by_code(const std::set<CItem>& items, const std::string& code_name)
    {
        auto it = std::find_if(items.begin(), items.end(), [&code_name](const CItem& prod) {
            return code_name == prod.code_name_;
        });
        if (it != items.end())
            return *it;

        return CItem({0, code_name.c_str()});
    }

    void modify_amount(std::set<CItem>& items, const std::string& codename, long new_amount)
    {
        if (new_amount == 0)
            return;

        CItem item = get_by_code(items, codename);
        item.amount_ += new_amount;
        items.erase(item);
        if (item.amount_ != 0)
            items.insert(item);
    }

    void item_to_stringstream(std::stringstream& ss, const CItem& item)
    {
        if (item.amount_ == 1)
        {
            std::string code_name, long_name, long_name_plural;
            gpDataHelper->ResolveAliasItems(item.code_name_, code_name, long_name, long_name_plural);
            ss << long_name << " [" << item.code_name_ << "]";
        }
        else if (item.amount_ > 1 || item.amount_ < 0)
        { //if items below zero, it's also interesting
            std::string code_name, long_name, long_name_plural;
            gpDataHelper->ResolveAliasItems(item.code_name_, code_name, long_name, long_name_plural);
            ss << std::to_string(item.amount_) << " " << long_name_plural << " [" << item.code_name_ << "]";
        }
    }

    void items_to_stringstream(std::stringstream& ss, const std::set<CItem>& items)
    {
        bool first_element = true;
        for (const auto& item : items)
        {
            if (item.amount_ == 0)
                continue;
            
            if (first_element)
                first_element = false;
            else
                ss << ", ";

            item_to_stringstream(ss, item);
        }
    }
}

namespace unit_control
{
    namespace flags
    {
        bool is_behind(CUnit* unit) {  return unit->Flags & UNIT_FLAG_BEHIND;  }
        bool is_guard(CUnit* unit) {  return unit->Flags & UNIT_FLAG_GUARDING;  }
        bool is_hold(CUnit* unit) {  return unit->Flags & UNIT_FLAG_HOLDING;  }
        bool is_noaid(CUnit* unit) {  return unit->Flags & UNIT_FLAG_RECEIVING_NO_AID;  }
        bool is_avoid(CUnit* unit) {  return unit->Flags & UNIT_FLAG_AVOIDING;  }
        bool is_nocross(CUnit* unit) {  return unit->Flags & UNIT_FLAG_NO_CROSS_WATER;  }
        bool is_sharing(CUnit* unit) {  return unit->Flags & UNIT_FLAG_SHARING;  }
        bool is_reveal(CUnit* unit, std::string flag) {
            for(size_t i = 0; i < flag.size(); ++i)
                flag[i] = std::tolower(flag[i]);
            if (!flag.compare("unit"))
                return unit->Flags & UNIT_FLAG_REVEALING_UNIT;
            if (!flag.compare("faction"))
                return unit->Flags & UNIT_FLAG_REVEALING_FACTION;
            return false;
        }
        bool is_spoils(CUnit* unit, const std::string flag) {
            if (stricmp("none", flag.c_str()) == 0)
                return unit->Flags & UNIT_FLAG_SPOILS_NONE;
            else if (stricmp("walk", flag.c_str()) == 0)
                return unit->Flags & UNIT_FLAG_SPOILS_WALK;
            else if (stricmp("ride", flag.c_str()) == 0)
                return unit->Flags & UNIT_FLAG_SPOILS_RIDE;
            else if (stricmp("fly", flag.c_str()) == 0)
                return unit->Flags & UNIT_FLAG_SPOILS_FLY;
            else if (stricmp("swim", flag.c_str()) == 0)
                return unit->Flags & UNIT_FLAG_SPOILS_SWIM;
            else if (stricmp("sail", flag.c_str()) == 0)
                return unit->Flags & UNIT_FLAG_SPOILS_SAIL;
            else if (stricmp("all", flag.c_str()) == 0)
                return unit->Flags ^ UNIT_FLAG_SPOILS_NONE &&
                       unit->Flags ^ UNIT_FLAG_SPOILS_WALK &&
                       unit->Flags ^ UNIT_FLAG_SPOILS_RIDE &&
                       unit->Flags ^ UNIT_FLAG_SPOILS_FLY &&
                       unit->Flags ^ UNIT_FLAG_SPOILS_SWIM &&
                       unit->Flags ^ UNIT_FLAG_SPOILS_SAIL;

            // not implemented
            return false;
        }

        bool is_consume(CUnit* unit, std::string flag) {
            for(size_t i = 0; i < flag.size(); ++i)
                flag[i] = std::tolower(flag[i]);
            if (!flag.compare("unit"))
                return unit->Flags & UNIT_FLAG_CONSUMING_UNIT;
            if (!flag.compare("faction"))
                return unit->Flags & UNIT_FLAG_CONSUMING_FACTION;
            return false;
        }
    }

    bool is_leader(CUnit* unit)
    {
        EValueType         type;
        const char       * leadership;
        return unit->GetProperty(PRP_LEADER, type, (const void *&)leadership, eNormal) == TRUE && 
               eCharPtr==type &&
               (0==strcmp(leadership, SZ_LEADER) || 0==strcmp(leadership, SZ_HERO));        
    }


    long structure_id(const CUnit* unit)
    {
        return unit->struct_id_ & 0x0000FFFF;
    }

    bool is_struct_owner(const CUnit* unit)
    {
        return unit->struct_id_ & 0xFFFF0000 ? true : false;
    }
    void set_structure(CUnit* unit, long struct_id, bool owns)
    {
        unit->struct_id_ = struct_id;
        if (owns)
            unit->struct_id_ |= 0x00010000;
    }

    long get_upkeep(CUnit* unit) 
    {
        long man_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
        if (is_leader(unit))
            return man_amount*game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_LEADER);
        else
            return man_amount*game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_PEASANT);
    }

    void get_weights(CUnit* unit, long weights[5])
    {
        int *cur_weights;
        const char **movenames;
        int movecount;

        weights[0] = 0;//weight
        weights[1] = 0;//move
        weights[2] = 0;//ride
        weights[3] = 0;//fly
        weights[4] = 0;//swim
        std::set<CItem> items = get_all_items(unit);

        static std::vector<std::string> wagon_list = game_control::get_game_config<std::string>(SZ_SECT_COMMON, SZ_KEY_WAGONS);
        static std::vector<std::string> wagon_puller_list = game_control::get_game_config<std::string>(SZ_SECT_COMMON, SZ_KEY_WAGON_PULLERS);
        static long wagon_capacity = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_WAGON_CAPACITY);

        long wagons_amount(0), pullers_amount(0);
        for (const CItem& item : items)
        {
            if (std::find(wagon_puller_list.begin(), wagon_puller_list.end(), item.code_name_) != wagon_puller_list.end())
                pullers_amount = item.amount_;
            if (std::find(wagon_list.begin(), wagon_list.end(), item.code_name_) != wagon_list.end())
                wagons_amount = item.amount_;
            gpApp->GetItemWeights(item.code_name_.c_str(), cur_weights, movenames, movecount);
            weights[0] += cur_weights[0] * item.amount_;
            weights[1] += cur_weights[1] * item.amount_;
            weights[2] += cur_weights[2] * item.amount_;
            weights[3] += cur_weights[3] * item.amount_;
            weights[4] += cur_weights[4] * item.amount_;
        }
        //hors gives +70, weight -50, wagon weight -50, which was already calculated. 
        //But together for moving they give 200. So for each pair we add 230.
        weights[1] += std::min(wagons_amount, pullers_amount) * wagon_capacity;
    }

    std::set<CItem> get_all_items(CUnit* unit)
    {
        std::set<CItem> ret;
        ret.insert(unit->men_.begin(), unit->men_.end());
        ret.insert(unit->items_.begin(), unit->items_.end());
        ret.insert(unit->silver_);
        return ret;
    }

    std::set<CItem> get_all_items_by_mask(CUnit* unit, const char* mask)
    {
        std::set<CItem> ret;
        std::set<CItem>::iterator it;
        CItem searching_key;
        std::vector<std::string> items = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, mask);        
        for (const std::string& item : items)
        {
            searching_key.code_name_ = item;
            it = unit->men_.find(searching_key);
            if (it != unit->men_.end())
                ret.insert(*it);

            it = unit->items_.find(searching_key);
            if (it != unit->items_.end())
                ret.insert(*it);

            if (item == unit->silver_.code_name_)
                ret.insert(unit->silver_);
        }        
        return ret;
    }

    long get_item_amount(CUnit* unit, const std::string& codename, bool initial)
    {
        if (initial)
        {
            if (codename == PRP_SILVER)
                return unit->silver_initial_.amount_;
            else if (gpApp->IsMan(codename.c_str()))
                return item_control::get_by_code(unit->men_initial_, codename).amount_;
            else
                return item_control::get_by_code(unit->items_initial_, codename).amount_;
        }
        else
        {
            if (codename == PRP_SILVER)
                return unit->silver_.amount_;
            else if (gpApp->IsMan(codename.c_str()))
                return item_control::get_by_code(unit->men_, codename).amount_;
            else
                return item_control::get_by_code(unit->items_, codename).amount_;
        }
    }
    
    long get_item_amount_by_mask(CUnit* unit, const char* mask)
    {
        long amount(0);
        std::set<CItem>::iterator it;
        CItem searching_key;
        std::vector<std::string> items = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, mask);        
        for (const std::string& item : items)
        {
            searching_key.code_name_ = item;
            it = unit->men_.find(searching_key);
            if (it != unit->men_.end())
                amount += it->amount_;

            it = unit->items_.find(searching_key);
            if (it != unit->items_.end())
                amount += it->amount_;

            if (item == unit->silver_.code_name_)
                amount += it->amount_;
        }
        return amount;   
    }

    std::string compose_unit_name(CUnit* unit)
    {
        if (unit == nullptr)
            return "general";
        std::string res;
        res.reserve(128);
        res.append(std::string(unit->Name.GetData(), unit->Name.GetLength()));
        res.append("(");
        res.append(compose_unit_number(unit->Id));
        res.append(")");
        return res;
    }

    std::string compose_unit_number(long number)
    {
        if (IS_NEW_UNIT_ID(number))
            return std::string("NEW ") + std::to_string(REVERSE_NEW_UNIT_ID(number));
        return std::to_string(number);
    }

    bool modify_item_property(CUnit* unit, const std::string& item) 
    {
        //old properties support
        if (PE_OK!=unit->SetProperty(item.c_str(), 
                                    eLong, 
                                    (const void *)unit_control::get_item_amount(unit, item, true),
                                    eOriginal) ||
            PE_OK!=unit->SetProperty(item.c_str(), 
                                    eLong, 
                                    (const void *)unit_control::get_item_amount(unit, item),
                                    eNormal))
            return false;
        return true;
    }

    void modify_silver(CUnit* unit, long new_amount, const std::string& reason)
    {
        if (new_amount == 0)
            return;

        unit->silver_.amount_ += new_amount;

        std::stringstream ss;
        if (new_amount > 0)
            ss << "gets " << new_amount << " of silver from " << reason;
        else
            ss << "loses " << new_amount << " of silver to " << reason;
        unit->impact_description_.push_back(ss.str());
        modify_item_property(unit, PRP_SILVER);        
    }

    void modify_item_by_produce(CUnit* unit, const std::string& codename, long new_amount)
    {
        if (new_amount == 0)
            return;
        
        if (gpDataHelper->ImmediateProdCheck())
        {
            item_control::modify_amount(unit->items_, codename, new_amount);
        }        

        std::stringstream ss;
        if (new_amount > 0)
            ss << "produce: " << new_amount << " " << codename;
        else
            ss << "spent: " << abs(new_amount) << " " << codename << " for production";

        unit->impact_description_.push_back(ss.str());
        //properties support
        modify_item_property(unit, codename);
        unit->CalcWeightsAndMovement();        
    }

    void modify_item_by_reason(CUnit* unit, const std::string& codename, long new_amount, const std::string& reason)
    {
        if (new_amount == 0)
            return;

        if (stricmp(codename.c_str(), PRP_SILVER) == 0)
            unit->silver_.amount_ += new_amount;
        else if (gpApp->IsMan(codename.c_str()))
            item_control::modify_amount(unit->men_, codename, new_amount);
        else 
            item_control::modify_amount(unit->items_, codename, new_amount);

        //pretty print
        std::string code, name, plural;
        if (!gpApp->ResolveAliasItems(codename, code, name, plural))
        {
            name = codename;
            plural = codename;
        }

        std::stringstream ss;
        if (new_amount > 0)
            ss << "got " << new_amount << " " << (abs(new_amount) == 1 ? name : plural) << " " << reason;
        else
            ss << "lost " << abs(new_amount) << " " << (abs(new_amount) == 1 ? name : plural) << " " << reason;

        unit->impact_description_.push_back(ss.str());

        //properties support
        modify_item_property(unit, codename);
        unit->CalcWeightsAndMovement();
    }

    void modify_item_from_market(CUnit* unit, const std::string& codename, long new_amount, long price)
    {
        if (new_amount == 0)
            return;

        unit->silver_.amount_ += -new_amount*price;
        item_control::modify_amount(unit->items_, codename, new_amount);

        std::stringstream ss;
        if (new_amount > 0)
            ss << "buy " << new_amount << " of " << codename << " by " << price << "$ per each";
        else
            ss << "sell " << abs(new_amount) << " of " << codename << " for " << price << "$ per each";

        unit->impact_description_.push_back(ss.str());

        modify_item_property(unit, PRP_SILVER);
        modify_item_property(unit, codename);
        unit->CalcWeightsAndMovement();        
    }

    void modify_man_from_market(CUnit* unit, const std::string& codename, long new_amount, long price)
    {
        if (new_amount == 0)
            return;

        unit->silver_.amount_ += -new_amount*price;
        item_control::modify_amount(unit->men_, codename, new_amount);

        std::stringstream ss;
        if (new_amount < 0)
        {//assuming we can sell peasants, but we actually can't
            ss << "sell " << abs(new_amount) << " of " << codename << " for " << price << "$ per each";
        }
        else //new_amount > 0
        {
            long current_man_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
            //for (const auto& nation : unit->men_)
            //    current_man_amount += nation.amount_;

            //calculate days of knowledge according to new amount of peasants
            for (auto& skill: unit->skills_)
                skill.second = skill.second * current_man_amount / (current_man_amount + new_amount);
            
            ss << "buy " << new_amount << " of " << codename << " by " << price << "$ per each";
        }
        unit->impact_description_.push_back(ss.str());
        modify_item_property(unit, PRP_SILVER);
        modify_item_property(unit, codename);
        unit->CalcWeightsAndMovement();         
    }

    void modify_item_from_unit(CUnit* unit, CUnit* source_unit, const std::string& codename, long new_amount)
    {
        if (new_amount == 0)
            return;

        if (codename == PRP_SILVER)
            unit->silver_.amount_ += new_amount;
        else
            item_control::modify_amount(unit->items_, codename, new_amount);

        std::stringstream ss;
        std::string action, direction, source_name;
        if (source_unit == nullptr)
            source_name = "nowhere";
        else
            source_name = compose_unit_name(source_unit);

        if (new_amount > 0)
        {
            action = "receives";
            direction = "from";
        }
        else
        {
            action = "gives";
            direction = "to";
        }
        
        //print out impact
        ss << action << " " << abs(new_amount) << " of " << codename << " ";
        ss << direction << " " << source_name;
        unit->impact_description_.push_back(ss.str());
        modify_item_property(unit, codename);
        unit->CalcWeightsAndMovement();     
    }

    void modify_man_from_unit(CUnit* unit, CUnit* source_unit, const std::string& codename, long new_amount)
    {
        if (new_amount == 0)
            return;

        std::stringstream ss;
        std::string action, direction, source_name;
        if (source_unit == nullptr)
            source_name = "nowhere";
        else
            source_name = compose_unit_name(source_unit);

        if (new_amount > 0)
        {
            action = "receives";
            direction = "from";

            long current_man_amount(0);
            for (const auto& nation : unit->men_)
                current_man_amount += nation.amount_;

            //calculate days of knowledge according to new amount of peasants
            std::map<std::string, long> united_skills = source_unit->skills_;
            for (const auto& skill : unit->skills_)
                if (united_skills.find(skill.first) == united_skills.end())
                    united_skills[skill.first] = 0;

            for (auto& skill: united_skills)
            {
                unit->skills_[skill.first] = ((unit->skills_[skill.first] * current_man_amount) +
                        (skill.second * new_amount)) / (current_man_amount + new_amount);
            }              
        }
        else
        {
            action = "gives";
            direction = "to";
        }
        item_control::modify_amount(unit->men_, codename, new_amount);

        //print out impact
        ss << action << " " << abs(new_amount) << " of " << codename << " ";
        ss << direction << " " << source_name;
        unit->impact_description_.push_back(ss.str());
        modify_item_property(unit, codename);
        unit->CalcWeightsAndMovement();     
    }


    void order_message(CUnit* unit, const char* line, const char* descr)
    {
        std::string description(line);
        description.append(" ");
        description.append(descr);
        unit->impact_description_.push_back(description);
    }

    std::string get_initial_description(CUnit* unit)
    {
        std::stringstream ss;
        if (IS_NEW_UNIT(unit))
        {
            ss << std::string(unit->Description.GetData(), unit->Description.GetLength()) << "." << EOL_SCR;
            return ss.str();
        }

        const char* begin = unit->Description.GetData();
        const char* end = begin + unit->Description.GetLength();        
        const char* runner = begin;

        //getting faction_and_flags line
        while (*runner != '[' && runner < end)
            ++runner;
        while (*runner != ',' && runner != begin)
            --runner;
        ss << std::string(begin, runner).c_str() << "." << EOL_SCR;

        //getting items line
        ++runner;
        while (*runner == ' ')
            ++runner;
        begin = runner;
        while (*runner != '.' && runner < end)
            ++runner;
        ss << std::string(begin, runner).c_str() << "." << EOL_SCR;

        //getting misc line
        if (unit->IsOurs)
        {
            ++runner;
            while (*runner == ' ')
                ++runner;
            begin = runner;
            while (memcmp(runner, "Skills", 6) != 0 && runner + 6 < end)
                ++runner;
            while (*runner != '.' && runner != begin)
                --runner;
            std::string misc(begin, runner);

            //getting skills line
            while (memcmp(runner, "Skills", 6) != 0 && runner + 6 < end)
                ++runner;
            begin = runner;
            while (*runner != '.' && *runner != ';' && runner < end)
                ++runner;
            ss << std::string(begin, runner).c_str() << "." << EOL_SCR;
            ss << misc.c_str() << "." << EOL_SCR;
        }
        if (*runner == ';') //we have description
        {
            ss << std::string(runner, end).c_str() << "." << EOL_SCR;
        }
        return ss.str();     
    }



    std::string get_actual_description(CUnit* unit)
    {
        std::stringstream ss;
        
        //first line
        if (unit->IsOurs)
            ss << " * ";
        else
            ss << " - ";

        ss << std::string(unit->Name.GetData(), unit->Name.GetLength()) << " (" << std::to_string(unit->Id) << ")";
        if (flags::is_guard(unit))
            ss << ", on guard";
        if (unit->pFaction != NULL)
            ss << ", " << std::string(unit->pFaction->Name.GetData(), unit->pFaction->Name.GetLength()) << "(" << std::to_string(unit->FactionId) << ")";
        if (flags::is_avoid(unit))
            ss << ", avoiding";
        if (flags::is_behind(unit))
            ss << ", behind";            
        if (flags::is_reveal(unit, "unit"))
            ss << ", revealing unit";
        else if (flags::is_reveal(unit, "faction"))
            ss << ", revealing faction";
        if (flags::is_hold(unit))
            ss << ", holding";
        if (flags::is_noaid(unit))
            ss << ", receiving no aid";
        if (flags::is_sharing(unit))
            ss << ", sharing";
        if (flags::is_consume(unit, "unit"))
            ss << ", consuming unit's food";
        else if (flags::is_consume(unit, "faction"))
            ss << ", consuming faction's food";
        if (flags::is_spoils(unit, "none"))
            ss << ", weightless battle spoils";
        else if (flags::is_spoils(unit, "walk"))
            ss << ", walking battle spoils";
        else if (flags::is_spoils(unit, "ride"))
            ss << ", riding battle spoils";
        else if (flags::is_spoils(unit, "fly"))
            ss << ", flying battle spoils";
        else if (flags::is_spoils(unit, "swim"))
            ss << ", swimming battle spoils";
        else if (flags::is_spoils(unit, "sail"))
            ss << ", sailing battle spoils";                                                            
        if (flags::is_nocross(unit))
            ss << ", won't cross water";
        ss << "." << EOL_SCR;

        //second line
        item_control::items_to_stringstream(ss, unit->men_);
        if (unit->silver_.amount_ != 0)
        {
            ss << ", ";
            item_control::item_to_stringstream(ss, unit->silver_);
        }
        if (unit->items_.size() > 0)
        {
            ss << ", ";
            item_control::items_to_stringstream(ss, unit->items_);
        }
        ss << "." << EOL_SCR;
        //Skills: tactics [TACT] 1 (30), mining [MINI] 1 (30).
        ss << "Skills: ";
        bool first = true;
        for (auto& skill : unit->skills_)
        {
            std::vector<Skill> skills;
            skills_control::get_skills_if(skills, [&skill](const Skill& cur_skill) {
                return !cur_skill.short_name_.compare(skill.first);
            });

            long level = skills_control::get_skill_lvl_from_days(skill.second);
            if (first)
                first = false;
            else
                ss << ", ";
            if (skills.size() == 0)
                ss << "Unknown" << " [" << skill.first << "] " << level << " (" << skill.second << ")";    
            else
                ss << skills[0].long_name_ << " [" << skill.first << "] " << level << " (" << skill.second << ")";
        }
        ss << "." << EOL_SCR;
        return ss.str();
    }

    long get_max_skill_lvl_for_race(const std::string& race, const std::string& skill)
    {
        std::vector<std::string> race_info = game_control::get_game_config<std::string>(SZ_SECT_MAX_SKILL_LVL, race.c_str());
        if (race_info.size() < 2)
            return 0;
        
        for (size_t i = 2, size = race_info.size(); i < size; ++i)
        {
            if (skill == race_info[i])
                return atol(race_info[0].c_str());
        }
        return atol(race_info[1].c_str());
    }

    long get_max_skill_lvl(CUnit* unit, const std::string& skill)
    {
        long ret(-1);
        for (const auto& item : unit->men_)
        {
            if (gpDataHelper->IsMan(item.code_name_.c_str()))
            {
                EValueType type;
                const char* lead;                
                unit->GetProperty(PRP_LEADER, type, (const void *&)lead, eNormal);
                if (ret == -1)
                    ret = get_max_skill_lvl_for_race(item.code_name_, skill);
                else
                    ret = std::min(ret, get_max_skill_lvl_for_race(item.code_name_, skill));
            }
        }
        return ret;
    }

    long get_current_skill_days(CUnit* unit, const std::string& skill)
    {
        if (unit->skills_.find(skill) == unit->skills_.end())
            return 0;
        return unit->skills_[skill];
    }

    bool init_caravan(CUnit* unit)
    {
        if (orders::autoorders_caravan::is_caravan(unit->orders_))
        {
            unit->caravan_info_ = orders::autoorders_caravan::get_caravan_info(unit->orders_);
            return true;
        }
        else
        {
            unit->caravan_info_ = nullptr;
            return false;
        }        
    }

    void clean_caravan(CUnit* unit)
    {
        if (unit->caravan_info_ != nullptr)
            unit->caravan_info_ = nullptr;
    }


}

namespace land_control
{
    void get_land_coordinates(long land_id, int& x, int& y, int& z) {
        LandIdToCoord(land_id, x, y, z);
    }
    CLand* get_land(int x, int y, int z)
    {
        return gpApp->m_pAtlantis->GetLand(x, y, z, TRUE);
    }

    CLand* get_land(long land_id)
    {
        return gpApp->m_pAtlantis->GetLand(land_id);
    }

    long get_plane_id(const char* plane_name)
    {
        CBaseObject Dummy;
        Dummy.Name = plane_name;
        for (size_t i=0; i<gpApp->m_pAtlantis->m_Planes.Count(); i++)
        {
            CPlane* pPlane = (CPlane*)gpApp->m_pAtlantis->m_Planes.At(i);
            if (strcasecmp(pPlane->Name.GetData(), plane_name) == 0)
                return pPlane->Id;
        }
        return -1;
    }

    void add_resource(LandState& state, const CItem& item)
    {
        for (auto& res : state.resources_)
        {
            if (res.code_name_ == item.code_name_)
            {
                res.amount_ = item.amount_;
                return;
            }
        }
        state.resources_.emplace_back(item);
    }

    CProductMarket get_wanted(LandState& state, const std::string& item_code)
    {
        if (state.wanted_.find(item_code) != state.wanted_.end())
            return state.wanted_[item_code];
        return {0, {0, item_code}};
    }

    CProductMarket get_for_sale(LandState& state, const std::string& item_code)
    {
        if (state.for_sale_.find(item_code) != state.for_sale_.end())
            return state.for_sale_[item_code];
        return {0, {0, item_code}};
    }

    long get_resource(LandState& state, const std::string& item_code)
    {
        for (const auto& resource : state.resources_)
        {
            if (resource.code_name_ == item_code)
                return resource.amount_;
        }
        return 0;
    }

    void set_produced_items(LandState& state, const std::string& item_code, long amount)
    {
        state.produced_items_[item_code] += amount;
    }
    long get_produced_items(LandState& state, const std::string& item_code)
    {
        if (state.produced_items_.find(item_code) != state.produced_items_.end())
            return state.produced_items_[item_code];
        return -1;
    }

    CStruct* get_struct(CLand* land, long struct_id)
    {
        return find_first_structure_if(land, [&](CStruct* structure) {
            return structure->Id == struct_id;
        });
    }

    long get_struct_weight(CLand* land, long struct_id)
    {
        long res(0);
        perform_on_each_unit(land, [&](CUnit* unit) {
            if (unit_control::structure_id(unit) == struct_id)
            {
                long weights[5] = {0};
                unit_control::get_weights(unit, weights);
                res += weights[0];
            }
        });
        return res;
    }

    namespace structures
    {
        void update_struct_weights(CLand* land)
        {
            perform_on_each_struct(land, [&](CStruct* structure) {
                structure->occupied_capacity_ = 0;
            });
            
            perform_on_each_unit(land, [&](CUnit* unit) {
                CStruct* cur_structure = get_struct(land, unit_control::structure_id(unit));
                if (cur_structure != nullptr)
                {
                    long weights[5] = {0};
                    unit_control::get_weights(unit, weights);
                    cur_structure->occupied_capacity_ += weights[0];
                }
            });
        }

        void clean_structures(LandState& lstate)
        {
            lstate.structures_.erase(std::remove_if(lstate.structures_.begin(), 
                                                    lstate.structures_.end(),
                                                    [](CStruct* structure){
                                            return (structure->Attr & SA_HIDDEN) == 0 &&   // keep the gates!
                                                (structure->Attr & SA_SHAFT) == 0;
                                    }), lstate.structures_.end());
        }

        CStruct* add_structure(CLand* land, LandState& lstate, CStruct* structure)
        {
            auto existing_struct_it = std::find_if(lstate.structures_.begin(), 
                                                   lstate.structures_.end(), 
                                                   [&](CStruct* cur_struct) {
                                        return cur_struct->Id == structure->Id;
                                    });
            if (existing_struct_it != lstate.structures_.end())
            {
                struct_control::copy_structure(structure, *existing_struct_it);
                delete(structure);//backward compatibility
                structure = *existing_struct_it;//for return
            }                
            else
            {
                lstate.structures_.push_back(structure);
                land_control::structures::land_flags_update(land, structure);
            }
                
            //backward compatibility
            return structure;
        }   

        bool link_shafts(CLand* from, CLand* to, long struct_id)
        {
            if (from && to)
            {
                CStruct* structure = land_control::get_struct(from, struct_id);
                if (structure != nullptr && struct_control::flags::is_shaft(structure))
                {
                    CStr         destLandDescription;
                    gpApp->m_pAtlantis->ComposeLandStrCoord(to, destLandDescription);
                    std::string new_link = "; links to (" + 
                        std::string(destLandDescription.GetData(),destLandDescription.GetLength()) + ")";

                    size_t links_pos = structure->original_description_.find("; links to");
                    size_t dot_pos = structure->original_description_.find_last_of('.');
                    if (links_pos != std::string::npos)
                        structure->original_description_.replace(links_pos, dot_pos - links_pos, new_link);
                    else 
                        structure->original_description_.insert(dot_pos, new_link);

                    return true;            
                }
            }
            return false;
        }

        void land_flags_update(CLand* land, CStruct* structure)
        {
            //flags manipulation
            if (0 == structure->Attr)                 land->Flags |= LAND_STR_GENERIC;
            else
            {
                if (structure->Attr & SA_HIDDEN )     land->Flags |= LAND_STR_HIDDEN ;
                if (structure->Attr & SA_MOBILE )     land->Flags |= LAND_STR_MOBILE ;
                if (structure->Attr & SA_SHAFT  )     land->Flags |= LAND_STR_SHAFT  ;
                if (structure->Attr & SA_GATE   )     land->Flags |= LAND_STR_GATE   ;
                if (structure->Attr & SA_ROAD_N )     land->Flags |= LAND_STR_ROAD_N ;
                if (structure->Attr & SA_ROAD_NE)     land->Flags |= LAND_STR_ROAD_NE;
                if (structure->Attr & SA_ROAD_SE)     land->Flags |= LAND_STR_ROAD_SE;
                if (structure->Attr & SA_ROAD_S )     land->Flags |= LAND_STR_ROAD_S ;
                if (structure->Attr & SA_ROAD_SW)     land->Flags |= LAND_STR_ROAD_SW;
                if (structure->Attr & SA_ROAD_NW)     land->Flags |= LAND_STR_ROAD_NW;
            }            
        }

    }

    std::string land_full_name(CLand* land)
    {
        std::string ret;
        CStr sCoord;
        gpApp->m_pAtlantis->ComposeLandStrCoord(land, sCoord);
        ret.append(std::string(land->TerrainType.GetData(), land->TerrainType.GetLength()));
        ret.append(" (");
        ret.append(std::string(sCoord.GetData(), sCoord.GetLength()));
        ret.append(") in ");
        ret.append(std::string(land->Name.GetData(), land->Name.GetLength()));
        if (!land->CityName.IsEmpty())
        {
            ret.append(", contains ");
            ret.append(std::string(land->CityName.GetData(), land->CityName.GetLength()));
            ret.append(" [");
            ret.append(std::string(land->CityType.GetData(), land->CityType.GetLength()));
            ret.append("]");
        }
        return ret;
    }

    namespace economy 
    {
        void economy_calculations(CLand* land, CEconomy& res, std::vector<unit_control::UnitError>& errors) 
        {//gpApp->GetUnitsMovingIntoHex(pLand->Id, ArrivingUnits);
        //            if (pUnit && pUnit->pMovement)
        //                  GuiColor = 1;
        //ShowLandFinancial
            


            long leader_upkeep = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_LEADER);
            long peasant_upkeep = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_PEASANT);
            long player_faction_id = game_control::get_game_config_val<long>("ATTITUDES", "PLAYER_FACTION_ID");
            
            //study
            std::unordered_map<long, Student> students = get_land_students(land, errors);
            for (const auto& student : students)
            {
                if (student.second.unit_->FactionId == player_faction_id)
                    res.study_expenses_ += student.second.skill_price_ * student.second.man_amount_;
            }

            //sells
            std::vector<Trader> sellers;
            get_land_sells(land, sellers, errors);
            for (const auto& seller : sellers)
            {
                if (seller.unit_->FactionId == player_faction_id)
                    res.sell_income_ += seller.market_price_ * seller.items_amount_;
            }

            //tax income
            Taxers taxers;
            get_land_taxers(land, taxers, errors);
            res.tax_income_ = taxers.expected_income_;

            //moving in
            CBaseColl           arriving_units;
            CUnit*              unit_temp;
            gpApp->GetUnitsMovingIntoHex(land->Id, arriving_units);
            for (int i=0; i<arriving_units.Count(); ++i)
            {
                unit_temp = (CUnit*)arriving_units.At(i);
                if (unit_temp != NULL)
                {
                    if (unit_control::is_leader(unit_temp)) {
                        res.maintenance_ += leader_upkeep*unit_control::get_item_amount_by_mask(unit_temp, PRP_MEN);
                    } else {
                        res.maintenance_ += peasant_upkeep*unit_control::get_item_amount_by_mask(unit_temp, PRP_MEN);
                    }
                    res.moving_in_ += unit_control::get_item_amount(unit_temp, PRP_SILVER);
                }
            }

            perform_on_each_unit(land, [&](CUnit* unit) {
                if (!unit->IsOurs || unit->FactionId != player_faction_id)
                    return;

                //initial amount
                long men_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                res.initial_amount_ += unit_control::get_item_amount(unit, PRP_SILVER);
                if (men_amount > 0) 
                {
                    //moving out
                    if (unit->pMovement != NULL && unit->pMovement->Count() > 0)
                        res.moving_out_ += unit_control::get_item_amount(unit, PRP_SILVER);
                    else 
                    {//maintenance
                        if (unit_control::is_leader(unit)) {
                            res.maintenance_ += leader_upkeep*men_amount;
                        } else {
                            res.maintenance_ += peasant_upkeep*men_amount;
                        }
                    }
                }
            });
        }
    }

    template<orders::Type TYPE>
    void affect_other_flags(CUnit* unit)  {  return;  }
    template<>
    void affect_other_flags<orders::Type::O_AVOID>(CUnit* unit)
    {
        unit->Flags &= ~(orders::control::flag_by_order_type<orders::Type::O_GUARD>());
        return;
    }    
    template<>
    void affect_other_flags<orders::Type::O_GUARD>(CUnit* unit)
    {
        unit->Flags &= ~(orders::control::flag_by_order_type<orders::Type::O_AVOID>());
        return;
    }    

    template<orders::Type TYPE>
    void apply_flag(CUnit* unit, std::vector<unit_control::UnitError>& errors) 
    {
        if (orders::control::has_orders_with_type(TYPE, unit->orders_)) {
            auto orders = orders::control::retrieve_orders_by_type(TYPE, unit->orders_);
            if (orders.size() > 0)
            {
                bool flag;
                if (orders::parser::specific::parse_flags(orders[orders.size()-1], flag))//take last
                {
                    if (flag)
                    {
                        unit->Flags |= orders::control::flag_by_order_type<TYPE>();
                        affect_other_flags<TYPE>(unit);
                    }                        
                    else
                        unit->Flags &= ~(orders::control::flag_by_order_type<TYPE>());

                }
                else 
                    errors.push_back({"Error", unit, "couldn't parse order: "+(orders[orders.size()-1])->original_string_});
            }
            else 
                errors.push_back({"Error", unit, "Internal error in hashtable: doesn't have order "+std::to_string((int)TYPE)});
        }        
    }

    template<>
    void apply_flag<orders::Type::O_REVEAL>(CUnit* unit, std::vector<unit_control::UnitError>& errors) 
    {
        if (orders::control::has_orders_with_type(orders::Type::O_REVEAL, unit->orders_)) {
            auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_REVEAL, unit->orders_);
            if (orders.size() > 0)
            {
                std::string param;
                if (orders::parser::specific::parse_flags_with_param(orders[orders.size()-1], param))//take last
                {
                    if (stricmp("unit", param.c_str()) == 0)
                    {
                        unit->Flags |= UNIT_FLAG_REVEALING_UNIT;
                        unit->Flags &= ~UNIT_FLAG_REVEALING_FACTION;
                    }
                    else if (stricmp("faction", param.c_str()) == 0)
                    {
                        unit->Flags &= ~UNIT_FLAG_REVEALING_UNIT;
                        unit->Flags |= UNIT_FLAG_REVEALING_FACTION;
                    }
                    else if (param.empty())
                    {
                        unit->Flags &= ~UNIT_FLAG_REVEALING_UNIT;
                        unit->Flags &= ~UNIT_FLAG_REVEALING_FACTION;
                    }
                    else
                        errors.push_back({"Error", unit, "unknown parameter in order: "+(orders[orders.size()-1])->original_string_});
                }
                else 
                    errors.push_back({"Error", unit, "couldn't parse order: "+(orders[orders.size()-1])->original_string_});
            }
            else 
                errors.push_back({"Error", unit, "Internal error in hashtable: doesn't have order `reveal`"});
        }        
    }

    template<>
    void apply_flag<orders::Type::O_CONSUME>(CUnit* unit, std::vector<unit_control::UnitError>& errors) 
    {
        if (orders::control::has_orders_with_type(orders::Type::O_CONSUME, unit->orders_)) {
            auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_CONSUME, unit->orders_);
            if (orders.size() > 0)
            {
                std::string param;
                if (orders::parser::specific::parse_flags_with_param(orders[orders.size()-1], param))//take last
                {
                    if (stricmp("unit", param.c_str()) == 0)
                    {
                        unit->Flags |= UNIT_FLAG_CONSUMING_UNIT;
                        unit->Flags &= ~UNIT_FLAG_CONSUMING_FACTION;
                    }
                    else if (stricmp("faction", param.c_str()) == 0)
                    {
                        unit->Flags &= ~UNIT_FLAG_CONSUMING_UNIT;
                        unit->Flags |= UNIT_FLAG_CONSUMING_FACTION;
                    }
                    else if (param.empty())
                    {
                        unit->Flags &= ~UNIT_FLAG_CONSUMING_UNIT;
                        unit->Flags &= ~UNIT_FLAG_CONSUMING_FACTION;
                    }
                    else
                        errors.push_back({"Error", unit, "unknown parameter in order: "+(orders[orders.size()-1])->original_string_});
                }
                else 
                    errors.push_back({"Error", unit, "couldn't parse order: "+(orders[orders.size()-1])->original_string_});
            }
            else 
                errors.push_back({"Error", unit, "Internal error in hashtable: doesn't have order `consume`"});
        }        
    }

    void set_spoils_flags(CUnit* unit, long flag)
    {
        unit->Flags &= ~UNIT_FLAG_SPOILS_NONE;
        unit->Flags &= ~UNIT_FLAG_SPOILS_WALK;
        unit->Flags &= ~UNIT_FLAG_SPOILS_RIDE;
        unit->Flags &= ~UNIT_FLAG_SPOILS_FLY;
        unit->Flags &= ~UNIT_FLAG_SPOILS_SWIM;
        unit->Flags &= ~UNIT_FLAG_SPOILS_SAIL;
        unit->Flags |= flag;
    }

    template<>
    void apply_flag<orders::Type::O_SPOILS>(CUnit* unit, std::vector<unit_control::UnitError>& errors) 
    {
        if (orders::control::has_orders_with_type(orders::Type::O_SPOILS, unit->orders_)) {
            auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_SPOILS, unit->orders_);
            if (orders.size() > 0)
            {
                std::string param;
                if (orders::parser::specific::parse_flags_with_param(orders[orders.size()-1], param))//take last
                {
                    if (stricmp("none", param.c_str()) == 0)
                        set_spoils_flags(unit, UNIT_FLAG_SPOILS_NONE);
                    else if (stricmp("walk", param.c_str()) == 0)
                        set_spoils_flags(unit, UNIT_FLAG_SPOILS_WALK);
                    else if (stricmp("ride", param.c_str()) == 0)
                        set_spoils_flags(unit, UNIT_FLAG_SPOILS_RIDE);
                    else if (stricmp("fly", param.c_str()) == 0)
                        set_spoils_flags(unit, UNIT_FLAG_SPOILS_FLY);
                    else if (stricmp("swim", param.c_str()) == 0)
                        set_spoils_flags(unit, UNIT_FLAG_SPOILS_SWIM);
                    else if (stricmp("sail", param.c_str()) == 0)
                        set_spoils_flags(unit, UNIT_FLAG_SPOILS_SAIL);
                    else if (param.empty() || stricmp("all", param.c_str()) == 0)
                        set_spoils_flags(unit, 0);
                    else
                        errors.push_back({"Error", unit, "unknown parameter in order: "+(orders[orders.size()-1])->original_string_});
                }
                else 
                    errors.push_back({"Error", unit, "couldn't parse order: "+(orders[orders.size()-1])->original_string_});
            }
            else 
                errors.push_back({"Error", unit, "Internal error in hashtable: doesn't have order `consume`"});
        }        
    }    

    void apply_land_flags(CLand* land, std::vector<unit_control::UnitError>& errors)
    {
        land_control::perform_on_each_unit(land, [&](CUnit* unit) {
            if (!unit->IsOurs)
                return;

            apply_flag<orders::Type::O_AUTOTAX>(unit, errors);
            apply_flag<orders::Type::O_AVOID>(unit, errors);
            apply_flag<orders::Type::O_BEHIND>(unit, errors);
            apply_flag<orders::Type::O_GUARD>(unit, errors);//discards avoid, should go after
            apply_flag<orders::Type::O_HOLD>(unit, errors);
            apply_flag<orders::Type::O_NOAID>(unit, errors);
            apply_flag<orders::Type::O_NOCROSS>(unit, errors);
            apply_flag<orders::Type::O_SPOILS>(unit, errors);
            apply_flag<orders::Type::O_SHARE>(unit, errors);
            apply_flag<orders::Type::O_REVEAL>(unit, errors);
            apply_flag<orders::Type::O_CONSUME>(unit, errors);
        });        
    }

    void get_land_builders(CLand* land, std::vector<ActionUnit>& out, std::vector<unit_control::UnitError>& errors)
    {
        land_control::perform_on_each_unit(land, [&](CUnit* unit) {
            auto ret = orders::control::retrieve_orders_by_type(orders::Type::O_BUILD, unit->orders_);
            if (ret.size() == 1) 
            {
                std::string building;
                long unit_id;
                bool helps;
                if (orders::parser::specific::parse_build(ret[0], building, helps, unit_id))
                {
                    if (helps)
                        out.push_back({"build", "helps to build to " + std::to_string(unit_id), unit});
                    else if (building.empty()) 
                    {
                        CStruct* structure = land_control::get_struct(land, unit_control::structure_id(unit));
                        if (structure != nullptr)
                            out.push_back({"build", "continue to build "+structure->name_ + " : " + structure->type_, unit});
                        else
                            errors.push_back({"Error", unit, "build: continue to build outside of building"});
                    }                        
                    else
                        out.push_back({"build", "build new " + building, unit});
                }
                else 
                {//unknown format
                    errors.push_back({"Error", unit, "build: unknown format: "+ret[0]->original_string_});
                }
            }
            else if (ret.size() > 1)
            {
                errors.push_back({"Error", unit, "build: multiple build orders."});
            }
        });        
    }


    void get_land_taxers(CLand* land, Taxers& out, std::vector<unit_control::UnitError>& errors)
    {
        long tax_per_man = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_TAX_PER_TAXER);

        out.is_pillaging_ = false;
        out.expected_income_ = 0;
        out.man_amount_ = 0;
        out.land_tax_available_ = land->current_state_.tax_amount_;

        Taxers tax;
        tax.is_pillaging_ = false;
        tax.expected_income_ = 0;
        tax.man_amount_ = 0;
        tax.land_tax_available_ = land->current_state_.tax_amount_;
        Taxers pillage;
        pillage.is_pillaging_ = true;
        pillage.expected_income_ = 0;
        pillage.man_amount_ = 0;
        pillage.land_tax_available_ = land->current_state_.tax_amount_;
        land_control::perform_on_each_unit(land, [&](CUnit* unit) {

            if (!unit->IsOurs)
                return;

            auto tax_orders = orders::control::retrieve_orders_by_type(orders::Type::O_TAX, unit->orders_);
            if (tax_orders.size() > 0)
            {
                tax.man_amount_ += unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                tax.units_.push_back(unit);
                return;
            }
            auto pillage_orders = orders::control::retrieve_orders_by_type(orders::Type::O_PILLAGE, unit->orders_);
            if (pillage_orders.size() > 0)
            {
                pillage.man_amount_ += unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                pillage.units_.push_back(unit);
                return;
            }
            auto autotax_orders = orders::control::retrieve_orders_by_type(orders::Type::O_AUTOTAX, unit->orders_);
            if (autotax_orders.size() > 0)
            {
                long flag = atol(autotax_orders[autotax_orders.size() - 1]->words_order_[1].c_str());//TODO: specific order
                if (flag == 1)
                {
                    tax.man_amount_ += unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                    tax.units_.push_back(unit);
                }
                return;//in case that flag is set to 0, we don't want to check actual AUTOTAX flag.
            }
            if (unit->Flags & UNIT_FLAG_TAXING)
            {
                tax.man_amount_ += unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                tax.units_.push_back(unit);
                return;
            }
        });
        if (pillage.man_amount_ > 0)
        {
            long required_pillagers = (pillage.land_tax_available_-1)/(2*tax_per_man) + 1;
            if (required_pillagers <= pillage.man_amount_) 
            {
                pillage.expected_income_ = 2*pillage.land_tax_available_;
                out = pillage;
            }                
            else
            {
                for (auto& unit : pillage.units_)
                    errors.push_back({"Warning", unit, " - Not enough pillagers, needs: "+
                        std::to_string(required_pillagers)+", but have: "+std::to_string(pillage.man_amount_)});
                pillage.expected_income_ = 0;
            }                    
        }  
        //if we have tax orders and pillage failed
        if (tax.man_amount_ > 0)
        {
            if (pillage.expected_income_ > 0) 
            {//pillage already succeed
                for (auto& unit : tax.units_)
                    errors.push_back({"Error", unit, " - is trying to tax, while region is pillaged!"});
            }
            else if (tax.land_tax_available_ < tax_per_man*tax.man_amount_)
            {
                tax.expected_income_ = tax.land_tax_available_;
                out = tax;
            }                
            else
            {
                tax.expected_income_ = tax_per_man*tax.man_amount_;
                out = tax;
            }                
        }
    }

    void get_land_buys(CLand* land, std::vector<Trader>& out, std::vector<unit_control::UnitError>& errors)
    {
        std::map<std::string, long> request_table;

        perform_on_each_unit(land, [&](CUnit* unit) {
            auto buy_orders = orders::control::retrieve_orders_by_type(orders::Type::O_BUY, unit->orders_);
            for (auto& buy_order : buy_orders)
            {
                std::string item_name;
                long amount_to_buy;
                bool all;
                if (!orders::parser::specific::parse_sellbuy(buy_order, item_name, amount_to_buy, all))
                {
                    errors.push_back({"Error", unit, " - Buy: couldn't parse order - " + buy_order->original_string_});
                    continue;
                }

                if (stricmp(item_name.c_str(), "peas") == 0 ||
                    stricmp(item_name.c_str(), "peasant") == 0 ||
                    stricmp(item_name.c_str(), "peasants") == 0)
                    item_name = land->current_state_.peasant_race_;

                CProductMarket sell_item = land_control::get_for_sale(land->current_state_, item_name);
                if (sell_item.item_.amount_ <= 0)
                {
                    errors.push_back({"Error", unit, " - Buy: items are not selling - " + buy_order->original_string_});
                    continue;
                }

                if (all)
                    amount_to_buy = sell_item.item_.amount_;

                if (sell_item.item_.amount_ < amount_to_buy) {
                    errors.push_back({"Warning", unit, " - Buy: attempt to buy " + std::to_string(amount_to_buy) +
                                                       ", but available just " + std::to_string(sell_item.item_.amount_)});
                    amount_to_buy = sell_item.item_.amount_;
                }
                request_table[item_name] += amount_to_buy;
                out.push_back({buy_order, item_name, amount_to_buy, sell_item.price_, unit});
            }
        });

        for (const auto& request : request_table)
        {
            CProductMarket sell_item = land_control::get_for_sale(land->current_state_, request.first);
            if (sell_item.item_.amount_ < request.second)
                errors.push_back({"Warning", nullptr, " - request ("+std::to_string(request.second)+
                                  ") is higher than the offer ("+std::to_string(sell_item.item_.amount_)+")"});
        }
    }


    void get_land_sells(CLand* land, std::vector<Trader>& out, std::vector<unit_control::UnitError>& errors)
    {
        std::map<std::string, long> requests;//item_name, amount
        perform_on_each_unit(land, [&](CUnit* unit) {
            auto sell_orders = orders::control::retrieve_orders_by_type(orders::Type::O_SELL, unit->orders_);
            for (auto& sell_order : sell_orders)
            {
                std::string item_name;
                long amount_to_sell;
                bool all;
                if (!orders::parser::specific::parse_sellbuy(sell_order, item_name, amount_to_sell, all))
                {
                    errors.push_back({"Error", unit, " - Sell: couldn't parse order - " + sell_order->original_string_});
                    continue;
                }

                long amount_at_unit = unit_control::get_item_amount(unit, item_name);
                if (amount_at_unit <= 0)
                {
                    errors.push_back({"Warning", unit, " - Sell: no items to sell - " + sell_order->original_string_});
                    continue;
                }

                CProductMarket wanted_item = land_control::get_wanted(land->current_state_, item_name);
                if (wanted_item.item_.amount_ <= 0)
                {
                    errors.push_back({"Error", unit, " - Sell: items are not wanted - " + sell_order->original_string_});
                    continue;
                }

                if (all)
                    amount_to_sell = std::min(amount_at_unit, wanted_item.item_.amount_);
                if (amount_to_sell <= 0)
                {
                    errors.push_back({"Warning", unit, " - Sell: specified amount is not correct - " + sell_order->original_string_});
                    continue;
                }

                if (amount_to_sell > amount_at_unit)
                {
                    std::string warning = "trying to sell " + std::to_string(amount_to_sell)+
                                          " but has " + std::to_string(amount_at_unit);
                    errors.push_back({"Warning", unit, " - Sell: " + warning});
                    amount_to_sell = amount_at_unit;
                }
                if (amount_to_sell > wanted_item.item_.amount_)
                {
                    std::string warning = "trying to sell "+std::to_string(amount_to_sell)+
                            " but wanted just "+std::to_string(wanted_item.item_.amount_);                    
                    errors.push_back({"Warning", unit, "sell: " + warning});
                    amount_to_sell = wanted_item.item_.amount_;
                }
                out.push_back({sell_order, item_name, amount_to_sell, wanted_item.price_, unit});
                requests[item_name] += amount_to_sell;
            }
        });
        
        for (auto& request : requests)
        {
            CProductMarket wanted_item = land_control::get_wanted(land->current_state_, request.first);
            if (request.second > wanted_item.item_.amount_)
            {
                errors.push_back({"Warning", nullptr, "region accepts: " + std::to_string(wanted_item.item_.amount_) +
                 ", but sell attempts: " + std::to_string(request.second)});
            }
        }
    }

    std::unordered_map<long, Student> get_land_students(CLand* land, std::vector<unit_control::UnitError>& errors)
    {
        std::unordered_map<long, Student> students;
        perform_on_each_unit(land, [&](CUnit* unit) {
            std::shared_ptr<orders::Order> studying_order = orders::control::get_studying_order(unit->orders_);
            if (studying_order != nullptr) 
            {
                std::string studying_skill;
                long goal_lvl;
                if (!orders::parser::specific::parse_study(studying_order, studying_skill, goal_lvl))
                {
                    errors.push_back({"Error", unit, "study: wrong studying command: " + studying_order->original_string_});
                    return;
                }

                long price = gpApp->GetStudyCost(studying_skill.c_str());
                if (price <= 0)
                {
                    errors.push_back({"Error", unit, "study: can not study " + studying_skill});
                    return;
                }

                long amount_of_man = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                if (amount_of_man == 0)//order is given, but unit is empty
                {
                    errors.push_back({"Warning", unit, "study: no men in the unit!"});
                    return;
                }

                long max_skill = unit_control::get_max_skill_lvl(unit, studying_skill);
                if (max_skill < 0)
                {
                    errors.push_back({"Error", unit, "study: skill max level wasn't determined."});
                    return;
                }

                long current_days = unit_control::get_current_skill_days(unit, studying_skill);
                long max_days = 30*(max_skill+1)*(max_skill)/2;
                if (current_days >= max_days)
                {
                    errors.push_back({"Error", unit, "study: skill is already at max level."});
                    return;
                }

                if (goal_lvl > 0)
                {
                    long goal_days = 30*(goal_lvl+1)*(goal_lvl)/2;
                    if (current_days >= goal_days)
                    {
                        errors.push_back({"Warning", unit, "study: skill already reached specified goal."});
                        return;
                    }
                }

                unit_control::modify_silver(unit, -price * amount_of_man, "studying");
                //support of Unit List functionality
                if (PE_OK!=unit->SetProperty(PRP_SILVER,   eLong, (const void *)(unit->silver_.amount_), eNormal))
                {
                    errors.push_back({"Error", unit, "property: cannot set, probably a bug."});
                }                  

                students[unit->Id];
                students[unit->Id].cur_days_ = current_days;
                students[unit->Id].max_days_ = max_days;
                students[unit->Id].days_of_teaching_ = 0;//amount of days got from a teacher
                students[unit->Id].man_amount_ = amount_of_man;
                students[unit->Id].order_ = studying_order;
                students[unit->Id].skill_price_ = price;
                students[unit->Id].unit_ = unit;                         
            }
        });
        return students;
    }

    void update_students_by_land_teachers(CLand* land, 
                                          std::unordered_map<long, Student>& students, 
                                          std::vector<unit_control::UnitError>& errors)
    {
        perform_on_each_unit(land, [&](CUnit* unit) {
            std::vector<long> studs = orders::control::get_students(unit);
            if (studs.size() == 0)
                return;

            long teachers_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
            if (teachers_amount <= 0)
            {
                errors.push_back({"Warning", unit, "teach: have no teaching men"});
                return;
            }
            std::vector<Student*> active_students;
            long students_amount(0);
            for (long studId : studs)
            {
                if (studId == unit->Id)
                {
                    errors.push_back({"Error", unit, "teach: can't teach himself"});
                    continue;
                }
                if (students.find(studId) == students.end())
                {
                    errors.push_back({"Warning", unit, "teach: " + unit_control::compose_unit_number(studId) + " is not studying"});
                    continue;
                }
                if (students[studId].max_days_ - students[studId].cur_days_ - students[studId].days_of_teaching_- (long)30 <= 0)
                {
                    errors.push_back({"Warning", unit, "teach: "+ unit_control::compose_unit_number(studId) + " doesn't need any teacher more"});
                    continue;
                }

                std::string studying_skill = students[studId].order_->words_order_[1];
                long teacher_days = unit_control::get_current_skill_days(unit, studying_skill);
                long teacher_lvl = skills_control::get_skill_lvl_from_days(teacher_days);
                long student_lvl = skills_control::get_skill_lvl_from_days(students[studId].cur_days_);
                if (teacher_lvl <= student_lvl)
                {
                    errors.push_back({"Error", unit, "teach: can't teach " + unit_control::compose_unit_name(students[studId].unit_)});
                    continue;                        
                }
                students_amount += students[studId].man_amount_;
            }
            
            if (students_amount <= 0)
                return;

            unit->SetProperty(PRP_TEACHING, eLong, (const void*)students_amount, EPropertyType::eBoth);
            //we assume that studying is correct: teacher can teach, student can study and so on
            long teaching_days = std::min((long)30, (30 * STUDENTS_PER_TEACHER * teachers_amount) / students_amount);
            for (long studId : studs)
            {
                if (studId == unit->Id)
                    continue;
                if (students.find(studId) == students.end())
                    continue;
                if (students[studId].max_days_ - students[studId].cur_days_ - students[studId].days_of_teaching_ - (long)30 <= 0)
                    continue;
                std::string studying_skill = students[studId].order_->words_order_[1];
                long teacher_days = unit_control::get_current_skill_days(unit, studying_skill);
                long teacher_lvl = skills_control::get_skill_lvl_from_days(teacher_days);
                long student_lvl = skills_control::get_skill_lvl_from_days(students[studId].cur_days_);
                if (teacher_lvl <= student_lvl)
                    continue;                        

                students[studId].days_of_teaching_ += teaching_days;
                students[studId].days_of_teaching_ = std::min(students[studId].days_of_teaching_, (long)30);
            }
        });
    }
}

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

    std::string get_gpapp_config(const char* section, const char* key)
    {
        return gpApp->GetConfig(section, key);
    }

    bool get_struct_attributes(const std::string& struct_type, long& capacity, long& sailPower, long& structFlag)
    {
        std::string codename, name, plural_name;
        if (!gpApp->ResolveAliasItems(struct_type, codename, name, plural_name))
            name = struct_type;

        std::vector<std::string> struct_parameters = game_control::get_game_config<std::string>(SZ_SECT_STRUCTS, name.c_str());
        if (struct_parameters.size() == 0 && name[name.size()-1] == 's')
        {
            std::string name_without_s = name.substr(0, name.size()-1);
            struct_parameters = game_control::get_game_config<std::string>(SZ_SECT_STRUCTS, name_without_s.c_str());
            if (struct_parameters.size() == 0)
                return false;
        }
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
                // Two-token attributes, MaxLoad & MinSailingPower.
                size_t separator = param.find(' ');
                if (separator == std::string::npos)
                {
                    //TODO: print out the wrong setting
                    continue;
                }

                std::string key = param.substr(0, separator);
                long        val = atol(param.substr(separator+1).c_str());
                if (key == SZ_ATTR_STRUCT_MAX_LOAD)
                    capacity += val;
                else if (key == SZ_ATTR_STRUCT_MIN_SAIL)
                    sailPower += val;
            }

        }

        if (0 == stricmp(name.c_str(), STRUCT_GATE))
            structFlag |= SA_GATE; // to compensate for legacy missing gate flag in the config
        return true;        
    }

}

#include <algorithm> 
#include <cctype>
#include <locale>

namespace struct_control 
{
    void split_to_chunks(const std::string& str, char c, std::vector<std::string>& chunks)
    {
        size_t pos = 0;
        size_t cur = str.find(c, pos);
        while(cur != std::string::npos)
        {
            chunks.push_back(trim(str.substr(pos, cur - pos)));
            pos = cur+1;
            cur = str.find(c, pos);
        }
        size_t end_point = str.find('.', pos);
        chunks.push_back(trim(str.substr(pos, end_point-pos)));
    }

    void parse_struct(const std::string& line, long& id, std::string& name, 
                      std::string& type, std::vector<std::pair<std::string, long>>& substructures)
    {
        std::vector<std::string> chunks;
        std::vector<std::string> pieces;
        split_to_chunks(line, ';', chunks);
        for (const std::string& chunk : chunks)
            split_to_chunks(chunk, ':', pieces);

        //0 - name & number
        //1 - type
        //>1 - parameters

        //get name and number
        {
            size_t start = pieces[0].find_first_of('+');
            if (start == std::string::npos)
                return;
            size_t open_bracer = pieces[0].find_first_of('[', start);
            if (open_bracer == std::string::npos)
                return;
            size_t close_bracer = pieces[0].find_first_of(']', open_bracer);
            if (close_bracer == std::string::npos)
                return;

            id = atoi(&(pieces[0][open_bracer+1]));
            name = pieces[0].substr(start+2, close_bracer+1);
            trim(name);
        }

        //get type and substructures
        {
            if (pieces.size() == 1)
                return;

            std::vector<std::string> parts;
            split_to_chunks(pieces[1], ',', parts);
            type = trim(parts[0]);
            if (type.find("Fleet") != std::string::npos)//known type with substructs
            {                
                std::vector<std::string> pair;
                for(size_t i = 1; i < parts.size(); ++i)
                {
                    pair.clear();
                    split_to_chunks(parts[i], ' ', pair);
                    substructures.push_back({pair[1], atoi(pair[0].c_str())});
                }
            }
            else 
            {
                long capacity, sailPower, structFlag;
                game_control::get_struct_attributes(type, capacity, sailPower, structFlag);
                if (structFlag & SA_MOBILE)
                {//its still the ship, so we will handle it as Fleet
                    substructures.push_back({type, 1});
                    type = "Fleet";
                }
            }
        }
    }

    bool has_link(CStruct* structure) {
        size_t  x1, x2, x3;
        x1   = structure->original_description_.find(";");
        x2   = structure->original_description_.find("links");
        x3   = structure->original_description_.find("to");
        return x1 != std::string::npos && x1<x2 && x2<x3;
    }

    void copy_structure(CStruct* fromStruct, CStruct* toStruct)
    {
        if (!flags::is_shaft(toStruct) || has_link(fromStruct) ||
                fromStruct->original_description_.size() > toStruct->original_description_.size())
            toStruct->original_description_= fromStruct->original_description_;
        
        //toStruct->Name                  = fromStruct->Name       ;
        toStruct->LandId                = fromStruct->LandId     ;
        toStruct->OwnerUnitId           = fromStruct->OwnerUnitId;
        toStruct->occupied_capacity_    = fromStruct->occupied_capacity_       ;
        toStruct->Attr                  = fromStruct->Attr       ;
        toStruct->type_                 = fromStruct->type_      ;
        toStruct->capacity_             = fromStruct->capacity_      ;
        toStruct->name_                 = fromStruct->name_      ;
	    toStruct->Location              = fromStruct->Location   ;
        toStruct->fleet_ships_          = fromStruct->fleet_ships_   ;        
    }

}

