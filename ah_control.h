#ifndef AH_CONTROL_INCL
#define AH_CONTROL_INCL

#include "ahapp.h"
#include "data.h"
#include "consts_ah.h"

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
    std::vector<T> convert_to_vector(const std::string& value_string) 
    {
        std::vector<T> ret;
        const char* beg = value_string.c_str();
        const char* end = value_string.c_str() + value_string.size();
        const char* runner = beg;
        while(beg < end)
        {
            while (beg < end && *beg == ' ')
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
    std::vector<T> get_game_config(const char* section, const char* key)
    {
        static std::unordered_map<std::string, std::unordered_map<std::string, std::vector<T>>> cache;
        if (cache.find(section) != cache.end())
        {
            if (cache[section].find(key) != cache[section].end())
                return cache[section][key];
        }
        
        std::string value_string = get_gpapp_config(section, key);
        std::vector<T> ret = convert_to_vector<T>(value_string);
        cache[section][key] = ret;
        return ret;
    }

    std::vector<std::pair<std::string, std::string>> get_all_configuration(const char* section);

    template<typename T>
    T get_game_config_val(const char* section, const char* key)
    {
        return convert_to<T>(get_gpapp_config(section, key));
    }

    bool get_struct_attributes(const std::string& struct_type, long& capacity, long& sailPower, long& structFlag, SHIP_TRAVEL& travel, long& speed);
    long get_study_cost(const std::string& skill);

    namespace specific
    {
        CUnit* create_scout(CUnit* parent);
    }
}

namespace skills_control
{
    template<typename Pred>
    void get_skills_if(std::vector<Skill>& skills, Pred pred)
    {
        std::set<Skill> ret;
        const char  * szName;
        const char  * szValue;
        int sectidx = gpApp->GetSectionFirst(SZ_SECT_SKILLS, szName, szValue);
        while (sectidx >= 0)
        {
            Skill temp;
            std::string name(szName);
            if (name.find(" ") != std::string::npos && 
                name.find("[") != std::string::npos && 
                name.find("]") != std::string::npos )
            {
                temp.short_name_ = name.substr(name.find("[")+1, name.find("]") - name.find("[")-1); 
                temp.long_name_ = name.substr(0, name.find("[")-1);
                temp.study_price_ = game_control::get_study_cost(temp.short_name_);
                if (pred(temp))
                    ret.insert(temp);
            }
            sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_SKILLS, szName, szValue);
        }
        skills.assign(ret.begin(), ret.end());
    }

    template<typename Pred>
    bool get_first_skill_if(Skill& skill, Pred pred)
    {
        std::set<Skill> ret;
        const char  * szName;
        const char  * szValue;
        int sectidx = gpApp->GetSectionFirst(SZ_SECT_SKILLS, szName, szValue);
        while (sectidx >= 0)
        {
            std::string name(szName);
            if (name.find(" ") != std::string::npos && 
                name.find("[") != std::string::npos && 
                name.find("]") != std::string::npos )
            {
                skill.short_name_ = name.substr(name.find("[")+1, name.find("]") - name.find("[")-1); 
                skill.long_name_ = name.substr(0, name.find("[")-1);
                skill.study_price_ = game_control::get_study_cost(skill.short_name_);
                if (pred(skill))
                    return true;
            }
            sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_SKILLS, szName, szValue);
        }
        return false;
    }    

    static long get_skill_lvl_from_days(long days)
    {
        long level = 0;
        long turns = days / 30;
        while (turns > 0) { ++level; turns -= (level + 1); };
        return level;
    }
}

#endif
