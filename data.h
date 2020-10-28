/*
 * This source file is part of the Atlantis Little Helper program.
 * Copyright (C) 2001 Maxim Shariy.
 *
 * Atlantis Little Helper is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atlantis Little Helper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atlantis Little Helper; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __DATA_H_INCL__
#define __DATA_H_INCL__

#include "wx/string.h"

#include "order_parser.h"
#include "cstr.h"
#include "collection.h"
#include "objs.h"
#include <string.h>
#include "compat.h"
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <string>

typedef enum {GT=0,GE,   EQ,   LE,   LT,  NE, NOP} eCompareOp;

// common properties
#define PRP_ID                          "id"
#define PRP_NAME                        "name"
#define PRP_FULL_TEXT                   "fulltext"

// special properties
#define PRP_ORG_NAME                    "org$name"
#define PRP_ORG_DESCR                   "org$descr"

// unit properties
#define PRP_FACTION_ID                  "factionid"
#define PRP_FACTION                     "faction"
#define PRP_LAND_ID                     "landid"
#define PRP_STRUCT_ID                   "structid"
#define PRP_COMMENTS                    "comments"
#define PRP_ORDERS                      "orders"
#define PRP_STRUCT_OWNER                "structowner"
#define PRP_STRUCT_NAME                 "structname"
#define PRP_TEACHING                    "teaching"
#define PRP_WEIGHT                      "weight"
#define PRP_WEIGHT_WALK                 "weight_walk"
#define PRP_WEIGHT_RIDE                 "weight_ride"
#define PRP_WEIGHT_FLY                  "weight_fly"
#define PRP_WEIGHT_SWIM                 "weight_swim"
#define PRP_BEST_SKILL                  "best_skill"
#define PRP_BEST_SKILL_DAYS             "best_skill_days"
#define PRP_MOVEMENT                    "movement"
#define PRP_SILVER                      "SILV"
#define PRP_LEADER                      "leadership"

#define PRP_FLAGS_STANDARD              "flags_standard"
#define PRP_FLAGS_CUSTOM                "flags_custom"
#define PRP_FLAGS_CUSTOM_ABBR           "flags_custom_abbr"


#define PRP_GUI_COLOR                   "gui_color"
#define PRP_MEN                         "men"
#define PRP_MEN_LEADER                  "men_leaders"
#define PRP_SKILLS                      "skills"
#define PRP_MONTHLONG_ACTION            "action_monthlong"
#define PRP_MAG_SKILLS                  "mag_skills"
#define PRP_STUFF                       "stuff"
#define PRP_FOOD                        "foods"
#define PRP_FLYING_MOUNTS               "flying_mounts"
#define PRP_MOUNTS                      "mounts"
#define PRP_ARMORS                      "item_armors"
#define PRP_WEAPONS                     "item_weapons"
#define PRP_RESOURCES                   "resources"
#define PRP_BOWS                        "item_bows"
#define PRP_SHIELDS                     "item_shields"
#define PRP_MAG_ITEMS                   "mag_items"
#define PRP_JUNK_ITEMS                  "junk_items"
#define PRP_TRADE_ITEMS                 "trade_items"
#define PRP_SEL_FACT_MEN                "sel_fact_men"
#define PRP_SEQUENCE                    "sequence"
#define PRP_DESCRIPTION                 "description"
#define PRP_COMBAT                      "combat_spell"
#define PRP_FRIEND_OR_FOE               "attitude"

#define PRP_SKILL_POSTFIX                  "_"
#define PRP_SKILL_STUDY_POSTFIX            "_s"
#define PRP_SKILL_EXPERIENCE_POSTFIX       "_x"
#define PRP_SKILL_DAYS_POSTFIX             "_d"
#define PRP_SKILL_DAYS_EXPERIENCE_POSTFIX  "_dx"
#define PRP_GIVE_ALL_POSTFIX               "_ga"

// land properties
//#define PRP_SALE_AMOUNT_POSTFIX         "_s"
//#define PRP_SALE_PRICE_POSTFIX          "_s$"
//#define PRP_WANTED_AMOUNT_POSTFIX       "_w"
//#define PRP_WANTED_PRICE_POSTFIX        "_w$"
#define PRP_SALE_AMOUNT_PREFIX         "Sell.Amount."
#define PRP_SALE_PRICE_PREFIX          "Sell.Price."
#define PRP_WANTED_AMOUNT_PREFIX       "Want.Amount."
#define PRP_WANTED_PRICE_PREFIX        "Want.Price."
#define PRP_RESOURCE_PREFIX            "Resource."
#define PRP_LAND_LINK                  "*landlink"

#define MOVE_MODE_MAX                   5

#define LAND_FLAG_COUNT  3

// land flags
#define LAND_UNITS          0x00000001
#define LAND_VISITED        0x00000002
#define LAND_TAX            0x00000004
#define LAND_TRADE          0x00000008
#define LAND_BATTLE         0x00000010
#define LAND_SET_EXITS      0x00000020
#define LAND_HAS_FLAGS      0x00000040
#define LAND_IS_CURRENT     0x00000080

#define LAND_STR_HIDDEN     0x00000100
#define LAND_STR_MOBILE     0x00000200
#define LAND_STR_SHAFT      0x00000400
#define LAND_STR_GATE       0x00000800
#define LAND_STR_ROAD_N     0x00001000
#define LAND_STR_ROAD_NE    0x00002000
#define LAND_STR_ROAD_SE    0x00004000
#define LAND_STR_ROAD_S     0x00008000
#define LAND_STR_ROAD_SW    0x00010000
#define LAND_STR_ROAD_NW    0x00020000
#define LAND_STR_GENERIC    0x00040000
#define LAND_STR_SPECIAL   (LAND_STR_HIDDEN  | LAND_STR_MOBILE  | LAND_STR_SHAFT   | LAND_STR_GATE   | \
                            LAND_STR_ROAD_N  | LAND_STR_ROAD_NE | LAND_STR_ROAD_SE | LAND_STR_ROAD_S | \
                            LAND_STR_ROAD_SW | LAND_STR_ROAD_NW)

#define LAND_TAX_NEXT       0x00080000
#define LAND_TRADE_NEXT     0x00100000
#define LAND_LOCATED_UNITS  0x00200000
#define LAND_LOCATED_LAND   0x00400000
#define LAND_TOWN           0x00800000
#define LAND_CITY           0x01000000
#define LAND_IS_WATER       0x02000000

// alarm flags
#define PRESENCE_OWN        0x0001
#define PRESENCE_FRIEND     0x0002
#define PRESENCE_NEUTRAL    0x0004
#define PRESENCE_ENEMY      0x0008
#define GUARDED_BY_OWN      0x0010
#define GUARDED_BY_FRIEND   0x0020
#define GUARDED_BY_NEUTRAL  0x0040
#define GUARDED_BY_ENEMY    0x0080
#define GUARDED             0x0100
#define CLAIMED_BY_OWN      0x0200
#define CLAIMED_BY_FRIEND   0x0400
#define CLAIMED_BY_NEUTRAL  0x0800
#define CLAIMED_BY_ENEMY    0x1000
#define ALARM               0x2000


// structure attributes
#define SA_HIDDEN   0x0001
#define SA_MOBILE   0x0002
#define SA_SHAFT    0x0004
#define SA_GATE     0x0008
#define SA_ROAD_N   0x0010
#define SA_ROAD_NE  0x0020
#define SA_ROAD_SE  0x0040
#define SA_ROAD_S   0x0080
#define SA_ROAD_SW  0x0100
#define SA_ROAD_NW  0x0200
#define SA_ROAD_BAD 0x0400

// unit flags - standard flags from the top, custom from the bottom

#define UNIT_FLAG_ENTERTAINING      0x80000000
#define UNIT_FLAG_WORKING           0x40000000
#define UNIT_FLAG_TEACHING          0x20000000
#define UNIT_FLAG_STUDYING          0x10000000
#define UNIT_FLAG_PILLAGING         0x08000000
#define UNIT_FLAG_TAXING            0x04000000
#define UNIT_FLAG_PRODUCING         0x02000000
#define UNIT_FLAG_GUARDING          0x01000000
#define UNIT_FLAG_AVOIDING          0x00800000
#define UNIT_FLAG_BEHIND            0x00400000
#define UNIT_FLAG_REVEALING_UNIT    0x00200000
#define UNIT_FLAG_REVEALING_FACTION 0x00100000
#define UNIT_FLAG_HOLDING           0x00080000
#define UNIT_FLAG_RECEIVING_NO_AID  0x00040000
#define UNIT_FLAG_CONSUMING_UNIT    0x00020000
#define UNIT_FLAG_CONSUMING_FACTION 0x00010000
#define UNIT_FLAG_NO_CROSS_WATER    0x00008000
#define UNIT_FLAG_SPOILS_NONE       0x00004000
#define UNIT_FLAG_SPOILS_WALK       0x00002000
#define UNIT_FLAG_SPOILS_RIDE       0x00001000
#define UNIT_FLAG_SPOILS_FLY        0x00000800
#define UNIT_FLAG_SPOILS_SWIM       0x00000400
#define UNIT_FLAG_SPOILS_SAIL       0x00000200
#define UNIT_FLAG_SHARING           0x00000100
#define UNIT_FLAG_TEMP              0x00000080
#define UNIT_FLAG_GIVEN             0x00000040
#define UNIT_FLAG_HAS_ERROR         0x00000020
#define UNIT_FLAG_MOVING            0x00000010

#define UNIT_CUSTOM_FLAG_COUNT   4
#define UNIT_CUSTOM_FLAG_MASK    0xF

#define NO_LOCATION         (-1)

extern const char * STD_UNIT_PROPS[];
extern const int    STD_UNIT_PROPS_COUNT;


/*extern const char * GRP_MEN;
extern const char * GRP_SKILLS;
extern const char * GRP_MAG_SKILLS;
extern const char * GRP_STUFF;
extern const char * GRP_HORS;
extern const char * GRP_MAG_ITEMS;
extern const char * GRP_JUNK_ITEMS;
extern const char * GRP_TRADE_ITEMS;
extern const char * GRP_ARMOURS;
extern const char * GRP_WEAPONS;*/
extern const char * STRUCT_GATE;

enum {
    ATT_FRIEND1     = 0,
    ATT_FRIEND2,
    ATT_NEUTRAL,
    ATT_ENEMY,
    ATT_UNDECLARED
};

//#define NEW_UNIT_ID(_n, _FactId) ((_FactId << 16) | _n)

static long compose_new_unit_id(long x, long y, long z, long Fid, long id) {
    assert(x < UINT16_MAX / 16);
    assert(y < UINT16_MAX / 16);
    assert(z < UINT8_MAX);
    assert(Fid < UINT16_MAX);
    assert(id < UINT16_MAX);
    return ((z << 56) | (y << 44) | (x << 32) | (Fid << 16) | id);
}

//assuming that x, y, z, FactId are below 4000, number is below 64k*256
//z -- 8 bytes
//y & x -- 12 bytes each
//faction id -- 16 bytes
//new unit id -- 16 bytes
#define NEW_UNIT_ID(_x, _y, _z, _FactId, _n) compose_new_unit_id(_x, _y, _z, _FactId, _n)

#define REVERSE_NEW_UNIT_ID(_n) (_n & 0xFFFF)
#define IS_NEW_UNIT_ID(_Id)   ((_Id & 0xFFFF0000) != 0)
#define IS_NEW_UNIT(_pUnit)   IS_NEW_UNIT_ID(_pUnit->Id)
#define GET_X_FROM_UNIT_ID(_Id) ((_Id >> 32) & 0xFFF)
#define GET_Y_FROM_UNIT_ID(_Id) ((_Id >> 44) & 0xFFF)
#define GET_Z_FROM_UNIT_ID(_Id) ((_Id >> 56) & 0xF)

typedef enum { North=0, Northeast,   Southeast,   South,   Southwest,   Northwest, Center }   eDirection;


class CPlane;

//-----------------------------------------------------------------

class CBaseObject : public TPropertyHolder
{
public:
    CBaseObject();
    virtual ~CBaseObject(){};
    virtual void Done();


    virtual const char  * ResolveAlias(const char * alias);
    virtual BOOL GetProperty(const char  *  name,
                             EValueType   & type,
                             const void  *& value, // returns pointer to inner location
                             EPropertyType  proptype = eNormal
                            );
    long Id;
    CStr Name;
    CStr Description;

    void SetName(const char * newname);
    void SetDescription(const char * newdescr);
    void ResetName();
    void ResetDescription();
    virtual void ResetNormalProperties();

    virtual void DebugPrint(CStr & sDest);
    virtual void Clear() {Id=0; Name.Empty(); Description.Empty();};

};

//-----------------------------------------------------------------

class CBaseColl : public CCollection
{
public:
    CBaseColl();
    CBaseColl(int nDelta);
protected:
    virtual void FreeItem(void * pItem);
};

//-----------------------------------------------------------------

class CBaseCollById : public CSortedCollection
{
public:
    CBaseCollById();
    CBaseCollById(int nDelta);
protected:
    virtual void FreeItem(void * pItem);
    virtual int Compare(void * pItem1, void * pItem2) const;
};

//-----------------------------------------------------------------

class CItem
{
public:
    long amount_;
    std::string code_name_;

    bool operator<(const CItem& item) const {
        return code_name_ < item.code_name_;
    }
    bool operator==(const CItem& item) const {
        return code_name_ == item.code_name_;
    }    
};

class CProductMarket
{
public:
    long        price_;
    CItem       item_;
};

struct Skill
{
    long study_price_;
    std::string short_name_;
    std::string long_name_;

    bool operator<(const Skill& sk) const { return long_name_ < sk.long_name_; }
};

//-----------------------------------------------------------------

class CProductColl : public CSortedCollection
{
public:
    CProductColl()            : CSortedCollection()      {};
    CProductColl(int nDelta)  : CSortedCollection(nDelta){};
protected:
    virtual void FreeItem(void * pItem) {delete (CItem*)pItem;};
    virtual int Compare(void * pItem1, void * pItem2) const
    {return(SafeCmp( ((CItem*)pItem1)->code_name_.c_str(),
                     ((CItem*)pItem2)->code_name_.c_str() ));};
};

//-----------------------------------------------------------------

class CFaction : public CBaseObject
{
public:
    CFaction() : CBaseObject() {UnclaimedSilver=0;};
    long UnclaimedSilver;

    virtual void DebugPrint(CStr & sDest);
};

//-----------------------------------------------------------------

class CAttitude : public CBaseObject
{
    public:
        int FactionId;
        int Stance;
        void    SetStance(int new_stance);
        BOOL    IsDeclaredAs(int attitude);
        BOOL    IsEqual(CAttitude *attitude);
};

//-----------------------------------------------------------------

struct UnitState
{
    std::string                 name_;
    std::string                 description_;
    std::set<CItem>             men_;
    CItem                       silver_;
    std::set<CItem>             items_;
    
    std::map<std::string, long> skills_;

    long                        struct_id_;//!<struct to which it belongs (0 if none)
};

class CUnit : public CBaseObject
{
public:
    CUnit();
    virtual ~CUnit();
    void GetBestSkill(wxString &name, long &days);
    virtual BOOL GetProperty(const char  *  name,
                             EValueType   & type,
                             const void  *& value, // returns pointer to inner location
                             EPropertyType  proptype = eNormal
                            );
    virtual CStrStrColl * GetPropertyGroups();
    void    ExtractCommentsFromDefOrders();
    virtual void ResetNormalProperties();
    void    CheckWeight(CStr & sErr);
    void    CalcWeightsAndMovement();

    CUnit * AllocSimpleCopy();

    static void         LoadCustomFlagNames();
    static void         ResetCustomFlagNames();
    static const char * GetCustomFlagName(int no);

    //when we load map in the beginning, we should preserve CUnit initial state.
    //and then should have possibility to reset CUnit by preserved initial state.
    //until we don't have that, we have to duplicate members (or have any other similar 
    //mechanisms) to have possibility restore state of CUnit. THat's for *_initial_.

    //UnitState current_state_;
    //UnitState initial_state_;

    std::set<CItem> men_;
    std::set<CItem> men_initial_;

    CItem           silver_;
    CItem           silver_initial_;

    std::set<CItem> items_;
    std::set<CItem> items_initial_;
    
    //list of received items, tought and so on, 
    std::vector<std::string> impact_description_;
    bool                     has_error_;

    orders::UnitOrders orders_;

    std::map<std::string, long> skills_;
    std::map<std::string, long> skills_initial_;

    std::shared_ptr<orders::CaravanInfo> caravan_info_;

    long struct_id_;//!<struct to which it belongs (0 if none)
    long struct_id_initial_;//!<struct to which it belongs (0 if none)

    std::string monthlong_descr_; //!< hint for flags

    //hex id where unit should stop (if movements_.size()>0 it must be positive):
    //it may be calculated stop position, or it will be the last hex id of movements_
    long              movement_stop_;
    std::vector<long> movements_;

    bool            IsOurs;
    long            FactionId;
    CFaction      * pFaction;
    long            LandId;
    long            Weight[MOVE_MODE_MAX];
    CStr            Comments;
    CStr            DefOrders;
    CStr            Orders;
    CStr            OrdersDecorated;
    CStr            Errors;
    CStr            Events;
    CStr            ProducingItem;

    unsigned long   Flags;
    unsigned long   FlagsOrg;
    unsigned long   FlagsLast;
    int             reqMovementSpeed; // When moving, it must have this speed. Long paths are allowed.

    static CStrStrColl * m_PropertyGroupsColl;

    virtual void DebugPrint(CStr & sDest);
protected:
    void    AddWeight(int nitems, int * weights, const char ** movenames, int nweights);

    static CStr     m_CustomFlagNames[UNIT_CUSTOM_FLAG_COUNT];
    static BOOL     m_CustomFlagNamesLoaded;

};



//-----------------------------------------------------------------
enum class SHIP_TRAVEL {
  NONE = 0,
  SAIL,
  FLY
};
class CStruct : public CBaseObject
{
public:
    CStruct() : CBaseObject(), LandId(0), OwnerUnitId(0), Attr(0), occupied_capacity_(0),
                                SailingPower(0), capacity_(0), MinSailingPower(0)
    {}
    virtual void ResetNormalProperties();
    long LandId;
    long OwnerUnitId;
    long Attr;
    long SailingPower;
    long MinSailingPower;
    int  Location;
    
    long dest_land_id_;//for shafts
    long occupied_capacity_;
    long capacity_;//for MOBILE
    std::string type_;
    SHIP_TRAVEL travel_;
    long max_speed_;
    std::string name_;
    std::string original_description_;
    std::vector<std::pair<std::string, long> > fleet_ships_;//for fleets
};

//-----------------------------------------------------------------

/*
class CEdgeStruct : public CStruct
{
public:
    CEdgeStruct() : CStruct() {};
    int   Direction;
};
*/
struct CStructure
{
    std::string                 name_;
    long                        capacity_;
    long                        flags_;
    std::vector<std::string>    substructures_;
    CLand*                      land_;
    std::vector<CUnit*>         units_;    
};

struct CError
{
    std::string type_;//"Error", "Warning"
    CUnit* unit_;
    std::string message_;
};

struct CEconomy
{
    long initial_amount_;
    long maintenance_;
    long work_income_;
    long buy_expenses_;
    long study_expenses_;
    long moving_out_;
    long moving_in_;
    long tax_income_;
    long sell_income_;
    long claim_income_;
};

void init_economy(CEconomy& res);

//-----------------------------------------------------------------

struct land_item_state
{
    long amount_;
    long requested_;
    long requesters_amount_;
};

void init_land_item_state(land_item_state& listate);

struct LandState
{
    land_item_state                         tax_;                   //! stats regarding tax activity in the region
    land_item_state                         work_;                  //! stats regarding work activity in the region
    land_item_state                         entertain_;             //! stats regarding entertain activity in the region

    long                                    peasants_amount_;       //! peasants amount of region
    std::string                             peasant_race_;          //! race of region

    std::vector<CItem>                      resources_;             //! stats regarding resources available for 
                                                                    //! production in the region  
    std::map<std::string, CProductMarket>   wanted_;                //! stats regarding buy offers in the region
    std::map<std::string, CProductMarket>   for_sale_;              //! stats regarding sale offers in the region

    std::map<std::string, std::pair<long, long>> shared_items_;     //! stats regarding shared items in the region (actual left/initially)
    std::map<std::string, std::pair<long, long>> produced_items_;   //! stats regarding attempts to produce items from region (actual/attempt)
    std::map<std::string, long>             sold_items_;            //! stats regarding attempts to sell items to region
    std::map<std::string, long>             bought_items_;          //! stats regarding attempts to buy items from region

    std::vector<CStruct*>                   structures_;            //! list of structures

    CEconomy                                economy_;               //! state of all silver flows
    std::vector<CError>                     run_orders_errors_;     //! list of errors happened during the run orders
};

void init_land_state(LandState& lstate);

//each region is affecting other regions
class AffectionsInfo
{
    //moving to/from handling
    std::set<CLand*>                                  affected_lands_;//! set of lands which were affected by current
    std::unordered_map<CLand*, std::vector<CUnit*>>   incoming_units_;//! separated by foreign lands from which they come
    std::unordered_map<CLand*, std::vector<CUnit*>>   going_to_come_units_;//! separated by foreign lands from which they come

public:
    //! cleans all affectors from affecting current region
    //! expected usage: during deletion of the land: all other lands shouldn't affect deleted land
    void          clear_affections_of_others(CLand* current);
    //! cleans all affected from affections of current region
    //! expected usage: during rerunning of orders: initially land should not affect any other land
    void          clear_affected(CLand* current);

    //setters
    //! adds land to list of affected lands. This land should get incoming and/or going_to_income unit
    inline void   add_affected(CLand* land)                   {  affected_lands_.insert(land);  }
    //! adds land & unit to list of incoming units of current land.
    inline void   add_incoming(CLand* land, CUnit* unit)      { incoming_units_[land].push_back(unit); }
    //! adds land & unit to list of going_to_come units of current land.
    inline void   add_going_to_come(CLand* land, CUnit* unit) { going_to_come_units_[land].push_back(unit); }

    //getters
    inline std::set<CLand*>&                                affected_lands()      {  return affected_lands_;  }
    inline std::unordered_map<CLand*, std::vector<CUnit*>>& incoming_units()      {  return incoming_units_; }
    inline std::unordered_map<CLand*, std::vector<CUnit*>>& going_to_come_units() {  return going_to_come_units_;  }
};


class CLand : public CBaseObject
{
public:
    CLand();
    virtual ~CLand();

    BOOL      AddUnit(CUnit * pUnit);
    void      RemoveUnit(CUnit * pUnit);
    void      DeleteAllNewUnits(int factionId);
    void      ResetUnitsAndStructs();
    void      SetFlagsFromUnits();
    //CStruct * AddNewStruct(CStruct * pNewStruct);
    void      RemoveEdgeStructs(int direction);
    void      AddNewEdgeStruct(const char * name, int direction);
    int       GetNextNewUnitNo();
    void      SetExit(int direction, int x, int y);
    void      CloseAllExits(); // disable all exits
    void      ResetAllExits(); // assume all exits would exist
    int       FindExit(long hexId) const; // returns direction or negative if not found.

    virtual void DebugPrint(CStr & sDest);

    //long          Taxable;

    CStr          TerrainType;
    CStr          CityName;
    CStr          CityType;
    CStr          FlagText[LAND_FLAG_COUNT];
    CStr          Exits;
    CStr          Events;
    //CBaseCollById Structs;
    CBaseCollById Units;
    std::vector<CUnit*> units_seq_;// keeps units in the sequence they were met in the report
    CBaseColl     EdgeStructs;
    CProductColl  Products;

    LandState initial_state_;
    LandState current_state_;

    //Flags handling
    unsigned long Flags;
    std::string                             text_for_region_search_;//! text used to be represented if region was a 
                                                                    //! result of a Region search command

    //info related to affected of one region by others
    //currently used for MovingFromOtherRegion info
    AffectionsInfo affections_;

    unsigned long AlarmFlags;
    int           xExit[6]; // storage for the coordinates of the exit
    int           yExit[6]; // storage for the coordinates of the exit
    int           CoastBits;
    CPlane      * pPlane;
    int           AtlaclientsLastTurnNo;
    BOOL          WeatherWillBeGood;
    double        Wages;
    long          Troops[ATT_UNDECLARED+1];

    //long          guiUnit;  // will be used by GUI only
    int           guiColor; // will be used by GUI only

    int           TotalMovementCost; // Used for calculating routes
    long          ArrivedFromHexId; // HexId we came from
    CStr          MoveDirection; // Direction we came from
};

const int EXIT_CLOSED = -255;
const int EXIT_MAYBE  = -254;

//-----------------------------------------------------------------

#define TROPIC_ZONE_MAX   0x00001000    // should be no less than XY_DELTA in data.cpp!

class CPlane : public CBaseObject
{
public:
    CPlane() : Lands(32) {EastEdge=0;  WestEdge=0;   Width=0;
                          EdgeSrcId=0; EdgeExitId=0; EdgeDir=0; ExitsCount=0;
                          TropicZoneMin  = TROPIC_ZONE_MAX; TropicZoneMax  = -(TROPIC_ZONE_MAX);
                         };
    virtual ~CPlane();

    CBaseCollById Lands;
    int           EastEdge;
    int           WestEdge;
    int           Width;

    long          EdgeSrcId ;
    long          EdgeExitId;
    int           EdgeDir   ;
    long          ExitsCount; // is used to determine 1.0.0 history file

    long          TropicZoneMin;
    long          TropicZoneMax;

};

//-----------------------------------------------------------------

class CShortNamedObj : public CBaseObject
{
public:
    CShortNamedObj() : CBaseObject() {Level = 0;};
    CStr ShortName;
    long Level;

};

class CBattle : public CBaseObject
{
public:
    CStr LandStrId;
};

//-----------------------------------------------------------------

class CBaseCollByName : public CBaseCollById
{
public:
    CBaseCollByName()           : CBaseCollById()       {};
    CBaseCollByName(int nDelta) : CBaseCollById(nDelta) {};
protected:
    virtual int Compare(void * pItem1, void * pItem2) const;
};

//-----------------------------------------------------------------

#define MAX_RES_NUM 8

class TProdDetails
{
public:
    std::string skill_name_;
    long skill_level_;

    double per_month_;
    std::vector<std::pair<std::string, double>> req_resources_;

    std::string tool_name_;
    long tool_plus_;

    void clear();
};

//-----------------------------------------------------------------

class CGameDataHelper
{
public:
    void         ReportError       (const char * msg, int msglen, BOOL orderrelated);
    //long         GetStudyCost      (const char * skill);
    //bool         GetStructAttr     (const char * kind, long & MaxLoad, long & MinSailingPower, long& flags);
    const char * GetConfString     (const char * section, const char * param);
    BOOL         GetOrderId        (const char * order, long & id);
    BOOL         IsTradeItem       (const char * item);
    BOOL         IsMan             (const char * item);
    const char * GetWeatherLine    (BOOL IsCurrent, BOOL IsGood, int Zone);
    const char * ResolveAlias      (const char * alias);
    bool         ResolveAliasItems (const std::string& phrase, std::string& codename, std::string& name, std::string& name_plural);
    BOOL         GetItemWeights    (const char * item, int *& weights, const char **& movenames, int & movecount );
    void         GetMoveNames      (const char **& movenames);
    BOOL         GetTropicZone     (const char * plane, long & y_min, long & y_max);
    const char * GetPlaneSize      (const char * plane);
    void         SetTropicZone     (const char * plane, long y_min, long y_max);
    std::shared_ptr<TProdDetails> GetProdDetails(const char * item);
    BOOL         ImmediateProdCheck();
    BOOL         CanSeeAdvResources(const char * skillname, const char * terrain, CLongColl & Levels, CBufColl & Resources);
    BOOL         IsRawMagicSkill   (const char * skillname);
    int          GetAttitudeForFaction(int id);
    void         SetAttitudeForFaction(int id, int attitude);
    void         SetPlayingFaction (long id);
    BOOL         IsWagon           (const char * item);
    BOOL         IsWagonPuller     (const char * item);
    int          WagonCapacity     ();
};

extern CGameDataHelper * gpDataHelper;

//-----------------------------------------------------------------

class CTaxProdDetails
{
public:
    CTaxProdDetails() {HexCount=0; FactionId=0; amount=0;}

    long  FactionId;
    long  HexCount;
    long  amount;
    CStr  Details;
};

class CTaxProdDetailsCollByFaction : public CSortedCollection
{
    public:
        CTaxProdDetailsCollByFaction() : CSortedCollection() {};
        CTaxProdDetailsCollByFaction(int nDelta) : CSortedCollection(nDelta) {};
    protected:
        virtual void FreeItem(void * pItem)
        {
            CTaxProdDetails * p = (CTaxProdDetails*)pItem;
            delete p;
        };
        virtual int Compare(void * pItem1, void * pItem2) const
        {
            CTaxProdDetails * p1 = (CTaxProdDetails*)pItem1;
            CTaxProdDetails * p2 = (CTaxProdDetails*)pItem2;

            if (p1->FactionId > p2->FactionId)
                return 1;
            else
                if (p1->FactionId < p2->FactionId)
                    return -1;
            else
                return 0;
        };
};

//-----------------------------------------------------------------



long LandCoordToId(int x, int y, int z);
void LandIdToCoord(long id, int & x, int & y, int & z);
void TestLandId();
BOOL IsASkillRelatedProperty(const char * propname);
void MakeQualifiedPropertyName(const char * prefix, const char * shortname, CStr & FullName);
void SplitQualifiedPropertyName(const char * fullname, CStr & Prefix, CStr & ShortName);
BOOL EvaluateBaseObjectByBoxes(CBaseObject * pObj, CStr * Property, eCompareOp * CompareOp, CStr * Value, long * lValue, int count);

#endif
