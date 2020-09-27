
#ifndef UNIT_TO_PANE_CONTROL_INCL
#define UNIT_TO_PANE_CONTROL_INCL

#include "data.h"
#include <string>
#include <map>

namespace unit_control
{
    namespace unitpane_control
    {
        enum class unitpane_columns
        {
            UPC_MEN = 1,
            UPC_FLAGS = 2,
            UPC_ACTION = 3
        };

        static std::map<unitpane_columns, std::string> columns = {
            {unitpane_columns::UPC_MEN, "PRP_MEN"},
            {unitpane_columns::UPC_FLAGS, "PRP_FLAGS"},
            {unitpane_columns::UPC_ACTION, "PRP_ACTION"},
        };

        template<unitpane_columns COLUMN>
        void evaluate_conditions(CUnit* unit, bool& bold, long& text_color, long& cell_color);

        template<unitpane_columns COLUMN>
        std::string get_property(CUnit* unit, bool& bold, long& text_color, long& cell_color);

    }



}

#endif //UNIT_TO_PANE_CONTROL_INCL