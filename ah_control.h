#ifndef AH_CONTROL_INCL
#define AH_CONTROL_INCL

#include "ahapp.h"
#include "data.h"
#include "consts_ah.h"

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
                temp.long_name_ = name.substr(0, name.find(" "));
                temp.study_price_ = gpDataHelper->GetStudyCost(temp.long_name_.c_str());
                if (pred(temp))
                    ret.insert(temp);
            }
            sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_SKILLS, szName, szValue);
        }
        skills.assign(ret.begin(), ret.end());
    }    
}

#endif