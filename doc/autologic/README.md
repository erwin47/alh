# alh (Atlantis Little Helper)
## AutoLogic:
Feature was designed to automate some orders handing & filtering the map. 
Its generally agnostic to whitespaces.

### List of actions:
    `;!WARN`/`;$WARN` -- generate warning if statement will be evaluated to TRUE
    `;!COND`/`;$COMD` -- will comment out order if statement evaluated to FALSE, and uncomment it if TRUE.

### List of evaluations:
    ITEM[NAME] -- represent amount of specified by `NAME` items in unit (current or in a region, depends on context)
    SKILL[NAME] -- represent level of specified by `NAME` skill in unit (current or in a region, depends on context)
    LOC[X,Y(,Z)] -- is true if unit is in the region with specified coordinates. Coordinate `Z` may be omitted (1 by default)
    SELL[NAME] -- represent amount of specified by `NAME` items in region to sell
    WANTED[NAME] -- represent amount of specified by `NAME` items in region to buy
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
    ||  - logica or.

### Evaluation:
Evaluation happens at the moment, when the order is evaluated. That's why, for example, desision to comment or uncomment of order 
`sell 5 GEM ;!COND ITEM[GEM] >= 5` will happens at phase of SELL, and if unit received 5 gems during GIVE phase, it will be taken into account.

### Example:
For example, we need `unit A` to buy as max as possible GEMs if it has 3000 silver in region 24,24,1. And then move to region 24,26,1 and attempt sell 5, and the rest give to `unit B, which should try to sell the rest`. Then they may have next orders:
`unit A`
@move S ;!COND LOC[24,24]
@buy all GEM ;!COND LOC[24,24]&& ITEM[SILV] >=3000

@move N ;!LOC[24,26]
@give `unit B` all GEM except 5 ;!COND LOC[24,26]
@sell all GEM ;!COND LOC[24,26]

`unit B`
@sell all GEM ;!COND ITEM[GEM]>0
@;!WARN SKILL["horse riding"] > 2

#### Explanation:
Unit A:
in region 24,24 all orders with `LOC[24,24]` will be uncommented. And all orders with `LOC[24,26]` will be commented (as any other orders which evaluates to FALSE). So unit A will get order to `move S` & buy all gems if also it has silver more or equal to 3000.

Next turn in region 24,26 all orders with `LOC[24,24]` will become commented, and orders with `LOC[24,26]` will become uncommented. So it will give all GEMs except 5 to another unit & will try to sell the rest, and move north.

Unit B:
As only it will reach skill of "horse training" more than 2, it will generate warning.
Also it will try to sell GEM in case it has them.
