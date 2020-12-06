# alh (Atlantis Little Helper)
## AutoLogic:
Feature was designed to automate some orders handing & filtering the map. 
Its generally agnostic to whitespaces and other non-letter/number symbols, except `[` and `]`.
Each statement should have evaluation at left side of operator and value at right side of operator.

### List of non-conditional actions:
    `;!GET`/`;$GET` -- adds item to unit (example: `;!GET 200 SILV`).
    `;!OWNER`/`;$OWNER` -- sets unit to be owner of building. Client can't really predict who will become owner in certain cases, but if user knows this info, its possible to set it up this way.
    `;!HELP`/`;$HELP` -- prints out show summary regarding the feature.

### List of conditional actions:
    `;!WARN`/`;$WARN` -- generate warning if statement will be evaluated to TRUE
    `;!COND`/`;$COND` -- will comment out order if statement evaluated to FALSE, and uncomment it if TRUE.
    `;!CONDEL`/`;$CONDEL` -- same as COND, but the entire order line will be deleted if a statement evaluates to FALSE, but the order line was uncommented before (i.e. the state was TRUE befure). Should be useful for orders which have to be executed once.
    `;!COND_D`/`;$COND_D` & `;!WARN_D`/`;$WARN_D` & `;!CONDEL_D`/`;$CONDEL_D` for debug mode. In debug mode all the statement calculations happen explicit, so its possible to observe what exactly went wrong if something not expected is happening.

### List of functions:
    ITEM[NAME] -- returns amount of specified by `NAME` items in unit
    SKILL[NAME] -- returns level of specified by `NAME` skill in unit
    SPEED[] -- returns the speed of unit (i.e. 2,4,6)
    FACTION[] -- returns faction number of unit
    AO_NEED[NAME] -- returns amount of unit requested items specified by `NAME` in AutoOrders terms. Useful for Hex Autologic Search.
    AO_SOURCE[NAME] -- returns amount of unit sourced items specified by `NAME` in AutoOrders terms. Useful for Hex Autologic Search.
    REG_NAME[] -- returns current region name.
    LOC[X,Y(,Z)] -- returns true if unit is in the region with specified coordinates. Coordinate `Z` may be omitted (1 by default), otherwise returns false.
    SELL[NAME] -- returns amount of specified by `NAME` items in region to sell
    SELL_PRICE[NAME] -- returns price of specified by `NAME` items in region to sell
    WANTED[NAME] -- represent amount of specified by `NAME` items in region to buy
    WANTED_PRICE[NAME] -- represent amount of specified by `NAME` items in region to buy
    RESOURCE[NAME] -- represent amount of specified by `NAME` items in region to produce

### List of operators:
    >=  - more or equal
    >   - more
    <=  - less or equal
    <   - less
    =   - equal
    ==  - equal
    !=  - not equal
    &&  - logical and
    ||  - logical or

### Evaluation:
Evaluation of autologic happens at a specified turn phase, at the moment, when the order is evaluated. That's why, for example, desision to comment or uncomment of order 
Example:
`sell 5 GEM ;!COND ITEM[GEM] >= 5` will happens at phase of SELL, and if unit received 5 gems during GIVE phase, it will be taken into account.
Another example:
`@study TACT ;!COND ITEM[SILV]>=200` -- will be evaluated at `STUDY` phase, so, if the unit will get 200 silver during a phase, happening before (i.e. `SELL` or `GIVE`), it will be evaluated to `True` & uncommented, even if unit didn't have this amount of silver in the beginning. And the opposite, if unit starts with 200 SILV, but give them out, it will be evaluated to `False`, and order will be commented out.

Its also possible to specify a phase where we want autologic to be evaluated. For this it needs just to add commented command without arguements in front of autologic.
For example:
`;move ;!GET 200 SILV` -- unit will get 200 SILV at a move phase.


### Example:
For example, we need `unit A` to buy as max as possible GEMs if it has 3000 silver in region 24,24,1. And then move to region 24,26,1 and attempt sell 5, and the rest give to `unit B, which should try to sell the rest`. Then they may have next orders:
`unit A`
@move S ;!COND LOC[24,24]
@buy all GEM ;!COND LOC[24,24]&& ITEM[SILV] >=3000
@;move ;!WARN SPEED[]!=4

@move N ;!COND LOC[24,26]
@give `unit B` all GEM except 5 ;!COND LOC[24,26]
@sell all GEM ;!COND LOC[24,26]

`unit B`
@sell all GEM ;!COND ITEM[GEM]>0
@;!WARN SKILL["horse riding"] > 2

#### Explanation:
Unit A:
In region 24,24 
    move order will be uncommented. 
    buy order will be uncommented if unit has 3000 silver or more.
    if at move phase (after buying) speed of unit will not be equal to 4, warning will be generated.
    all orders with `LOC[24,26]` will be commented (as any other orders which evaluates to FALSE). So unit A will get order to `move S` & buy all gems if also it has silver more or equal to 3000.

Next turn in region 24,26 all orders with `LOC[24,24]` will become commented, and orders with `LOC[24,26]` will become uncommented. So it will give all GEMs except 5 to another unit & will try to sell the rest, and move north.

Unit B:
As only it will reach skill of "horse training" more than 2, it will generate warning.
Also it will try to sell GEM in case it has them.
