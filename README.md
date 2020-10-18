# alh (Atlantis Little Helper)

## How to build
### Requirements
It needs to have `wxWidgets` library installed/precompiled && `CMake` tools.

### Instructions
Linux/Mac (from root source folder):
Cmd: `rm -rf ./build; mkdir build; cd build; cmake -DCMAKE_BUILD_TYPE=Release ..`
Cmd: `clear; cd build; make -j 16`

For Windows it needs set up path to wxWidgets & use CMake. It is possible to use commands, similar to those for Linux, but also there is a nice way to use CMake GUI.

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

### TODO
* enable for users to choose a phase of calculations. [future]
* fix issue with speed of turtles in the ocean [future]
* new units not counted in production stats [non-reproducable]
* TAX_RULES -- should have default values, as STRUCTURES.
* Layout 1 failure -- figure out why [non-reproducable]
* Performance: `ResolveAlias` split to 2 new methods `AliasSkills` and `AliasGeneral`(for the rest weird aliases if they really exist)
* Performance: `CalcWeightsAndMovement` replace by existing `unit_control::get_weights(unit, weights)` && `unit_control::get_move_state(unit)`
* move to config settings for Phase Calculation
* move to config settings for Warehouse categories
* caravan autogenerate movement -> should be parsed
* unit_pane doesn't represent skills of a new unit with given skilled peasants
* allowed multiple monthlong orders (not just teach) - or is it a feature to avoid mistakes?
* add to hex filter possibility to show unit with specific orders/comments


## Changelog:
### Oct 18 2020
- UnitPane: added action_monthlong field. Short record about monthlong order.
- Weird bug: on close ALH it doesn't close MsgFrame. I didn't find why main frame stopped sending close event to the child window, so for a while it just have explicit call of it. Probably something based on new config reading/writing.
- 

### Oct 17 2020
- Config: added comments, which will be stored.
- Config property: added `leaders` for men, who are leaders (not hardcoded `LEAD`/`HERO`)
- Config: load/save, internal mechanic reworked.
- Terrain type WATER: optimized
- Create New Form: added additional custom orders window.
- Create New Form: added possibility to set up STUDY lvl.
- default layout is 4

### Oct 03 2020
- Caravan: Paint Roadmap unit's menu to paint arrow over regions of the caravan. Useful for caravans which spread silver over many regions.
- Region description: fixed issue with mixed ships & static buildings.
- Autologic: added `AO_NEED[X]` & `AO_SOURCE[X]` to search for regions where item X is needed and where is sourced with which priority. Function returns lowest priority for `AO_NEED` & highest priority for `AO_SOURCE`.
- Autoorders: added `EQUIP` command. `@;!EQUIP MSWO MARM MSHD P 12` will require listed items in amount of men in unit with specified priority (default is still 10). Also in combination with `STORE_ALL` is useful to collect spoils.
- Autoorders: fixed bug with unresolved items.
- Receive DLG: `FROM ALL` option became more brute: it will REMOVE all give & take orders, related to chosen item in the region. And then it will generate give/take order to get chosen item from any unit which have it.
- Unit popup menu: Share as caravan: shares items of the unit as if its a caravan.

### Sep 28 2020
- UnitPane: added `Sell All` command, which allows to sell all possible items to the market.
- Autoorders: `NEED`, `STORE`, `SOURCE` now can accept list of items in one command.
- Autoorders: added `STORE_ALL` command, which allows to store any non-men or silv item, and source them with provided priority. `@;!STORE_ALL P 90` will generate request for ALL accessible sourced items with priority 90, except silver and men. It works as STORE, but for all such items. Is useful to generate general flow for all items.

### Sep 27 2020
- enemy Warehouse by `Alt + W`
- bug: loading report with new info about 0 resources is represented correctly
- Teach: no automatical teaching to upcoming units if they were just created (server doesn't allow it)
- New Unit: automatically generated ID takes into accout region coordinates, so two `NEW X` units in the same region can be correctly handled.

### Aug 16 2020
- Messages and Errors: all region's coordinates by default are double-clickable
- Give: fixed bug with giving unit's order modifications
- Autologic: added function `SPEED[]`, `FACTION[]`, `REG_NAME[]` (last for filtering regions by name for quests)
- Autologic: added Land & Unit support.
- Region Filter Dialog: added instead of old filtering form. Based on Autologic.

### Aug 10 2020
- Selection of units: new region selection discards any selection. Selection of the same region discards filter, if was, but doesn't discard selection. Removed recursion.
- Movement: movement stop in case of movement is calculated for current region initially. That resolved some bugs based on movement logic.
- Fixed bug with Movement Phase calculation.
- Teaching: takes into account units, coming from other regions.
- Autoorders generation: for caravans, autonaming, moved to the beginning of order parsing.

### Jul 26 2020
- Autocommands: `$ne` now doesn't mean ignore order, but just supress order. We assume that if order will be partly parsed and any existing mechanism of prediction or analysis will start working incorrectly, its by intention of the user. Furthermore, order duplications and some other cases may be insensitive to `$ne` currently.
- Advance Give System (Ctrl+G): reload units immediately.
- Fixed bug with adding/removing column to unit pane (it didn't work ever)

### Jul 25 2020
- Hex description: crafted info shows attempt & actual craft.
- Autologic: $get, $warn, $cond and others now can provide phase on which they should be evaluated. For example `;move ;$GET 5 hors` will add 5 horses on phase of order move. Its possible to use any order to force evaluation at the phase of that order.
- orders now are not replaced, but modified.

### Jul 23 2020
- Unit Pane: added possibility to filter out units in Unit Pane by items they have (supposed to helr during redistribution of items). Works as popup or Ctrl+G.
- Unit Pane: receiveDlg in drop items list prints out amount of items, in drop units list units show their weight parameters also.
- Performance: added cache for ResolveAlias & ResolveAliasItems.
- Performance: Added cache for incoming units, recalculating lands now recalculates affection of other lands.
- Performance: Added order rules, suggesting resolving aliasing just for specific word, according to the order.
- Autoorders - Caravans: added automatic speed check.
- Movement: added check for ending in the ocean.
- Teaching: added correct behaving with students from other factions.
- RunOrders: only if order was actually modified.
- Region movement phase calculation: sailing bug fixed.
- internal: fixed memory bug.
- internal: fixed autologic bug, related to `!=`

### Jul 18 2020
- Hex Description: added Ctrl+E (for Economy output), Ctrl+W (for item's flow output), Ctrl+A (for autoorders status), Ctrl+M (for move phase status). So now this window may represent a region description by default or any of those info.
- Hex/Unit Description: double-click now allow to select unit, which number is printed in the line.
- Hex Description - removed Economy (shifted to Ctrl+E or land's popup)
- Autologic: added function `SPEED[]` to statements, which returns speed of the unit, and command `$HELP`, which prints out help regarding autologic.
- Produce: errors displayed correctly.
- Map Popup: added options for all `Ctrl+` from above.
- Region search: it prints out resulting name or number on the map.

### Jul 15 2020
A lot of strong new features:
- Region description: added items state reports (after economy) incuding information about current amount of items, incoming amount and outgoing amount. For next categories: Resources [PRP_RESOURCES], Equipment[PRP_ARMORS, PRP_WEAPONS, PRP_BOWS, PRP_SHIELDS], Artifacts[PRP_MAG_ITEMS], Mounts[PRP_MOUNTS, PRP_FLYING_MOUNTS], Trade items[PRP_TRADE_ITEMS]. Should be useful for analysis of the whole picture of region regarding stuff in it.
- Region description: shared resources now list all the shared resources, not just of a type which is used for production. Should be useful to see what is exactly shared in the region and can be used.
- Movement tracking: fixed bug, related to tracking units, coming from different levels.
- Teaching orders generation: now if teacher is sailing away, it will automatically try to teach units at the destination region (including those who will arrive there). Such units will be marked by specific comment in teaching order. Should help to manage teaching when a teacher or a student is sailing.
- Running orders in the end will automatically check economy of regions, and those who are below 0 will be printed out in events log. Should help to avoid unexpected starvation.
- Caravans: added `SAIL` variant of speed: now it loads stuff to not overweight the fleet.
- Region popup: added "Move phases" button, which prints out all moving phases with units presenting in the region at those phases. Taking into account speed, type of movement, territories and etc. Should help planning operations.
- Fleets: added expected speed of the vessel into parameters, to predict speed of the Fleet.
- Autonaming: unit without men automatically gets internal name `000`, to be sorted out.
- Autonaming: seriously upgraded logic and possibilities of unit's external naming.
- internal: fixed bug with economy calculation when unit has amount of silver below zero.
- internal: fixed bug with `move P` of unit with overweight.
- internal: fixed bug with region masks related to BUY_AMOUNT and BUY_PRICE.
- ReceiveDlg: fixed bug related to showing working units as moving units in drop-list of units.
- conditional logic: fixed bug, which lead to wrong calculation of `&&`

### Jul 05 2020
- name generation. Created rules which, based on items & races, generate names. Rules are part of dictionaries (can be found in `autonaming.cpp`, but were not moved out as a setting, so, hardcoded for a while). Is handled by `SZ_KEY_AUTONAMING`, as internal name generation. General idea: to not give a clue to enemy about meaning of unit by it's internal state, that's why names generated (mostly, with few obvious exceptions) based on visible part of unit.
With enabled feature any unit with order `@name unit ""` will automatically get generated name.
- changed interface of "Find Hexes" functionality. Added next evaluations:
`REG[NAME]` -- allows compare with region name property
`REG[MITH]` -- allows search for hexes where it is possible to produce `MITH` (0 -- means that the hex was checked, but item wasn't found, -1 -- no info)
`SELL_AMOUNT[MITH]`, `SELL_PRICE[MITH]`, `BUY_AMOUNT[MITH]`, `BUY_PRICE[MITH]` -- generally duplicate old interface

### Jun 22 2020
- internal: prod detail performance optimization
- internal: units_seq_ of region is now a std collection
- NEEDREG bugfix: it doesn't take into account silver of other known units, not belonging to faction.
- bugfix: GIVE order doesn't crush program anymore if filled incorrectly
- Production: takes into account also shared items for production.
- MSVC compatibility problem: removed `ERROR` enumeration name
- starting bugfix: Gate parsing doesn't lead to crush anymore.
- Region economy: added shared resources production statistics (it takes into account resources which will come to region with units, having `share 1` flag)

### May 31 2020
- check tax & trade: added more statistic info.
- economy bug: work income is counted during all the income calculation.
- moving/sailing: crush bug fixed.
- join: added join to list of known commands (still without parsing or checks).
- moving/sailing: unknown land counts as movepoint 1 cost.
- sailing: added `$owner` comment for cases when order to sail is given to unit, which is not owner, but is expected to be. For example, by entering the ship, or other faction is going to promote it.
- receive dlg: added monthlong action letter to names of giving units (to make it simple to differ different units with similar names)

### May 29 2020
- sailing/moving: calculates movepoints for the whole path (not just until it's not negative, as it was), print it out in events.
- bug: fixed bug with calculation of actual ship's speed.

### May 26 2020
- moving: unit's moving prediction now predicts two positions: where unit is expected to stay in the end of the turn, and where unit's order leads. All predictions now based on actual expected position of the unit in the end of turn.
- moving: moving now calculates automatically cases with overload, moving over ocean, flags (i.e. nocross) & etc.
- sailing: it checks weight, ships has types (flying or sailing -> ah.cfg change), it checks territories according to types, capacity & it also extends prediction for future position to all units in the fleet, not just to sailors.
- moving/sailing/config: added specific colour `SZ_UNIT_END_MOVEMENT_ORDER` for position in the tail of moving entire order.
- config: moving cost of ocean and lake reduced to 1, as it actually is. Internal logic is using flag & information about speed for correct prediction.
- ships moving: now prediction of ship's position is based on speed of the ship, written in the report.
- errors handling: multiple monthlong orders now is a usual check.
- bug: fixed empty skills issue invented in last commit.
- bug: fixed wrong taxers calculation invented in last commit.
- internal: all move & sail handling completely rewritten
- internal: all monthlong orders have their own flags
- internal: LAND_TAX_NEXT & LAND_TRADE_NEXT, and all monthlong unit's flags are finally in order.

### May 23 2020
- internal: performance tuning
- bug: ships capacity calculation, works correctly now.
- error: added the check for overweight during sailing to order parsing part.
- autonaming: more consistant name generation, added capacity for ship owners.
- receive dialog: name of item owners is better.

### May 21 2020
- config: added `UNIT_OWNS_BUILDING` color.
- unit pane: added specific color for buildings owners (it's not the frequent characteristic to keep the column for it from one side, but from another it's very important to know who is owner of the fleet).
- autonaming: ship owners has `weight/capacity` description in their names
- bug: fixed skill calculation when buying new men into existing unit with existing skills.
- bug: fixed entertain calculation, taking into account the skill level

### May 20 2020
- autonaming: added entirely new feature. in `alh.cfg` its flag "AUTONAMING", by default it is set to 0. It will automatically generate internal names for unit based on their orders and skills, according to hardcoded logic, which personally I use for many years (since I used this feature of internal names in AtlaClient). Names would look like `[MA] armo3 stea2`, and will be sorted accordingly. More detailed explanation you can find in documentation (see `doc/autonaming/README.md`).
- config: added `AUTONAMING` flag in `COMMON`.
- hex description: tax, work, entertain. If there is a unit taxing/working/entertaining in the region, it calculates amount of units need to collect all the region's income, and amount of current people doing it, and represent it in hex description inplace (in sections with tax/work/entertain sequentially).
- hex description: sell, buy. If there is a unit buying or selling something in the region, it will be represented inplace in sections `Wanted:` and `For Sale:`.
- hex description: economy: it supports and correctly calculated work & entertain income for Economy section.
- export hexes: bugfix. Removed whitespace.
- errors handling: added specific flag for errors, reflecting if unit has error. In unit pane such units will be marked by specific color.
- config: added specific `UNIT_HAS_ERRORS` color in `COLORS`.
- autologic: improvement. Now autologic comment executed exactly at phase to which the order belongs. 
For example, if unit has order `@sell all GRAI ;!COND ITEM[GRAI] >0`, it will check amount of GRAI at the phase of SELL, and will comment it out or uncomment accordingly.
- autologic: improvement: debug option. If instead of `COND` or `WARN` user writes `COND_D` or `WARN_D`, it gots all the details of logic performing. Should be helpful if condition works not as expected.
- config: tax. Added specific section `TAX_RULES`. With positions `SKILL_TAX` -- list of skills, allowed to tax by themselves. `NO_SKILL_TAX` -- list of items, which allows to tax without any skill. `SKILLS_LIST` -- list of skills, which have specific items, with which they can tax.
Current settings (as example) comes with this:
    [TAX_RULES]
    NO_SKILL_TAX            = SWOR, MSWO, RUNE, FSWO, LANC, PIKE, BAXE, ASWR, JAVE
    SKILL_TAX               = COMB
    SKILLS_LIST             = LBOW,XBOW
    LBOW                    = DBOW,LBOW
    XBOW                    = XBOW,MXBO,DBOW
Tax calculations now use those rules.
- flags: improvement. Now each monthlong order is represented my specific letter (`P` for production, `W` for work, `S` for studying and etc), and is separated from other flags by `|`. So it's easily seen in unit panel if unit doesn't have monthlong order, or which exactly order does it have.
- autoorders: improvement. Revert back autoorders to the end of GIVE phase. This means, it ignores sell orders (because sell phase happens after), but doesn't try to redistribute silver which received as a result of phases which happen after GIVE, and can't be handled at GIVE phase.
- autoorders: added possibility to run it for specific region.
- map pane: roads: fixed representation of road if there are few roads of one direction, and some of them are not finished, but some are finished.
- receive dialog: it calculates state up (including) GIVE phase. So it's impossible to see income which may happen after the give phase.
- internal: monthlong orders flags aligned & handled properly.
- internal: reworked handling of work & entertain in region.
- internal: region tracks amount of requests to tax, work & entertain, buy & sell.
- internal: added fully renewed LandWork & LandEntertain orders handling.



### May 11 2020
- internal: added autologic functionality.
- internal: in ResolveAliasItems fixed upper/lower case issue.
- internal: item's handling was modified, added handling of Properties inside, not a parallel logic.
- internal: give order parser & all infrastructure added.
- orders parsing: added basic checks for order's validity.
- map land filter: added variant `REG[NAME]` for region name, useful for searching of quests regions.
- sell order: added warning if attempt to sell more than market has
- autologic: added `;$COND` and `;$WARN` orders with evaluation logic (see `doc/autologic/README.md`). In case of `COND`, if statement evaluates to TRUE, then order is uncommented. Otherwise commented. In case of `WARN`, warning will be generated if statement evaluates to TRUE.
- pseudocommand: `;$get X ITEM` works again, artificially adding specified amount of items to unit.

### Apr 30 2020
- internal: added state to land: initial and current (a step for ships giving)
- internal: simplified & upgraded sailing events handling
- internal: give order parsing & handling upgraded (a step for ships giving)
- internal: build orders parser upgraded
- routeplanner: upgraded, now it works with swimming units (impacts caravans & Zorky's automove orders)
- caravan: acts as caravan just in regions from it's list
- region description: added `crafts` section after `production`
- production: added warning for production of items, which are not crafts, and not presented among resources of region
- shaft linking: fixed & upgraded, should work correctly now
- ships: correct forecast of sailors & weight
- teaching: feature, for leader students it prints `lead` instead of `man` in comments
- CreateNewUnit dialog: fixed representation of item if it's unknown to game data

### Apr 16 2020
- bug: fixed teach orders generation for cases when students were given/bought, it wasn't calculated correctly.

### Apr 15 2020
- bug: fixed resetting structure's membership.

### Apr 14 2020
- bug: fixed bug with structures which were not saved in history.
- internal: slightly upgraded structure management, shaft linking management.

### Apr 13 2020
- Ships: report slightly extended.
- Structures: fixed bug with representing non-mobile structures.

### Apr 12 2020
- internal: modified ships representation, fleets support.
- internal: unified and upgraded error handling.
- internal: buy order parsing upgraded.
- internal: upgraded `leave`, `enter`, `promote`.
- internal: structure parsing replaced.
- Ships: Fleets represented as ships, capacity, sailors amount and load calculated correctly.

### Apr 09 2020
- Flags: added parsing of spoils flags, support of all variants of `share` and `spoils` and others.
- Create New Unit dialog: added support for `share` and `spoils` flags
- Internal: all flags handling completely redone

### Apr 08 2020
- Land description: upgraded Economy, still requires work/entertain and claim.
- Internal: added better type system to messages
- Internal: buying handling with new mechanics
- Steal: better warnings (regarding item weight & visibility)

### Apr 06 2020
- Land description: all the parameters of Economy calculated correctly (except buying yet), added Balance
- Land description: report about errors is represented correctly if there is no errors :-)
- Land description: added building weight calculation until Fleet support
- Orders: each time we change selection of a region on the map, it recalculates Orders for the region.
- Orders: each time order is modified, it will be racalculated for the land.
 
### Apr 04 2020
- partly added Economy & errors in Land report.
- tax-trade-report: added claiming info
- tax-trade-report: fix, calculations for different factions separately
- tax-trade-report: fix, tax calculation restored
- added support of modern formats of next orders: `STUDY`, `PRODUCE` 
- added flag `share`
- added `EQUIP WEAP ARMO`, `X` as parameter to number (will be used current amount of man in the unit), `STORE` (works as `SOURCE` and `NEED` simultaneously)
- UI: deselects unit when we change the selection of a region
- new-unit-dialogue: fix, bug with Ctrl+F when no unit selected
- general: removed some internal events triggering
- autoorders: internal refactoring
- autoorders: fixed bug with duplicated orders
- orders: added internal parsers for specific orders

### Mar 17 2020
- Added fix for Windows build.

### Mar 03 2020
- Trade report is improved.
- MacOS: OSXDisableAllSmartSubstitutions() to avoid mess attempt.
- Autoorders: `;!STORE` added. Works for now exactly as a pair of `SOURCE` and `NEED` for the same item.

### Mar 01 2020
- internal: fixed event at changing orders of units.

### Feb 26 2020
- internal: requested_resources_ replaced by produced_items_ with extended meaning (also items which are not a resource)
- fixed production analysis.
- internal: RunOrder_LandProduce fixed.

### Feb 19 2020
- GetProdDetails: allows float variables. This means one skill level can produce 0.5 of item, if it actually does that.
- Attack/Steal/Assassinate parser upgraded: it adds to description of the unit about it's orders. Also it adds the record to description of target units, so it's easy to see if you assassinate someone twice and so on.
- ImmediateProdCheck now influence just the actual modification of production, but all errors, warnings and messages will be calculated and printed.
- Too much items in GIVE order is a warning, but not error: it will be described, and actual amount of items will be changed according to rules of server.
- GIVE 0 is valid and proceed correctly.
- Disabled SZ_KEY_CHECK_TEACH_LVL as redundand.
- internal: RunLandOrders accepts starting and ending TurnSequence. So it is possible to run specific part of RunOrders.
- internal: Basic initialization of RunLandOrders happens in TurnSequence::SQ_FIRST
- internal: get_item_amount now can return initial amount of items and current (after parsing of orders) amount of items
- internal: perform_on_each_unit is using same sequence of units as server would use.

### Feb 02 2020
- internal: produce flow refactored. Added resources_ & requested_resources_ parameters to region.
- internal: extracted `RunOrder_LandStudyTeach` & `RunOrder_LandProduce` functions.
- internal: added config parsing object `NameAndAmount` to handle `NAME X` parameters
- region produce report extended: added info regarding requested production this turn.
- unit produce report extended: added modifications of items, prediction of unit state according to them.
- flags report: added `rU`/`rF` & `cU`/`cF` for reveal/consume unit/faction versus old `r` & `c`
- bugfix: handling of TURN-ENDTURN orders intruduced at Jan 31 2020.
- bugfix: config crash on CAhApp::GetMaxRaceSkillLevel.
- bugfix: duplication of production.

### Jan 31 2020
- internal: added sanity check mechanism for orders and O_ERROR as a type in case of error
- bugfix: turn-endturn are not considered as orders anymore
- autoorders: caravans will give out items just in listed regions
- ReceiveDlg: groups are not listed if there is no item in the region with an item from the group

### Jan 25 2020
- Autoorders: added automatic deduction of the move order for caravan, if there is no move order presented, it will be generated automatically. List of regions now is the complete definition for the path and move orders. The sequence of regions in the order is the sequence of regions to visit for autogeneration.

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
