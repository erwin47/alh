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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <algorithm>

#include "stdhdr.h"

#include "data.h"
#include "data_control.h"

#include "cstr.h"
#include "collection.h"
#include "cfgfile.h"
#include "files.h"
#include "atlaparser.h"
#include "consts.h"
#include "consts_ah.h"
#include "objs.h"
#include "hash.h"

#include "ahapp.h"

CGameDataHelper * gpDataHelper = NULL;

const char * STRUCT_GATE    = "GATE";

const char * STD_UNIT_PROPS [] =
{
    PRP_FACTION_ID           ,
    PRP_FACTION              ,
    PRP_LAND_ID              ,
    PRP_STRUCT_ID            ,
    PRP_COMMENTS             ,
    PRP_ORDERS               ,
    PRP_STRUCT_OWNER         ,
    PRP_STRUCT_NAME          ,
    PRP_TEACHING             ,
    PRP_MOVEMENT             ,
    PRP_DESCRIPTION          ,
    PRP_COMBAT               ,
    PRP_GUI_COLOR            ,
    PRP_MEN                  ,
    PRP_BEST_SKILL           ,
    PRP_BEST_SKILL_DAYS      ,
    PRP_SKILLS               ,
    PRP_MAG_SKILLS           ,
    PRP_STUFF                ,
    PRP_MOUNTS               ,
    PRP_WEAPONS              ,
    PRP_ARMORS               ,
    PRP_MAG_ITEMS            ,
    PRP_JUNK_ITEMS           ,
    PRP_SEL_FACT_MEN         ,
    PRP_SEQUENCE             ,
    PRP_FRIEND_OR_FOE
};
const int STD_UNIT_PROPS_COUNT = sizeof(STD_UNIT_PROPS)/sizeof(*STD_UNIT_PROPS);

const char * SKILL_UNIT_POSTFIXES [] =
{
    PRP_SKILL_POSTFIX                ,
    PRP_SKILL_STUDY_POSTFIX          ,
    PRP_SKILL_EXPERIENCE_POSTFIX     ,
    PRP_SKILL_DAYS_POSTFIX           ,
    PRP_SKILL_DAYS_EXPERIENCE_POSTFIX,
};
const int SKILL_UNIT_POSTFIXES_COUNT = sizeof(SKILL_UNIT_POSTFIXES)/sizeof(*SKILL_UNIT_POSTFIXES);


//=============================================================

BOOL IsASkillRelatedProperty(const char * propname)
{
    int          i;
    const char * p;

    for (i=0; i<SKILL_UNIT_POSTFIXES_COUNT; i++)
    {
        p = strrchr(propname, SKILL_UNIT_POSTFIXES[i][0]);
        if (p && 0==strcmp(p, SKILL_UNIT_POSTFIXES[i]))
            return TRUE;
    }
    return FALSE;
}

//=============================================================

CBaseObject::CBaseObject() : Name(32), Description(128)
{
    Id = 0;
}

//-------------------------------------------------------------

void CBaseObject::Done()
{
}

//-------------------------------------------------------------

const char * CBaseObject::ResolveAlias(const char * alias)
{
    if (gpDataHelper)
        return gpDataHelper->ResolveAlias(alias);
    else
        return alias;
}

//-------------------------------------------------------------

BOOL CBaseObject::GetProperty(const char  *  name,
                              EValueType   & type,
                              const void  *& value,
                              EPropertyType  proptype
                              )
{
    BOOL          Ok = TRUE;

    name = ResolveAlias(name);

    if (!TPropertyHolder::GetProperty(name, type, value, proptype))
    {
        if      (0==stricmp(name, PRP_ID))
        {
            type  = eLong;
            value = (void*)Id;
        }
        else if (0==stricmp(name, PRP_NAME))
        {
            type  = eCharPtr;
            value = Name.GetData();
        }
        else if (0==stricmp(name, PRP_FULL_TEXT))
        {
            type  = eCharPtr;
            value = Description.GetData();
        }
        else
            Ok = FALSE;
    }
    return Ok;
}

//-------------------------------------------------------------

void CBaseObject::SetName(const char * newname)
{
    EValueType    type;
    const void  * value;

    // save org value if not saved
    if (!GetProperty(PRP_ORG_NAME, type, value, eNormal))
        SetProperty(PRP_ORG_NAME, eCharPtr, (void*)Name.GetData(), eBoth);
    Name = newname;
}

//-------------------------------------------------------------

void CBaseObject::SetDescription(const char * newdescr)
{
    EValueType    type;
    const void  * value;

    // save org value if not saved
    if (!GetProperty(PRP_ORG_DESCR, type, value, eNormal))
        SetProperty(PRP_ORG_DESCR, eCharPtr, (void*)Description.GetData(), eBoth);
    Description = newdescr;
}

//-------------------------------------------------------------

void CBaseObject::ResetName()
{
    EValueType    type;
    const void  * value;

    if (GetProperty(PRP_ORG_NAME, type, value, eNormal) && eCharPtr==type)
        Name = (const char *)value;
}

//-------------------------------------------------------------

void CBaseObject::ResetDescription()
{
    EValueType    type;
    const void  * value;

    if (GetProperty(PRP_ORG_DESCR, type, value, eNormal) && eCharPtr==type)
        Description = (const char *)value;
}

//-------------------------------------------------------------

void CBaseObject::ResetNormalProperties()
{
    TPropertyHolder::ResetNormalProperties();
    ResetName();
    ResetDescription();
}

//-------------------------------------------------------------

void CBaseObject::DebugPrint(CStr & sDest)
{
    sDest << "\n"
          << "Id          = " << Id << "\n"
          << "Name        = " << Name.GetData() << "\n"
          << "Description = " << Description.GetData() << "\n";
}

//=============================================================

void CFaction::DebugPrint(CStr & sDest)
{
    CBaseObject::DebugPrint(sDest);

    sDest << "UnclaimedSilver = " << UnclaimedSilver << "\n";
}

//=============================================================

void CAttitude::SetStance(int newstance)
{
    Stance = newstance; // include some logic for precedence JR
}

//-------------------------------------------------------------

BOOL CAttitude::IsDeclaredAs(int attitude)
{
    return (Stance==attitude);
}

//-------------------------------------------------------------

BOOL CAttitude::IsEqual(CAttitude * attitude)
{
    return (attitude->IsDeclaredAs(Stance));
}

//=============================================================

CLand::CLand() : CBaseObject()
{
    Flags=0;
    AlarmFlags=0;
    //guiUnit=0;
    pPlane=NULL;
    CoastBits=0;
    AtlaclientsLastTurnNo=0;
    guiColor=-1;
    WeatherWillBeGood=FALSE;
    Wages = 0.0;
    init_land_state(initial_state_);
    init_land_state(current_state_);
    units_seq_.reserve(10);
    for(int i=0; i<=ATT_UNDECLARED; i++) Troops[i]=0;
    ResetAllExits();
}

//-------------------------------------------------------------

CLand::~CLand()
{
    affections_.clear_affections_of_others(this);

    ResetUnitsAndStructs();
    //Structs.FreeAll();
    EdgeStructs.FreeAll();
    
    DeleteAllNewUnits();
    
    //UnitsSeq.DeleteAll();
    Products.FreeAll();
}

void CLand::DebugPrint(CStr & sDest)
{
    CBaseObject::DebugPrint(sDest);

    sDest << "Taxable   = " << current_state_.tax_.amount_ << "\n";
}

//-------------------------------------------------------------

BOOL CLand::AddUnit(CUnit * pUnit)
{
    units_seq_.push_back(pUnit);
    pUnit->LandId = Id;//TODO PRP_SEQ remove
    pUnit->SetProperty(PRP_SEQUENCE, eLong, (void*)units_seq_.size(), eNormal);
    Flags |= LAND_UNITS;
    return TRUE;
}

//-------------------------------------------------------------

void CLand::remove_new_unit(CUnit* unit)
            {
                units_seq_.erase(std::remove_if(units_seq_.begin(), units_seq_.end(), [&](CUnit* p) {
        if (unit->Id == p->Id) {
            delete(p);
            return true;
        }
        return false;
                    }), units_seq_.end());
}

//-------------------------------------------------------------

void CLand::DeleteAllNewUnits(int factionId)
{
    // Delete all new units of factionId, or all new units if factionId==0.
    units_seq_.erase(std::remove_if(units_seq_.begin(), units_seq_.end(), [&](CUnit* unit) {
            if (IS_NEW_UNIT(unit) && (unit->FactionId == factionId || 0 == factionId)) {
                delete(unit);
                return true;
            }
            return false;
        }), units_seq_.end());
}

void init_economy(CEconomy& res)
{
    res.initial_amount_ = 0;
    res.buy_expenses_ = 0;
    res.claim_income_ = 0;
    res.maintenance_ = 0;
    res.work_income_ = 0;
    res.moving_in_ = 0;
    res.moving_out_ = 0;
    res.sell_income_ = 0;
    res.study_expenses_ = 0;
    res.tax_income_ = 0;
}

void init_land_item_state(land_item_state& listate)
{
    listate.amount_ = 0;
    listate.requested_ = 0;
    listate.requesters_amount_ = 0;
}

void init_land_state(LandState& lstate)
{
    init_land_item_state(lstate.tax_);
    init_land_item_state(lstate.work_);
    init_land_item_state(lstate.entertain_);
    lstate.peasants_amount_ = 0;
    lstate.resources_.clear();
    lstate.shared_items_.clear();
    lstate.produced_items_.clear();    
    lstate.sold_items_.clear();
    lstate.bought_items_.clear();
    lstate.wanted_.clear();
    lstate.for_sale_.clear();
    lstate.structures_.clear();

    init_economy(lstate.economy_);
    lstate.run_orders_errors_.clear();
}

//cleans all affectors from affecting current region
void AffectionsInfo::clear_affections_of_others(CLand* current) 
{
    //need to notify all other objects that this land is not affected by them anymore, because it doesn't exist
    for (auto& pair : incoming_units_)
    {
        pair.first->affections_.affected_lands_.erase(current);
    }
    for (auto& pair : going_to_come_units_)
    {
        pair.first->affections_.affected_lands_.erase(current);
    } 
}

//cleans all affected from affections of current region
void AffectionsInfo::clear_affected(CLand* current)
{
    for (CLand* rem_land : affected_lands_)
    {
        if (rem_land->affections_.incoming_units_.find(current) != rem_land->affections_.incoming_units_.end())
            rem_land->affections_.incoming_units_.erase(current);
    
        if (rem_land->affections_.going_to_come_units_.find(current) != rem_land->affections_.going_to_come_units_.end())
            rem_land->affections_.going_to_come_units_.erase(current);
    }
    affected_lands_.clear();  
}


void CLand::ResetUnitsAndStructs()
{
    current_state_ = initial_state_;

    land_control::perform_on_each_unit(this, [](CUnit* unit) {
        unit->ResetNormalProperties();
        unit->movements_.clear();
        unit->movement_stop_ = 0;
    });

    land_control::moving::perform_on_each_incoming_unit(this, [](CUnit* unit) {
        //we need to reset skills for case of moving ship with studying passangers, 
        //because each recalculation of current region will update skills, and they have to be reset.
        //Silver also have to be reset.
        unit->skills_ = unit->skills_initial_;
    });

    affections_.clear_affected(this);

    for (size_t k=0; k<initial_state_.structures_.size(); k++)
    {
        initial_state_.structures_[k]->ResetNormalProperties();
    }
}

//-------------------------------------------------------------

int CLand::GetNextNewUnitNo()
{
    int     no = 1;
    for (CUnit* unit : this->units_seq_)
    {
        if (IS_NEW_UNIT(unit) && (REVERSE_NEW_UNIT_ID(unit->Id) >= no))
            no= REVERSE_NEW_UNIT_ID(unit->Id);
    }

    return no;
}

//-------------------------------------------------------------
/*
CStruct * CLand::AddNewStruct(CStruct * pNewStruct)
{
    int       idx;
    CStruct * pStruct;

    if (Structs.Search(pNewStruct, idx))
    {
        pStruct = (CStruct*)Structs.At(idx);
//        if (0==stricmp(pStruct->Kind.GetData(), "Shaft") )
        if (pStruct->Attr & SA_SHAFT  )
        {
            // preserve links to shafts
            int  x1, x2, x3;
            BOOL Link;

            x1   = pNewStruct->original_description_.find(";");
            x2   = pNewStruct->original_description_.find("links");
            x3   = pNewStruct->original_description_.find("to");
            Link = ( x1 != std::string::npos && x1<x2 && x2<x3 );

            if (Link || pNewStruct->original_description_.size() > pStruct->original_description_.size())
                pStruct->original_description_= pNewStruct->original_description_;
        }
        else
            pStruct->original_description_= pNewStruct->original_description_;
        //pStruct->Name           = pNewStruct->Name       ;
        pStruct->LandId         = pNewStruct->LandId     ;
        pStruct->OwnerUnitId    = pNewStruct->OwnerUnitId;
        pStruct->occupied_capacity_           = pNewStruct->occupied_capacity_       ;
        pStruct->Attr           = pNewStruct->Attr       ;
        pStruct->type_          = pNewStruct->type_      ;
        pStruct->capacity_      = pNewStruct->capacity_      ;
        pStruct->name_          = pNewStruct->name_      ;
	    pStruct->Location       = pNewStruct->Location   ;
        pStruct->fleet_ships_   = pNewStruct->fleet_ships_   ;
        delete pNewStruct;
    }
    else
    {
        Structs.Insert(pNewStruct);

        //flags manipulation
        if (0 == pNewStruct->Attr)                 Flags |= LAND_STR_GENERIC;
        else
        {
            if (pNewStruct->Attr & SA_HIDDEN )     Flags |= LAND_STR_HIDDEN ;
            if (pNewStruct->Attr & SA_MOBILE )     Flags |= LAND_STR_MOBILE ;
            if (pNewStruct->Attr & SA_SHAFT  )     Flags |= LAND_STR_SHAFT  ;
            if (pNewStruct->Attr & SA_GATE   )     Flags |= LAND_STR_GATE   ;
            if (pNewStruct->Attr & SA_ROAD_N )     Flags |= LAND_STR_ROAD_N ;
            if (pNewStruct->Attr & SA_ROAD_NE)     Flags |= LAND_STR_ROAD_NE;
            if (pNewStruct->Attr & SA_ROAD_SE)     Flags |= LAND_STR_ROAD_SE;
            if (pNewStruct->Attr & SA_ROAD_S )     Flags |= LAND_STR_ROAD_S ;
            if (pNewStruct->Attr & SA_ROAD_SW)     Flags |= LAND_STR_ROAD_SW;
            if (pNewStruct->Attr & SA_ROAD_NW)     Flags |= LAND_STR_ROAD_NW;
        }


        pStruct = pNewStruct;
    }

    return pStruct;
}*/

//-------------------------------------------------------------


void CLand::SetFlagsFromUnits()
{
    int                 i;
    int                 alarm=ATT_FRIEND1;
    int                 guard_stance=-1;
    int                 claim=-1;
    long                 player_id;
    long                men, weapons, armours;
    const void        * stance;
    const void        * troops;
    const void        * gear_weapon;
    const void        * gear_armour;
    EValueType          type;

    Flags &= ~(LAND_TAX_NEXT | LAND_TRADE_NEXT);
    AlarmFlags = 0;
    player_id=atol(gpApp->GetConfig(SZ_SECT_ATTITUDES  , SZ_ATT_PLAYER_ID));

    for(i=ATT_UNDECLARED; i>=0; i--) Troops[i]=0;

    for (CUnit* pUnit : this->units_seq_) {
        men=0;
        weapons=0;
        armours=0;
        if (pUnit->FactionId==player_id) AlarmFlags |= PRESENCE_OWN;
        if (((pUnit->Flags & UNIT_FLAG_TAXING) || (pUnit->Flags & UNIT_FLAG_PILLAGING)) && !(pUnit->Flags & UNIT_FLAG_GIVEN))
            Flags |= LAND_TAX_NEXT;
        if ((pUnit->Flags & UNIT_FLAG_PRODUCING) && !(pUnit->Flags & UNIT_FLAG_GIVEN))
            Flags |= LAND_TRADE_NEXT;
        if (!((pUnit->GetProperty(PRP_FRIEND_OR_FOE,type,stance,eNormal)) && (type==eLong)))
        {
            // set the default stance if none is defined
            stance = (void *) gpDataHelper->GetAttitudeForFaction(0);
        }
        if ((pUnit->Flags & UNIT_FLAG_GUARDING) && !(pUnit->Flags & UNIT_FLAG_GIVEN))
        {
            if(((long) stance > guard_stance) && ((long) stance < ATT_UNDECLARED))
                guard_stance = (long) stance; // stance of guards, if any
        }
        if (((pUnit->Flags & UNIT_FLAG_TAXING) || (pUnit->Flags & UNIT_FLAG_PILLAGING) || (pUnit->Flags & UNIT_FLAG_PRODUCING))
              && !(pUnit->Flags & UNIT_FLAG_GIVEN))
        {
            if(((long) stance > claim) && ((long) stance < ATT_UNDECLARED))
                claim = (long) stance; // stance of taxers/producers
        }
        if(((long) stance <= ATT_UNDECLARED) && ((long) stance >= 0))
        {
            // stance due to presence
            if((long) stance > alarm) alarm = (long) stance;
            if ((pUnit->GetProperty(PRP_MEN,type,troops,eNormal)) && (type==eLong))
                men = (long) troops;
            if ((pUnit->GetProperty(PRP_WEAPONS,type,gear_weapon,eNormal)) && (type==eLong))
               	weapons = (long) gear_weapon;
            if ((pUnit->GetProperty(PRP_ARMORS,type,gear_armour,eNormal)) && (type==eLong))
                armours = (long) gear_armour;
            if(weapons > (long) men) weapons = men;
            if(armours > weapons) armours = weapons;
            Troops[(long) stance] += 2*men + 2* weapons + armours + 1;
        }
    }

    if(claim<0) // pick the strongest stance group
    {
        long max = 0;
        for(i=ATT_UNDECLARED-1; i>=0; i--)
        {
            if(Troops[i] > max)
            {
                claim = i;
                max = Troops[i];
            }
        }
    }
    switch(claim)
    {
        case ATT_FRIEND1:
            AlarmFlags |= CLAIMED_BY_FRIEND;
            break;
        case ATT_ME:
            AlarmFlags |= CLAIMED_BY_OWN;
            break;
        case ATT_NEUTRAL:
            AlarmFlags |= CLAIMED_BY_NEUTRAL;
            break;
        case ATT_ENEMY:
            AlarmFlags |= CLAIMED_BY_ENEMY;
    }

    if((guard_stance >=0) && (guard_stance < ATT_UNDECLARED)) AlarmFlags |= GUARDED;
    switch(guard_stance)
    {
        case ATT_FRIEND1:
            AlarmFlags |= GUARDED_BY_FRIEND;
            break;
        case ATT_ME:
            AlarmFlags |= GUARDED_BY_OWN;
            break;
        case ATT_NEUTRAL:
            AlarmFlags |= GUARDED_BY_NEUTRAL;
            break;
        case ATT_ENEMY:
            AlarmFlags |= GUARDED_BY_ENEMY;
    }

    switch(alarm)
    {
        case ATT_FRIEND1:
            AlarmFlags |= PRESENCE_FRIEND;
            break;
        case ATT_NEUTRAL:
            AlarmFlags |= PRESENCE_NEUTRAL;
            break;
        case ATT_ENEMY:
            AlarmFlags |= PRESENCE_ENEMY;
    }
    if((alarm > guard_stance) && (alarm > claim) && (alarm > ATT_ME)) AlarmFlags |= ALARM;

    // normalise troops
    int minimen = 2*atol(gpApp->GetConfig(SZ_SECT_COMMON  , SZ_KEY_MIN_SEL_MEN));
    for(i=ATT_UNDECLARED; i>=0; i--)
    {
        long limit = 2000;
        men = Troops[i];
        Troops[i]=0;
        for(int f=10; f>=3; f-=1)
        {
            long c = (long) ((float) f * f / 100 * limit);
            if(men >= c) Troops[i]++;
        }
        if(men >= minimen) Troops[i]++;
    }
}

//-------------------------------------------------------------

void CLand::SetExit(int direction, int x, int y)
{
    if (direction >= 0 && direction < 6)
    {
        xExit[direction] = x;
        yExit[direction] = y;
    }
}

//-------------------------------------------------------------

void CLand::CloseAllExits()
{
    for(int i=0; i<6; ++i) { xExit[i]=EXIT_CLOSED; yExit[i]=EXIT_CLOSED; }
}

//-------------------------------------------------------------

void CLand::ResetAllExits()
{
    for(int i=0; i<6; ++i) { xExit[i]=EXIT_MAYBE; yExit[i]=EXIT_MAYBE; }
}

int CLand::FindExit(long hexId) const
{
    int x, y, z;
    LandIdToCoord(hexId, x, y, z);
    for(int i=0; i<6; ++i)
    {
        if (xExit[i] == x && yExit[i] == y)
            return i;
    }
    return -1;
}

//-------------------------------------------------------------

void CLand::RemoveEdgeStructs(int direction)
{
    CStruct * pEdge;
    for (int i=EdgeStructs.Count()-1; i>=0; i--)
    {
        pEdge = (CStruct*) EdgeStructs.At(i);
        if((pEdge != NULL) && (pEdge->Location == direction%6))
        {
            EdgeStructs.AtFree(i);
        }
    }
}
//-------------------------------------------------------------

void CLand::AddNewEdgeStruct(const char * name, int direction)
{
    CStruct * pEdge = new CStruct;

    pEdge->type_    = name;
    pEdge->Location = direction % 6;
    EdgeStructs.Insert(pEdge);
}

//=============================================================

CStrStrColl * CUnit::m_PropertyGroupsColl = NULL;
CStr          CUnit::m_CustomFlagNames[UNIT_CUSTOM_FLAG_COUNT];
BOOL          CUnit::m_CustomFlagNamesLoaded = FALSE;



CStrStrColl * CUnit::GetPropertyGroups()
{
    return m_PropertyGroupsColl;
}

//-------------------------------------------------------------

CUnit::CUnit() : CBaseObject(), Comments(16), DefOrders(32), Orders(32), Errors(32), Events(32)
{
    IsOurs        = false;
    FactionId     = 0;
    pFaction      = NULL;
    LandId        = 0;
    Flags         = 0;
    FlagsOrg      = 0;
    silver_initial_.amount_ = 0;
    silver_initial_.code_name_ = PRP_SILVER;
    struct_id_ = 0;
    struct_id_initial_ = 0;
    has_error_  = false;
    movement_stop_ = 0;

    FlagsLast     = ~Flags;
};

//-------------------------------------------------------------

CUnit::~CUnit()
{
}

//-------------------------------------------------------------

CUnit * CUnit::AllocSimpleCopy()
{
    CUnit * pUnit = new CUnit;
    int     idx;

    pUnit->Id                     = Id                   ;
    pUnit->Name                   = Name                 ;
    pUnit->Description            = Description          ;

    pUnit->IsOurs                 = IsOurs               ;
    pUnit->FactionId              = FactionId            ;
    pUnit->pFaction               = pFaction             ;
    pUnit->LandId                 = LandId               ;
    pUnit->Comments               = Comments             ;
    pUnit->DefOrders              = DefOrders            ;
    pUnit->orders_                = orders_              ;
    pUnit->skills_                = skills_              ;
    pUnit->items_                 = items_              ;
    pUnit->items_initial_         = items_initial_;
    pUnit->silver_ = silver_;
    pUnit->silver_initial_ = silver_initial_;
    pUnit->men_ = men_;
    pUnit->men_initial_ = men_initial_;
    pUnit->impact_description_ = impact_description_;
    pUnit->struct_id_ = struct_id_;
    pUnit->struct_id_initial_ = struct_id_initial_;
    pUnit->movement_stop_ = movement_stop_;
    pUnit->movements_ = movements_;
    pUnit->Orders                 = Orders               ;
    pUnit->Errors                 = Errors               ;
    pUnit->Events                 = Events               ;
    pUnit->Flags                  = Flags                ;
    pUnit->FlagsOrg               = FlagsOrg             ;
    pUnit->FlagsLast              = FlagsLast            ;

    const char  * propname;
    EValueType    type;
    const void  * value;

    idx = 0;
    propname = GetPropertyName(idx);
    while (propname)
    {
        if (GetProperty(propname, type, value, eNormal))
            pUnit->SetProperty(propname, type, value, eNormal);

        propname = GetPropertyName(++idx);
    }

    return pUnit;
}

//-------------------------------------------------------------

void CUnit::ExtractCommentsFromDefOrders()
{
    const char * p;
    CStr         S;

    Comments.Empty();
    p = DefOrders.GetData();
    while (p)
    {
        p = S.GetToken(p, '\n', TRIM_ALL);
        if ( (S.GetLength() > 1) && (';' == S.GetData()[0]) )
        {
            Comments.SetStr(&S.GetData()[1], S.GetLength()-1);
            Comments.TrimLeft(TRIM_ALL);
            break;
        }
    }

}

//-------------------------------------------------------------

void CUnit::ResetNormalProperties()
{
    CBaseObject::ResetNormalProperties();
    DelProperty(PRP_TEACHING);
    if (silver_initial_.amount_ != 0)
    {
        SetProperty(PRP_SILVER, eLong, (const void*)silver_initial_.amount_, eNormal);
    } 
    else 
    {
        DelProperty(PRP_SILVER);
    }
    ProducingItem.Empty();
    men_ = men_initial_;
    silver_ = silver_initial_;
    items_ = items_initial_;
    skills_ = skills_initial_;
    orders_ = orders::parser::parse_lines_to_orders(std::string(Orders.GetData(), Orders.GetLength()));
    impact_description_.clear();
    caravan_info_ = nullptr;
    struct_id_ = struct_id_initial_;
    monthlong_descr_.clear();
    
    has_error_ = false;
    Flags     = FlagsOrg;
    FlagsLast = ~Flags;
}

//-------------------------------------------------------------

void CUnit::GetBestSkill(wxString &name, long &days)
{
    int idx = 0;
    EValueType type;
    const wxString Suffix = wxString::FromUTF8(PRP_SKILL_DAYS_POSTFIX);
    wxString skillName;
    long daysCurrent;
    name = wxEmptyString;
    days = 0;

    const char * propname_days = GetPropertyName(idx);
    while (propname_days)
    {
        skillName = wxString::FromUTF8(propname_days);
        if (skillName.EndsWith(Suffix))
        {
            if (GetProperty(skillName.ToUTF8(), type, (const void*&)daysCurrent, eNormal) )
            {
                if (daysCurrent > days)
                {
                    days = daysCurrent;
                    name = skillName;
                }
            }
        }
        propname_days = GetPropertyName(++idx);
    }
    if (days > 0)
        name.Truncate(name.Length() - Suffix.Length());

    name.MakeLower();
}

//-------------------------------------------------------------
BOOL CUnit::GetProperty(const char  *  name,
                        EValueType   & type,
                        const void  *& value,
                        EPropertyType  proptype
                        )
{
    BOOL         Ok = TRUE;

    name = ResolveAlias(name);

    if (0==stricmp(name, PRP_ID))
    {
        type  = eLong;
        value = (void*)Id;
        return TRUE;
    }

    // Custom and Standard flags
    if ( FlagsLast != Flags &&
         (0==stricmp(name, PRP_FLAGS_STANDARD   ) ||
          0==stricmp(name, PRP_FLAGS_CUSTOM     ) ||
          0==stricmp(name, PRP_FLAGS_CUSTOM_ABBR)
         )
       )
    {
        CStr sValue, sValueAbbr, sKey;
        int  i, x;

        if (Flags & UNIT_FLAG_PILLAGING        )  sValue << "€";
        if (Flags & UNIT_FLAG_TAXING           )  sValue << '$';
        if (Flags & UNIT_FLAG_PRODUCING        )  sValue << 'P';
        if (Flags & UNIT_FLAG_ENTERTAINING     )  sValue << 'E';
        if (Flags & UNIT_FLAG_STUDYING         )  sValue << 'S';
        if (Flags & UNIT_FLAG_TEACHING         )  sValue << 'T';        
        if (Flags & UNIT_FLAG_WORKING          )  sValue << 'W';
        if (Flags & UNIT_FLAG_MOVING           )  sValue << 'M';
        sValue << '|';
        if (Flags & UNIT_FLAG_GUARDING         )  sValue << 'g';
        if (Flags & UNIT_FLAG_AVOIDING         )  sValue << 'a';
        if (Flags & UNIT_FLAG_BEHIND           )  sValue << 'b';
        if (Flags & UNIT_FLAG_REVEALING_UNIT   )  sValue << "rU";
        else if (Flags & UNIT_FLAG_REVEALING_FACTION)  sValue << "rF";
        if (Flags & UNIT_FLAG_HOLDING          )  sValue << 'h';
        if (Flags & UNIT_FLAG_RECEIVING_NO_AID )  sValue << 'i';
        if (Flags & UNIT_FLAG_CONSUMING_UNIT   )  sValue << "cU";
        else if (Flags & UNIT_FLAG_CONSUMING_FACTION)  sValue << "cF";
        if (Flags & UNIT_FLAG_NO_CROSS_WATER   )  sValue << 'x';
        if (Flags & UNIT_FLAG_SPOILS_NONE)  sValue << "sN"; 
        if (Flags & UNIT_FLAG_SPOILS_WALK)  sValue << "sW";
        if (Flags & UNIT_FLAG_SPOILS_RIDE)  sValue << "sR";
        if (Flags & UNIT_FLAG_SPOILS_FLY)  sValue << "sF";
        if (Flags & UNIT_FLAG_SPOILS_SWIM)  sValue << "sS";
        if (Flags & UNIT_FLAG_SPOILS_SAIL)  sValue << "sL";

        // MZ - Added for Arcadia
        if (Flags & UNIT_FLAG_SHARING          )  sValue << 'z';

        type  = eCharPtr;
        SetProperty(PRP_FLAGS_STANDARD, type, sValue.GetData(), eNormal);

        LoadCustomFlagNames();
        sValue.Empty();
        x = 1;
        for (i=0; i<UNIT_CUSTOM_FLAG_COUNT; i++)
        {
            if (Flags & x)
            {
                if (!sValue.IsEmpty())
                    sValue << ',';
                sValue     << GetCustomFlagName(i);
                sValueAbbr << (long)(i+1);
            }
            x <<= 1;
        }
        SetProperty(PRP_FLAGS_CUSTOM     , type, sValue.GetData()    , eNormal);
        SetProperty(PRP_FLAGS_CUSTOM_ABBR, type, sValueAbbr.GetData(), eNormal);

        FlagsLast = Flags;
    }

    // now the usual stuff...
    if (!CBaseObject::GetProperty(name, type, value, proptype))
    {
        if (0==stricmp(name, PRP_COMMENTS  ))
        {
            type  = eCharPtr;
            value = Comments.GetData();
        }
        else if (0==stricmp(name, PRP_ORDERS    ))
        {
            // decorate for stupid wxw 2.8.0 list control
            const char * src = Orders.GetData();
            char       * dest;
            int          destlen = 0;

            OrdersDecorated.Empty();
            dest = OrdersDecorated.AllocExtraBuf(Orders.GetLength()*3+1);

            while (src && *src)
            {
                if (*src != '\r' && *src != '\n')
                {
                    destlen++;
                    *dest = *src;
                    dest++;
                }
                else if ('\n' == *src)
                {
                    destlen += 3;
                    *dest = ' ';     dest++;
                    *dest = '|';     dest++;
                    *dest = ' ';     dest++;
                }
                src++;
            }
            OrdersDecorated.UseExtraBuf(destlen);

            type  = eCharPtr;
            value = OrdersDecorated.GetData();
        }
        else if (0==stricmp(name, PRP_FACTION_ID))
        {
            type  = eLong;
            value = (void*)FactionId;
        }
        else if (0==stricmp(name, PRP_FACTION))
        {
            type  = eCharPtr;
            if (pFaction)
                value = pFaction->Name.GetData();
            else
                value = "";
        }
        else if (0==stricmp(name, PRP_LAND_ID   ))
        {
            type  = eLong;
            value = (void*)LandId;
        }
        else if (0==stricmp(name, PRP_BEST_SKILL))
        {
            wxString S;
            long days;
            GetBestSkill(S, days);
            type = eCharPtr;
            const char * s = S.ToUTF8().data();

            // Need a storage for all these skill-strings
            static char * arr[200];
            static int arrMax = 0;
            bool found = false;

            for (int i = 0; i < arrMax; ++i)
            {
                if (0 == strcmp(arr[i], s))
                {
                    s = arr[i];
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                arr[arrMax] = strdup(s);
                s = arr[arrMax++];
            }

            value = (void*)(s);
        }
        else if (0==stricmp(name, PRP_BEST_SKILL_DAYS))
        {
            wxString S;
            long days;
            GetBestSkill(S, days);
            type = eLong;
            value = (void*)days;
        }
        else
            Ok = FALSE;
    }

    return Ok;
}


//-------------------------------------------------------------

void CUnit::LoadCustomFlagNames()
{
    if (!m_CustomFlagNamesLoaded)
    {
        int  i;
        CStr sKey;

        for (i=0; i<UNIT_CUSTOM_FLAG_COUNT; i++)
        {
            sKey.Empty();
            sKey << (long)i;
            m_CustomFlagNames[i] = gpApp->GetConfig(SZ_SECT_UNIT_FLAG_NAMES, sKey.GetData());
        }
        m_CustomFlagNamesLoaded = TRUE;
    }
}

//-------------------------------------------------------------

void CUnit::ResetCustomFlagNames()
{
    int i;

    m_CustomFlagNamesLoaded = FALSE;
    for (i=0; i<UNIT_CUSTOM_FLAG_COUNT; i++)
        m_CustomFlagNames[i].Empty();
}

//-------------------------------------------------------------

const char * CUnit::GetCustomFlagName(int no)
{
    if (m_CustomFlagNamesLoaded && no>=0 && no<UNIT_CUSTOM_FLAG_COUNT)
        return m_CustomFlagNames[no].GetData();
    else
        return NULL;
}

//-------------------------------------------------------------

void CUnit::DebugPrint(CStr & sDest)
{
    CBaseObject::DebugPrint(sDest);

    sDest << "FactionId = " << FactionId << "\n"
          << "LandId    = " << LandId << "\n"
        ;
}

//-------------------------------------------------------------

void CStruct::ResetNormalProperties()
{
    TPropertyHolder::ResetNormalProperties();
    occupied_capacity_  = 0;
    SailingPower = 0;
}

//=============================================================

CBaseColl::CBaseColl() : CCollection()
{
}

CBaseColl::CBaseColl(int nDelta) : CCollection(nDelta)
{
}

void CBaseColl::FreeItem(void * pItem)
{
    if (pItem)
    {
        CBaseObject * pBase = (CBaseObject*)pItem;

        pBase->Done();
        delete pBase;
    }
}

//-------------------------------------------------------------

CBaseCollById::CBaseCollById() : CSortedCollection()
{
}

CBaseCollById::CBaseCollById(int nDelta) : CSortedCollection(nDelta)
{
}

void CBaseCollById::FreeItem(void * pItem)
{
    if (pItem)
    {
        CBaseObject * pBase = (CBaseObject*)pItem;

        pBase->Done();
        delete pBase;
    }
}

//-------------------------------------------------------------

int  CBaseCollById::Compare(void * pItem1, void * pItem2) const
{
    CBaseObject * pBase1 = (CBaseObject*)pItem1;
    CBaseObject * pBase2 = (CBaseObject*)pItem2;

    if (pBase1->Id > pBase2->Id)
        return 1;
    else
        if (pBase1->Id < pBase2->Id)
            return -1;
        else
            return 0;
}


//-------------------------------------------------------------

CPlane::~CPlane()
{
    Lands.FreeAll();
}

//-------------------------------------------------------------

int  CBaseCollByName::Compare(void * pItem1, void * pItem2) const
{
    CBaseObject * pBase1 = (CBaseObject*)pItem1;
    CBaseObject * pBase2 = (CBaseObject*)pItem2;

    return stricmp(pBase1->Name.GetData(), pBase2->Name.GetData());
}

//-------------------------------------------------------------

void TProdDetails::clear()
{
    skill_name_.clear();
    skill_level_=0;
    per_month_=0;
    req_resources_.clear();
    tool_name_.clear();
    tool_plus_=0;
}

//=============================================================

void MakeQualifiedPropertyName(const char * prefix, const char * shortname, CStr & FullName)
{
    FullName.Empty();
    FullName << prefix;

    if (!FullName.IsEmpty() && '.' != FullName.GetData()[FullName.GetLength()-1])
        FullName << ".";
    FullName << shortname;
}

//-------------------------------------------------------------

void SplitQualifiedPropertyName(const char * fullname, CStr & Prefix, CStr & ShortName)
{
    const char * p;

    Prefix.Empty();
    ShortName.Empty();

    if (fullname && *fullname)
    {
        p = strrchr(fullname, '.');
        if (p)
        {
            ShortName = p+1;
            Prefix.AddBuf(fullname, p-fullname);
        }
    }
}

//--------------------------------------------------------------------------




BOOL EvaluateBaseObjectByBoxes(CBaseObject * pObj, CStr * Property, eCompareOp * CompareOp, CStr * sValue, long * lValue, int count)
{
    int i;
    EValueType       type;
    const void     * value;
    BOOL             ok = TRUE;

    for (i=0; i<count; i++)
    {
        if (!Property[i].IsEmpty() && (NOP!=CompareOp[i]))
        {
            if ( !pObj->GetProperty(Property[i].GetData(), type, value, eNormal))
            {
                // make an empty sValue
                CStrInt * pSI, SI(Property[i].GetData(), 0);
                int       idx;

                if (gpApp->m_pAtlantis->m_UnitPropertyTypes.Search(&SI, idx))
                {
                    pSI = (CStrInt*)gpApp->m_pAtlantis->m_UnitPropertyTypes.At(idx);
                    type = (EValueType)pSI->m_value;
                    if (eLong == type)
                        value = 0;
                    else
                        value = "";
                }
                else
                {
                    type = eLong;
                    value = 0;
                }
            }
            switch (type)
            {
            case eLong:
                switch (CompareOp[i])
                {
                case GT: ok = ((long)value >  lValue[i]); break;
                case GE: ok = ((long)value >= lValue[i]); break;
                case EQ: ok = ((long)value == lValue[i]); break;
                case LE: ok = ((long)value <= lValue[i]); break;
                case LT: ok = ((long)value <  lValue[i]); break;
                case NE: ok = ((long)value != lValue[i]); break;
                default: break;
                }
                break;

            case eCharPtr:
                switch (CompareOp[i])
                {
                case GT: ok = (stricmp((const char *)value, sValue[i].GetData()) >  0); break;
                case GE: ok = (stricmp((const char *)value, sValue[i].GetData()) >= 0); break;
                case EQ: ok = (stricmp((const char *)value, sValue[i].GetData()) == 0); break;
                case LE: ok = (stricmp((const char *)value, sValue[i].GetData()) <= 0); break;
                case LT: ok = (stricmp((const char *)value, sValue[i].GetData()) <  0); break;
                case NE: ok = (stricmp((const char *)value, sValue[i].GetData()) != 0); break;
                default: break;
                }
                break;

            default:
                ok = FALSE;
            }
            if (!ok)
                break;
        }
    }

    return ok;
}

//=============================================================

/* creates problems with NEW_UNIT_ID!!!
   but that macro was changed to be land id independent */

#define XY_DELTA   0x00000800
#define X_MASK     0x00000FFF
#define Y_MASK     0x00FFF000
#define Z_MASK     0xFF000000
#define Y_SHIFT    12
#define Z_SHIFT    24


long LandCoordToId(int x, int y, int z)
{
    return ( (z           << Z_SHIFT) & Z_MASK) |
           (((y+XY_DELTA) << Y_SHIFT) & Y_MASK) |
           ( (x+XY_DELTA)             & X_MASK);
}

//-------------------------------------------------------------

void LandIdToCoord(long id, int & x, int & y, int & z)
{
    x = ( id & X_MASK)             - XY_DELTA;
    y = ((id & Y_MASK) >> Y_SHIFT) - XY_DELTA;
    z = ( id & Z_MASK) >> Z_SHIFT;
}
/**/



//-------------------------------------------------------------

/* slow

long LandCoordToId(int x, int y, int z)
{
    return (z*1000 + (y+500))*1000 + (x+500);  // range is -499 - +499
}

//-------------------------------------------------------------

void LandIdToCoord(long id, int & x, int & y, int & z)
{
    x  = id % 1000;
    x -= 500;

    id = id/1000;
    y  = id % 1000;
    y -=500;
    z  = id/1000;
}
*/

//-------------------------------------------------------------

void TestLandId()
{
    int  x0, y0, z0;
    int  x1, y1, z1;
    int  x = 0;
    long id;

#define _x_min_  -48
#define _x_max_  500

#define _y_min_  -48
#define _y_max_  500

#define _z_min_  0
#define _z_max_  255

    for (z0=_z_min_; z0<=_z_max_; z0++)
        for (y0=_y_min_; y0<=_y_max_; y0++)
            for (x0=_x_min_; x0<=_x_max_; x0++)
            {
                id = LandCoordToId(x0, y0, z0);
                LandIdToCoord(id, x1, y1, z1);
                if ( (x0!=x1) || (y0!=y1) || (z0!=z1) )
                {
                    x = 10/x;
                }
            }
    x = 0;
    x = 1;
};



//-------------------------------------------------------------
