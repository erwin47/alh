
#include "data_control.h"
#include "unit_to_pane_control.h"
#include <sstream>

namespace unit_control
{
    namespace unitpane_control
    {
        template<>
        std::string get_property<unitpane_columns::UPC_MEN>(CUnit* unit, bool& bold, long& text_color, long& cell_color) 
        {
            return "";
        }

        template<>
        std::string get_property<unitpane_columns::UPC_FLAGS>(CUnit* unit, bool& bold, long& text_color, long& cell_color) 
        {
            std::stringstream ss;
            if (unit_control::flags::is_pillaging(unit)   )  ss << "$$";
            if (unit_control::flags::is_taxing(unit)      )  ss << '$';
            if (unit_control::flags::is_producing(unit)   )  ss << 'P';
            if (unit_control::flags::is_entertaining(unit))  ss << 'E';
            if (unit_control::flags::is_studying(unit)    )  ss << 'S';
            if (unit_control::flags::is_teaching(unit)    )  ss << 'T';        
            if (unit_control::flags::is_working(unit)     )  ss << 'W';
            if (unit_control::flags::is_moving(unit)      )  ss << 'M';  
            ss << '|';
            if (unit_control::flags::is_guard(unit)       )  ss << 'g';
            if (unit_control::flags::is_avoid(unit)       )  ss << 'a';
            if (unit_control::flags::is_behind(unit)      )  ss << 'b';
            
            if (unit->Flags & UNIT_FLAG_REVEALING_UNIT    )  ss << "rU";
            else if (unit->Flags & UNIT_FLAG_REVEALING_FACTION)  ss << "rF";
            
            if (unit_control::flags::is_hold(unit)        )  ss << 'h';
            
            if (unit_control::flags::is_noaid(unit)       )  ss << 'i';
            if (unit->Flags & UNIT_FLAG_CONSUMING_UNIT    )  ss << "cU";
            else if (unit->Flags & UNIT_FLAG_CONSUMING_FACTION)  ss << "cF";

            if (unit_control::flags::is_nocross(unit)     )  ss << 'x';
            if (unit->Flags & UNIT_FLAG_SPOILS_NONE       )  ss << "sN"; 
            if (unit->Flags & UNIT_FLAG_SPOILS_WALK       )  ss << "sW";
            if (unit->Flags & UNIT_FLAG_SPOILS_RIDE       )  ss << "sR";
            if (unit->Flags & UNIT_FLAG_SPOILS_FLY        )  ss << "sF";
            if (unit->Flags & UNIT_FLAG_SPOILS_SWIM       )  ss << "sS";
            if (unit->Flags & UNIT_FLAG_SPOILS_SAIL       )  ss << "sL";
            if (unit_control::flags::is_sharing(unit)     )  ss << 'z';
            return ss.str();        
        }

        template<>
        std::string get_property<unitpane_columns::UPC_ACTION>(CUnit* unit, bool& bold, long& text_color, long& cell_color) 
        {
            return "";
        }


    }



}