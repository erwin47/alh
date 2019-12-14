# alh (Atlantis Little Helper)

## Changelog:
### Dec 14 2019
- Internal: items of units splitted to 3 categories: man, silver, items.
- Added (experimental) skills prediction. Teaching updated.
- Again, many bug fixes.

### Dec 12 2019
- Internal: CEditPane replaced by CUnitOrderEditPane, specified for orders of unit.
- Internal: Added list of skills to CUnit. Initial and modified (to implement studying prediction)
- Internal: GenOrdersTeach rewritten.
- Internal: RunOrder_Teach rewritten.
- "Teach" popup menu works a bit differently:
    it checks all units, if they may be tought (fixed issue with units which need 30 days to max level, and thus don't need teaching)
    it checks all existing teachers and their coverage of students.
    it adds comment in front of each teach command, describing tought skill, current amount of days of unit, max amount of days of unit, amount of man in unit.
    it adds ALL students with room for teaching to teaching list. Assumption is that user can easily remove students from the list, but he will always get full list of students with room for teaching.
- "Buy" and "Sell" modifies amount of items in unit.
- Fixed many minor bugs related to last changes (corner cases, useless warnings and so on).

### Dec 07 2019
- Added order_parser functionality.
- Added usage of order_parser at LoadOrders.
- Fixed issue with ShareSilv
- Refactored Teach popup function: now it adds comment to teacher describing students and to student describing fact of teaching
- Added "COMMON" - "PEASANT_CAN_TEACH" flag, which may be set to 1/0, allows client to handle peasants as teachers.
- Added CUserOrderPane (but no use yet). Have to replace current CEditPane.
- Added functions to ReceiveDLG: SILV now is always first in the list, if it exists, there is option "FROM ALL" which allows to get chosen item from all units in region.

### Dec 03 2019
- Added LAYOUT 4: it unites "Hex Description" and "Unit Description" windows.
- fixed minor bugs.

### Dec 01 2019
- Added to Unit Description screen partial actual state of unit (flags, name and items)
- @;;XXXXX -- with this order now name will be replaced by XXXXX in region list of units.
- Extended description for new units
- Small bugfixes

### Nov 30 2019
- Fixed bug with representation of unit description (print out everything between ";" and end of description)

### Nov 27 2019
- ReceiveDLG: removed from list of items which are already left from other units
- ReceiveDLG: fixed bug with concatinating order to previous one
- FormNewDLG: removed from list of units units which have no silver

### Nov 23 2019
- Removed empty lines at automatic order's addings

### Nov 19 2019
- Fixed small "writings" in orders.
- Fixed resolve Aliases function.
- Fixed "none" issue with advanced resources.
- No data about race warning added

### Nov 16 2019.
- Removed `SilvRcvd` variable -- as redundant and useless.
- Fixed bug with recursion of `Def Orders`
- Added "impact_description" functionality, recording indirect events

### Nov 15 2019.
- Initial message, summarizing last month.
- Now we have next list of functional changes:
    * Cmake instead of old good gnu tools
    * Ctrl + R -- Receive form to receive items
    * Ctrl + F -- Create new unit form, to create new unit
