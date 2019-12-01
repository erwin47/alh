
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

    std::string get_initial_description(CUnit* unit)
    {
        std::stringstream ss;
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
        if (*runner == ';') //we have description
        {
            ss << std::string(runner, end).c_str();
        }
        return ss.str();     
    }

    std::string get_actual_description(CUnit* unit)
    {
        std::stringstream ss;
        
        //first line
        if (unit->IsOurs)
            ss << "* ";
        else
            ss << "- ";
        ss << std::string(unit->Name.GetData(), unit->Name.GetLength()) << " (" << std::to_string(unit->Id) << ")";
        if (flags::is_guard(unit))
            ss << ", on guard";
        ss << ", " << std::string(unit->pFaction->Name.GetData(), unit->pFaction->Name.GetLength()) << "(" << std::to_string(unit->FactionId) << ")";
        if (flags::is_avoid(unit))
            ss << ", avoiding";
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
            else if (item.amount_ > 1)
            {
                std::string code_name, long_name, long_name_plural;
                gpDataHelper->ResolveAliasItems(item.code_name_, code_name, long_name, long_name_plural);
                ss << std::to_string(item.amount_) << " " << long_name_plural << " [" << item.code_name_ << "]";
            }
        }
        ss << ".\r\n";

        return ss.str();
    }
}
