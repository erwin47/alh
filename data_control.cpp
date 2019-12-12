
#include "consts.h"
#include "data_control.h"
#include <algorithm>
#include <sstream>

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
   
    CItem get_item_by_name(CUnit* unit, const std::string& code_name)
    {
        auto it = std::find_if(unit->items_.begin(), unit->items_.end(), [&code_name](const CItem& prod) {
            return code_name == prod.code_name_;
        });
        if (it != unit->items_.end())
            return *it;

        return CItem({0, code_name.c_str()});
    }

    std::set<CItem>& get_items(CUnit* unit)
    {
        return unit->items_;
    }

    long get_item_amount(CUnit* unit, const std::string& codename)
    {
        return get_item_by_name(unit, codename).amount_;
    }
    void modify_item_amount(CUnit* unit, const std::string& source_name, const std::string& codename, long new_amount)
    {
        if (new_amount == 0)
            return;

        CItem item = get_item_by_name(unit, codename);
        item.amount_ += new_amount;
        unit->items_.erase(item);
        unit->items_.insert(item);
        
        std::stringstream ss;
        if (new_amount > 0)
            ss << "receives " << new_amount << " of " << codename << " from " << source_name;
        if (new_amount < 0)
            ss << "loses " << new_amount << " of " << codename << " for " << source_name;
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
        bool first_element = true;
        for (const auto& item : unit->items_)
        {
            if (item.amount_ == 0)
                continue;
            
            if (first_element)
                first_element = false;
            else
                ss << ", ";

            if (item.amount_ == 1)
            {
                std::string code_name, long_name, long_name_plural;
                gpDataHelper->ResolveAliasItems(item.code_name_, code_name, long_name, long_name_plural);
                ss << long_name << " [" << item.code_name_ << "]";
            }
            else // if amount_ is below zero, that also may be interesting
            {
                std::string code_name, long_name, long_name_plural;
                gpDataHelper->ResolveAliasItems(item.code_name_, code_name, long_name, long_name_plural);
                ss << std::to_string(item.amount_) << " " << long_name_plural << " [" << item.code_name_ << "]";
            }
        }
        ss << ".\r\n";

        return ss.str();
    }

    long get_max_skill_lvl(CUnit* unit, const std::string& skill)
    {
        long ret(-1);
        for (const auto& item : unit->items_)
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
}

namespace land_control
{
    std::unordered_map<long, Student> get_land_students(CLand* land)
    {
        //students
        std::unordered_map<long, Student> students;
        for (size_t idx=0; idx<land->UnitsSeq.Count(); idx++)
        {
            CUnit* pUnit = (CUnit*)land->UnitsSeq.At(idx);
            std::string studying_skill = orders::control::get_studying_skill(pUnit->orders_);
            if (studying_skill.size() > 0)
            {
                EValueType type;
                long amount_of_man;
                pUnit->GetProperty(PRP_MEN, type, (const void *&)amount_of_man, eNormal);

                students[pUnit->Id];
                students[pUnit->Id].man_amount_ = amount_of_man;
                students[pUnit->Id].studying_skill_ = studying_skill;
                students[pUnit->Id].unit_ = pUnit;

                //student may have limited amount of days to be tought. For example, if max student
                //level is 3, and it has 140 days. This means it needs 40 days of studying to get max, or
                //10 days of teaching. This is equal to situation, when it already is tought to 20 days.
                //so in this case days_of_teaching_ = 20.
                long max_skill = unit_control::get_max_skill_lvl(pUnit, studying_skill);
                if (max_skill < 0)
                    max_skill = 0;

                studying_skill.append(PRP_SKILL_DAYS_POSTFIX); 
                pUnit->GetProperty(studying_skill.c_str(), type, (const void *&)students[pUnit->Id].cur_days_, eNormal);

                students[pUnit->Id].max_days_ = 30*(max_skill+1)*(max_skill)/2;
                students[pUnit->Id].days_of_teaching_ = std::max((long)0, 60 - (students[pUnit->Id].max_days_ - students[pUnit->Id].cur_days_));
            }            
        }
        return students;
    }

    void update_students_by_land_teachers(CLand* land, std::unordered_map<long, Student>& students)
    {
        for (size_t idx=0; idx<land->UnitsSeq.Count(); idx++)
        {
            CUnit* pUnit = (CUnit*)land->UnitsSeq.At(idx);
            std::vector<long> studs = orders::control::get_students(pUnit);
            if (studs.size() > 0)
            {
                EValueType type;
                long teachers_amount;
                pUnit->GetProperty(PRP_MEN, type, (const void *&)teachers_amount, eNormal);

                long students_amount(0);
                for (long studId : studs)
                {
                    if (students.find(studId) == students.end())
                    {
                        pUnit->impact_description_.push_back(std::to_string(studId) + " is not studying");
                        continue;
                    }
                    if (students[studId].days_of_teaching_ >= 30)
                    {
                        pUnit->impact_description_.push_back(std::to_string(studId) + " is already tought");
                        continue;
                    }
                    //TODO: add also check that this teacher actually can teach that student
                    //I don't add it now, because I want to change the entire mechanism in future:
                    //At RunOrders_Study units will get skill +30, and then
                    //at RunOrders_Teach teachers, if they can teach and fit all the checks, they add additional
                    //teaching days.
                    //And thus this function will be obsolete.
                    students_amount += students[studId].man_amount_;
                }
                
                if (students_amount <= 0)
                    continue;

                //we assume that studying is correct: teacher can teach, student can study and so on
                long teaching_days = std::min((long)30, (30 * STUDENTS_PER_TEACHER * teachers_amount) / students_amount);
                for (long studId : studs)
                {
                    if (students.find(studId) == students.end())
                        continue;
                    if (students[studId].days_of_teaching_ >= 30)
                        continue;
                    students[studId].days_of_teaching_ += teaching_days;
                    students[studId].days_of_teaching_ = std::min(students[studId].days_of_teaching_, (long)30);
                }
            }
        }
    }
}
