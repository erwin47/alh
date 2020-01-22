# alh (Atlantis Little Helper)

## Main info
* Autoorders description: ./doc/autoorders/README.md
* Client names format: `@;;Name`, invisible "internal" name. Will be represented instead of unit's name in the client, but will be inivisible for other players.

For example, I use notation:
`@;;[{X}] {y} skill{z}` for my units, where
`{X}` -- type, such as `A` for an army, `M` for masters, `P` for production and etc. It's useful to have them sorted in the client by their real meaning. Sometimes I extend it, and then `AL` -- for longbows, `AX` for crossbows and so on.
`{y}` -- amount of peasants/leaders in the unit. It may be represented in specific column, but I prefer to see it there.
`z` -- level of skill. Sometimes I extend it with `+`, if it's in the process of studying.
Quite useful to manage big faction, and to not reveal to others the purpose of your units.

* Shortcuts: 
    - Ctrl + R -- "Receive" window.
    - Ctrl + F -- form new unit window.

* Description window.
Now it has description of unit in the beginning of the turn, list of actions and description in the end of the turn. With items and skills prediction.

* Teaching.
After pressing "Teach" option, unit will automatically get all possible teaching orders for all possible students. Avoiding students, who is already under full teaching, who needs just 30 days to their upper limit and so on. Also with prediction in unit's description.
Also there will be explicitely written if any of students is not studying anymore.

* ReceiveDlg.
In alh.cfg there is a section `RECEIVE_DLG_SETTINGS` related to it. In field `REC_DLG_GROUPS` may be listed groups with which ReceiveDlg will work, as with items. For example, `REC_DLG_GROUPS = trade_items,food` mean, that in ReceiveDlg in drop up list there will be possibility to choose `trade_items` or `food`, and get list of all units having any items belonging to the group. Those entities (trade_items and food) have to be defined in section `UNIT_PROPERTY_GROUPS`.

## Changelog:
### Jan 23 2020
- internal: Autoorders - kind of optimization, now unit stores caravan's info as a sign of being caravan, which happens once per RunLandOrders and reduces amount of order parsing 4-5 times.
- format of `@;!REGION` was extended, it may be just copy of region name like `plain (17,19)`, or any other combination of literals, it will actually collect just numbers separated with `,` and deduce Plane level if needs.
- internal: started preparing to use Zorky's automove mechanism to generate move orders according to current list of `@;!REGION`.

### Jan 22 2020
- Skill mixing bug fix (when gives peasants/leaders from unit to unit).
- Autoorders fix: caravan will not give items to another caravan if they have same route (identical list of regions).

### Jan 19 2020
- Sell order: logic changed.
    * trying to sell more than you can (if you can more than 0) is not a warning anymore, but message about it may arrive in Unit Description field. Prediction will be done correctly according to actual possibility to sell.
    * trying to sell all (if you can more than 0, but less than "all") is not a warning anymore, but message about it may arrive in Unit Description field. Prediction will be done correctly according to actual possibility to sell.
    * trying to sell more than market accepts (if you can more than 0) is not a warning anymore, but message about it may arrive in Unit Description field. Prediction will be done correctly according to actual possibility to sell.
    * if unit tries to sell less than it have, and less than market accepts, its a new message in Unit Description field. Prediction will be done correctly.
    * messages became more correct and precise.
- ReceiveDlg: group logic added, instead of choosing an item, it is possible to choose a group, defined in UNIT_PROPERTY_GROUPS.
- Internal: sell order parsing was moved to new rails.
- Internal: teaching/studying bug with new units fixed.

### Jan 18 2020
- Studying/Teaching update: number in "Teaching" field represents amoun if students for teacher and amount of days left for teaching for stundents.
- Internal: Studying/Teaching parsing during the turn finally was moved to new logic.

### Jan 14 2020
- receiveDlg: bugfix related adding new lines to orders, which lead to empty lines or concatinating one order to another.
- autoorders: first caravans unload themselves, if possible, then load. Generally this makes them work more effectively.
- ISSUE: if caravan A is full have to give items to caravan B, caravan B at the same time is full, and have to give another items to A, and no one has a free space, then they will not exchange items because of weight restrictions. For not it is suggested to use a unit-storage in regions where such issue may happen.
- autoorders: added extended description if item is giving to caravan: for which region, to which unit, which amount and priority.

### Jan 13 2020
- autoorders: SOURCE now also have priority. If it is set, t will be used for NEED with higher priority only (higher means that the number is lower). CARAVAN's items not protected by it's NEED have no priority (will be given to NEED with any priorities).
- autoorders: added sanity check for caravan speed and region coordinates (it prints in unit's description if something is wrong with them). Also extended comment ";!ao", now it explans for which request with which priority it is given. May be useful to understand what is going on, if something is going with autoorders not as expected.

### Jan 10 2020
- Internal: orders representation was modified
- Major: ApplyDefaultOrders now runs autoorders feature. Will be described below.
- CreateNewUnitDlg bugfix.
- Internal: clever weight calculation based on new data representation.

#### AutoOrders:
Feature was designed to automate caravans and production. 
There is an order to determine unit as a source of item. And order to determine unit as the need of item with priority.
Also a way to determine caravans, which will be able to collect items from sources for needs in regions where this caravan is heading.

##### List of orders:
- order: `@;!SOURCE X ITEM` means, that this unit should be considered as a source of item `ITEM`. Amount of available items is calculated as `current amount` of unit's items minus `X`.
Example1: unit with `10 WOOD` has order `@;!SOURCE 4 WOOD`, this means it may give out up to 6 (10-4) items of WOOD.
Example2: unit with `10 WOOD` has order `@;!SOURCE 15 WOOD`, this means it will not give out any WOOD this turn.
Example3: unit with `10 WOOD` has order `@;!SOURCE 0 WOOD`, this means it may give out up to 10 items of WOOD.
- order: `@;!NEED X ITEM P Y` means, that this unit needs `X` items `ITEM` with priority `Y`. It is possible to set `X=-1`, then it will be considered as endless need, it should be useful for storages. If there will be few units which need same item, first will receive item one with lowest `Y` (the lower Y - the higher priority). By default Y=10 for `X>=0` and Y=20 for `X=-1`.
Example1: unit with `10 WOOD` has order `@;!NEED 4 WOOD`, this means it will not try to receive any items this turn, because it already has more. Request has priority 10.
Example2: unit with `10 WOOD` has order `@;!NEED 15 WOOD P 15`, this means it will need 5 (5, because it already has 10) WOOD with priority 15.
Example3: unit with `10 WOOD` has order `@;!NEED -1 WOOD P 25`, this means it need as many wood as it can receive with priority 25.
- order: `@;!NEEDREG X ITEM P Y` -- same as need, but it takes into account all `ITEM` in region.
Example1: unit with `1000 SILV` has order `@;!NEEDREG 3000 SILV`. And in the same region there is another unit of this faction, which has `1500 SILV`. Then actual demand will be `500 SILV` (3000 - 1000 - 1500)
- order: `@;!CARAVAN SPEED` means that this unit is considered as caravan with speed at least equal to `SPEED`. It may take next values: `MOVE`|`WALK`|`RIDE`|`FLY`|`SWIM`. This means that it is a caravan, which will try to load itself being able to move at least with specified speed.
Example1: `@;!CARAVAN MOVE` means it is a caravan, and it's moving type should be at least MOVE. It will not load item if it lose ability to move.
Example2: `@;!CARAVAN WALK` same as MOVE.
Example3: `@;!CARAVAN RIDE` means it is a caravan, and it's moving type should be at least RIDE. It will not load item if it lose ability to ride.
- order: `@;!REGION X1,Y1,Z1 X2,Y2,Z2 ...` is order which has meaning just if it is a caravan. Then it consider regions listed in `@;!REGION` as target regions for caravan. X,Y,Z -- are coordinates of region, where Z -- is number of level (nexus = 0, surface = 1 and so on, but your ALH may be set up differently)
Example1: `@;!REGION 15,17,1 20,22,1` -- means that if it is a caravan, and it is located in `15,17,1`, then it will try to load items according to needs of `20,22,1`. If it is located in `20,22,1`m it will try to load items according to needs of `15,17,1`. If it's located in any else region, it will try to load items according to needs of both those regions.

Each item in unit which is CARAVAN is considered as a SOURCE. To preserve items in caravan it needs to add him `@;!NEED X ITEM` order.
Example: unit has next orders: `@;!CARAVAN RIDE`, `@;!NEED 2 WELF`, `@;!NEED 10 HORS`. This means all items of this unit may be given out if there will be requests, except `2 WELF` and `10 HORS`.

##### How it works.
To be clean, I'll descrive steps.
- clean all unit's orders marked as `;!ao` -- this is a mark of orders, generated by autoorders.
- run current orders, so the state of ALH will take into account all orders, except autogenerated.
- move over regions one by one (from left to right, from top to bottom), so if caravan is heading to already processed region, it will not take into account fulfilled needs of the region. And vice versa, if it is heading to region which was not processed, it may take needs.
- within each region:
    * generate map of all sources (including units with `@;!SOURCE` order and caravans).
    * generate map of needs (from units with `@;!NEED`)
    * generate needs of caravans. Iterate over each `REGION` (except current) in list of regions, and if there are needs, this caravan would get get similar `NEED` request.
    * sort all `NEED` requests according to their priority.
    * iterate over `NEED` requests, trying to fulfill them from `SOURCE` if possible. If current `NEED` request belong to caravan, then it's `SPEED` and amount of possible weight is taken into account.
    * for each positive amount of items which may be given from `SOURCE` to `NEED`, generate `GIVE` order with comment `;!ao`, as a sign of autogenerated order.

##### Recomendations
* inside one region we assume that giving items should be done by `@give` order. It's not suggested to use `NEED`/`SOURCE` for transfering items inside one region, if you do not expect once that items would be transfered somewhere (where request had lower priority), or a caravan from somewhere will bring items because it saw `NEED` and coundn't expect that it is fulfulled.
* don't set up two caravans moving simultaneusly over one route. For each `NEED` from target region each of those caravans would generate it's own need, so amount of items transfered there would be doubled.
* it may happen that item will be transfered to caravan, but not to local unit, if `NEED` of a unit in target caravan's region has higher priority. But that's the goal. If you have sequence of caravans, then items from one caravan may be taken by another caravan instead of giving/taking to/from storage.
* if you have a region which requires finite amount of goods regularly, it's recommended to create there a NEED with high priority and specified amount of items, and create there a storage: a NEED with low priority, which also SOURCE the same item. So at first X items will be bypassed to unit which requires them with high priority, and the rest stored in Storage and may be will be given in future.

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
