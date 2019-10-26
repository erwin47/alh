
#include "data_control.h"
#include <algorithm>

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

   
    CProduct get_item_by_name(CUnit* unit, const std::string& short_name)
    {
        auto it = std::find_if(unit->items_.begin(), unit->items_.end(), [&short_name](const CProduct& prod) {
            return SafeCmp(short_name.c_str(), prod.ShortName.GetData()) == 0;
        });
        if (it != unit->items_.end())
            return *it;
        return CProduct({0, short_name.c_str(), gpDataHelper->ResolveAlias(short_name.c_str())});
    }

    std::set<CProduct>& get_items(CUnit* unit)
    {
        return unit->items_;
    }

    long get_item_amount(CUnit* unit, const std::string& short_name)
    {
        return get_item_by_name(unit, short_name).Amount;
    }
    void modify_item_amount(CUnit* unit, const std::string& short_name, long new_amount)
    {
        CProduct item = get_item_by_name(unit, short_name);
        item.Amount += new_amount;
        unit->items_.erase(item);
        unit->items_.insert(item);
    }

}