
#include "consts.h"
#include "data_control.h"
#include <algorithm>
#include <sstream>

#include "ah_control.h"

namespace items_control
{
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
        bool is_sharing(CUnit* unit) {  return false;  } //not implemented
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

    long get_item_amount(CUnit* unit, const std::string& codename)
    {
        if (codename == PRP_SILVER)
            return unit->silver_.amount_;
        else if (gpApp->IsMan(codename.c_str()))
            return items_control::get_by_code(unit->men_, codename).amount_;
        else
            return items_control::get_by_code(unit->items_, codename).amount_;
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
    }

    void modify_item_from_market(CUnit* unit, const std::string& codename, long new_amount, long price)
    {
        if (new_amount == 0)
            return;

        unit->silver_.amount_ += -new_amount*price;
        items_control::modify_amount(unit->items_, codename, new_amount);

        std::stringstream ss;
        if (new_amount > 0)
            ss << "buy " << new_amount << " of " << codename << " by " << price << "$ per each";
        else
            ss << "sell " << abs(new_amount) << " of " << codename << " for " << price << "$ per each";

        unit->impact_description_.push_back(ss.str());
    }
    void modify_man_from_market(CUnit* unit, const std::string& codename, long new_amount, long price)
    {
        if (new_amount == 0)
            return;

        unit->silver_.amount_ += -new_amount*price;
        std::stringstream ss;
        if (new_amount < 0)
        {//assuming we can sell peasants, but we actually can't
            items_control::modify_amount(unit->men_, codename, new_amount);
            ss << "sell " << abs(new_amount) << " of " << codename << " for " << price << "$ per each";
        }
        else //new_amount > 0
        {
            long current_man_amount(0);
            for (const auto& nation : unit->men_)
                current_man_amount += nation.amount_;

            //calculate days of knowledge according to new amount of peasants
            for (auto& skill: unit->skills_)
                skill.second = skill.second * current_man_amount / (current_man_amount + new_amount);
            
            items_control::modify_amount(unit->men_, codename, new_amount);
            ss << "buy " << new_amount << " of " << codename << " by " << price << "$ per each";
        }
        unit->impact_description_.push_back(ss.str());
    }

    void modify_item_from_unit(CUnit* unit, CUnit* source_unit, const std::string& codename, long new_amount)
    {
        if (new_amount == 0)
            return;

        if (codename == PRP_SILVER)
            unit->silver_.amount_ += new_amount;
        else
            items_control::modify_amount(unit->items_, codename, new_amount);

        std::stringstream ss;
        std::string action, direction;
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
        ss << action << " " << new_amount << " of " << codename << " " << direction << " ";
        ss << compose_unit_name(source_unit);
        unit->impact_description_.push_back(ss.str());
    }

    void modify_man_from_unit(CUnit* unit, CUnit* source_unit, const std::string& codename, long new_amount)
    {
        if (new_amount == 0)
            return;

        std::stringstream ss;
        std::string action, direction;
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
        items_control::modify_amount(unit->men_, codename, new_amount);

        //print out impact
        ss << action << " " << new_amount << " of " << codename << " " << direction << " ";
        ss << compose_unit_name(source_unit);
        unit->impact_description_.push_back(ss.str());
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
            ss << std::string(unit->Description.GetData(), unit->Description.GetLength()) << ".\r\n";
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
        ss << std::string(begin, runner).c_str() << ".\r\n";

        //getting items line
        ++runner;
        while (*runner == ' ')
            ++runner;
        begin = runner;
        while (*runner != '.' && runner < end)
            ++runner;
        ss << std::string(begin, runner).c_str() << ".\r\n";

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
            ss << std::string(begin, runner).c_str() << ".\r\n";
            ss << misc.c_str() << ".\r\n";
        }
        if (*runner == ';') //we have description
        {
            ss << std::string(runner, end).c_str() << ".\r\n";
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
        else if (flags::is_spoils(unit, "ride"))
            ss << ", riding battle spoils";
        if (flags::is_nocross(unit))
            ss << ", won't cross water";
        ss << ".\r\n";

        //second line
        items_control::items_to_stringstream(ss, unit->men_);
        if (unit->silver_.amount_ != 0)
        {
            ss << ", ";
            items_control::item_to_stringstream(ss, unit->silver_);
        }
        if (unit->items_.size() > 0)
        {
            ss << ", ";
            items_control::items_to_stringstream(ss, unit->items_);
        }
        ss << ".\r\n";
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
        ss << ".\r\n";
        return ss.str();
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
                    ret = gpDataHelper->MaxSkillLevel(item.code_name_.c_str(), skill.c_str(), lead, FALSE);
                else
                    ret = std::min(ret, gpDataHelper->MaxSkillLevel(item.code_name_.c_str(), skill.c_str(), lead, FALSE));
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
        if (orders::autoorders::is_caravan(unit->orders_))
        {
            unit->caravan_info_ = orders::autoorders::get_caravan_info(unit->orders_);
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
    CLand* get_land(int x, int y, int z)
    {
        return gpApp->m_pAtlantis->GetLand(x, y, z, TRUE);
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

    CProductMarket get_wanted(CLand* land, const std::string& item_code)
    {
        if (land->wanted_.find(item_code) != land->wanted_.end())
            return land->wanted_[item_code];
        return {0, {0, item_code}};
    }

    CProductMarket get_for_sale(CLand* land, const std::string& item_code)
    {
        if (land->for_sale_.find(item_code) != land->for_sale_.end())
            return land->for_sale_[item_code];
        return {0, {0, item_code}};
    }

    std::unordered_map<long, Student> get_land_students(CLand* land, std::vector<unit_control::UnitError>& errors)
    {
        std::unordered_map<long, Student> students;
        perform_on_each_unit(land, [&](CUnit* unit) {
            std::shared_ptr<orders::Order> studying_order = orders::control::get_studying_order(unit->orders_);
            if (studying_order != nullptr) 
            {
                if (studying_order->words_order_.size() != 2)
                {
                    unit->impact_description_.push_back("study error: wrong command: " + studying_order->original_string_);
                    errors.push_back({unit, " - Wrong studying command!"});
                    return;
                }

                long price = gpApp->GetStudyCost(studying_order->words_order_[1].c_str());
                if (price <= 0)
                {
                    unit->impact_description_.push_back("study error: can't study " + studying_order->words_order_[1]);
                    errors.push_back({unit, " - Can not study that!"});
                    return;
                }

                long amount_of_man = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                if (amount_of_man == 0)//order is given, but unit is empty
                {
                    unit->impact_description_.push_back("study error: no men in unit to study anything");
                    errors.push_back({unit, " - There are no men in the unit!"});
                    return;
                }

                std::string studying_skill = studying_order->words_order_[1];
                long max_skill = unit_control::get_max_skill_lvl(unit, studying_skill);
                if (max_skill < 0)
                {
                    unit->impact_description_.push_back("study error: skill wasn't determined");
                    errors.push_back({unit, " - Skill max level wasn't determined!"});
                    return;
                }

                long current_days = unit_control::get_current_skill_days(unit, studying_skill);
                long max_days = 30*(max_skill+1)*(max_skill)/2;
                if (current_days >= max_days)
                {
                    unit->impact_description_.push_back("study error: skill is already at max level");
                    errors.push_back({unit, " - Skill is already at max level!"});
                    return;
                }
                
                unit_control::modify_silver(unit, -price * amount_of_man, "studying");
                //support of Unit List functionality
                if (PE_OK!=unit->SetProperty(PRP_SILVER,   eLong, (const void *)(unit->silver_.amount_), eNormal))
                {
                    errors.push_back({unit, " - Cannot set unit's property - it's a bug!"});
                }                  

                students[unit->Id];
                students[unit->Id].cur_days_ = current_days;
                students[unit->Id].max_days_ = max_days;
                students[unit->Id].days_of_teaching_ = 0;//amount of days got from a teacher
                students[unit->Id].man_amount_ = amount_of_man;
                students[unit->Id].order_ = studying_order;
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
                unit->impact_description_.push_back("teaching: have no men to teach");
                errors.push_back({unit, " have no men to teach"});
                return;
            }
            std::vector<Student*> active_students;
            long students_amount(0);
            for (long studId : studs)
            {
                if (studId == unit->Id)
                {
                    unit->impact_description_.push_back(unit_control::compose_unit_number(studId) + " can't teach himself");
                    errors.push_back({unit, " can't teach himself"});
                    continue;
                }
                if (students.find(studId) == students.end())
                {
                    unit->impact_description_.push_back(unit_control::compose_unit_number(studId) + " is not studying");
                    errors.push_back({unit, unit_control::compose_unit_number(studId) + " is not studying"});
                    continue;
                }
                if (students[studId].max_days_ - students[studId].cur_days_ - students[studId].days_of_teaching_- (long)30 <= 0)
                {
                    unit->impact_description_.push_back(unit_control::compose_unit_name(students[studId].unit_) + " doesn't need any teacher more");
                    errors.push_back({unit, unit_control::compose_unit_number(studId) + " doesn't need any teacher more"});
                    continue;
                }

                std::string studying_skill = students[studId].order_->words_order_[1];
                long teacher_days = unit_control::get_current_skill_days(unit, studying_skill);
                long teacher_lvl = skills_control::get_skill_lvl_from_days(teacher_days);
                long student_lvl = skills_control::get_skill_lvl_from_days(students[studId].cur_days_);
                if (teacher_lvl <= student_lvl)
                {
                    unit->impact_description_.push_back("Can't teach " + unit_control::compose_unit_name(students[studId].unit_));
                    errors.push_back({unit, "Can't teach " + unit_control::compose_unit_number(studId)});
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
    long convert_to<long>(const std::string& str)
    {
        return atol(str.c_str());
    }

    std::string get_gpapp_config(const char* section, const char* key)
    {
        return gpApp->GetConfig(section, key);
    }
}
