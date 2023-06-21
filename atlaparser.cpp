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

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <numeric>
#include <list>

#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "wx/string.h"

#include "ahapp.h"
#include "files.h"
#include "consts.h"
#include "cstr.h"
#include "collection.h"
#include "cfgfile.h"

#include "objs.h"
#include "data.h"
#include "atlaparser.h"
#include "errs.h"
#include "consts_ah.h" // not very good, but will do for now
#include "routeplanner.h"
#include "data_control.h"
#include "ah_control.h"
#include "autologic.h"
#include "autonaming.h"

#define CONTAINS    "contains "
#define SILVER      "silver"

#define FLAG_HDR    "$Flag"

#define YES         "Yes"
#define ORDER_CMNT  ";*** "



const char * Monthes[] = {"Jan", "Feb", "Mar",  "Apr", "May", "Jun",  "Jul", "Aug", "Sep",  "Oct", "Nov", "Dec"};

const char * EOL_MS    = "\r\n";
const char * EOL_UNIX  = "\n";
const char * EOL_SCR   = EOL_UNIX;
const char * EOL_FILE  = EOL_UNIX;

const char * STRUCT_UNIT_START = "-+*";

const char * Directions[] = {"North", "Northeast", "Southeast", "South", "Southwest", "Northwest",
                             "N",     "NE",        "SE",        "S",     "SW",        "NW"       };
//enum       eDirection     { North=0, Northeast,   Southeast,   South,   Southwest,   Northwest };
int          ExitFlags [] = { 0x01,    0x02,        0x04,        0x08,    0x10,        0x20      };
int          EntryFlags[] = { 0x08,    0x10,        0x20,        0x01,    0x02,        0x04      };

int Flags_NW_N_NE = 0x01 | 0x02 | 0x20;
int Flags_N       = 0x01;
int Flags_SW_S_SE = 0x04 | 0x08 | 0x10;
int Flags_S       = 0x08;

const char * LocationsShipsArcadia[]  = { "Northern hexside",
                                          "North Eastern hexside",
                                          "South Eastern hexside",
                                          "Southern hexside",
                                          "South Western hexside",
                                          "North Western hexside",
                                          "hex centre" };



static const char * BUG        = " - it's a bug!";
static const char * NOSETUNIT  = " - Can not set property for unit ";
static const char * NOSET      = " - Can not set unit property ";
static const char * NOTNUMERIC = " - Property is not numeric for unit ";
//static const char * NOTSTRING  = " - Property type is not string for unit ";

const char * ExitEndHeader[]  = { HDR_FACTION          ,
                                  HDR_FACTION_STATUS   ,
                                  HDR_ERRORS           ,
                                  HDR_EVENTS           ,
                                  HDR_SILVER           ,
                                  HDR_BATTLES          ,
                                  HDR_ATTACKERS        ,
                                  HDR_ATTITUDES        ,
                                  HDR_SKILLS           ,
                                  HDR_ITEMS            ,
                                  HDR_OBJECTS
                                };
int ExitEndHeaderLen[] =        { sizeof(HDR_FACTION       ) - 1,
                                  sizeof(HDR_FACTION_STATUS) - 1,
                                  sizeof(HDR_ERRORS        ) - 1,
                                  sizeof(HDR_EVENTS        ) - 1,
                                  sizeof(HDR_SILVER        ) - 1,
                                  sizeof(HDR_BATTLES       ) - 1,
                                  sizeof(HDR_ATTACKERS     ) - 1,
                                  sizeof(HDR_ATTITUDES     ) - 1,
                                  sizeof(HDR_SKILLS        ) - 1,
                                  sizeof(HDR_ITEMS         ) - 1,
                                  sizeof(HDR_OBJECTS       ) - 1
                                };


const char * BattleEndHeader[]= { HDR_ERRORS           ,
                                  HDR_EVENTS           ,
                                  HDR_SILVER           ,
                                  HDR_ATTITUDES        ,
                                  HDR_SKILLS           ,
                                  HDR_ITEMS            ,
                                  HDR_OBJECTS          ,
                                  HDR_FACTION          ,
                                  HDR_FACTION_STATUS   ,
                                  HDR_SILVER
                                };
int BattleEndHeaderLen[] =      { sizeof(HDR_ERRORS        ) - 1,
                                  sizeof(HDR_EVENTS        ) - 1,
                                  sizeof(HDR_SILVER        ) - 1,
                                  sizeof(HDR_ATTITUDES     ) - 1,
                                  sizeof(HDR_SKILLS        ) - 1,
                                  sizeof(HDR_ITEMS         ) - 1,
                                  sizeof(HDR_OBJECTS       ) - 1,
                                  sizeof(HDR_FACTION       ) - 1,
                                  sizeof(HDR_FACTION_STATUS) - 1,
                                  sizeof(HDR_SILVER        ) - 1
                                };



//----------------------------------------------------------------------

BOOL IsInteger(const char * s)
{
    int n = 0;

    if ((!s) || (!*s))
        return FALSE;

    while (*s)
    {
        if ( ('-'==*s) && (n>0) )
            return FALSE;
        else
            if ( (*s<'0') || (*s>'9') )
                return FALSE;

        n++;
        s++;
    }
    return TRUE;
}

//======================================================================

CAtlaParser::CAtlaParser()
            : m_UnitFlagsHash(1)
{
    int x = 2;
    x = 1/(x-2);
    //assert(0);
}

//----------------------------------------------------------------------

CAtlaParser::CAtlaParser(CGameDataHelper * pHelper)
            :m_UnitFlagsHash(1), m_sOrderErrors(256)
{
    gpDataHelper      = pHelper;

    m_CrntFactionId   = 0;
    m_ParseErr        = ERR_NOTHING;
    m_nCurLine        = 0;
    m_GatesCount      = 0;
    m_YearMon         = 0;
    m_CurYearMon      = 0;
    m_pSource         = NULL;
    m_pCurLand        = NULL;
    m_pCurStruct      = NULL;
    m_NextStructId    = 1;
    m_OrdersLoaded    = FALSE;
//    m_MaxSkillDays    = 450;
    m_JoiningRep      = FALSE;
    m_IsHistory       = FALSE;
    m_Events.Name     = "Events";
    m_SecurityEvents.Name = "Security Events";
    m_HexEvents.Name  = "Hex Events";
    m_Errors.Name     = "Errors";
    m_ArcadiaSkills   = FALSE;

    m_EconomyTaxPillage=false;
    m_EconomyWork     = false;

    m_UnitFlagsHash.Insert("taxing"                      ,     (void*)UNIT_FLAG_TAXING            );
    m_UnitFlagsHash.Insert("on guard"                    ,     (void*)UNIT_FLAG_GUARDING          );
    m_UnitFlagsHash.Insert("avoiding"                    ,     (void*)UNIT_FLAG_AVOIDING          );
    m_UnitFlagsHash.Insert("behind"                      ,     (void*)UNIT_FLAG_BEHIND            );
    m_UnitFlagsHash.Insert("revealing unit"              ,     (void*)UNIT_FLAG_REVEALING_UNIT    );
    m_UnitFlagsHash.Insert("revealing faction"           ,     (void*)UNIT_FLAG_REVEALING_FACTION );
    m_UnitFlagsHash.Insert("holding"                     ,     (void*)UNIT_FLAG_HOLDING           );
    m_UnitFlagsHash.Insert("receiving no aid"            ,     (void*)UNIT_FLAG_RECEIVING_NO_AID  );
    m_UnitFlagsHash.Insert("consuming unit's food"       ,     (void*)UNIT_FLAG_CONSUMING_UNIT    );
    m_UnitFlagsHash.Insert("consuming faction's food"    ,     (void*)UNIT_FLAG_CONSUMING_FACTION );
    m_UnitFlagsHash.Insert("won't cross water"           ,     (void*)UNIT_FLAG_NO_CROSS_WATER    );
    // MZ - Added for Arcadia
    m_UnitFlagsHash.Insert("sharing"                     ,     (void*)UNIT_FLAG_SHARING           );

    m_UnitFlagsHash.Insert("weightless battle spoils"    ,     (void*)UNIT_FLAG_SPOILS_NONE       );
    m_UnitFlagsHash.Insert("flying battle spoils"        ,     (void*)UNIT_FLAG_SPOILS_FLY        );
    m_UnitFlagsHash.Insert("walking battle spoils"       ,     (void*)UNIT_FLAG_SPOILS_WALK       );
    m_UnitFlagsHash.Insert("riding battle spoils"        ,     (void*)UNIT_FLAG_SPOILS_RIDE       );
    m_UnitFlagsHash.Insert("swimming battle spoils"      ,     (void*)UNIT_FLAG_SPOILS_SWIM       );
    m_UnitFlagsHash.Insert("sailing battle spoils"       ,     (void*)UNIT_FLAG_SPOILS_SAIL       );
}

//----------------------------------------------------------------------

CAtlaParser::~CAtlaParser()
{
    Clear();
    m_UnitFlagsHash.FreeAll();
}




//----------------------------------------------------------------------

void CAtlaParser::Clear()
{
    m_Factions.FreeAll();


    m_YearMon       = 0;
    m_CurYearMon    = 0;
    m_JoiningRep    = FALSE;
    m_IsHistory     = FALSE;
    m_Planes.DeleteAll();
    m_PlanesNamed.FreeAll();  // this one must go before units since it checks contents of CLand::Units collection!!!
    units_ids_.clear();
    units_.clear();
    

    m_CrntFactionId = 0;
    m_CrntFactionPwd.Empty();
    m_OurFactions.FreeAll();
    m_TaxLandStrs.FreeAll();
    m_TradeLandStrs.FreeAll();
    m_BattleLandStrs.FreeAll();
    m_UnitPropertyNames.FreeAll();
    m_UnitPropertyTypes.FreeAll();
    m_LandPropertyNames.FreeAll();
    //m_LandPropertyTypes.FreeAll();

    m_TradeUnitIds.DeleteAll();
    m_Skills.FreeAll();
    m_Items.FreeAll();
    m_Objects.FreeAll();
    m_Battles.FreeAll();
    m_Gates.FreeAll();
    m_Events.Description.Empty();
    m_SecurityEvents.Description.Empty();
    m_HexEvents.Description.Empty();
    m_Errors.Description.Empty();
    m_NewProducts.FreeAll();
    sailing_events_.clear();
}

//----------------------------------------------------------------------

BOOL CAtlaParser::ReadNextLine(CStr & s)
{
    const bool MergeLines = true;
    if (MergeLines)
    {
        return ReadNextLineMerged(s);
    }
    BOOL Ok=FALSE;

    if (m_pSource)
    {
        Ok = m_pSource->GetNextLine(s);
        if (Ok)
        {
            m_nCurLine++;
            s.TrimRight(TRIM_ALL);
            s << EOL_SCR;
        }
    }
    return Ok;
}

//----------------------------------------------------------------------

BOOL CAtlaParser::ReadNextLineMerged(CStr & s)
{
    bool ok = false;

    if (!m_pSource) return false;
    ok = m_pSource->GetNextLine(s);
    if (!ok) return false;

    m_nCurLine++;
    s.TrimRight(TRIM_ALL);

    const char * p = s.GetData();

    // Do not fold the region seperator line or the region exit list
    bool tryMerge = (strncmp(p, "---------------", 15) != 0 && strncmp(p, "Exits:", 6) != 0);
    if (strlen(p) == 0) tryMerge = false;

    int indent[2] = {0,0};
    while ((*p) == ' ') {++p; ++indent[0]; }
    const bool startsWithPlusSign = *p == '+';
    CStr nextLine;

    while (tryMerge)
    {
        if (!m_pSource) break;
        tryMerge = m_pSource->GetNextLine(nextLine);
        if (tryMerge)
        {
            m_nCurLine++;
            p = nextLine.GetData();
            indent[1] = 0;
            while ((*p) == ' ') {++p; ++indent[1]; }

            if ((indent[0] + 2 == indent[1]) && (!startsWithPlusSign || (*p != '-' && *p != '*')))
            {
                // merge
                nextLine.TrimRight(TRIM_ALL);
                nextLine.TrimLeft(TRIM_ALL);
                s << " " << nextLine;
            }
            else
            {
                tryMerge = false;
                PutLineBack(nextLine);
            }
        }
    }
    s << EOL_SCR;
    return ok;
}

//----------------------------------------------------------------------

void CAtlaParser::PutLineBack (CStr & s)
{
    m_nCurLine--;
    if (m_pSource)
        m_pSource->QueueString(s.GetData());
};

//----------------------------------------------------------------------

int CAtlaParser::ParseFactionInfo(BOOL GetNo, BOOL Join)
{
    int          err    = ERR_OK;
    CStr         Line(128);
    CStr         FNo;
    CStr         Str;
    int          LineNo = 0;
    const char * p;
    const char * s;
    unsigned int i;
    CFaction   * pMyFaction;
    long         yearmon = 0;

    while ((ERR_OK==err) && ReadNextLine(Line))
    {
        m_FactionInfo << Line;

        Line.TrimRight(TRIM_ALL);

        if (Line.IsEmpty())
            break;  // stop at empty line


        if (GetNo)
            switch (LineNo)
            {
            case 0:  // faction number
                s = Line.GetData();
                p = strchr(s, '(');
                if (p)
                {
                    pMyFaction = new CFaction;
                    pMyFaction->Description << Line.GetData() << EOL_SCR;
                    pMyFaction->Name.SetStr(s, (p-s));
                    s = p+1;
                    p = strchr(s, ')');
                    FNo.SetStr(s, (p-s));
                    m_CrntFactionId = atol(FNo.GetData());
                    pMyFaction->Id  = m_CrntFactionId;
                    if (!m_Factions.Insert(pMyFaction))
                        delete pMyFaction;
                    m_OurFactions.Insert((void*)m_CrntFactionId);
                    if (!Join) gpDataHelper->SetPlayingFaction((long) m_CrntFactionId);
                }
                break;
            case 1:  // date  December, Year 2
                p = Str.GetToken(Line.GetData(), ',');
                for (i=0; i<sizeof(Monthes)/sizeof(char*); i++)
                    if (0==Str.FindSubStr(Monthes[i]))
                    {
                        yearmon = i+1;
                        break;
                    }
                p = SkipSpaces(p);
                //while (p && *p<=' ')
                //    p++;
                p = Str.GetToken(p, ' '); // 'Year'
                p = Str.GetToken(p, ' ');
                yearmon += 100*atol(Str.GetData());

                if (m_JoiningRep && m_YearMon != yearmon)
                {
                    err = ERR_INV_TURN;
                    if (m_CrntFactionId > 0)
                        for (i=0; i<(unsigned int)m_OurFactions.Count(); i++)
                            if ((long)m_OurFactions.At(i)==m_CrntFactionId)
                            {
                                m_OurFactions.AtDelete(i);
                                break;
                            }
                }
                else
                {
                    m_YearMon    = yearmon;
                    m_CurYearMon = yearmon; // file being currently loaded may not contain year/month info
                }
                break;
            }

        LineNo++;
    }


    return err;
}

//----------------------------------------------------------------------

int  CAtlaParser::SetLandFlag(const char * p, long flag)
{
    CLand * pLand = GetLand(p);

    if (pLand)
        pLand->Flags |= flag;

    return 0;
}

//----------------------------------------------------------------------

int  CAtlaParser::SetLandFlag(long LandId, long flag)
{
    CLand * pLand;

    pLand = GetLand(LandId);
    if (pLand)
        pLand->Flags |= flag;

    return 0;
}

//----------------------------------------------------------------------

int  CAtlaParser::ApplyLandFlags()
{

    int          i;
    const char * s;
    CUnit      * pUnit;

    for (i=0; i<m_TaxLandStrs.Count(); i++)
    {
        s = (const char*)m_TaxLandStrs.At(i);
        SetLandFlag(s, LAND_TAX);
    }

    for (i=0; i<m_TradeLandStrs.Count(); i++)
    {
        s = (const char*)m_TradeLandStrs.At(i);
        SetLandFlag(s, LAND_TRADE);
    }

    for (i=0; i<m_BattleLandStrs.Count(); i++)
    {
        s = (const char*)m_BattleLandStrs.At(i);
        SetLandFlag(s, LAND_BATTLE);
    }

    for (i=0; i<m_TradeUnitIds.Count(); i++)
    {
        long unit_id = (long)m_TradeUnitIds.At(i);
        pUnit = this->global_find_unit(unit_id);
        if (pUnit != nullptr)
        {
            SetLandFlag(pUnit->LandId, LAND_TRADE);
        }
    }

    m_TaxLandStrs.FreeAll();
    m_TradeLandStrs.FreeAll();
    m_BattleLandStrs.FreeAll();
    m_TradeUnitIds.DeleteAll();


    return 0;
}

//----------------------------------------------------------------------

void CAtlaParser::ParseOneMovementEvent(const char * params, const char * structid, const char * fullevent)
{
    CStr         Buf(64);
    CStr         Buf2(64);
    char         ch;
    CLand      * pLand1 = NULL;
    CLand      * pLand2 = NULL;
    CStr         S;

    while (params)
    {
        params = SkipSpaces(S.GetToken(params, " \n", ch, TRIM_ALL));
        if (0==stricmp(S.GetData(), "to"))
            break;
        Buf << S << ' ';
    }
    if (0==stricmp(S.GetData(), "to"))
    {
        while (params)
        {
            params = SkipSpaces(S.GetToken(params, " \n", ch, TRIM_ALL));
            Buf2 << S << ' ';
        }
    }

    ParseTerrain(NULL, 0, Buf, FALSE, &pLand1);

    // It is nice to parse 'to' terrain as well, since the unit
    // can be killed at the destination on sight...
    if (!Buf2.IsEmpty())
        ParseTerrain(NULL, 0, Buf2, FALSE, &pLand2);

    // check for links between planes
    if (pLand1 && pLand2  &&  pLand1->pPlane && pLand2->pPlane  && pLand1->pPlane != pLand2->pPlane)
    {
        pLand1->SetProperty(PRP_LAND_LINK, eLong, (void*)pLand2->Id, eBoth);
        pLand2->SetProperty(PRP_LAND_LINK, eLong, (void*)pLand1->Id, eBoth);
        m_LandsToBeLinked.Insert(pLand1);
        m_LandsToBeLinked.Insert(pLand2);
    }

    // collect info for linking sail events to captains
    if (structid && fullevent)
    {
        sailing_events_[atol(structid)].append(fullevent);
    }

}

//----------------------------------------------------------------------

BOOL CAtlaParser::ParseOneUnitEvent(CStr & EventLine, BOOL IsEvent, int UnitId)
{
    CUnit      * pUnit = NULL;
    const char * p;
    CStr         Name;
    BOOL         Taken = FALSE;
    long         x;
    int          idx;
    CStr         Buf(64);


    p = Name.GetToken(EventLine.GetData(), '(');
    if (UnitId>0)
    {
        pUnit = construct_or_retrieve_unit(UnitId);
        if (pUnit->Name.IsEmpty())
            pUnit->Name = Name;
    }
    if (pUnit)
    {
        if (IsEvent)
            pUnit->Events << EventLine;
        else
            pUnit->Errors << EventLine;
        Taken = TRUE;
    }

    if (IsEvent)
    {
        // Try to get something of the unit event...
        p = strchr(p, ')');
        while (p && (*p>' ') )
            p++;
        p = SkipSpaces(p);

        p = SkipSpaces(Buf.GetToken(p, ' ', TRIM_ALL));
        if ( (0==stricmp("walks"   , Buf.GetData())) ||
             (0==stricmp("rides"   , Buf.GetData())) ||
             (0==stricmp("flies"   , Buf.GetData())) )
        {
            p = SkipSpaces(Buf.GetToken(p, ' ', TRIM_ALL));
            if (0==stricmp("from"  , Buf.GetData()))
                ParseOneMovementEvent(p, NULL, NULL);
        }

        else if ( p && ('$'==*p) && ( (0==stricmp("collects", Buf.GetData())) ||
                                      (0==stricmp("pillages", Buf.GetData())) )
                )
        {
            p = Buf.GetToken(p, '(', TRIM_ALL);
            p = Buf.GetToken(p, ')', TRIM_ALL);
            if (!m_TaxLandStrs.Search((void*)Buf.GetData(), idx))
                m_TaxLandStrs.Insert(strdup(Buf.GetData()));
        }
        else if (0==stricmp("produces", Buf.GetData()))
        {
            p = Buf.GetToken(p, '(', TRIM_ALL);
            p = Buf.GetToken(p, ')', TRIM_ALL);
            if (!m_TradeLandStrs.Search((void*)Buf.GetData(), idx))
                m_TradeLandStrs.Insert(strdup(Buf.GetData()));
        }

        // Performs work, is a trade activity
        // And now it is called 'construction'

        // TBD: maybe implement buying and selling trade goods later
        else if (0==stricmp("performs", Buf.GetData()))
        {
            p = SkipSpaces(Buf.GetToken(p, ' ', TRIM_ALL));
            if (0==stricmp("work"        , Buf.GetData()) ||
                0==stricmp("construction", Buf.GetData()) )
            {
                p = Buf.GetToken(EventLine.GetData(), '(', TRIM_ALL);
                p = Buf.GetToken(p                  , ')', TRIM_ALL);
                x = atol(Buf.GetData());
                m_TradeUnitIds.Insert((void*)x);

            }
        }

        // Mary Loo (1104): Has mithril sword [MSWO] stolen.
        // Unit (3849) is caught attempting to steal from Unit (1662) in Lotan.
        // Unit (3595) steals double bow [DBOW] from So many farmers (1766).
        // Unit (1023): Is forbidden entry to swamp (31,17) in Dorantor by
        else if ((0==stricmp("has"    , Buf.GetData()) && EventLine.FindSubStr("stolen")>=0) ||
                 (0==stricmp("is"     , Buf.GetData()) && EventLine.FindSubStr("caught")>=0) ||
                 (0==stricmp("steals" , Buf.GetData())) ||
                 (0==stricmp("is"     , Buf.GetData()) && EventLine.FindSubStr("forbidden")>=0) ||
                 (0==stricmp("forbids", Buf.GetData()) && EventLine.FindSubStr("entry")>=0)
                )
        {
            m_SecurityEvents.Description << EventLine;
        }

    }

    return Taken;
}

//----------------------------------------------------------------------

BOOL CAtlaParser::ParseOneLandEvent(CStr & EventLine, BOOL IsEvent)
{
    const char * p;
    CStr         Buf;
    CStr         S;
    BOOL         Taken = FALSE;
    CLand      * pLand = NULL;

    p = Buf.GetToken(EventLine.GetData(), ')');
    p = S.GetToken(p, ',');
    Buf << ") " << S;
    ParseTerrain(NULL, 0, Buf, FALSE, &pLand);
    if (pLand)
        m_HexEvents.Description << EventLine;

    return Taken;
}


//----------------------------------------------------------------------

//Speedy (1356): Rides from swamp (7,35) in Moffat to plain (7,37) in
//  Partry.
//Speedy (1356): Rides from plain (7,37) in Partry to plain (7,39) in
//  Partry.

//Magoga (892): Walks from plain (10,34) in Grue to plain (9,35) in
//  Grue.

//Choppers (1101): Produces 13 wood [WOOD] in swamp (7,35) in Moffat.

int CAtlaParser::ParseOneEvent(CStr & EventLine, BOOL IsEvent)
{
    const char * p;
    CStr         Buf(64);
    CStr         StructId;
    CStr         Name;
    long         x;
    char         ch;
    BOOL         Taken = FALSE;
    BOOL         Valid;

    if (EventLine.IsEmpty())
        return 0;
    EventLine << EOL_SCR;


    p = Name.GetToken(EventLine.GetData(), "([", ch, TRIM_ALL);
    switch (ch)
    {
    case '(':
        p = Buf.GetInteger(p, Valid);
        if (*p == ')')
        {
            if (0==strnicmp(Name.GetData(), "The address of ", 15))
            {
                // it will goto generic events
            }
            else
            {
                // it is  a unit!
                x = atol(Buf.GetData());
                Taken = ParseOneUnitEvent(EventLine, IsEvent, x);
            }
        }
        else
        {
            // could be a land event
            Taken = ParseOneLandEvent(EventLine, IsEvent);
        }
        break;

    case '[':   // ship, probably
        if (IsEvent)
        {
            p = SkipSpaces(StructId.GetToken(p, ']', TRIM_ALL));
            //while (p && (*p>' ') )
            //    p++;
            //p = SkipSpaces(p);
            p = SkipSpaces(Buf.GetToken(p, ' ', TRIM_ALL));
            if (0==stricmp("sails"   , Buf.GetData()))
            {
                p = SkipSpaces(Buf.GetToken(p, ' ', TRIM_ALL));
                if (0==stricmp("from"  , Buf.GetData()))
                    ParseOneMovementEvent(p, StructId.GetData(), EventLine.GetData());
//                {
//                    Buf = p;
//                    x = Buf.FindSubStr(" to ");
//                    if (x>0)
//                        Buf.DelSubStr(x, Buf.GetLength()-x);
//                    ParseTerrain(NULL, 0, Buf, FALSE, NULL);
//                }
            }
        }
        break;
    }


    if (IsEvent)
    {
        if (!Taken)
            m_Events.Description << EventLine;
    }
    else
        m_Errors.Description << EventLine;

    return 0;
}


//----------------------------------------------------------------------

int CAtlaParser::ParseEvents(BOOL IsEvents)
{
    int          err   = ERR_OK;
    CStr         Line(128);
    CStr         OneEvent(128);
    char         ch;


    while ((ERR_OK==err) && ReadNextLine(Line))
    {
        Line.TrimRight(TRIM_ALL);

        if (Line.IsEmpty())
        {
            ParseOneEvent(OneEvent, IsEvents);
            break;  // stop at empty line
        }

//        // comment/error may take more than one line.
//        // Dot at the end is not reliable!
//        if (strchr(Line.GetData(), ':') ||
//            strchr(Line.GetData(), '[') ||
//            (!OneEvent.IsEmpty() && ('.'==OneEvent.GetData()[OneEvent.GetLength()-1]))
//           )

        // Looks like it is time to check spaces at the line start :((
        // additional event lines start with spaces
        ch = Line.GetData()[0];
        if (ch != ' ' && ch != '\t')
        {
            // That is hopefully a new event
            ParseOneEvent(OneEvent, IsEvents);
            OneEvent.Empty();
        }
        if (!OneEvent.IsEmpty())
            OneEvent << EOL_SCR;
        OneEvent << Line;

    }


    return err;
}

//----------------------------------------------------------------------

int CAtlaParser::ParseOneImportantEvent(CStr & EventLine)
{
    m_HexEvents.Description << EventLine << EOL_SCR;
    m_Events.Description << EventLine << EOL_SCR;
    return ERR_OK;
}

//----------------------------------------------------------------------

int CAtlaParser::ParseImportantEvents()
{
    int          err   = ERR_OK;
    CStr         Line(128);
    CStr         OneEvent(128);
    char         ch;
    int          i;
    BOOL         DoBreak = FALSE;



    while ((ERR_OK==err) && ReadNextLine(Line))
    {
        Line.TrimRight(TRIM_ALL);

        for (i=0; i<(int)sizeof(ExitEndHeader)/(int)sizeof(const char *); i++)
            if (0==strnicmp(Line.GetData(), ExitEndHeader[i], ExitEndHeaderLen[i] ))
        {
            Line << EOL_FILE;
            PutLineBack(Line);
            DoBreak = TRUE;
            break;
        }
        if (DoBreak)
            break;


        // Looks like it is time to check spaces at the line start :((
        // additional event lines start with spaces
        ch = Line.GetData()[0];
        if (ch != ' ' && ch != '\t')
        {
            // That is hopefully a new event
            ParseOneImportantEvent(OneEvent);
            OneEvent.Empty();
        }
        if (!OneEvent.IsEmpty())
            OneEvent << EOL_SCR;
        OneEvent << Line;

    }
    ParseOneImportantEvent(OneEvent);


    return err;

}

//----------------------------------------------------------------------

int CAtlaParser::ParseErrors()
{
    return ParseEvents(FALSE);
}

//----------------------------------------------------------------------

int CAtlaParser::ParseUnclSilver(CStr & Line)
{
    const char * p;
    const char * s;
    CStr         N;
    CFaction   * pFaction;

    m_FactionInfo << Line;

    Line.TrimRight(TRIM_ALL);
    s = Line.GetData() + sizeof(HDR_SILVER)-1;
    p = strchr(s, '.');

    if (p)
        N.SetStr(s, p-s);
    else
        N.SetStr(s);
    N.TrimLeft();

    N.TrimRight(TRIM_ALL);

    pFaction = GetFaction(m_CrntFactionId);
    if (pFaction)
        pFaction->UnclaimedSilver = atol(N.GetData());

    return ERR_OK;
}

//----------------------------------------------------------------------

/*
Declared Attitudes (default Neutral):
Hostile : none.
Unfriendly : none.
Neutral : none.
Friendly : none.
Ally : none.
*/
int CAtlaParser::ParseAttitudes(CStr & Line, BOOL Join)
{
    CStr         Info;
    CStr         FNo;
    CStr         S1;
    const char * str;
    const char * p;
    const char * s;
    char         ch, c;
    int          attitude = ATT_FRIEND1;
    CStr         attitudes[4];
    BOOL         apply_attitudes = TRUE;
    BOOL         def;

    attitudes[ATT_FRIEND1]  = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_FRIEND1);
    attitudes[ATT_ME]       = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_ME);
    attitudes[ATT_NEUTRAL]  = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_NEUTRAL);
    attitudes[ATT_ENEMY]    = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_ENEMY);

    if(Join)
    {   // check config whether to apply allied attitudes
        apply_attitudes  = (0!=SafeCmp(gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_APPLY_ON_JOIN),"0"));
    }
    else
    {
        while(attitude <= ATT_ENEMY)
        {
            if(0<=attitudes[attitude].FindSubStr("Own")) break;
            attitude++;
        }
        gpDataHelper->SetAttitudeForFaction(-1, attitude);
        attitude = ATT_ENEMY;
    }

    while (!Line.IsEmpty())
    {
        str = Line.GetData();
        m_FactionInfo << Line;

        if(apply_attitudes) // parse attitudes
        {
            str = Info.GetToken(str, ":,.", ch, TRIM_ALL);
            p   = Info.GetData();

            def = FALSE;
            if(Info.FindSubStr("(default") > 0)
            {
                // parse the default line
                s = S1.GetToken(p, "(", c, TRIM_ALL);
                S1.GetToken(s, ")" ,c , TRIM_ALL);
                s = S1.GetData();
                p = Info.GetToken(s, " ", c, TRIM_ALL);
                def = TRUE;
                m_FactionInfo << EOL_SCR;
            }
            // determine the attitude
            while(attitude >= ATT_FRIEND1)
            {
                if(0<=attitudes[attitude].FindSubStr(p)) break;
                attitude--;
            }
            if((!Join) && def && (attitude >= ATT_FRIEND1) && (attitude < ATT_UNDECLARED))
            {
                gpDataHelper->SetAttitudeForFaction(0, attitude);
            }

            while(str)
            {
                str = Info.GetToken(str, ",.", ch, TRIM_ALL);
                p   = Info.GetData();
                switch(ch)
                {
                    case '.':
                        m_FactionInfo << EOL_SCR;
                        break;
                    case ',':
                        if((attitude <= ATT_UNDECLARED) && (attitude >= ATT_FRIEND1))
                        {
                            // parse faction id
                            if(0==strcmp(p,"none")) break;
                            s = S1.GetToken(p, "(", c, TRIM_ALL);
                            FNo.GetToken(s, ")", c, TRIM_ALL);
                            if (!FNo.IsEmpty())
                            {
                                int id = atol(FNo.GetData());
                                gpDataHelper->SetAttitudeForFaction(id, attitude);
                            }
                        }
                        break;
                }
            }
        }
        ReadNextLineMerged(Line);
        Line.TrimRight(TRIM_ALL);
    }

    return ERR_OK;
}

//----------------------------------------------------------------------

void CAtlaParser::CheckExit(CPlane * pPlane, int Direction, CLand * pLandSrc, CLand * pLandExit)
{
    int x1,y1,x2,y2, z, width;

    LandIdToCoord(pLandSrc ->Id, x1, y1, z);
    LandIdToCoord(pLandExit->Id, x2, y2, z);

    //if (0==pPlane->Width)
        switch (Direction%6)
        {
        case Northeast:

        case Southeast:
	    width = x1-x2+1;
            if (x2<x1 && width>pPlane->Width)
            {
                pPlane->WestEdge   = x2;
                pPlane->EastEdge   = x1;
                pPlane->Width      = width;

                pPlane->EdgeSrcId  = pLandSrc ->Id;
                pPlane->EdgeExitId = pLandExit->Id;
                pPlane->EdgeDir    = Direction%6;
            }
            break;

        case Northwest:
        case Southwest:
	    width = x2-x1+1;
            if (x2>x1 && width>pPlane->Width)
            {
                pPlane->WestEdge   = x1;
                pPlane->EastEdge   = x2;
                pPlane->Width      = width;

                pPlane->EdgeSrcId  = pLandSrc ->Id;
                pPlane->EdgeExitId = pLandExit->Id;
                pPlane->EdgeDir    = Direction%6;
            }
            break;
        }

		if (pPlane->Width > 0)
		{
			if (x1 > pPlane->EastEdge)
				pPlane->EastEdge = x1;
			if (x1 < pPlane->WestEdge)
				pPlane->WestEdge = x1;
			if (x2 > pPlane->EastEdge)
				pPlane->EastEdge = x2;
			if (x2 < pPlane->WestEdge)
				pPlane->WestEdge = x2;
			pPlane->Width = pPlane->EastEdge - pPlane->WestEdge + 1;
			pPlane->Width += pPlane->Width & 1;
		}
}

//----------------------------------------------------------------------

void CAtlaParser::ParseWeather(const char * src, CLand * pLand)
{
    BOOL         IsCurrent;
    BOOL         IsGood;
    int          Zone;
    unsigned int i;
    CStr         S1, S2;
    const char * p;
    int          x,y,z;
    CPlane     * pPlane = pLand->pPlane;

    if (!src || !pPlane)
        return;

    if (m_WeatherLine[0].IsEmpty())
    {
        for (i=0; i<sizeof(m_WeatherLine)/sizeof(*m_WeatherLine); i++)
        {
            // read weather lines
            // bit 0 is IsCurrent
            // bit 1 is IsGood
            // the rest is Zone
            IsCurrent = i & 1;
            IsGood    = (i & 2) >> 1;
            Zone      = i >> 2;

            m_WeatherLine[i] = gpDataHelper->GetWeatherLine(IsCurrent, IsGood, Zone);
            m_WeatherLine[i].Normalize();
        }
    }

    src = SkipSpaces(src);
    if ('-'==src[0] && '-'==src[1] && '-'==src[2] )
    {
        while (*src > ' ')
            src++;
        src = SkipSpaces(src);
        src = S1.GetToken(src, ';', TRIM_ALL);
        src = S2.GetToken(src, '.', TRIM_ALL);
        S1.Normalize();
        S2.Normalize();

        for (i=0; i<sizeof(m_WeatherLine)/sizeof(*m_WeatherLine); i++)
        {
            // bit 0 is IsCurrent
            // bit 1 is IsGood
            // the rest is Zone
            IsCurrent = i & 1;
            IsGood    = (i & 2) >> 1;
            Zone      = i >> 2;

            /*
            if (IsGood || Zone>0)
                continue; // looks like good weather is exactly the same everywhere
                          // and we only handle Tropic zone for now
            LandIdToCoord(pLand->Id, x,y,z);
            if (y>=pPlane->TropicZoneMin && y<=pPlane->TropicZoneMax)
                continue; // known coordinate
            */

            if (IsCurrent)
                p = S1.GetData();
            else
                p = S2.GetData();
            if (0==stricmp(p, m_WeatherLine[i].GetData()))
            {
                if (IsGood )
                {
                    if (!IsCurrent)
                       pLand->WeatherWillBeGood = TRUE;
                }
                else
                {
                    LandIdToCoord(pLand->Id, x,y,z);
                    if (Zone>0)
                        continue; // we only draw the tropic line!
                    if (y>=pPlane->TropicZoneMin && y<=pPlane->TropicZoneMax)
                        continue; // known coordinate, don't let them shrink when some hexes are not visible any more!

                    if (y >= pPlane->TropicZoneMax)
                        pPlane->TropicZoneMax = y;
                    if (y <= pPlane->TropicZoneMin)
                        pPlane->TropicZoneMin = y;
                }
            }
        }
    }
}

//----------------------------------------------------------------------

int CAtlaParser::AnalyzeTerrain(CLand * pMotherLand, CLand * pLand, BOOL IsExit, int ExitDir, CStr & Description)
{
/*
plain (5,39) in Partry, contains Drimnin [city], 3217 peasants (high
  elves), $22519.
------------------------------------------------------------
  It was winter last month; it will be winter next month.
  Wages: $17 (Max: $10937).
  Wanted: 144 grain [GRAI] at $20, 108 livestock [LIVE] at $21, 33
    longbows [LBOW] at $126, 28 plate armor [PARM] at $374, 10 caviar
    [CAVI] at $158, 12 cotton [COTT] at $149.
  For Sale: 35 horses [HORS] at $62, 28 wagons [WAGO] at $157, 17
    pearls [PEAR] at $71, 16 wool [WOOL] at $76, 643 high elves [HELF]
    at $68, 128 leaders [LEAD] at $136.
  Entertainment available: $1125.
  Products: 40 grain [GRAI], 33 horses [HORS].
*/
    enum         {eMain, eSale, eWanted, eProduct, eNone} SectType;
    CStr         Section(64);
    CStr         Struct (64);
    CStr         S1     (32);
    CStr         S2     (32);
    CStr         N1     (32);
    CStr         N2     (32);
    CStr         Buf    (32);
    long         n1;
    long         n2;
    const char * src;
    const char * str;
    const char * srcold;
    const char * p;
    char         ch;
    int          delpos = 0;
    int          dellen = 0;
    BOOL         TerrainPassed = FALSE;
    BOOL         Valid;

    SectType = eMain;

    // skip terrain coordinates - they are confusing for the edge thingy
    srcold   = strchr(Description.GetData(), ')');
    if (srcold)
        srcold = SkipSpaces(srcold++);
    else
        srcold = Description.GetData(); // something must be very wrong here, must never happen

    src      = Section.GetToken(srcold, '.', TRIM_ALL);
    if (!m_IsHistory && m_CurYearMon>0)
        ParseWeather(src, pLand);  // weather description should be right after the first section
        
    while (!Section.IsEmpty())
    {
        BOOL RerunSection = FALSE;

        str = Section.GetData();
        
        // When weather is not activated the dashed (----) is not cleaned. Do it here
	    str = SkipSpaces(str);
	    if ('-'==str[0] && '-'==str[1] && '-'==str[2] )
	    {
	        while (*str > ' ')
	            str++;
	        str = SkipSpaces(str);
	    }
	    
	    // Parse land data
        while (str)
        {
            if (RerunSection)
                break;

            str = Struct.GetToken(str, ":,", ch, TRIM_ALL);
            p   = Struct.GetData();
            switch(ch)
            {
            case ':': // that's a section name!
                pLand->Flags|=LAND_VISITED; // Just simple presense in the report is not enough!
                if      (0==stricmp("For Sale", p))
                    SectType = eSale;
                else if (0==stricmp("Wanted"  , p))
                    SectType = eWanted;
                else if (0==stricmp("Products", p))
                {
                    SectType = eProduct;
                    delpos   = srcold - Description.GetData();
                    dellen   = src - srcold;
                }
                else if (0==stricmp("Wages"  , p))
                {
                    // Wages does not match the common pattern
                    ParseWages(pLand, str, src);
                }
                else if (0==stricmp("Entertainment available", p))
                {
                    // Entertainment available: $1125.
                    const char * pTmp = strstr(str, "$");
                    if (pTmp)
                    {
                        pLand->initial_state_.entertain_.amount_ = atoi(++pTmp);
                        pLand->current_state_.entertain_.amount_ = pLand->initial_state_.entertain_.amount_;
                    }
                }
                else
                    SectType = eNone;
                break;

            case ',':
            case  0 :
                switch (SectType)
                {
                case eMain:    // recognize contains, $ and peasants
                    if ('$'==*p)
                    {
                        N1.GetInteger(++p, Valid);
                        
                        pLand->initial_state_.tax_.amount_ = atol(N1.GetData());
                    }
                    else
                    {
                        p  = SkipSpaces(S1.GetToken(p, ' ', TRIM_ALL));
                        p  = S2.GetToken(p, ' ', TRIM_ALL);
                        n1 = atol(S1.GetData());
                        if (n1>0)
                        {
                            if (0==stricmp("peasants", S2.GetData()))
                            {
                                CStr peasant_race;
                                pLand->initial_state_.peasants_amount_ = n1;
                                p  = S2.GetToken(p, '(', TRIM_ALL);
                                p  = peasant_race.GetToken(p, ')', TRIM_ALL);
                                peasant_race.Replace('\r', ' ');
                                peasant_race.Replace('\n', ' ');
                                peasant_race.Replace('\t', ' ');
                                peasant_race.Normalize();
                                peasant_race.Replace(' ', '_');
                                pLand->initial_state_.peasant_race_ = std::string(peasant_race.GetData(), peasant_race.GetLength());
                            }
                        }
                        else if (0==stricmp("contains", S1.GetData()))
                        {
                            //pLand->CityName = S2;
                            //p = S2.GetToken(p, '[');
                            //p = pLand->CityType.GetToken(p, ']');

                            //There may be a space in the city name!
                            p = S1.GetToken(Struct.GetData(), ' ', TRIM_ALL);

                            p = pLand->CityName.GetToken(p, '[', TRIM_ALL);
                            if (!p)
                            {
                                // ok, it is a dot in the city name! need to append and rerun the section!
                                RerunSection = TRUE;
                                break;
                            }
                            p = pLand->CityType.GetToken(p, ']', TRIM_ALL);
                            // set town type LandFlags
                            if(0==SafeCmp(pLand->CityType.ToLower(),"town"))
                            {
                                pLand->Flags |= LAND_TOWN;
                            }
                            else if (0==SafeCmp(pLand->CityType.ToLower(),"city"))
                            {
                                pLand->Flags |= LAND_CITY;
                            }
                            // what about villages??
                        }
                        else if (!TerrainPassed)
                            TerrainPassed = TRUE; //so we can do special parsing for Arcadia III edge objects below
                        else if (IsExit)
                        {
                            // it must be an edge object...
                            if (pMotherLand)
                            {
                                pMotherLand->AddNewEdgeStruct(S1.GetData(), ExitDir);
                                // also add to neighbouring hex
                                int adj_dir = ExitDir -3;
                                if(adj_dir < 0) adj_dir += 6;
                                pLand->AddNewEdgeStruct(S1.GetData(), adj_dir);
                            }
                        }
                    }
                    break;

                case eSale:
                case eWanted:  // 35 horses [HORS] at $62
                               // N1 S1     [S2]   at $N2

//                    p = SkipSpaces(N1.GetToken(p, ' ', TRIM_ALL));
//                    n1= atol(N1.GetData());
                    // First number is optional, if missing it is 1
                    p = N1.GetInteger(p, Valid);
                    if (N1.IsEmpty())
                    {
                        N1.GetToken(p, " [", ch);
                        if (0==stricmp(N1.GetData(), "none"))
                            N1 = "-1";
                        else if (0==stricmp(N1.GetData(), "unlimited"))
                            N1 = "10000000";
                        else
                            N1 = "1";

                    }
                    n1 = atol(N1.GetData());
                    p = S1.GetToken(p, '[', TRIM_ALL);
                    p = S2.GetToken(p, ']', TRIM_ALL);
                    p = N2.GetToken(p, '$', TRIM_ALL);
                    N2= p;
                    n2= atol(N2.GetData());

                    if ((!S2.IsEmpty()) && (n2>0) )
                    {
                        //temporary I'll keep both, and then I'll remove Property part
                        CProductMarket temp_product;
                        temp_product.price_ = n2;
                        temp_product.item_.amount_ = n1;
                        temp_product.item_.code_name_ = std::string(S2.GetData(), S2.GetLength());
                        //temp_product.long_name_ = S1.GetData();
                        
                        if (eSale == SectType)
                            pLand->initial_state_.for_sale_[temp_product.item_.code_name_] = temp_product;
                        else
                            pLand->initial_state_.wanted_[temp_product.item_.code_name_] = temp_product;

                        std::string long_name, long_name_plural;
                        gpApp->ResolveAliasItems(temp_product.item_.code_name_, temp_product.item_.code_name_, long_name, long_name_plural);
                        if (n1 > 1)
                            long_name_plural = std::string(S1.GetData(), S1.GetLength());
                        else
                            long_name = std::string(S1.GetData(), S1.GetLength());
                        gpApp->SetAliasItems(temp_product.item_.code_name_, long_name, long_name_plural);

                        if (eSale == SectType)
                            MakeQualifiedPropertyName(PRP_SALE_AMOUNT_PREFIX, S2.GetData(), Buf);
                        else
                            MakeQualifiedPropertyName(PRP_WANTED_AMOUNT_PREFIX, S2.GetData(), Buf);
                        SetLandProperty(pLand, Buf.GetData(), eLong, (void*)n1, eBoth);

                        if (eSale == SectType)
                            MakeQualifiedPropertyName(PRP_SALE_PRICE_PREFIX, S2.GetData(), Buf);
                        else
                            MakeQualifiedPropertyName(PRP_WANTED_PRICE_PREFIX, S2.GetData(), Buf);
                        SetLandProperty(pLand, Buf.GetData(), eLong, (void*)n2, eBoth);

                    }
                    break;

                case eProduct: // Products: 40 grain [GRAI], 33 horses [HORS].
                               //           N1 S1    [S2]
                    p = SkipSpaces(N1.GetToken(p, ' ', TRIM_ALL));
                    n1= atol(N1.GetData());
                    if (0==n1)
                    {
                        if (0==stricmp(N1.GetData(), "none"))
                            n1 = -1;
                        else if  (0==stricmp(N1.GetData(), "unlimited"))
                            n1 = 10000000;
                    }
                    if (n1 >= 0)
                    {
                        CStr LongName, ShortName;
                        p = LongName.GetToken(p, '[', TRIM_ALL);
                        p = ShortName.GetToken(p, ']', TRIM_ALL);
                        std::string codename = std::string(ShortName.GetData(), ShortName.GetLength());

                        if (n1 > 0 && land_control::get_resource(pLand->initial_state_, codename) == 0)
                        {
                            //filling m_NewProducts for messaging regarding new products
                            // we have found a new product! Woo-hoo!
                            CStr          sCoord;
                            CBaseObject * pNewProd = new CBaseObject;

                            ComposeLandStrCoord(pLand, sCoord);
                            pNewProd->Name        << LongName << " discovered in (" << sCoord << ")";
                            pNewProd->Description << pLand->TerrainType << " (" << sCoord << ")"
                                                  << " is a new source of "  << LongName << EOL_SCR;

                            m_NewProducts.Insert(pNewProd);
                        }

                        land_control::add_resource(pLand->initial_state_, {n1, codename});

                        // also set as a property to simplify searching
                        MakeQualifiedPropertyName(PRP_RESOURCE_PREFIX, ShortName.GetData(), Buf);
                        SetLandProperty(pLand, Buf.GetData(), eLong, (void*)n1, eBoth);
                    }

                    break;

                    /*
                case eWages: //   Wages: $17.2 (Max: $10937).
                    p = SkipSpaces(N1.GetToken(p, ' ', TRIM_ALL));
                    if (!p || !*p)
                    {
                        RerunSection = TRUE;
                        break;
                    }

                    break;
                    */


                default:
                    break;

                }
                break; // case 0:
            }
        }
        if (RerunSection)
        {
            // a dot in the city name
            CStr S;
            src      = S.GetToken(src, '.');
            Section << '.' << S;
        }
        else
        {
            srcold   = src;
            src      = Section.GetToken(src, '.');
            SectType = eNone;
        }
    }

    if (dellen)
        Description.DelSubStr(delpos, dellen);

    return 0;
}

//----------------------------------------------------------------------

void CAtlaParser::ParseWages(CLand * pLand, const char * str1, const char * str2)
{
    //   Wages: $12.4 (Max: $350).    // str1 = "$12"  str2 = "4 (Max: $350)"
    //   Wages: $15 (Max: $10273).

    const char * src;
    CStr         N1, N2;
    CStr         sSrc;
    BOOL         Valid;

    str1 = SkipSpaces(str1);
    if (*str1 != '$')
        return;
    str1++;

    if (strchr(str1, '('))
    {
        src = SkipSpaces(N1.GetInteger(str1, Valid));
    }
    else
    {
        sSrc << str1 << '.' << str2;
        src = N1.GetDouble(sSrc.GetData(), Valid);
    }
    pLand->Wages = atof(N1.GetData());

    src = SkipSpaces(N1.GetToken(src, '$'));
    src = N1.GetInteger(src, Valid);

    pLand->initial_state_.work_.amount_ = atol(N1.GetData());
    pLand->current_state_.work_.amount_ = pLand->initial_state_.work_.amount_;
}

//----------------------------------------------------------------------

CPlane * CAtlaParser::MakePlane(const char * planename)
{
    static CBaseObject Dummy;  // we do not want it to be constructed/destructed all the way
                               // should be ok with multiple instances - single-threaded
    int      i;
    CPlane * pPlane;

    Dummy.Name = planename;
    if (m_PlanesNamed.Search(&Dummy, i))
        pPlane = (CPlane*)m_PlanesNamed.At(i);
    else
    {
        pPlane       = new CPlane;
        pPlane->Id   = m_Planes.Count();
        pPlane->Name = planename;
        if (!gpDataHelper->GetTropicZone(pPlane->Name.GetData(), pPlane->TropicZoneMin, pPlane->TropicZoneMax))
        {
            pPlane->TropicZoneMin  = TROPIC_ZONE_MAX;
            pPlane->TropicZoneMax  = -(TROPIC_ZONE_MAX);
        }

        // Format: <x-min>,<y-min>,<x-max>,<y-max>  Eg: 0,0,31,23
        const char * value = gpDataHelper->GetPlaneSize(pPlane->Name.GetData());
        if (value && strlen(value)>4)
        {
            CStr S;
            value = S.GetToken(value, ',');
            pPlane->WestEdge = atol(S.GetData());

            value = S.GetToken(value, ',');
            //y_min = atol(S.GetData());

            value = S.GetToken(value, ',');
            pPlane->EastEdge = atol(S.GetData());

            value = S.GetToken(value, ',');
            //y_max = atol(S.GetData());
            pPlane->Width = pPlane->EastEdge - pPlane->WestEdge + 1;
        }
        m_PlanesNamed.Insert(pPlane);
        m_Planes.Insert(pPlane);
    }

    return pPlane;
}


//----------------------------------------------------------------------

int CAtlaParser::ParseTerrain(CLand * pMotherLand, int ExitDir, CStr & FirstLine, BOOL FullMode, CLand ** ppParsedLand)
{
// FirstLine looks somewhat like:
//    swamp (48,52[,somewhere]) in Aghleam, 118 peasants (tribesmen), $354.

    int                  err = ERR_OK;
    const char         * p;
    CStr                 Name;
    CStr                 S(64);
    long                 x, y;
    CLand              * pLand = NULL;
    CStr                 CurLine(128);
    CStr                 PlaneName(32);
    CStr                 LandName(32);
    int                  i;
    int                  idxland;
    char                 ch;
    CPlane             * pPlane;
    CStruct            * pStruct;
    BOOL                 DoBreak;
    CBaseObject        * pGate;
    CBaseObject          Dummy;
    int                  no;
    int                  idx;
    CStr                 TempDescr(64); // Land description is collected in here. It can replace already existing description
    BOOL                 HaveEvents = FALSE;
    CStr                 CompositeDescr(64);

    if (FirstLine.IsEmpty())
        goto Exit;

    p = SkipSpaces(Name.GetToken(FirstLine.GetData(), '(', TRIM_ALL));
    if (!p )   // it must be '('
        goto Exit;

    // ok, now goes (xxx,yyy[,somewhere]) bla-bla-bla
    p = S.GetToken(p, ',');
    if (!IsInteger(S.GetData()))
        goto Exit;
    x = atol(S.GetData());

    // yyy[,somewhere]) bla-bla-bla
    p = S.GetToken(p, ",)", ch);
    if (!IsInteger(S.GetData()))
        goto Exit;
    y = atol(S.GetData());
    if (','==ch)
        // we have a plane name
        p = PlaneName.GetToken(p, ')');
    else
        PlaneName = DEFAULT_PLANE;

    if (!p)
        goto Exit;

    p = SkipSpaces(S.GetToken(SkipSpaces(p), ' ', TRIM_ALL));
    if (0!=stricmp(S.GetData(),"in"))
        goto Exit;
    LandName.GetToken(p, ",.", ch, TRIM_ALL);

    // Remove Arcadia III reference to edge location for sailing events
    if (strchr(LandName.GetData(), '('))
    {
        int x = strchr(LandName.GetData(), '(') - LandName.GetData();
        LandName.DelSubStr(x, LandName.GetLength()-x);
        LandName.TrimRight(TRIM_ALL);
    }

    pPlane = MakePlane(PlaneName.GetData());

    Dummy.Id = LandCoordToId(x,y, pPlane->Id);
    if (pPlane->Lands.Search(&Dummy, idxland))
    {
        pLand = (CLand*)pPlane->Lands.At(idxland);
        if (0!=stricmp(pLand->TerrainType.GetData(), Name.GetData()))
        {
            S.Format("*** Terrain changed for %s from '%s' to '%s' - clearing stored description and products! ***",
                     FirstLine.GetData(), pLand->TerrainType.GetData(), Name.GetData());
            GenericErr(1, S.GetData());
            pLand->TerrainType  = Name;
            pLand->Description.Empty();
            init_land_state(pLand->initial_state_);
        }
        if (0!=stricmp(pLand->Name.GetData(), LandName.GetData()) )
        {
            S.Format("*** Province changed for %s from '%s' to '%s' - clearing stored description! ***",
                     FirstLine.GetData(), pLand->Name.GetData(), LandName.GetData());
            GenericErr(1, S.GetData());
            pLand->Name     = LandName;
            pLand->Description.Empty();
            init_land_state(pLand->initial_state_);
        }
    }
    else
    {
        pLand               = new CLand;
        pLand->Id           = LandCoordToId(x,y, pPlane->Id);
        pLand->pPlane       = pPlane;
        pLand->Name         = LandName;
        pLand->TerrainType  = Name;
        init_land_state(pLand->initial_state_);
        pPlane->Lands.Insert(pLand);
    }

    if(gpApp->terrain_type_water(pLand))
    {
        pLand->Flags |= LAND_IS_WATER;
    }            


    TempDescr = FirstLine;

    if (pMotherLand)  // this is an exit description
    {
        // Exit description takes more then one line!
        while (ReadNextLine(CurLine))
        {
            // the line must not start from -+* and must not contain : .
            const char * s = SkipSpaces(CurLine.GetData());
            if (!s || !*s)
                break;

            if (strchr(STRUCT_UNIT_START, *s) || strchr(s, ':'))
            {
                PutLineBack(CurLine);
                break;
            }

            CurLine.TrimRight(TRIM_ALL);
            TempDescr << CurLine << EOL_SCR;

            if (strchr(CurLine.GetData(), '.'))
                break;
        }
    }

    if (pMotherLand && (pPlane==pMotherLand->pPlane))
        CheckExit(pPlane, ExitDir, pMotherLand, pLand);


    // It is only scan for exits from the land
    if (!FullMode)
    {
        TempDescr.TrimRight(TRIM_ALL);
        pLand->Description.TrimRight(TRIM_ALL);

        //this creates problems with rivers - once you have seen a river,
        //it will be there for all following exits to the same hex

//        if (TempDescr.GetLength() > pLand->Description.GetLength())
//            pLand->Description = TempDescr;

        if (pLand->Description.IsEmpty())
            pLand->Description = TempDescr;

        AnalyzeTerrain(pMotherLand, pLand, pMotherLand!=NULL, ExitDir, TempDescr);
        if (pMotherLand)
            pMotherLand->Exits << TempDescr << EOL_SCR;
        goto Exit;
    }

    //Structures can be destroyed, so remove those coming from history
    land_control::structures::clean_structures(pLand->initial_state_);

    //for (i=pLand->Structs.Count()-1; i>=0; i--)
    //{
    //    pStruct = (CStruct*)pLand->Structs.At(i);
    //    if (0==(pStruct->Attr & SA_HIDDEN) &&   // keep the gates!
    //        0==(pStruct->Attr & SA_SHAFT ) )    // keep the shafts!
    //        pLand->Structs.AtFree(i);
    //}

    // now  goes extended land description terminated by exits list
    // And check for other headers just in case!
    DoBreak = FALSE;
    while (ReadNextLine(CurLine))
    {
        CurLine.TrimRight(TRIM_ALL);
        p = SkipSpaces(CurLine.GetData());
        if (0==stricmp("Events:", p))
        {
            HaveEvents = TRUE;
            break;
        }
        if (0==stricmp("Exits:", p))
            break;
        for (i=0; i<(int)sizeof(ExitEndHeader)/(int)sizeof(const char *); i++)
            if (0==strnicmp(p, ExitEndHeader[i], ExitEndHeaderLen[i] ))
            {
                CurLine << EOL_FILE;
                PutLineBack(CurLine);
                DoBreak = TRUE;
                break;
            }
        if (DoBreak)
            break;

        // remove the atlaclient's turn mark
        no=0;
        while (p && *p && '-'==*p)
        {
            no++;
            p++;
        }
        if (no>20 && ';'==*p)
        {
            no = strlen(p++);
            CurLine.DelSubStr(CurLine.GetLength()-no, no);
            pLand->AtlaclientsLastTurnNo = atol(p);
        }

        TempDescr << CurLine << EOL_SCR;
    }

    // When should we replace old description with the new one?
    // My guess is - everytime for the full parsing!
    //pLand->Description = TempDescr;

    // Unfortunately, Arno in his latest game shows restricted description for hexes
    // through which your scout pass if there are no stationary units in the hex.

    ComposeHexDescriptionForArnoGame(pLand->Description.GetData(), TempDescr.GetData(), CompositeDescr);
    pLand->Description = CompositeDescr;
    pLand->Description.TrimRight(TRIM_ALL);
    AnalyzeTerrain(NULL, pLand, FALSE, ExitDir, pLand->Description);

    //Read Events and skip till Exits:
    DoBreak = FALSE;
    if (HaveEvents)
    {
        while (ReadNextLine(CurLine))
        {
            CurLine.TrimRight(TRIM_ALL);
            p = SkipSpaces(CurLine.GetData());
            if (0==stricmp("Exits:", p))
                break;
            for (i=0; i<(int)sizeof(ExitEndHeader)/(int)sizeof(const char *); i++)
                if (0==strnicmp(p, ExitEndHeader[i], ExitEndHeaderLen[i] ))
            {
                CurLine << EOL_FILE;
                PutLineBack(CurLine);
                DoBreak = TRUE;
                break;
            }
            if (DoBreak)
                break;
            pLand->Events << CurLine << EOL_SCR;
        }
        pLand->Events.TrimRight(TRIM_ALL);
    }


    if (!m_IsHistory && m_CurYearMon>0)
        pLand->Flags |= LAND_IS_CURRENT;

    m_pCurLand   = pLand;
    m_pCurStruct = NULL;

    if (pLand->Description.FindSubStr("Wanted") != -1)
    {
        // Full region report, clear all the Edge structures which were loaded from history
        pLand->EdgeStructs.FreeAll();
        pLand->Exits.Empty();
        pLand->CloseAllExits();
    }

    // now is a list of exits and gates terminated by unit or structure
    while (ReadNextLine(CurLine))
    {
        if (0==SafeCmp(EOL_SCR, CurLine.GetData()))
            continue;
        DoBreak = TRUE;
        p       = S.GetToken(CurLine.GetData(), ":(", ch);
        switch (ch)
        {
        case ':':  // is it an exit?

            if (0==stricmp(S.GetData(), FLAG_HDR))

            {
                pLand->FlagText[0] = SkipSpaces(p);
                pLand->FlagText[0].TrimRight(TRIM_ALL);
            }
            for (i=0; i<(int)sizeof(Directions)/(int)sizeof(const char*); i++)
                if (0==stricmp(S.GetData(), Directions[i]))
                {
                    // yes!
                    pLand->Exits << "  " << S << " : ";
                    pPlane->ExitsCount++;
                    S = p;
                    S.TrimLeft();
                    CLand * pLandRef = NULL;
                    ParseTerrain(pLand, i, S, FALSE, &pLandRef);
                    if (pLandRef)
                    {
                        int x, y, z;
                        LandIdToCoord(pLandRef->Id, x, y, z);
                        pLand->SetExit(i, x, y);
                    }
                    DoBreak = FALSE;
                    break;
                }
            break;
        case '(': // is it a gate?
            // There is a Gate here (Gate 18 of 35).
            if (0==stricmp(S.GetData(), "There is a Gate here"))
            {
                CStr sCoord;

                p  = SkipSpaces(S.GetToken(p, ' '));
                p  = S.GetToken(p, ' ');  // S = 18
                no = atol(S.GetData());
                pStruct     = new CStruct;
                pStruct->Id = -no;  // negative, so it does not clash with structures!
                pStruct->original_description_ = std::string(CurLine.GetData(), CurLine.GetLength());
                pStruct->type_        = STRUCT_GATE;
                if (!game_control::get_struct_attributes(pStruct->type_, 
                        pStruct->capacity_, pStruct->MinSailingPower, pStruct->Attr, pStruct->travel_, pStruct->max_speed_))
                    OrderError("Error", pLand, nullptr, nullptr, "Couldn't parse Gate data");

                land_control::structures::add_structure(pLand, pLand->initial_state_, pStruct);
                //pLand->AddNewStruct(pStruct);

                p = SkipSpaces(p);
                p = SkipSpaces(S.GetToken(p, ' '));  // S = of
                p = S.GetToken(p, ' ');  // S = 35
                m_GatesCount = atol(S.GetData());
                DoBreak = FALSE;

                pGate = new CBaseObject;
                pGate->Id = no;
                if (m_Gates.Search(pGate, idx))
                {
                    delete pGate;
                    pGate = (CBaseObject*)m_Gates.At(idx);
                }
                else
                    idx = -1;

                ComposeLandStrCoord(pLand, sCoord);
                pGate->Description.Format("Gate % 4d. ", no);
                pGate->Description << pLand->TerrainType << " (" << sCoord << ")";
                pGate->Name        = pGate->Description;

                if (idx<0)
                    m_Gates.Insert(pGate);                
            }
            break;
        }

        if (DoBreak)
        {
            PutLineBack(CurLine);
            goto Exit;

        }
    }




    // list of units....
    // will be obtained separately!

Exit:
    if (ppParsedLand)
        *ppParsedLand = pLand;

    return err;
}

//----------------------------------------------------------------------

const char * CountTokensForArno(const char * src, int & count)
{
    const char * p;
    char         ch;
    CStr         Token(32);

    count = 0;
    p = Token.GetToken(src, ')');
    while (p && *p)
    {
        p = Token.GetToken(p, ",.", ch, TRIM_NONE);
        count++;
        if ('.'==ch)
            break;
    }
    return p;
}

/*
desert (67,21) in Groddland, 182 peasants (nomads), $182.
------------------------------------------------------------
  The weather was clear last month; it will be clear next month.
  Wages: $11 (Max: $667).
  Wanted: none.
  For Sale: 36 nomads [NOMA] at $44, 7 leaders [LEAD] at $88.
  Entertainment available: $9.
  Products: 16 livestock [LIVE], 12 iron [IRON], 12 stone [STON].

desert (67,21) in Groddland.
plain (55,3) in Lothmarlun, contains Rudoeton [village].
------------------------------------------------------------
  The weather was clear last month; it will be winter next month.
  Wages: $0.
  Wanted: none.
  For Sale: none.
  Entertainment available: $0.
  Products: none.
*/

void CAtlaParser::ComposeHexDescriptionForArnoGame(const char * olddescr, const char * newdescr, CStr & CompositeDescr)
{
    const char * pnew;
    CStr         NewWeather(32);
    int          oldcount=0, newcount;
    CStr         Token(32);

    if (!olddescr || !*olddescr)
    {
        CompositeDescr = newdescr;
        return;
    }
    if (!newdescr || !*newdescr)
    {
        CompositeDescr = olddescr;
        return;
    }

    pnew = CountTokensForArno(newdescr, newcount);

    // Is new descr good?
    // We do not have to do full parsing here. Good descr has more pieces than bad
    if (newcount>oldcount)
    {
        CompositeDescr = newdescr;
        return;
    }

    // Maybe they are basically the same?
    if (newcount==oldcount)
    {
        if (strlen(olddescr) < strlen(newdescr))
            CompositeDescr = newdescr;
        return;
    }



    // now our new descr is worse, but it contains good weather line, we need to extract it and merge with old descr
    while (pnew && *pnew && *pnew!='-')
        pnew++;
    pnew = Token.GetToken(pnew, '\n');
    pnew = NewWeather.GetToken(pnew, '.', TRIM_NONE);

    CompositeDescr.Empty();
    pnew = Token.GetToken(olddescr, '.');
    CompositeDescr << Token << "." << EOL_SCR;

    while (pnew && *pnew && *pnew!='-')
        pnew++;
    pnew = Token.GetToken(pnew, '\n');
    CompositeDescr << Token << EOL_SCR;

    pnew = Token.GetToken(pnew, '.');    // old weather
    CompositeDescr << NewWeather << "." << pnew;

}

//----------------------------------------------------------------------

CUnit * CAtlaParser::construct_or_retrieve_unit(long unit_id)
{
    CUnit* pUnit = this->global_find_unit(unit_id);
    if (pUnit == nullptr)
    {
        pUnit = new CUnit;
        pUnit->Id = unit_id;
        units_.push_back(pUnit);
        units_ids_[unit_id] = units_.size()-1;
    }
    return pUnit;
}


long CAtlaParser::SkillDaysToLevel(long days)
{
    long level = 0;

    while (days>0)
    {
        days -= (level+1)*30;
        if (days>=0)
            level++;
    }
    return level;
}

//----------------------------------------------------------------------

/*
  Taxmen (767), on guard, Yellow Pants (34), revealing faction,
  taxing, 84 high elves [HELF], vodka [VODK], spear [SPEA], 2578
  silver [SILV]. Skills: combat [COMB] 2 (90).
*/

// Only normal brackets are banned, everything else can be in names and descriptions, including .,[]


int CAtlaParser::ParseUnit(CStr & FirstLine, BOOL Join)
{
#define DOT          '.'
#define COMMA        ','
#define SEMICOLON    ';'
#define STRUCT_LIMIT ",.;"
#define SECT_ITEMS   "Items"
#define SECT_SKILLS  "Skills"
#define SECT_COMBAT  "Combatspell"

    CFaction    * pFaction = NULL;
    CUnit       * pUnit    = NULL;
    CStr          CurLine (128);
    CStr          UnitText(128);
    CStr          UnitPrefix;
    CStr          Line    (128);
    CStr          FactName;
    CStr          Section ( 32);
    CStr          Buf     (128);
    CStr          S1      ( 32);
    CStr          S2      ( 32);
    CStr          N1      ( 32);
    CStr          N2      ( 32);
    CStr          N3      ( 32);
//    EValueType    type;
//    const void  * stance;
    long          n1;
    const char  * src = NULL;
    const char  * p;
    char          Delimiter;
    char          LastDelimiter = DOT;
    char          ch;
    int           err = ERR_OK;
    int64_t       attitude;
    BOOL          SkillsFound = FALSE;
    BOOL          Valid;

    FirstLine.TrimRight(TRIM_ALL);
    p = FirstLine.GetData();

    // Get the prefix - leading spaces and the first word
    while (p && *p && *p<=' ')
        UnitPrefix << *p++;
    while (p && *p && *p>' ')
        UnitPrefix << *p++;
    while (p && *p && *p<=' ')
        UnitPrefix << *p++;

    UnitText = p;
    UnitText << EOL_SCR;

    while (ReadNextLine(CurLine))
    {
        p = SkipSpaces(CurLine.GetData());

        // maybe terminated by an empty string
        if (!p || !*p)
            break;

        // or a next unit may just follow...
        if ( (0==strnicmp(p, HDR_UNIT_ALIEN        , sizeof(HDR_UNIT_ALIEN)-1 )) ||
              (0==strnicmp(p, HDR_UNIT_OWN          , sizeof(HDR_UNIT_OWN  )-1 )) ||
              (0==strnicmp(p, HDR_STRUCTURE         , sizeof(HDR_STRUCTURE )-1 ))
           )
        {
            PutLineBack(CurLine);
            break;
        }

        CurLine.TrimRight(TRIM_ALL);
        UnitText.AddStr(CurLine.GetData(), CurLine.GetLength());

        UnitText.AddStr(EOL_SCR);
    }

    // ===== Read Unit Name, which always goes first!

    p  = S1.GetToken(UnitText.GetData(), '(');
    p  = N1.GetToken(p, ')');
    n1 = atol(N1.GetData());
    if (n1<=0)
        return ERR_INV_UNIT;

    pUnit = construct_or_retrieve_unit(n1);
    if (m_pCurLand)
        m_pCurLand->AddUnit(pUnit);
    if (0 == strnicmp(SkipSpaces(UnitPrefix.GetData()), HDR_UNIT_OWN, sizeof(HDR_UNIT_OWN)-1) ||
        UnitPrefix.GetLength() + UnitText.GetLength() > pUnit->Description.GetLength())
    {
        pUnit->Description = UnitPrefix;
        pUnit->Description << UnitText;
        pUnit->Name = S1;
        //pUnit->initial_state_.name_ = std::string(S1.GetData(), S1.GetLength());
        //pUnit->initial_state_.description_ = std::string(S1.GetData(), S1.GetLength());

    }
    if (m_pCurStruct)
    {
        if (0==m_pCurStruct->OwnerUnitId)
        {
            SetUnitProperty(pUnit, PRP_STRUCT_OWNER, eCharPtr, YES, eBoth);
            SetUnitProperty(pUnit, PRP_STRUCT_ID,   eLong,    (void*)m_pCurStruct->Id,      eBoth);
            SetUnitProperty(pUnit, PRP_STRUCT_NAME, eCharPtr, m_pCurStruct->name_.c_str(), eBoth);
            //unit_control::set_structure(pUnit, m_pCurStruct->Id, true);
            pUnit->struct_id_initial_ = m_pCurStruct->Id | 0x00010000;//TODO: generalize
            pUnit->struct_id_ = pUnit->struct_id_initial_;
            m_pCurStruct->OwnerUnitId = pUnit->Id;
        }
        else
        {
            SetUnitProperty(pUnit, PRP_STRUCT_ID,   eLong,    (void*)m_pCurStruct->Id,      eBoth);
            SetUnitProperty(pUnit, PRP_STRUCT_NAME, eCharPtr, m_pCurStruct->name_.c_str(), eBoth);
            //unit_control::set_structure(pUnit, m_pCurStruct->Id, false);
            pUnit->struct_id_initial_ = m_pCurStruct->Id;//TODO: generalize
            pUnit->struct_id_ = pUnit->struct_id_initial_;            
        }        
    }
    else 
    {
        pUnit->struct_id_initial_ = 0;
        pUnit->struct_id_ = pUnit->struct_id_initial_;
    }

    // ===== Read Faction Name, which may follow!

    src = p;
    p   = strchr(src, '(');
    if (p)
    {
        while (p > src && *(p-1) != ',')
            p--; // position to the start of faction name

        p  = S1.GetToken(p, '(');
        p  = N1.GetToken(p, ')');
        n1 = atol(N1.GetData());
//        if (n1<=0)
//            return ERR_INV_UNIT;
        if (n1>0)
		{
			pFaction = GetFaction(n1);
			if (!pFaction)
			{
				pFaction       = new CFaction;
				pFaction->Name = S1;
				pFaction->Id   = n1;
				m_Factions.Insert(pFaction);
			}
			pUnit->FactionId= pFaction->Id;
			pUnit->pFaction = pFaction;

			pUnit->IsOurs = pUnit->IsOurs ||
					(m_CrntFactionId > 0 && pUnit->FactionId == m_CrntFactionId);
		}
    }

    // ===== Check faction attitude and set stances prop
    attitude = ATT_UNDECLARED;
    if(pUnit->IsOurs && (!Join)) // set own units to ATT_ME
    {
        attitude = ATT_ME;
    }
    else if(pUnit->IsOurs) // set unrevealing units of reporting faction
    {
        attitude = gpDataHelper->GetAttitudeForFaction(m_CrntFactionId);
    }
    else if(pUnit->IsOurs || (pUnit->FactionId!=0)) // set PRP_FRIEND_OR_FOE according to FactionId
    {
        attitude = gpDataHelper->GetAttitudeForFaction(pUnit->FactionId);
    }
    if((attitude >= 0) && (attitude < ATT_UNDECLARED))
    {
        SetUnitProperty(pUnit,PRP_FRIEND_OR_FOE,eLong,(void *) attitude,eNormal);
    }

    // ===== Now analize the rest of unit text

    if (','==*src)
        src++;
    while (src && *src)
    {
        src  = SkipSpaces(Line.GetToken(src, STRUCT_LIMIT, Delimiter, TRIM_ALL));

        if (DOT == LastDelimiter)
        {
            // New section. Find out the section name,
            p = SkipSpaces(Section.GetToken(Line.GetData(), ':'));
            if (p)
            {
                // remove section name from the structure
                Line.DelSubStr(0, p-Line.GetData());
            }
            else
            {
                // section has no name.
                if (!SkillsFound && 0!=stricmp(SECT_SKILLS, Section.GetData()))
                    Section = SECT_ITEMS;
            }
        }

/*
        Taxmen (767), on guard, Yellow Pants (34), revealing faction,
        taxing, 84 high elves [HELF], vodka [VODK], spear [SPEA], 2578
        silver [SILV]. Skills: combat [COMB] 2 (90).
*/
        if      (0==stricmp(SECT_ITEMS, Section.GetData()))
        {
            const void * data = NULL;
            if (pUnit)
            {
                Line.Normalize();
                if (m_UnitFlagsHash.Locate(Line.GetData(), data) )
                {
                    pUnit->Flags    |= (unsigned long)data;
                    pUnit->FlagsOrg |= (unsigned long)data;

                }
            }

            // recognize patterns:
            // S1 (N1) - faction name
            // N1 S1 [S2]
            // S1 [S2]

            // '[' can be present in the unit or faction name, so give '(' precedence.
            p = Buf.GetToken(Line.GetData(), "(", ch);
            if (!p)
                p = Buf.GetToken(Line.GetData(), "[", ch);
            switch (ch)
            {
                case '(':     // it is faction name again
                    p  = N1.GetToken(p, ')');
                    break;

                case '[':
                    p = N1.GetInteger(Line.GetData(), Valid);
                    if (N1.IsEmpty())
                        n1 = 1;
                    else
                        n1 = atol(N1.GetData());

                    p = S1.GetToken(p, '[', TRIM_ALL);
                    p = S2.GetToken(p, ']', TRIM_ALL);

                    if (!p || S2.IsEmpty() || !pUnit)
                    {
                    // there is no shortname
                        Buf.Format("Unit description - flag/item error at line %d", m_nCurLine);
                        LOG_ERR(ERR_DESIGN, Buf.GetData());
                    }
                    else
                    {
                        //update item aliases
                        std::string codename(S2.GetData(), S2.GetLength());
                        std::string long_name, long_name_plural;
                        gpApp->ResolveAliasItems(codename, codename, long_name, long_name_plural);
                        if (n1 > 1)
                            long_name_plural = std::string(S1.GetData(), S1.GetLength());
                        else
                            long_name = std::string(S1.GetData(), S1.GetLength());                        
                        gpApp->SetAliasItems(codename, long_name, long_name_plural);

                        //keep Properties while they are useful, and then remove them
                        SetUnitProperty(pUnit, S2.GetData(), eLong, (void*)n1, eBoth);
                        // is this a man property?
                        if (gpDataHelper->IsMan(S2.GetData()))
                        {
                            //peasants of current unit:
                            pUnit->men_initial_.insert({n1, codename});

                        } 
                        else if (codename == PRP_SILVER)
                        {
                            //silver of current unit
                            pUnit->silver_initial_.amount_ = n1;
                        }
                        else
                        {
                            //items of current unit:
                            pUnit->items_initial_.insert({n1, codename});
                        }
                    }
                    break;
            }
        }

        else if (0==stricmp(SECT_SKILLS, Section.GetData()))
        {
            SkillsFound = TRUE;

            // recognize patterns:
            // S1 [S2] N1 (N2)
            // S1 [S2] N1 (N2/N3)
            p = S1.GetToken(Line.GetData(), '[', TRIM_ALL);
            p = S2.GetToken(p, ']', TRIM_ALL);
            p = N1.GetToken(p, '(', TRIM_ALL);
            p = N2.GetToken(p, ")/", ch, TRIM_ALL);
            if ('/'==ch)
            {
                p = N3.GetToken(p, ')', TRIM_ALL);
                m_ArcadiaSkills = TRUE;
            }


            if (0!=stricmp("none", S1.GetData()))
            {
                if (!p || !pUnit)
                {
                    Buf.Format("Unit description - skills error at line %d", m_nCurLine);
                    LOG_ERR(ERR_DESIGN, Buf.GetData());
                }
                else
                {
                        // classic skills
                    Buf = S2;
                    Buf << PRP_SKILL_POSTFIX; // That is a skill!
                    SetUnitProperty(pUnit, Buf.GetData(), eLong, (void*)atol(N1.GetData()), eBoth);

                    Buf = S2;
                    Buf << PRP_SKILL_DAYS_POSTFIX;
                    SetUnitProperty(pUnit, Buf.GetData(), eLong, (void*)atol(N2.GetData()), eBoth);

                    pUnit->skills_initial_[std::string(S2.GetData(), S2.GetLength())] = atol(N2.GetData());
                    pUnit->skills_[std::string(S2.GetData(), S2.GetLength())] = atol(N2.GetData());

                    if (m_ArcadiaSkills)
                    {
                            // Arcadia III skills
                        long             n;
                        unsigned long    i;

                        n = atol(N2.GetData());
                        i = SkillDaysToLevel(n);

                        Buf = S2;
                        Buf << PRP_SKILL_STUDY_POSTFIX;
                        SetUnitProperty(pUnit, Buf.GetData(), eLong, (void*)i, eBoth);

                        n = atol(N3.GetData());
                        i = SkillDaysToLevel(n);
                        Buf = S2;
                        Buf << PRP_SKILL_EXPERIENCE_POSTFIX;
                        SetUnitProperty(pUnit, Buf.GetData(), eLong, (void*)i, eBoth);

                        Buf = S2;
                        Buf << PRP_SKILL_DAYS_EXPERIENCE_POSTFIX;
                        SetUnitProperty(pUnit, Buf.GetData(), eLong, (void*)n, eBoth);
                    }
                }
            }
        }


        else if (0 == SafeCmpNoSpaces(SECT_COMBAT, Section.GetData())) // looks like combat, but need to ignore spaces....
        {
            // recognize pattern:
            // S1 [S2]
            p = S1.GetToken(Line.GetData(), '[', TRIM_ALL);
            if (p)
                p = S2.GetToken(p, ']', TRIM_ALL);
            else
                S2 = Line;
            SetUnitProperty(pUnit, PRP_COMBAT, eCharPtr, S2.GetData(), eBoth);
        }


        if (SEMICOLON == Delimiter)
        {
            // That is a description, it comes last
            SetUnitProperty(pUnit, PRP_DESCRIPTION, eCharPtr, src, eBoth);
            break;
        }
        LastDelimiter = Delimiter;
    }

    // now check if this guy can see any advanced resources
    land_control::update_observable_resources(m_pCurLand, pUnit);
    //LookupAdvancedResourceVisibility(pUnit, m_pCurLand);

    return err;
}



//----------------------------------------------------------------------


int CAtlaParser::ParseStructure(CStr & FirstLine)
{
    //+ Ruin [1] : Ruin, closed to player units.
    //+ Ship [100] : Longboat, needs 10.
    //+ Forager [101] : Longboat.
    //+ Shaft [1] : Shaft, contains an inner location.
    //+ Three aspens [141] : Fleet, 3 Longships; Load: 263/300; Sailors: 12/12; MaxSpeed: 6.
    //+ Ship [197] : Fleet, 3 Galleons, 1 Galley; Sail directions: N, NW.
    // Description is terminated by a unit or an empty line
    if (!m_pCurLand)
        return ERR_OK;

    CStruct* pStruct = new CStruct();
    pStruct->original_description_ = std::string(FirstLine.GetData(), FirstLine.GetLength());

    struct_control::parse_struct(pStruct->original_description_,
                                 pStruct->Id, pStruct->name_, pStruct->type_, pStruct->fleet_ships_, pStruct->max_speed_);

    //pStruct->Name = pStruct->name_.c_str();
    long speed_from_types(50);
    for (const auto& subship : pStruct->fleet_ships_)
    {
        long load(0), spower(0), speed;
        std::string code, name, longname;
        game_control::get_struct_attributes(subship.first, load, spower, pStruct->Attr, 
                                            pStruct->travel_, speed);
        pStruct->capacity_ += load * subship.second;
        pStruct->MinSailingPower += spower * subship.second;
        speed_from_types = std::min(speed_from_types, speed);
    }

    if (pStruct->fleet_ships_.size() == 0)
    {//for non-mobile it needs to load flags
        game_control::get_struct_attributes(pStruct->type_, pStruct->capacity_, pStruct->MinSailingPower, 
                                            pStruct->Attr, pStruct->travel_, pStruct->max_speed_);
    } 
    else 
    {
        //its a fleet, so we should determine its speed.
        //in case that previously ship could not sail by any reason, it's speed may be below the speed
        //of the fleet's type. In this case we need to define speed according to the ship's type.
        //but if parsed speed was above the speed of the type of the ship (probably because of SWIN)
        //it's better to keep using it
        pStruct->max_speed_ = std::max(pStruct->max_speed_, speed_from_types);
    }

    if (pStruct->Attr & (SA_ROAD_N | SA_ROAD_NE | SA_ROAD_SE | SA_ROAD_S | SA_ROAD_SW | SA_ROAD_NW ))
        if (pStruct->original_description_.find("needs") != std::string::npos || 
            pStruct->original_description_.find("decay") != std::string::npos)
            pStruct->Attr |= SA_ROAD_BAD;

    m_pCurStruct = land_control::structures::add_structure(m_pCurLand, m_pCurLand->initial_state_, pStruct);
    //m_pCurStruct = m_pCurLand->AddNewStruct(pStruct);
    return ERR_OK;
}

//----------------------------------------------------------------------

wxString CAtlaParser::getFullStrLandCoord(CLand * pLand)
{
    CStr sCoord;
    wxString s;

    ComposeLandStrCoord(pLand, sCoord);
    s.Printf(wxT("%s (%s)"), pLand->TerrainType.GetData(), sCoord.GetData());
    if (!pLand->CityName.IsEmpty())
        s+= wxString::Format(wxT(" contains %s"), pLand->CityName.GetData());
    return s;
}

//----------------------------------------------------------------------

void CAtlaParser::SetShaftLinks()
{
    EValueType   type;
    const void * value;

    for (int i=0; i<m_LandsToBeLinked.Count(); i++)
    {
        CLand* land = (CLand*)m_LandsToBeLinked.At(i);

        if (!land->GetProperty(PRP_LAND_LINK, type, value, eOriginal) || eLong!=type)
            continue;

        land_control::perform_on_each_struct(land, [&](CStruct* structure) {
            if (struct_control::flags::is_shaft(structure))
            {
                CLand* target_land = land_control::get_land((long)value);

                size_t links_pos = structure->original_description_.find("links");
                if (target_land && links_pos != std::string::npos && 
                    GetLandFlexible(wxString::FromUTF8(structure->original_description_.c_str())) == target_land)
                    //link exists, and this link is already point to this land
                    return;

                if (target_land && links_pos == std::string::npos)
                {
                    land_control::structures::link_shafts(land, target_land, structure->Id);
                }
            }
        });
    }
    m_LandsToBeLinked.DeleteAll();
}

//----------------------------------------------------------------------

void CAtlaParser::ApplySailingEvents()
{
    for (int i=0; i<m_Planes.Count(); ++i)
    {    
        CPlane* plane = (CPlane*)m_Planes.At(i);
        for (int j = 0; j < plane->Lands.Count(); ++j)
        {
            CLand* land = (CLand*)plane->Lands.At(j);
            land_control::perform_on_each_unit(land, [&](CUnit* unit) {
                long unit_struct_id = unit_control::structure_id(unit);

                if (unit_struct_id > 0 && unit_control::is_struct_owner(unit))
                    unit->Events << sailing_events_[unit_struct_id].c_str();
            });
        }
    }
    sailing_events_.clear();
}

//----------------------------------------------------------------------

/* regular unit
* Beer Watcher (1099), Two Beers (23), behind, won't cross water,
  leader [LEAD], mithril sword [MSWO], cloth armor [CLAR], 4 rootstone
  [ROOT]. Weight: 212. Capacity: 0/0/15/0. Skills: observation [OBSE]
  5 (450), combat [COMB] 4 (442), stealth [STEA] 5 (450).

*/

/* Battle
Unit (3732) attacks City Guard (4429) in mountain (32,20) in Cromarty!

Attackers:
Unit (1769), behind, leader [LEAD], winged horse [WING], magic
  crossbow [MXBO], combat 1, crossbow 5.
Unit (2530), Two Beers (23), behind, 10 nomads [NOMA], winged horse
  [WING], horse [HORS].
Balrog (4423), Creatures (2), balrog [BALR] (Combat 6/6, Attacks 200,
  Hits 200, Tactics 5).

Defenders:
City Guard (4429), The Guardsmen (1), 4 leaders [LEAD], 4 swords
  [SWOR], combat 1.

Unit (3732) gets a free round of attacks.
City Guard (4429) loses 1.

Round 1:
Unit (3732) loses 0.
City Guard (4429) loses 0.

*/

const char * CAtlaParser::AnalyzeBattle_ParseUnit(const char * src, CUnit *& pUnit, BOOL & InFrontLine)
{
    CStr         Item, S, N1, S1, S2;
    char         ch, ch1;
    const char * p;
    long         n;

    InFrontLine = TRUE;
    pUnit       = NULL;

    while (src && *src)
    {
        src = Item.GetToken(src, ",.(", ch, TRIM_ALL );
        if (Item.IsEmpty())
            return src;
        if (!pUnit)
            pUnit = new CUnit;
        if ('('==ch)
        {
            // It is unit id, faction id or balrog. We do not give a shit about what exactly!
            src = S.GetToken(src, ')', TRIM_ALL);
            src = S.GetToken(src, ",.", ch, TRIM_ALL );
        }

        // now it can be one of the following which we are interested in:
        //    behind
        //    leader [LEAD]
        //    4 leaders [LEAD]
        //    crossbow 5
        if (0==stricmp(Item.GetData(), "behind"))
            InFrontLine = FALSE;
        else
        {
            p = Item.GetData();
            p = N1.GetToken(p, " \n", ch1, TRIM_ALL);
            p = S1.GetToken(p, '[', TRIM_ALL);
            p = S2.GetToken(p, ']', TRIM_ALL);

            if (!S2.IsEmpty())  // it is item
            {
                n = 1;
                if (N1.IsInteger())
                    n = atol(N1.GetData());
                SetUnitProperty(pUnit, S2.GetData(), eLong, (void*)n, eBoth);
            }
            else     // it is skill
            {
                S1.Empty();
                N1.Empty();
                p = Item.GetData();
                while (p && *p)
                {
                    p = N1.GetToken(p, " \n", ch1, TRIM_ALL);
                    if (p && *p)
                    {
                        if (!S1.IsEmpty())
                            S1 << ' ';
                        S1 << N1;
                    }
                }

                if (N1.IsInteger() && !S1.IsEmpty())
                {
                    S2 = gpDataHelper->ResolveAlias(S1.GetData());
                    S2 << PRP_SKILL_POSTFIX; // That is a skill!
                    n = atol(N1.GetData());
                    SetUnitProperty(pUnit, S2.GetData(), eLong, (void*)n, eBoth);
                }
            }
        }

        if ('.'==ch)
            break;
    }

    return src;
}

//----------------------------------------------------------------------

void CAtlaParser::AnalyzeBattle_SummarizeUnits(CBaseColl & Units, CStr & Details)
{
    CStr            propname;
    int             i, propidx;
    CUnit         * pUnit;
    CBaseObject     Faction;
    EValueType      type;
    const void    * value;
    const void    * valuetot;
    int             skilllen, maxproplen=0;

    skilllen    = strlen(PRP_SKILL_POSTFIX);
    for (i=0; i<Units.Count(); i++)
    {
        pUnit = (CUnit*)Units.At(i);

        for (propidx=0; propidx<m_UnitPropertyNames.Count(); propidx++)
        {
            propname = (const char *) m_UnitPropertyNames.At(propidx);
            if (!pUnit->GetProperty(propname.GetData(), type, value, eOriginal) || eLong!=type )
                continue;
            if (propname.FindSubStrR(PRP_SKILL_POSTFIX) == propname.GetLength()-skilllen)
            {
                // it is a skill
                propname << (long)value;
                if (!pUnit->GetProperty(PRP_MEN, type, value, eOriginal) || eLong!=type )
                    continue;
            }
            if (!Faction.GetProperty(propname.GetData(), type, valuetot, eNormal))
                valuetot = (void*)0;

            if (-1==(long)valuetot || 0x7fffffff - (long)value < (long)valuetot )
                valuetot = (void*)(long)-1; // overflow protection
            else
                valuetot = (void*)((long)valuetot + (long)value);
            Faction.SetProperty(propname.GetData(), eLong, valuetot, eNormal);
            if (propname.GetLength() > maxproplen)
                maxproplen = propname.GetLength();
        }
    }

    propidx  = 0;
    propname = Faction.GetPropertyName(propidx);
    while (!propname.IsEmpty())
    {
        if (Faction.GetProperty(propname.GetData(), type, value, eNormal) &&
            (eLong==type) )
        {
            while (propname.GetLength() < maxproplen)
                propname.AddCh(' ');
            Details << "   " << propname << "  " << (long)value << EOL_SCR;
        }

        propname = Faction.GetPropertyName(++propidx);
    }


}


//----------------------------------------------------------------------

void CAtlaParser::AnalyzeBattle_OneSide(const char * src, CStr & Details)
{
    CBaseColl   Frontline(64), Backline(64);
    CUnit     * pUnit;
    BOOL        InFrontLine;
    CStr        S1(64), S2(64);

    while (src && *src)
    {
        src = AnalyzeBattle_ParseUnit(src, pUnit, InFrontLine);
        if (!pUnit)
            continue;
        if (InFrontLine)
            Frontline.Insert(pUnit);
        else
            Backline.Insert(pUnit);
    }

    AnalyzeBattle_SummarizeUnits(Frontline, S1);
    AnalyzeBattle_SummarizeUnits(Backline, S2);

    Details << "Front line" << EOL_SCR << S1;
    Details << "Back line" << EOL_SCR << S2 ;

    Frontline.FreeAll();
    Backline.FreeAll();
}

//----------------------------------------------------------------------

void CAtlaParser::AnalyzeBattle(const char * src, CStr & Details)
{
    CStr         Line(64), Attackers(64), Defenders(64);
    CStr         S1, N1, S2;
    const char * p;

    Details.Empty();

    // skip start
    while (src && *src)
    {
        src = Line.GetToken(src, '\n', TRIM_ALL);
        if (0==strnicmp(Line.GetData(), HDR_ATTACKERS, sizeof(HDR_ATTACKERS)-1))
            break;
    }
    // read attackers
    while (src && *src)
    {
        src = Line.GetToken(src, '\n', TRIM_ALL);
        if (0==strnicmp(Line.GetData(), HDR_DEFENDERS, sizeof(HDR_ATTACKERS)-1))
            break;
        Attackers << Line << EOL_SCR;
    }

    // read defenders
    while (src && *src)
    {
        src = Line.GetToken(src, '\n', TRIM_ALL);

        // Unit (3732) gets a free round of attacks.
        p = Line.GetData();
        p = S1.GetToken(p, '(', TRIM_ALL);
        p = N1.GetToken(p, ')', TRIM_ALL);
        p = S2.GetToken(p, '.', TRIM_ALL);
        if (!S1.IsEmpty() && N1.IsInteger() && 0==stricmp(S2.GetData(), "gets a free round of attacks"))
            break;

        //Round 1:
        p = Line.GetData();
        p = S1.GetToken(p, ' ', TRIM_ALL);
        p = N1.GetToken(p, ':', TRIM_ALL);
        if (0==stricmp(S1.GetData(), "Round") && N1.IsInteger())
            break;

        Defenders << Line << EOL_SCR;
    }

    Details << EOL_SCR << EOL_SCR << "----------------------------------------------"
            << EOL_SCR << "Statistics for the battle:" << EOL_SCR << EOL_SCR;
    Details << HDR_ATTACKERS << EOL_SCR;
    AnalyzeBattle_OneSide(Attackers.GetData(), Details);

    Details << EOL_SCR;
    Details << HDR_DEFENDERS << EOL_SCR;
    AnalyzeBattle_OneSide(Defenders.GetData(), Details);
}

//----------------------------------------------------------------------

void CAtlaParser::StoreBattle(CStr & Source)
{
    CStr          S1, S2, S3;
    CStr          N1, N2, N3;
    CBattle     * pBattle;
    const char  * p;
    int           i;
    BOOL          Ok = FALSE;
    BOOL          RegularBattle = FALSE;

    Source.TrimRight(TRIM_ALL);
    if (Source.IsEmpty())
        return;
    //Source.Normalize();

    // Captain (15166) is assassinated in forest (83,129) in Imyld!
    p = Source.GetData();
    p = SkipSpaces(N1.GetToken(p, '(', TRIM_ALL));
    p = SkipSpaces(N1.GetToken(p, ')', TRIM_ALL));
    p = SkipSpaces(S1.GetToken(p, ' ', TRIM_ALL));
    p = SkipSpaces(S2.GetToken(p, ' ', TRIM_ALL));

    if ( IsInteger(N1.GetData()) &&
         0 == stricmp("is"          , S1.GetData()) &&
         0 == stricmp("assassinated", S2.GetData())
       )
    {
        p  = S3.GetToken(p, '(', TRIM_ALL);
        p  = N3.GetToken(p, ',', TRIM_ALL);
        Ok = TRUE;
    }
    else
    {
        //Xbowmen (591) attacks City Guard (24) in swamp (13,33) in Salen!
        p  = Source.GetData();
        p  = S1.GetToken(p, '(', TRIM_ALL);
        p  = N1.GetToken(p, ')', TRIM_ALL);
        p  = S2.GetToken(p, '(', TRIM_ALL);
        p  = N2.GetToken(p, ')', TRIM_ALL);
        p  = S3.GetToken(p, '(', TRIM_ALL);
        p  = N3.GetToken(p, ',', TRIM_ALL);
        Ok = (  IsInteger(N1.GetData()) &&
                IsInteger(N2.GetData()) &&
                IsInteger(N3.GetData()) &&
                (0==strnicmp(S3.GetData(), "in", 2)) );
        RegularBattle = TRUE;
    }

    if (Ok)
    {
        p = S3.GetToken(p, ')', TRIM_ALL);
        N3 << "," << S3;
        N3.Normalize();

        if (!m_BattleLandStrs.Search((void*)N3.GetData(), i))
            m_BattleLandStrs.Insert(strdup(N3.GetData()));

        pBattle              = new CBattle;
        pBattle->LandStrId   = N3;
        pBattle->Description = Source;
        pBattle->Name.SetStr(Source.GetData(), p-Source.GetData());
        pBattle->Name.Normalize();

        if (RegularBattle && atol(gpDataHelper->GetConfString(SZ_SECT_COMMON, SZ_KEY_BATTLE_STATISTICS)))
        {
            CStr Details(64);
            AnalyzeBattle(Source.GetData(), Details);
            pBattle->Description << EOL_SCR <<  Details;
        }

        /*
        if (!m_Battles.Insert(pBattle))
            delete pBattle;
            */
        for (i=0; i<m_Battles.Count(); i++)
        {
            CBattle * pExisting = (CBattle*)m_Battles.At(i);
            if (0==stricmp(pBattle->Name.GetData(), pExisting->Name.GetData()))
            {
                delete pBattle;
                pBattle = NULL;
                break;
            }
        }
        if (pBattle)
            m_Battles.Insert(pBattle);
    }
}

//----------------------------------------------------------------------

// It suxx to build parsing on empty strings, but I can not see any clear sign of a battle end

int CAtlaParser::ParseBattles()
{
    CStr         CurLine(64);
    CStr         Battle(128);
    CStr         Block1(64), Block2(64);
    const char * p;
    int          i;
    CStr         S1, S2, N1;

    while (ReadNextLine(CurLine))
    {
        CurLine.TrimRight(TRIM_ALL);

        if (CurLine.IsEmpty())
        {
            // Captain (15166) is assassinated in forest (83,129) in Imyld!
            p = SkipSpaces(Block1.GetData());
            p = SkipSpaces(N1.GetToken(p, '(', TRIM_ALL));
            p = SkipSpaces(N1.GetToken(p, ')', TRIM_ALL));
            p = SkipSpaces(S1.GetToken(p, ' ', TRIM_ALL));
            p = SkipSpaces(S2.GetToken(p, ' ', TRIM_ALL));


            if (IsInteger(N1.GetData()) &&
                0 == stricmp("is"          , S1.GetData()) &&
                0 == stricmp("assassinated", S2.GetData())
                )
            {
                // It is an assassination
                Battle << Block2 << EOL_SCR;
                StoreBattle(Battle);
                Battle.Empty();
                Block2 = Block1;
                Block1.Empty();
            }
            else
            {
                Battle << Block2 << EOL_SCR;
                Block2 = Block1;
                Block1.Empty();
            }
        }
        else
        {
            if (0==strnicmp(CurLine.GetData(), HDR_ATTACKERS, sizeof(HDR_ATTACKERS)-1))
            {
                // Ho-ho! New battle starting!  Finish the old one first.
                StoreBattle(Battle);
                Battle.Empty();
                Battle << Block2 << EOL_SCR;
                Block2 = Block1;
                Block1.Empty();
            }
            else
            {
                // Is the battle list over yet?
                p = SkipSpaces(CurLine.GetData());
                for (i=0; i<(int)sizeof(BattleEndHeader)/(int)sizeof(const char *); i++)
                    if (0==strnicmp(p, BattleEndHeader[i], BattleEndHeaderLen[i] ))
                    {
                        CurLine << EOL_FILE;
                        PutLineBack(CurLine);
                        Battle << Block2 << Block1 << EOL_SCR;
                        StoreBattle(Battle);
                        Battle.Empty();
                        return 0;
                    }
            }
            Block1 << CurLine << EOL_SCR;
        }
    }

    return 0;
}


//----------------------------------------------------------------------

//Skill reports:
//
//armorer [ARMO] 5: A unit with this skill may PRODUCE mithril armor
//  from one unit of mithril. Mithril armor provides a 9/10 chance of
//  surviving a successful attack in battle from a normal weapon and a
//  2/3 chance of surviving an attack from a good weapon.  Mithril armor
//  weighs one unit. Production of mithril armor can be increased by
//  using hammers.
//
//entertainment [ENTE] 1: A unit with this skill may use the ENTERTAIN
//  order to generate funds. The amount of silver gained will be 20 per
//  man, times the level of the entertainers. This amount is limited by
//  the region that the unit is in.

int CAtlaParser::ParseSkills()
{
    CStr             CurLine(64);

    CStr             OneSkill(128);
    CStr             S;
    const char     * p;
    CShortNamedObj * pSkill;

    while (ReadNextLine(CurLine))
    {
        CurLine.TrimRight(TRIM_ALL);
        if (CurLine.IsEmpty())
        {
            if (!OneSkill.IsEmpty())
            {
                pSkill = new CShortNamedObj;

                p = OneSkill.GetData();
                p = S.GetToken(p, '[');     pSkill->Name        = S;
                p = S.GetToken(p, ']');     pSkill->ShortName   = S;
                p = S.GetToken(p, ':');     pSkill->Level       = atol(S.GetData());

                pSkill->Description = OneSkill;

                pSkill->Name.GetToken(OneSkill.GetData(), ':');

                m_Skills.Insert(pSkill);
                OneSkill.Empty();
            }
        }
        else
        {
            // check for invalid format in the first line
            if (OneSkill.IsEmpty())
            {
                p = S.GetToken(CurLine.GetData(), '[');
                if (!p)
                    break;
                p = S.GetToken(p, ']');
                if (!p)
                    break;
                p = S.GetToken(p, ':');
                if (!p)
                    break;
                if (atol(S.GetData()) <= 0)
                    break;
            }

            OneSkill << CurLine << EOL_SCR;
        }
    }
    CurLine << EOL_FILE;
    PutLineBack(CurLine);

    return 0;
}


//----------------------------------------------------------------------

//ironwood [IRWD], weight 10. Units with lumberjack [LUMB] 3 may PRODUCE
//  this item at a rate of 1 per man-month.
//
//mithril [MITH], weight 10. Units with mining [MINI] 3 may PRODUCE this
//  item at a rate of 1 per man-month.
//

int CAtlaParser::ParseItems()
{
    CStr             CurLine(64);
    CStr             OneItem(128);
    CStr             S;
    const char     * p;
    CShortNamedObj * pItem;

    while (ReadNextLine(CurLine))
    {
        CurLine.TrimRight(TRIM_ALL);
        if (CurLine.IsEmpty())
        {
            if (!OneItem.IsEmpty())
            {
                pItem = new CShortNamedObj;

                p = OneItem.GetData();
                p = S.GetToken(p, '[');     pItem->Name        = S;
                p = S.GetToken(p, ']');     pItem->ShortName   = S;

                pItem->Description = OneItem;
                pItem->Name.GetToken(OneItem.GetData(), ',');

                m_Items.Insert(pItem);
                OneItem.Empty();
            }
        }
        else
        {
            // check for invalid format in the first line
            if (OneItem.IsEmpty())
            {
                p = S.GetToken(CurLine.GetData(), '[');
                if (!p)
                    break;
                p = S.GetToken(p, ']');
                if (!p)
                    break;
                p = S.GetToken(p, ',');
                if (!p)
                    break;
                if (!S.IsEmpty())
                    break;
            }

            OneItem << CurLine << EOL_SCR;
        }
    }
    CurLine << EOL_FILE;
    PutLineBack(CurLine);

    return 0;
}

//----------------------------------------------------------------------

//Object reports:
//
//Longboat: This is a ship. Units may enter this structure. This ship
//  requires 5 total levels of sailing skill to sail. This structure is
//  built using shipbuilding [SHIP] 1 and requires 25 wood to build.
//
//Clipper: This is a ship. Units may enter this structure. This ship
//  requires 10 total levels of sailing skill to sail. This structure is
//  built using shipbuilding [SHIP] 1 and requires 50 wood to build.


int CAtlaParser::ParseObjects()
{
    CStr             CurLine(64);
    CStr             OneItem(128);
    CStr             S;
    const char     * p;
    CShortNamedObj * pItem;
    char             ch;

    while (ReadNextLine(CurLine))
    {
        CurLine.TrimRight(TRIM_ALL);
        if (CurLine.IsEmpty())
        {
            if (!OneItem.IsEmpty())
            {
                pItem = new CShortNamedObj;

                p = OneItem.GetData();
                p = S.GetToken(p, ':');
                pItem->Name        = S;
                pItem->ShortName   = S;
                pItem->Description = OneItem;

                m_Objects.Insert(pItem);
                OneItem.Empty();
            }
        }
        else
        {
            // check for invalid format in the first line
            if (OneItem.IsEmpty())
            {
                p = SkipSpaces(S.GetToken(CurLine.GetData(), ':'));
                if (!p)
                    break;
                p = SkipSpaces(S.GetToken(p, " \t", ch));
                if (0!=stricmp(S.GetData(), "This"))
                    break;
                p = SkipSpaces(S.GetToken(p, " \t", ch));
                if (0!=stricmp(S.GetData(), "is"))
                    break;
            }

            OneItem << CurLine << EOL_SCR;
        }
    }
    CurLine << EOL_FILE;
    PutLineBack(CurLine);

    return 0;
}


//----------------------------------------------------------------------

int CAtlaParser::ParseLines(BOOL Join)
{
    int        err      = ERR_OK;
    CStr       CurLine(64);
    CStr       sErr;
    const char * p;

    while (ReadNextLine(CurLine))
    {
        // recognize header line and then call specific handler

        p = SkipSpaces(CurLine.GetData());

        if      (0==strnicmp(p, HDR_FACTION           , sizeof(HDR_FACTION)-1 ))
            err = ParseFactionInfo(TRUE, Join);

        else if (0==strnicmp(p, HDR_FACTION_STATUS    , sizeof(HDR_FACTION_STATUS)-1 ))
            err = ParseFactionInfo(FALSE, Join);

        else if (0==strnicmp(p, HDR_EVENTS            , sizeof(HDR_EVENTS)-1 ))
            err = ParseEvents();

        else if (0==strnicmp(p, HDR_EVENTS_2          , sizeof(HDR_EVENTS_2)-1 ))
            err = ParseImportantEvents();

        else if (0==strnicmp(p, HDR_ERRORS            , sizeof(HDR_ERRORS)-1 ))
            err = ParseErrors();

        else if (0==strnicmp(p, HDR_SILVER            , sizeof(HDR_SILVER)-1 ))
            err = ParseUnclSilver(CurLine);

        else if (0==strnicmp(p, HDR_ATTITUDES         , sizeof(HDR_ATTITUDES)-1 ))
            err = ParseAttitudes(CurLine, Join);

        else if (0==strnicmp(p, HDR_UNIT_OWN          , sizeof(HDR_UNIT_OWN)-1 ))
            err = ParseUnit(CurLine, Join);

        else if (0==strnicmp(p, HDR_UNIT_ALIEN        , sizeof(HDR_UNIT_ALIEN)-1 ))
            err = ParseUnit(CurLine, Join);

        else if (0==strnicmp(p, HDR_STRUCTURE         , sizeof(HDR_STRUCTURE)-1 ))
            err = ParseStructure(CurLine);

        else if (0==strnicmp(p, HDR_BATTLES           , sizeof(HDR_BATTLES)-1 ))
            err = ParseBattles();

        else if (0==strnicmp(p, HDR_SKILLS            , sizeof(HDR_SKILLS)-1 ))
            err = ParseSkills();

        else if (0==strnicmp(p, HDR_ITEMS             , sizeof(HDR_ITEMS)-1 ))
            err = ParseItems();

        else if (0==strnicmp(p, HDR_OBJECTS           , sizeof(HDR_OBJECTS)-1 ))
            err = ParseObjects();

        else if (0==strnicmp(p, HDR_ORDER_TEMPLATE    , sizeof(HDR_ORDER_TEMPLATE)-1 ))
            err = LoadOrders(*m_pSource, m_CrntFactionId, FALSE); // That's a non-return point.
                                                            // Hope there is nothing else after the order template.

        else

            err = ParseTerrain(NULL, 0, CurLine, TRUE, NULL);

        if (ERR_OK!=err)
        {
            sErr.Empty();
            sErr << "Error parsing report related to line " << (long)m_nCurLine  << EOL_SCR
                 << "\"" << CurLine << "\"" << "." << EOL_SCR;

            if (ERR_INV_TURN==err)
                sErr << EOL_SCR << "Joined reports must be for the same turn. This is intended for joining your ally reports.";

            LOG_ERR(ERR_PARSE, sErr.GetData());

            if (ERR_INV_TURN==err)
                break;
        }
    }

    return ERR_OK;
}

//----------------------------------------------------------------------

int CAtlaParser::ParseRep(const char * FNameIn, BOOL Join, BOOL IsHistory)
{
    m_pCurLand   = NULL;
    m_pCurStruct = NULL;
    m_ParseErr   = ERR_NOTHING;
    m_pSource    = new CFileReader;
    m_JoiningRep = Join;
    m_IsHistory  = IsHistory;
    m_CrntFactionId = 0;
    m_CrntFactionPwd.Empty();
    m_IsHistory  = IsHistory;
    m_nCurLine   = 0;

    if (Join)
        m_FactionInfo << EOL_SCR << EOL_SCR << "-----------------------------------------------"
                      << EOL_SCR << EOL_SCR;

    if (!Join)
        m_YearMon = 0;
    m_CurYearMon  = 0;

    if (!m_pSource->Open(FNameIn))
    {
        m_ParseErr = ERR_FOPEN;
        goto Done;
    }

//    m_MaxSkillDays = atol(gpDataHelper->GetConfString(SZ_SECT_COMMON, SZ_KEY_MAX_SKILL_DAYS));

    m_ParseErr = ParseLines(Join);

    ApplyLandFlags();
    SetExitFlagsAndTropicZone();
    SetShaftLinks();
    ApplySailingEvents();

Done:
    m_pSource->Close();
    delete m_pSource;
    m_pSource = NULL;
    return m_ParseErr;
}


//----------------------------------------------------------------------

void CAtlaParser::SetExitFlagsAndTropicZone()
{
    int           np;
    CPlane      * pPlane;

    for (np=0; np<m_Planes.Count(); np++)
    {
        pPlane = (CPlane*)m_Planes.At(np);

        //tropic zone
        if (!m_IsHistory && m_CurYearMon>0 && pPlane->TropicZoneMin <= pPlane->TropicZoneMax)
            gpDataHelper->SetTropicZone(pPlane->Name.GetData(), pPlane->TropicZoneMin, pPlane->TropicZoneMax);
    }
}

//----------------------------------------------------------------------


void AddTabbed(CStr & Dest, const char * Src, int Offs)
{
#define SPC "  "
    int  i;
    int  n;
    CStr Spc;

    for (i=0; i<Offs; i++)
        Spc << SPC;
    n = Spc.GetLength();
    Dest << Spc;

    while (Src && *Src)
    {
        if ( (n>=64) && (' '==*Src) )
        {
            Dest << EOL_SCR << Spc << SPC;
            n = Spc.GetLength() + sizeof(SPC) - 1;
        }
        Dest << *Src;
        Src++;
        n++;
    }
    Dest << EOL_SCR;
}

//----------------------------------------------------------------------

void CAtlaParser::GetUnitList(CCollection * pResultColl, int x, int y, int z)
{
    CLand* pLand = GetLand(x, y, z);
    if (pLand)
        for (CUnit* unit : pLand->units_seq_)
            pResultColl->Insert(unit);
}

//-------------------------------------------------------------

CFaction * CAtlaParser::GetFaction(int id)
{
    CBaseObject  DummyFaction;
    int          idx;

    DummyFaction.Id = id;
    if (m_Factions.Search(&DummyFaction, idx))
        return (CFaction*)m_Factions.At(idx);
    else
        return NULL;
}

//--------------------------------------------------------------------------

// Closed means that a unit will never be able to pass here.
bool CAtlaParser::IsLandExitClosed(CLand * pLand, int direction) const
{
    if (!pLand) return false;

    if (pLand->xExit[direction] == EXIT_CLOSED)
    {
        return true;
    }
    if (pLand->xExit[direction] == EXIT_MAYBE)
    {
        // Should try to extend the map and see if we can get more information.
        int x, y, z;
        LandIdToCoord(pLand->Id, x, y, z);
        ExtrapolateLandCoord(x, y, z, direction);
        CLand * pLand = GetLand(x, y, z, TRUE);
        if (pLand)
        {
            if (pLand->xExit[(direction+3)%6] == EXIT_CLOSED)
            {
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------

int  CAtlaParser::NormalizeHexX(int NoX, CPlane * pPlane) const
{
    if (pPlane && pPlane->Width>0)
    {
        while (NoX < pPlane->WestEdge)
            NoX += pPlane->Width;
        while (NoX > pPlane->EastEdge)
            NoX -= pPlane->Width;
    }
    return NoX;
}

//-------------------------------------------------------------

void CAtlaParser::ExtrapolateLandCoord(int &x, int &y, int z, int direction) const
{
    // Try to go in a direction on the map by assuming a simple grid.
    switch (direction % 6)
    {
        case North     : y -= 2;     break;
        case Northeast : y--; x++;   break;
        case Southeast : y++; x++;   break;
        case South     : y += 2;     break;
        case Southwest : y++; x--;   break;
        case Northwest : y--; x--;   break;
    }

    CPlane * pPlane = (CPlane*)m_Planes.At(z);
    x = NormalizeHexX(x, pPlane);
}

//-------------------------------------------------------------

CLand * CAtlaParser::GetLandExit(CLand * pLand, int direction) const
{
    if (pLand->xExit[direction] == EXIT_MAYBE)
    {
        // Should try to extend the map and proceed
        int x, y, z;
        LandIdToCoord(pLand->Id, x, y, z);
        ExtrapolateLandCoord(x, y, z, direction);
        return GetLand(x, y, z, FALSE);
    }
    if (pLand->xExit[direction] == EXIT_CLOSED)
    {
        return NULL;
    }
    return GetLand(pLand->xExit[direction], pLand->yExit[direction], pLand->pPlane->Id, TRUE);
}

//-------------------------------------------------------------
CLand * CAtlaParser::GetLand(int x, int y, int nPlane, BOOL AdjustForEdge) const
{
    char     dummy[sizeof(CLand)];
    int      idx;
    CPlane * pPlane;

    pPlane = (CPlane*)m_Planes.At(nPlane);
    if (!pPlane)
        return NULL;

    if (AdjustForEdge)
    {
        x = NormalizeHexX(x, pPlane);
    }

    ((CLand*)&dummy)->Id = LandCoordToId(x,y, pPlane->Id);

    if (pPlane->Lands.Search(&dummy, idx))
        return (CLand*)pPlane->Lands.At(idx);
    else
        return NULL;
}

//-------------------------------------------------------------

CLand * CAtlaParser::GetLand(long LandId) const
{
    int x, y, z;

    LandIdToCoord(LandId, x, y, z);
    return GetLand(x, y, z, FALSE);
}


//-------------------------------------------------------------

void CAtlaParser::ComposeLandStrCoord(CLand * pLand, CStr & LandStr)
{
    int      x, y, z;
    CPlane * pPlane;

    LandStr.Empty();
    if (!pLand)
        return;

    LandIdToCoord(pLand->Id, x, y, z);
    pPlane = (CPlane*)m_Planes.At(z);
    if (!pPlane)
        return;

    LandStr << (long)x << "," << (long)y;
    if (0!=SafeCmp(pPlane->Name.GetData(), DEFAULT_PLANE))
        LandStr << "," << pPlane->Name.GetData();
}

//-------------------------------------------------------------

BOOL CAtlaParser::LandStrCoordToId(const char * landcoords, long & id) const
{
    CStr                 S;
    long                 x, y;
    CPlane             * pPlane;
    CBaseObject          Dummy;
    int                  i;

    // xxx,yyy[,somewhere]
    landcoords = S.GetToken(landcoords, ',');
    if (!IsInteger(S.GetData()))
        return FALSE;
    x = atol(S.GetData());

    // yyy[,somewhere]
    landcoords = S.GetToken(landcoords, ',');
    if (!IsInteger(S.GetData()))
        return FALSE;
    y = atol(S.GetData());


    if ( (NULL==landcoords) || (0==*landcoords) )
        landcoords = DEFAULT_PLANE;

    Dummy.Name = landcoords;
    if (m_PlanesNamed.Search(&Dummy, i))
        pPlane = (CPlane*)m_PlanesNamed.At(i);
    else
        return FALSE;

    id = LandCoordToId(x, y, pPlane->Id);
    return TRUE;
}



//-------------------------------------------------------------

CLand * CAtlaParser::GetLand(const char * landcoords) const //  "48,52[,somewhere]"
{
    long                 id;

    if (LandStrCoordToId(landcoords, id))
        return GetLand(id);
    else
        return NULL;
}


//----------------------------------------------------------------------

void CAtlaParser::ComposeProductsLine(CLand * pLand, const char * eol, CStr & S)
{
    CStr       Line(64);

    if (pLand->current_state_.resources_.size() > 0)
    {
        S << "  Products:";
        bool first(true);
        for (const CItem& resource : pLand->current_state_.resources_)
        {
            if (first)
                first = false;
            else
                S << ",";
            
            if (resource.amount_>1)
            {
                std::string name, code, plural;
                gpApp->ResolveAliasItems(resource.code_name_, code, name, plural);
                S << " " << resource.amount_ << " " << plural.c_str() << " ["<< resource.code_name_.c_str() << "]";
            }
            else
            {
                std::string name, code, plural;
                gpApp->ResolveAliasItems(resource.code_name_, code, name, plural);
                S << " " << resource.amount_ << " " << name.c_str() << " ["<< resource.code_name_.c_str() << "]";
            }
        }
        S << "." << eol;
    }
}

void CAtlaParser::compose_products_detailed(CLand* land, std::stringstream& out)
{
    std::string code, name, plural;

    out << "  Products:";
    std::sort(land->current_state_.resources_.begin(), land->current_state_.resources_.end(), [](const CItem& it1, const CItem& it2) {
        return it1.amount_ > it2.amount_;
    });
    for (const auto& resource : land->current_state_.resources_)
    {
        if (!gpApp->ResolveAliasItems(resource.code_name_, code, name, plural))
        {
            code = resource.code_name_;
            name = resource.code_name_;
            plural = resource.code_name_;
        }
    
        if (resource.amount_ > 1)
            out << std::endl << "    " << resource.amount_ << " " << plural;
        else
            out << std::endl << "    " << resource.amount_ << " " << name;

        if (land->current_state_.produced_items_.find(code) != land->current_state_.produced_items_.end())
        {
            out << " (attempt: " << land->current_state_.produced_items_[code].second << ")";
        }
    }
    out << std::endl;

    std::map<std::string, std::pair<long, long>> crafted;
    for (const auto& production : land->current_state_.produced_items_)
    {
        if (std::find_if(land->current_state_.resources_.begin(),
                     land->current_state_.resources_.end(), [&](const CItem& item) {
                return production.first == item.code_name_;
            }) == land->current_state_.resources_.end())
        {
            crafted[production.first] = production.second;
            //out << std::endl << "    " << production.second << " " << production.first;
        }
    }

    if (crafted.size() > 0)
    {
        out << "  Crafts:";
        for (auto& craft : crafted)
        {
            std::string item_name = craft.first;
            long actual_production = craft.second.first;
            long attempt_production = craft.second.second;
            if (!gpApp->ResolveAliasItems(item_name, code, name, plural))
            {
                code = item_name;
                name = item_name;
                plural = item_name;
            }
            if (actual_production > 1)
                out << std::endl << "    " << actual_production << " " << plural << " (attempt: " << attempt_production << ")";
            else
                out << std::endl << "    " << actual_production << " " << name << " (attempt: " << attempt_production << ")";
        }
        out << std::endl;
    }

    if (land->current_state_.shared_items_.size() > 0)
    {
        out << "  Shared PRODUCTION_RESOURCE (left/total):";
        for (auto& shared_item : land->current_state_.shared_items_)
        {
            if (shared_item.second.second == 0)
                continue;

            if (!gpApp->ResolveAliasItems(shared_item.first, code, name, plural))
            {
                name = shared_item.first;
            }
            out << std::endl << "    " << shared_item.second.first << "/" << shared_item.second.second << " " << name;
        }            
    }
    out << std::endl;
}

//-------------------------------------------------------------

BOOL CAtlaParser::SaveOneHex(CFileWriter & Dest, CLand * pLand, CPlane * pPlane, SAVE_HEX_OPTIONS * pOptions)
{
    CLand            * pLandExit;
    int                i;

    CStr               sLine (128);
    CStr               sExits(128);
    const char       * p;
    BOOL               IsLinked;
    BOOL               TurnNoMarkerWritten = FALSE;
    int                nstr;
    CStruct          * pEdge;

    IsLinked = FALSE;
    sExits.Empty();
    for (i=0; i<6; i++)
    {
        pLandExit = GetLandExit(pLand, i);
        if (pLandExit)
        {
            CStr sCoord;
            if (pLandExit->Flags&LAND_VISITED)
            {
                IsLinked = TRUE;
            }
            //  Northeast : mountain (20,0) in Lautaro, contains Krod [city].
            ComposeLandStrCoord(pLandExit, sCoord);
            sExits << "  " << Directions[i] << " : "
                  << pLandExit->TerrainType << " (" << sCoord << ") in " << pLandExit->Name;
            if (!pLandExit->CityName.IsEmpty())
                sExits << ", contains " << pLandExit->CityName << " [" << pLandExit->CityType << "]";

            // save edge structs
            for (nstr = 0; nstr < pLand->EdgeStructs.Count(); nstr++)
            {
                pEdge = (CStruct*)pLand->EdgeStructs.At(nstr);
                if (pEdge->Location == i)
                    sExits << ", " << pEdge->type_.c_str();
            }
            sExits  << "." << EOL_FILE;;
        }
    }

    if ( (0==(pLand->Flags&LAND_VISITED)) && IsLinked)
        return FALSE;


    p = pLand->Description.GetData();
    while (p)
    {
        p = sLine.GetToken(p, '\n', TRIM_NONE);
        sLine.TrimRight(TRIM_ALL);

        if (pOptions->WriteTurnNo > 0 && !TurnNoMarkerWritten && sLine.GetLength() > 20)
        {
            BOOL         SkipIt = FALSE;
            const char * p;

            p = sLine.GetData();
            while (*p)
            {
                if ('-' != *p)
                {
                    SkipIt = TRUE;
                    break;
                }
                p++;
            }
            if (!SkipIt)
            {
                sLine << ";" << pOptions->WriteTurnNo;
                TurnNoMarkerWritten = TRUE;
            }
        }

        sLine << EOL_FILE;
        Dest.WriteBuf(sLine.GetData (), sLine.GetLength ());
    }

    sLine.Empty();

    if (pOptions->SaveResources)
        ComposeProductsLine(pLand, EOL_FILE, sLine);

    sLine << EOL_FILE << "Exits:" << EOL_FILE ;
    sLine << pLand->Exits;
    sLine.TrimRight(TRIM_ALL);
    sLine << EOL_FILE << EOL_FILE;

    Dest.WriteBuf(sLine.GetData (), sLine.GetLength());
    sLine.Empty();

    // Units out of structs, not optimized, does not matter
    if (pOptions->SaveUnits)
        for (CUnit* unit : pLand->units_seq_)
        {
            if (!IS_NEW_UNIT(unit) && unit_control::structure_id(unit) == 0)//!pUnit->GetProperty(PRP_STRUCT_ID, type, value, eOriginal) )
            {
                sLine << unit->Description;
                sLine.TrimRight(TRIM_ALL);
                sLine << EOL_FILE;
            }
        }

    // Gates and structs
    land_control::perform_on_each_struct(pLand, [&](CStruct* structure) {
        if (!struct_control::flags::is_ship(structure) && 
            (pOptions->AlwaysSaveImmobStructs || pOptions->SaveStructs || pOptions->SaveUnits))
        {
            sLine << EOL_FILE;
            sLine << structure->original_description_.c_str();
            sLine.TrimRight(TRIM_ALL);
            sLine << EOL_FILE;

            // Units inside structs, not optimized, does not matter
            if (pOptions->SaveUnits)
            {
                land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {
                    if (!IS_NEW_UNIT(unit) && 
                        unit_control::structure_id(unit) == structure->Id)
                    {
                        sLine << unit->Description;
                        sLine.TrimRight(TRIM_ALL);
                        sLine << EOL_FILE;
                    }
                });
            }
            Dest.WriteBuf(sLine.GetData(), sLine.GetLength());
            sLine.Empty();            
        }

    });

    sLine << EOL_FILE << EOL_FILE;
    Dest.WriteBuf(sLine.GetData (), sLine.GetLength());

    return TRUE;
}

//-------------------------------------------------------------

int  CAtlaParser::SaveOrders(const char * FNameOut, const char * password, BOOL decorate, int factid)
{
    int           i, n;
    CFileWriter   Dest;
    CStr          S(64), S1(64);
    int           err = ERR_OK;
    const char  * p;
    CLand       * pLand, DummyLand;
    CPlane      * pPlane;
    char          buf[64];
    struct tm     t;
    time_t        now;
    CFaction    * pFaction = NULL;

    if (Dest.Open(FNameOut))
    {
        pFaction =  GetFaction(factid);

        // write orders

        S.Empty();
        S << "#atlantis " << (long)factid << " \"" << password << '\"' << EOL_FILE << EOL_FILE;
        if (pFaction)
        {
            time(&now);
            t = *localtime(&now);
            i = m_YearMon%100-1;
            if ( (i < 12) && (i >=0) )
                sprintf(buf, "%s year %ld,  %s", Monthes[i], m_YearMon/100, asctime(&t));
            else
                sprintf(buf, "%02ld/%02ld,  %s", m_YearMon%100, m_YearMon/100, asctime(&t));
            S << ORDER_CMNT << pFaction->Name << " orders for " << buf  << EOL_FILE << EOL_FILE;
        }
        Dest.WriteBuf(S.GetData(), S.GetLength());

        std::map<long, wxString> FormOrders; // [unit number] = Form Orders

        // Loop through all our units, per region.
        for (n=0; n<m_Planes.Count(); n++)
        {
            pPlane = (CPlane*)m_Planes.At(n);
            for (i=0; i<pPlane->Lands.Count(); i++)
            {
                pLand = (CLand*)pPlane->Lands.At(i);

                // Store the new units orders, then append the orders to the creating unit
                for (CUnit* pUnit : pLand->units_seq_)
                {
                    if (pUnit->FactionId == factid && IS_NEW_UNIT(pUnit))
                    {
                        // Parse the number from the description
                        wxString s = wxString::FromUTF8(pUnit->Description.GetData());
                        long unitNumber;
                        if (s.AfterFirst('(').BeforeFirst(')').ToLong(&unitNumber))
                        {
                            std::map<long, wxString>::iterator mi = FormOrders.find(unitNumber);
                            wxString value;
                            if (mi != FormOrders.end())
                            {
                                value = mi->second;
                            }
                            value << wxString::FromUTF8(EOL_SCR)
                              << wxT("FORM ") << REVERSE_NEW_UNIT_ID(pUnit->Id) << wxString::FromUTF8(EOL_SCR)
                              << wxString::FromUTF8(pUnit->Orders.GetData());
                            value.Trim();
                            value << wxString::FromUTF8(EOL_SCR) << wxT("END") << wxString::FromUTF8(EOL_SCR);
                            FormOrders[unitNumber] = value;
                        }
                        else
                        {
                            wxString landDescr = wxString::FromUTF8(pLand->Description.GetData());
                            wxString s = wxString::Format(wxT("Unable to save new unit orders in %s."), landDescr.BeforeFirst('.').c_str());
                            OrderError("Warning", pLand, pUnit, nullptr, s.ToStdString());
                        }
                    }
                }

                bool LandDecorationShown = false;
                for (CUnit* pUnit : pLand->units_seq_)
                {
                    if (pUnit->FactionId == factid && !IS_NEW_UNIT(pUnit))
                    {
                        if (decorate && !LandDecorationShown)
                        {
                            S1.GetToken(pLand->Description.GetData(), '\n');
                            S1.TrimRight(TRIM_ALL);
                            if (S1.FindSubStr(")") > 0)
                            {
                                S1.DelSubStr(S1.FindSubStr(")")+1, 999);
                            }
                            S.Empty();
                            S << EOL_FILE << ORDER_CMNT << S1 << EOL_FILE;
                            Dest.WriteBuf(S.GetData(), S.GetLength());
                            LandDecorationShown = true;
                        }
                        // unit orders
                        S.Empty();
                        S << EOL_FILE << "unit " << pUnit->Id << " ; " << pUnit->Name << EOL_FILE;
                        Dest.WriteBuf(S.GetData(), S.GetLength());

                        p = pUnit->Orders.GetData();
                        while (p && *p)
                        {
                            p = S.GetToken(p, '\n', TRIM_NONE);
                            S.TrimRight(TRIM_ALL);
                            S << EOL_FILE;
                            Dest.WriteBuf(S.GetData(), S.GetLength());
                        }
                        std::map<long, wxString>::iterator mi = FormOrders.find(pUnit->Id);
                        if (mi != FormOrders.end())
                        {
                            p = mi->second.ToUTF8();
                            while (p && *p)
                            {
                                p = S.GetToken(p, '\n', TRIM_NONE);
                                S.TrimRight(TRIM_ALL);
                                S << EOL_FILE;
                                Dest.WriteBuf(S.GetData(), S.GetLength());
                            }
                            FormOrders.erase(mi);
                        }
                    }
                }
                if (!FormOrders.empty())
                {
                    wxString landDescr = wxString::FromUTF8(pLand->Description.GetData());
                    wxString s = wxString::Format(wxT("Unable to save new unit orders in %s."), landDescr.BeforeFirst('.').c_str());
                    OrderError("Warning", pLand, 0, nullptr, s.ToStdString());
                    FormOrders.clear();
                }
            }
        }
        OrderErrFinalize();
        S.Empty();
        S << EOL_FILE << "#end" << EOL_FILE;
        Dest.WriteBuf(S.GetData(), S.GetLength());
        Dest.Close();
    }
    else {
        OrderError("Error", nullptr, nullptr, nullptr, "Couldn't open file to save order");
        err = ERR_FOPEN;
    }
    return err;
}

//-------------------------------------------------------------

// count number of men for the specified faction in every hex.
// store as a property

void CAtlaParser::CountMenForTheFaction(int FactionId)
{
    int           nPlane;
    int           nLand;
    CPlane      * pPlane;
    CLand       * pLand;
    long          nMen, x;
    EValueType    type;

    for (nPlane=0; nPlane<m_Planes.Count(); nPlane++)
    {
        pPlane = (CPlane*)m_Planes.At(nPlane);
        for (nLand=0; nLand<pPlane->Lands.Count(); nLand++)
        {
            pLand = (CLand*)pPlane->Lands.At(nLand);
            if (!pLand->GetProperty(PRP_SEL_FACT_MEN, type, (const void *&)nMen, eNormal) || (eLong!=type))
                nMen=0;

            for (CUnit* unit : pLand->units_seq_)
            {
                if (FactionId == unit->FactionId)
                {
                    if (unit->GetProperty(PRP_MEN, type, (const void *&)x, eNormal) && (eLong==type))
                        nMen+=x;
                }
            }
            pLand->SetProperty(PRP_SEL_FACT_MEN, eLong, (void*)nMen, eBoth);
        }
    }

}

//-------------------------------------------------------------

CUnit * CAtlaParser::SplitUnit(CUnit * unit, long newId)
{
    long x, y, z;
    land_control::get_land_coordinates(unit->LandId, x, y, z);
    long new_unit_id = NEW_UNIT_ID(x, y, z, unit->FactionId, newId);

    CLand* land = GetLand(unit->LandId);
    wxASSERT(land);

    CUnit* new_unit = land_control::find_first_unit_if(land, [&](CUnit* unit) {  return unit->Id == new_unit_id;  });
    if (new_unit == nullptr)
    {
        CStr            propname;
        EValueType      type;
        const void    * value;

        new_unit = new CUnit;
        new_unit->Id = new_unit_id;
        new_unit->FactionId = unit->FactionId;
        new_unit->pFaction = unit->pFaction;
        new_unit->Flags = unit->Flags;
        new_unit->FlagsOrg = unit->FlagsOrg;
        new_unit->IsOurs = unit->IsOurs;
        new_unit->Name << "NEW " << newId;
        new_unit->Description << "Created by " << unit->Name << " (" << unit->Id << ")";

        // Copy persistent settings from the creating unit
        if (unit->GetProperty(PRP_FRIEND_OR_FOE, type, value, eNormal) && eLong==type)
            SetUnitProperty(new_unit,PRP_FRIEND_OR_FOE, type, value, eNormal);
        
        unit_control::set_structure(new_unit, unit_control::structure_id(unit), false);
        if (unit->GetProperty(PRP_STRUCT_ID, type, value, eOriginal) && eLong==type)
            SetUnitProperty(new_unit,PRP_STRUCT_ID, type, value, eBoth);
        if (unit->GetProperty(PRP_STRUCT_NAME, type, value, eOriginal) && eCharPtr==type)
            SetUnitProperty(new_unit,PRP_STRUCT_NAME, type, value, eBoth);
        // add new unit to the land of pOrigUnit
        land->AddUnit(new_unit);
    }
    return new_unit;
}

//-------------------------------------------------------------

int CAtlaParser::LoadOrders  (CFileReader & F, int FactionId, BOOL GetComments)
{
    CStr          Line(64), S(64), No(32);
    const char  * p;
    CPlane      * pPlane;
    CLand       * pLand;
    CUnit       * pOrigUnit;
    CUnit         Dummy;
    char          ch;

    m_CrntFactionPwd.Empty();
    for (int nPlane=0; nPlane<m_Planes.Count(); nPlane++)
    {
        pPlane = (CPlane*)m_Planes.At(nPlane);
        for (int nLand=0; nLand<pPlane->Lands.Count(); nLand++)
        {
            pLand = (CLand*)pPlane->Lands.At(nLand);
            pLand->DeleteAllNewUnits(FactionId);
            for (CUnit* unit : pLand->units_seq_) {
                if (FactionId == unit->FactionId)
                    unit->Orders.Empty();
            }
        }
    }

    CUnit* pUnit = NULL;
    pOrigUnit = NULL;

    while (F.GetNextLine(Line))
    {
        Line.TrimRight(TRIM_ALL);
        p = SkipSpaces(Line.GetData());

        if (0==SafeCmp(p, "#end"))  // that is the end of report
            break;
        if (!p || !*p || (';'==*p && ('*'==*(p+1) || !GetComments)) )
            continue;

        p = S.GetToken(p, " \t", ch, TRIM_ALL);
        if (0==stricmp(S.GetData(),"unit"))
        {
            p = No.GetToken(p, " \t", ch);
            long unit_id = atol(No.GetData());
            pUnit = this->global_find_unit(unit_id);
        }
        else if (0==stricmp(S.GetData(),"form") && !pOrigUnit)
        {
            if (pUnit == nullptr) {
                //can't parse form unit order from non-existing unit
                continue;
            }

            p = No.GetToken(p, " \t", ch);
            pOrigUnit = pUnit;
            if (pUnit == NULL)
            {
                pOrigUnit = pUnit;
            }

            pUnit = SplitUnit(pOrigUnit, atol(No.GetData()));
        }
        else if (0==stricmp(S.GetData(),"end") && pOrigUnit)
        {
            pUnit = pOrigUnit;
            pOrigUnit = NULL;
        }
        else
            if (0==stricmp(S.GetData(),"#atlantis"))
            {
                // #atlantis NN "password"
                m_CrntFactionPwd = S.GetToken(p, " \t", ch, TRIM_ALL);
                if (!m_CrntFactionPwd.IsEmpty() && '\"'==m_CrntFactionPwd.GetData()[0])
                {
                    m_CrntFactionPwd.DelCh(0);
                    if (!m_CrntFactionPwd.IsEmpty() && '\"'==m_CrntFactionPwd.GetData()[m_CrntFactionPwd.GetLength()-1])
                        m_CrntFactionPwd.DelCh(m_CrntFactionPwd.GetLength()-1);
                }
            }
            else
                if (pUnit)
                {
                    orders::control::add_order_to_unit(std::string(Line.GetData(), Line.GetLength()), pUnit);
                }
    }

    RunOrders(NULL);
    m_OrdersLoaded = TRUE;

    return ERR_OK;
}

//-------------------------------------------------------------

int  CAtlaParser::LoadOrders  (const char * FNameIn, int & FactionId)
{
    CFileReader   F;
    CStr          Line(64), S(64), No(32);
    CUnit       * pUnit;
    CUnit         Dummy;
    const char  * p;
    char          ch;

    if (FNameIn && *FNameIn && F.Open(FNameIn))
    {
        // find out what is the faction id
        FactionId = 0;
        while (F.GetNextLine(Line))
        {
            Line.TrimRight(TRIM_ALL);
            p = SkipSpaces(Line.GetData());

            if (0==SafeCmp(p, "#end"))  // that is the end of report
                break;
            if (!p || !*p || ';'==*p || '#'==*p)
                continue;

            p = S.GetToken(p, " \t", ch, TRIM_ALL);
            if (0==stricmp(S.GetData(),"unit"))
            {
                p = No.GetToken(p, " \t", ch);
                long unit_id = atol(No.GetData());
                pUnit = this->global_find_unit(unit_id);
                if (pUnit != nullptr && pUnit->FactionId > 0)
                {
                        FactionId = pUnit->FactionId; // just take the first unit
                        break;
                }
            }
        }

        F.Close();
        F.Open(FNameIn);
        LoadOrders(F, FactionId, TRUE);
        F.Close();
        return ERR_OK;
    }
    return ERR_FOPEN;
}

//-------------------------------------------------------------

void CAtlaParser::OrderErrFinalize()
{
    if (gpDataHelper && !m_sOrderErrors.IsEmpty())
        gpDataHelper->ReportError(m_sOrderErrors.GetData(), m_sOrderErrors.GetLength(), TRUE);

    m_sOrderErrors.Empty();
}


void CAtlaParser::OrderError(const std::string& type, CLand* land, CUnit* unit, 
                             const std::shared_ptr<orders::Order>& order, const std::string& Msg)
{
    CStr         S(32);
    CStr         prefix;

    if (order != nullptr && orders::control::should_supress_error(order))
        return;

    std::string land_name;
    if (land != nullptr)
    {
        CStr         land_name_cstr;
        ComposeLandStrCoord(land, land_name_cstr);
        land_name = "("+std::string(land_name_cstr.GetData(), land_name_cstr.GetLength()) + ") ";
        land->current_state_.run_orders_errors_.push_back({type, unit, Msg});
    }

    std::string unit_name;
    if (unit != nullptr)
    {
        if (IS_NEW_UNIT(unit))
            unit_name = land_name + " " + unit_control::compose_unit_name(unit);
        else
            unit_name = unit_control::compose_unit_name(unit);

        unit->impact_description_.push_back(Msg);
        unit->Flags |= UNIT_FLAG_HAS_ERROR;
        unit->has_error_ = true;
    }
    
    S.Format("%s%s %s : %s%s", land_name.c_str(), unit_name.c_str(), type.c_str(), Msg.c_str(), EOL_SCR);
    m_sOrderErrors << S;
}

//-------------------------------------------------------------

void CAtlaParser::GenericErr(int Severity, const char * Msg)
{
    const char * type;
    CStr         S(32);

    if (!gpDataHelper)
        return;

    if (0==Severity)
        type = "Error  ";
    else
        type = "Warning";
    S.Format("%s : %s%s", type, Msg, EOL_SCR);

    gpDataHelper->ReportError(S.GetData(), S.GetLength(), FALSE);
}

//-------------------------------------------------------------

#define GEN_ERR(pUnit, msg)                 \
{                                           \
    Line << msg;                            \
    OrderError("Error", pLand, pUnit, nullptr, Line.GetData()); \
    return Changed;                         \
}


BOOL CAtlaParser::ShareSilver(CUnit * pMainUnit)
{
    CLand             * pLand = NULL;
    long                unitmoney;
    long                mainmoney;
    long                n;
    EValueType          type;
    CStr                Line(32);
    BOOL                Changed = FALSE;

    do
    {
        if (!pMainUnit || IS_NEW_UNIT(pMainUnit) || !unit_control::of_player(pMainUnit))
            break;

        pLand = GetLand(pMainUnit->LandId);
        if (!pLand)
            break;

        RunLandOrders(pLand); // just in case...

        if (!pMainUnit->GetProperty(PRP_SILVER, type, (const void *&)mainmoney, eNormal) )
        {
            mainmoney = 0;
            type      = eLong;
            if (PE_OK!=pMainUnit->SetProperty(PRP_SILVER, type, (const void*)mainmoney, eBoth))
                GEN_ERR(pMainUnit, NOSETUNIT << pMainUnit->Id << BUG);
        }
        else if (eLong!=type)
            GEN_ERR(pMainUnit, NOTNUMERIC << pMainUnit->Id << BUG);

        mainmoney = unit_control::get_item_amount(pMainUnit, PRP_SILVER);

        if (mainmoney<=0)
            break;


        for (CUnit* pUnit : pLand->units_seq_)
        {
            if (!unit_control::of_player(pUnit))
                continue;

            if (!pUnit->GetProperty(PRP_SILVER, type, (const void *&)unitmoney, eNormal) )
                continue;
            else if (eLong!=type)
                GEN_ERR(pUnit, NOTNUMERIC << pUnit->Id << BUG);

            if (unitmoney>=0)
                continue;

            n = -unitmoney;
            if (n>mainmoney)
            {
                n         = mainmoney;
                mainmoney = 0;
            }
            else
                mainmoney -= n;

            pMainUnit->Orders.TrimRight(TRIM_ALL);
            if (!pMainUnit->Orders.IsEmpty())
                pMainUnit->Orders << EOL_SCR ;
            if (IS_NEW_UNIT(pUnit))
                pMainUnit->Orders << "GIVE NEW " << (long)REVERSE_NEW_UNIT_ID(pUnit->Id) << " " << n << " SILV";
            else
                pMainUnit->Orders << "GIVE " << pUnit->Id   << " " << n << " SILV";
            Changed = TRUE;
            if (mainmoney<=0)
                break;
        }
        if (Changed)
            RunLandOrders(pLand);
    } while (FALSE);

    OrderErrFinalize();
    return Changed;
}

//-------------------------------------------------------------

BOOL CAtlaParser::GenGiveEverything(CLand* land, CUnit * pFrom, const char * To)
{
    CLand             * pLand = NULL;
    int                 no;
    BOOL                Changed = FALSE;
    const char        * propname;
    int                 i;
    BOOL                skipit;
    EValueType          type;
    long                amount;
    long                amountorg;
    CUnit               Dummy;
    CStr                S;
    const char        * p;
    long                n1;

    do
    {
        if (!pFrom || IS_NEW_UNIT(pFrom) || !unit_control::of_player(pFrom) || !To || !*To )
            break;

        pLand = GetLand(pFrom->LandId);
        if (!pLand)
            break;

        p  = To;
        long x, y, z;
        land_control::get_land_coordinates(land->Id, x, y, z);
        if (!GetTargetUnitId(x, y, z, p, pFrom->FactionId, n1))
        {
            S << "Invalid unit id " << To;
            OrderError("Error", pLand, pFrom, nullptr, S.GetData());
            break;
        }

        CUnit* target_unit = land_control::find_first_unit_if(pLand, [&](CUnit* unit) {  return unit->Id == n1;  });
        if (n1 != 0 && target_unit == nullptr)
        {
            S << "Can not find unit " << To;
            OrderError("Error", pLand, pFrom, nullptr, S.GetData());
            break;
        }

        RunLandOrders(pLand); // just in case...

        pFrom->Orders.TrimRight(TRIM_ALL);
        if (!pFrom->Orders.IsEmpty())
            pFrom->Orders << EOL_SCR ;

        no = 0;
        do
        {
            propname = pFrom->GetPropertyName(no++);
            skipit   = FALSE;

            if (!propname)
                break;

            // ignore standard properties
            for (i=0; i<STD_UNIT_PROPS_COUNT; i++)
                if (0==stricmp(propname, STD_UNIT_PROPS[i]))
                {
                    skipit = TRUE;
                    break;
                }
            if (skipit)
                continue;

            // ignore skills and such
//            for (i=0; i<STD_UNIT_POSTFIXES_COUNT; i++)
//            {
//                p       = propname;
//                while (p)
//                {
//                    p = strstr(p, STD_UNIT_POSTFIXES[i]);
//                    if (p)
//                    {
//                        p += strlen(STD_UNIT_POSTFIXES[i]);
//                        if (!*p)
//                        {
//                            skipit = TRUE;
//                            break;
//                        }
//                    }
//                }
//                if (skipit)
//                    break;
//            }
//            if (skipit)
            if (IsASkillRelatedProperty(propname))
                continue;


            // ignore strings
            if (!pFrom->GetProperty(propname, type, (const void *&)amount, eNormal) || eLong!=type)
                continue;
            // ignore strings
            if (!pFrom->GetProperty(propname, type, (const void *&)amountorg, eOriginal) || eLong!=type)
                continue;
            x = std::min(amount, amountorg);
            if (x<=0)
                continue;

            pFrom->Orders << "GIVE " << To << " " << x << " " << propname << EOL_SCR;
            Changed = TRUE;

        } while (propname);

        if (Changed)
            RunLandOrders(pLand);
    }
    while (FALSE);

    OrderErrFinalize();
    return Changed;
}

//-------------------------------------------------------------

BOOL CAtlaParser::GenOrdersTeach(CUnit * pMainUnit)
{
    CStr                Line(32);

    if (!pMainUnit || !unit_control::of_local(pMainUnit))
        return FALSE;

    CLand* pLand = nullptr;
    if (pMainUnit->movements_.size() > 0)
        pLand = GetLand(pMainUnit->movement_stop_);
    else
        pLand = GetLand(pMainUnit->LandId);

    if (!pLand)
        return FALSE;

    static long peasant_can_teach = game_control::get_game_config_val<long>(SZ_SECT_COMMON, "PEASANT_CAN_TEACH");

    if (peasant_can_teach == 0 && !unit_control::is_leader(pMainUnit))
        return FALSE;

    //recalculate state to previous to TEACH action, so all RunOrders teach & study modification
    //wouldn't affect the process, but all give & buy would.
    RunLandOrders(pLand, TurnSequence::SQ_FIRST, TurnSequence::SQ_MOVE);

    //get teaching days map
    std::vector<unit_control::UnitError> errors;
    std::unordered_map<long, land_control::Student> students = land_control::get_land_students(pLand, errors);
    land_control::update_students_by_land_teachers(pLand, students, errors);

    for (auto& error : errors)
    {
        OrderError(error.type_, pLand, error.unit_, error.order_, error.message_);
    }

    BOOL changed = FALSE;
    for (const auto& stud : students)
    {
        if (IS_NEW_UNIT_ID(stud.first) && stud.second.unit_->LandId != pLand->Id)
            continue; //can't teach new created units from other regions

        //student have room for teachers
        if ((stud.second.days_of_teaching_ < 30) && //there are days of teaching, and there is a room for it
            (stud.second.max_days_ - stud.second.cur_days_ - stud.second.days_of_teaching_ - (long)30 > 0))
        {
            std::string skill;
            long study_lvl_goal;
            if (!orders::parser::specific::parse_study(stud.second.order_, skill, study_lvl_goal))
                continue;//TODO warning?

            long teacher_skill_days = unit_control::get_current_skill_days(pMainUnit, skill);
            long teacher_skill_lvl = skills_control::get_skill_lvl_from_days(teacher_skill_days);
            
            long student_skill_lvl = this->SkillDaysToLevel(stud.second.cur_days_);
            if (study_lvl_goal != -1 && study_lvl_goal <= student_skill_lvl)
                continue;

            if (teacher_skill_lvl > student_skill_lvl)
            {
                pMainUnit->Orders.TrimRight(TRIM_ALL);
                if (!pMainUnit->Orders.IsEmpty())
                    pMainUnit->Orders << EOL_SCR ;

                size_t indent = sizeof("TEACH NEW XXXXX");
                std::string order;
                if (IS_NEW_UNIT(stud.second.unit_))
                {
                    order = "TEACH NEW ";
                    order.append(std::to_string((long)REVERSE_NEW_UNIT_ID(stud.second.unit_->Id)));
                }
                else
                {
                    order = "TEACH ";
                    order.append(std::to_string(stud.second.unit_->Id));
                }
                pMainUnit->Orders << order.c_str();
                for (size_t i = order.size(); i < indent; ++i)
                    pMainUnit->Orders << " ";
                pMainUnit->Orders << ";";
                if (pMainUnit->LandId != pLand->Id) 
                {
                    long x,y,z;
                    land_control::get_land_coordinates(pLand->Id, x, y, z);
                    if (z > 1)
                        pMainUnit->Orders << "[" << x << "," << y << "," << z << "] ";
                    else
                        pMainUnit->Orders << "[" << x << "," << y << "] ";
                }
                pMainUnit->Orders << skill.c_str() << " " << stud.second.cur_days_ << "(" << stud.second.max_days_ << ")";
                if (unit_control::is_leader(stud.second.unit_))
                    pMainUnit->Orders << " lead: " << stud.second.man_amount_;
                else
                    pMainUnit->Orders << " man: " << stud.second.man_amount_;
                changed = TRUE;
            }
        }
    }

    RunLandOrders(pLand); // just in case...
    return changed;
}

//-------------------------------------------------------------

BOOL CAtlaParser::DiscardJunkItems(CUnit * pUnit, const char * junk)
{
    CLand             * pLand = NULL;
    EValueType          type;
    long                value;
    CStr                sJunkItem(32);
    BOOL                Changed = FALSE;

    if (!pUnit || IS_NEW_UNIT(pUnit) || !unit_control::of_player(pUnit))
        return FALSE;

    pLand = GetLand(pUnit->LandId);
    if (!pLand)
        return FALSE;

    RunLandOrders(pLand); // just in case...

    while (junk && *junk)
    {
        junk = sJunkItem.GetToken(junk, ',');
        if (!pUnit->GetProperty(sJunkItem.GetData(), type, (const void *&)value, eNormal) || (eLong!=type))
            continue;

        pUnit->Orders.TrimRight(TRIM_ALL);
        if (!pUnit->Orders.IsEmpty())
            pUnit->Orders << EOL_SCR ;
        pUnit->Orders << "GIVE 0 " << value << " " << sJunkItem;
        Changed = TRUE;
    }
    if (Changed)
        RunLandOrders(pLand);
    return Changed;
}


//-------------------------------------------------------------

const char * CAtlaParser::ReadPropertyName(const char * src, CStr & Name)
{
    char ch;
    int  i;
    CStr S;

    Name.Empty();
    src = SkipSpaces(S.GetToken(src, " \t;\n", ch, TRIM_ALL)); // SH-EXCPT

    if (!S.IsEmpty())
    {
//        if ('"'==S.GetData()[0])
//        {
//            if ('"'==S.GetData()[S.GetLength()-1])
//            {
                // remove quotes, replace spaces with underscores

//                S.DelCh(S.GetLength()-1);
//                S.DelCh(0);
                for (i=0; i<S.GetLength(); i++)
                    if (' '==S.GetData()[i])
                        S.SetCh(i, '_');
//            }
//            else
//                return src;
//        }
        Name = gpDataHelper->ResolveAlias(S.GetData());
    }

    return src;
}

//-------------------------------------------------------------

BOOL CAtlaParser::GetTargetUnitId(long x, long y, long z, const char *& p, long FactionId, long & nId)
{
    CStr                N1(32), N(32), X, Y;
    char                ch;

    p = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));
    if (0==stricmp("FACTION", N1.GetData()))
    {
        // FACTION X NEW Y
        p = SkipSpaces(X.GetToken(p, " \t", ch, TRIM_ALL));  // X
        if (!X.IsInteger())
            return FALSE;
        p = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));  // NEW
        if (0!=stricmp(N1.GetData(), "NEW"))
            return FALSE;
        p = SkipSpaces(Y.GetToken(p, " \t", ch, TRIM_ALL));  // Y
        if (!Y.IsInteger())
            return FALSE;

        if ( atol(gpDataHelper->GetConfString(SZ_SECT_COMMON, SZ_KEY_CHECK_NEW_UNIT_FACTION)))
            nId = NEW_UNIT_ID(x, y, z, atol(X.GetData()), atol(Y.GetData()));
        else
            nId = 0;
        return TRUE;
    }
    if (0==stricmp("NEW", N1.GetData()))
    {
        p = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));
        if (!N1.IsInteger())
            return FALSE;
        nId = NEW_UNIT_ID(x, y, z, FactionId, atol(N1.GetData()));
        return TRUE;
    }

    if (!N1.IsInteger())
        return FALSE;
    nId = atol(N1.GetData());
    return TRUE;
}

//-------------------------------------------------------------

#define SHOW_WARN_CONTINUE(msg)                      \
{                                                    \
    if (!skiperror)                                  \
    {                                                \
        ErrorLine.Empty();                           \
        ErrorLine << Line << msg;                    \
        OrderError("Warning", pLand, pUnit, nullptr, ErrorLine.GetData()); \
    }                                                \
    continue;                                        \
}

#define SHOW_WARN(msg)                               \
{                                                    \
    if (!skiperror)                                  \
    {                                                \
        ErrorLine.Empty();                           \
        ErrorLine << Line << msg;                    \
        OrderError("Warning", pLand, pUnit, nullptr, ErrorLine.GetData()); \
    }                                                \
}

#define SHOW_WARN_BREAK(msg)                         \
{                                                    \
    if (!skiperror)                                  \
    {                                                \
        ErrorLine.Empty();                           \
        ErrorLine << Line << msg;                    \
        OrderError("Warning", pLand, pUnit, nullptr, ErrorLine.GetData()); \
        break;                                       \
    }                                                \
}

void CAtlaParser::RunLandOrders(CLand * pLand, TurnSequence beg_step, TurnSequence stop_step)
{
    CBaseObject         Dummy;
    CStr                Line(32);
    CStr                Cmd (32);
    CStr                S   (32);
    CStr                S1  (32);
    CStr                N1  (32);
    CStr                ErrorLine(32);


    // Run Orders

    //auto start = std::chrono::high_resolution_clock::now();
    //std::map<long, std::chrono::system_clock::time_point> time_points;

    for (TurnSequence sequence = beg_step; sequence <= stop_step; ++sequence)
    {
        if (sequence == TurnSequence::SQ_FIRST)
        {
            // Reset land and old units and remove new units
            pLand->ResetNormalProperties();
            pLand->ResetUnitsAndStructs();

            //TODO: Arkady
            //for each incoming unit we need to recalculate their region up to moving phase,
            //but not recalculate their incoming units
            //gpApp->m_pAtlantis->RunLandOrders()

            // AutoOrders initialization & sanity check section
            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {
                //caravan sanity check
                if (unit_control::of_player(unit) && unit_control::init_caravan(unit))
                {
                    for (size_t i = 0; i < unit->caravan_info_->regions_.size(); ++i)
                    {
                        orders::RegionInfo& reg = unit->caravan_info_->regions_[i];
                        CLand* region_land = GetLand(reg.x_, reg.y_, reg.z_, TRUE);
                        if (region_land == nullptr)
                        {
                            std::stringstream ss;
                            ss << "("<< reg.x_ << ", " << reg.y_ << ", " << reg.z_ << ")";
                            unit_control::order_message(unit, "Didn't find region", ss.str().c_str());
                        }
                        else if (region_land == pLand)//we can deduct destination land, overwrite parsing
                        {
                            size_t next_id = (i+1) % unit->caravan_info_->regions_.size();
                            orders::RegionInfo& destination_reg = unit->caravan_info_->regions_[next_id];
                            unit->caravan_info_->goal_land_ = GetLand(destination_reg.x_, destination_reg.y_, destination_reg.z_, TRUE);
                        }
                    }
                    if (unit->caravan_info_->speed_ == orders::CaravanSpeed::UNDEFINED)
                    {
                        unit_control::order_message(unit, "Caravan speed is undefined", "(unlimited load - Storage Mode)");
                    }
                }

                //initial amount of silver
                if (unit_control::of_player(unit))
                    pLand->current_state_.economy_.initial_amount_ += unit_control::get_item_amount(unit, PRP_SILVER);

                std::vector<std::shared_ptr<orders::Order>> errors = orders::control::retrieve_orders_by_type(orders::Type::O_ERROR, unit->orders_);
                for (auto& order : errors)
                {
                    OrderError("error", pLand, unit, order, order->comment_ + ": " + order->original_string_);
                }
            });
            std::list<CUnit*> no_monthlong_orders;
            CheckOrder_LandMonthlong(pLand, no_monthlong_orders);
            //Automatic Orders Generation place during run of the orders
            RunOrder_AOComments<orders::Type::O_COMMENT>(pLand);//
            //RunCaravanAutomoveOrderGeneration(pLand); //generate caravan automatic move orders
            //CheckOrder_LandMonthlong(pLand);
        }
        else if (sequence == TurnSequence::SQ_FORM)
        {            
        }
        else if (sequence == TurnSequence::SQ_CLAIM) //all the flags, name and etc
        {
            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {    

                auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_CLAIM, unit->orders_);
                for (const auto& order : orders)
                {
                    long amount;
                    if (orders::parser::specific::parse_claim(order, amount))
                    {
                        unit_control::modify_silver(unit, amount, "claiming");
                        if (unit_control::of_player(unit))
                            pLand->current_state_.economy_.claim_income_ += amount;
                    }                            
                    else
                        OrderError("Error", pLand, unit, order, "claim: couldn't parse order: "+order->original_string_);
                }
            });
            
            //All Flags
            RunOrder_LandFlags(pLand);

            //Name & Describe
            RunOrder_LandNameDescribe(pLand);

        }
        else if (sequence == TurnSequence::SQ_LEAVE)
        {
            //NEW OWNER DETERMINITION
            //because we perform on each unit sequentially according to their appearance in the report
            //we can rely on fact that if the leaving unit is an owner, then the rest unit of the same
            //structure definitely located AFTER, so we can just search for other units of the same structure
            //and first will be the new owner
            std::set<long> empty_structures;
            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {    
                if (orders::control::has_orders_with_type(orders::Type::O_LEAVE, unit->orders_) ||
                    orders::control::has_orders_with_type(orders::Type::O_ENTER, unit->orders_))
                {
                    //looking for the last order of LEAVE or ENTER: all the rest are meaningless
                    std::shared_ptr<orders::Order> last_order(nullptr);
                    for (auto it = unit->orders_.orders_.rbegin(); it != unit->orders_.orders_.rend(); ++it) {
                        if ((*it)->type_ == orders::Type::O_LEAVE || (*it)->type_ == orders::Type::O_ENTER) {
                            last_order = *it;
                            break;
                        }   
                    }
                    if (last_order == nullptr)
                        return;

                    long struct_id = unit_control::structure_id(unit);
                    if (struct_id > 0) {
                        //in both cases we leave current structure
                        CStruct* structure  = land_control::get_struct(pLand, struct_id);
                        if (structure && structure->OwnerUnitId == unit->Id) {
                            structure->OwnerUnitId = 0;
                            empty_structures.insert(structure->Id);
                        }

                        //properly leave previous structure for LEAVE and ENTER case
                        unit_control::set_structure(unit, 0, false);

                        if ( (PE_OK!=unit->SetProperty(PRP_STRUCT_NAME,  eCharPtr, "", eNormal)) ||
                            (PE_OK!=unit->SetProperty(PRP_STRUCT_OWNER, eCharPtr, "", eNormal)) ||
                            (PE_OK!=unit->SetProperty(PRP_STRUCT_ID,    eLong,  0, eNormal)) )
                                OrderError("Error", pLand, unit, last_order, "enter or leave: couldn't set property to unit by order: "+last_order->original_string_);
                    }

                    if (last_order->type_ == orders::Type::O_ENTER) {
                        if (orders::parser::specific::parse_enter(last_order, struct_id)) {
                            CStruct* structure = land_control::get_struct(pLand, struct_id);
                            if (structure != nullptr) {
                                std::string yes_string;
                                if (structure->OwnerUnitId == 0)
                                {
                                    structure->OwnerUnitId = unit->Id;
                                    empty_structures.erase(struct_id); //just in case
                                    yes_string = YES;
                                }

                                if ((PE_OK!=unit->SetProperty(PRP_STRUCT_ID,  eLong, (void*)structure->Id, eNormal)) ||
                                    (PE_OK!=unit->SetProperty(PRP_STRUCT_NAME,  eCharPtr, structure->name_.c_str(), eNormal)) ||
                                    (PE_OK!=unit->SetProperty(PRP_STRUCT_OWNER, eCharPtr, yes_string.c_str(), eNormal)) )
                                    OrderError("Error", pLand, unit, last_order, "enter: couldn't set property to unit by order: "+last_order->original_string_);

                            }
                            else
                                OrderError("Error", pLand, unit, last_order, "enter: invalid structure number: "+last_order->original_string_); 
                        }
                        else 
                            OrderError("Error", pLand, unit, last_order, "enter: couldn't parse order: "+last_order->original_string_);
                    }
                }
                else if (!empty_structures.empty()) {//region has new empty structures -> check potential owners
                    long struct_id = unit_control::structure_id(unit);
                    if (struct_id > 0 && empty_structures.find(struct_id) != empty_structures.end()) {
                        //it is in the structure which lost an owner -> new one founded!
                        CStruct* structure  = land_control::get_struct(pLand, struct_id);
                        if (structure && structure->OwnerUnitId == 0) {
                            structure->OwnerUnitId = unit->Id;
                        }
                        empty_structures.erase(struct_id);
                    }
                }
            });
        }
        else if (sequence == TurnSequence::SQ_ENTER)
        {//is implemented together with LEAVE, because it includes LEAVE
        }
        else if (sequence == TurnSequence::SQ_PROMOTE)
        {
            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {   
                auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_PROMOTE, unit->orders_);
                if (orders.empty())
                    return;

                long struct_id = unit_control::structure_id(unit);
                if (struct_id == 0) {
                    OrderError("Error", pLand, unit, orders[0], "promote: unit is not in a structure: "+ orders[0]->original_string_);
                    return;
                }

                CStruct* structure  = land_control::get_struct(pLand, struct_id);
                if (structure == nullptr) {
                    OrderError("Error", pLand, unit, orders[0], "promote: unit's structure doesn't exist... "+ orders[0]->original_string_);
                    return;
                }

                if (structure->OwnerUnitId != unit->Id) {
                    OrderError("Error", pLand, unit, orders[0], "promote: unit is not an owner: "+ orders[0]->original_string_);
                    return;
                }

                long target_id(0);
                if (orders::parser::specific::parse_promote(orders[0], target_id)) {
                    CUnit* target_unit = land_control::find_first_unit_if(pLand, [&](CUnit* tunit) {
                        return tunit->Id == target_id;
                    });

                    if (target_unit == nullptr) {
                        OrderError("Error", pLand, unit, orders[0], "promote: can't locate target unit: "+ orders[0]->original_string_);
                        return;
                    }

                    if (unit_control::structure_id(target_unit) != struct_id) {
                        OrderError("Error", pLand, unit, orders[0], "promote: target unit is not in the structure: "+ orders[0]->original_string_);
                        return;
                    }

                    unit_control::set_structure(unit, struct_id, false);
                    unit_control::set_structure(target_unit, struct_id, true);
                    unit->SetProperty(PRP_STRUCT_OWNER, eCharPtr, "",  eNormal);
                    target_unit->SetProperty(PRP_STRUCT_OWNER, eCharPtr, "",  eNormal);
                    target_unit->SetProperty(PRP_STRUCT_OWNER, eCharPtr, YES, eNormal);
                    
                    structure->OwnerUnitId = target_unit->Id;
                }
                else
                    OrderError("Error", pLand, unit, orders[0], "promote: couldn't parse order: "+orders[0]->original_string_);


            });

        }
        else if (sequence == TurnSequence::SQ_STEAL) //for SQ_ATTACK, SQ_ASSASSINATE
        {
            RunOrder_AOComments<orders::Type::O_STEAL>(pLand);
            RunOrder_AOComments<orders::Type::O_ATTACK>(pLand);
            RunOrder_AOComments<orders::Type::O_ASSASSINATE>(pLand);
            RunOrder_LandAggression(pLand);
        } 
        else if (sequence == TurnSequence::SQ_GIVE)
        {
            
            RunOrder_AOComments<orders::Type::O_GIVE>(pLand);
            RunOrder_AOComments<orders::Type::O_TAKE>(pLand);
            RunOrder_LandGive(pLand);
        } 
        else if (sequence == TurnSequence::SQ_JOIN) {}
        else if (sequence == TurnSequence::SQ_EXCHANGE) {}
        else if (sequence == TurnSequence::SQ_PILLAGE) {}

        else if (sequence == TurnSequence::SQ_TAX)
        {
            RunOrder_AOComments<orders::Type::O_TAX>(pLand);
            RunOrder_AOComments<orders::Type::O_PILLAGE>(pLand);
            RunOrder_LandTaxPillage(pLand, m_EconomyTaxPillage);
        }
        else if (sequence == TurnSequence::SQ_CAST) {}

        else if (sequence == TurnSequence::SQ_SELL)
        {//no need to parse sequentially
            RunOrder_AOComments<orders::Type::O_SELL>(pLand);
            RunOrder_LandSell(pLand);
        }

        else if (sequence == TurnSequence::SQ_BUY)
        {//no need to parse sequentially
            RunOrder_AOComments<orders::Type::O_BUY>(pLand);
            RunOrder_LandBuy(pLand);
        }
        else if (sequence == TurnSequence::SQ_FORGET) {}
        else if (sequence == TurnSequence::SQ_WITHDRAW) {
            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {    
                auto orders_withdraw = orders::control::retrieve_orders_by_type(orders::Type::O_WITHDRAW, unit->orders_);
                for (const auto& order : orders_withdraw)
                {
                    long amount;
                    std::string item;
                    if (orders::parser::specific::parse_withdraw(order, item, amount))
                    {
                        unit_control::modify_item_by_reason(unit, item, amount, "from withdrawal");
                    }                            
                    else
                        OrderError("Error", pLand, unit, order, "withdraw: couldn't parse order: "+order->original_string_);
                }
            });
        }
        else if (sequence == TurnSequence::SQ_SAIL) {
            land_control::structures::update_struct_weights(pLand);
            RunOrder_AOComments<orders::Type::O_SAIL>(pLand);
            RunOrder_LandSail(pLand);
        }
        else if (sequence == TurnSequence::SQ_MOVE) {
            RunOrder_AOComments<orders::Type::O_MOVE>(pLand);
            RunOrder_AOComments<orders::Type::O_ADVANCE>(pLand);

            RunCaravanAutomoveOrderGeneration(pLand); //generate caravan automatic move orders
            RunOrder_LandMove(pLand);

            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {
                if (unit != NULL && unit_control::of_player(unit) && 
                    unit->movement_stop_ != 0 && //0 means its not moving at all
                    unit->movement_stop_ != pLand->Id) //we check that it is not going to finish in the same region
                {
                    pLand->current_state_.economy_.moving_out_ += std::max(unit_control::get_item_amount(unit, PRP_SILVER), (long)0);//it have to be zero if its below 0
                }
            });

            land_control::moving::perform_on_each_incoming_unit(pLand, [&](CUnit* unit) {
                if (unit != NULL && unit_control::of_player(unit)) {
                    pLand->current_state_.economy_.moving_in_ += std::max(unit_control::get_item_amount(unit, PRP_SILVER), (long)0);//it have to be zero if its below 0
                }
            });
        }
        else if (sequence == TurnSequence::SQ_TEACH) {}

        else if (sequence == TurnSequence::SQ_STUDY)
        {//no need to parse sequentially
            RunOrder_AOComments<orders::Type::O_TEACH>(pLand);
            RunOrder_AOComments<orders::Type::O_STUDY>(pLand);
            RunOrder_LandStudyTeach(pLand);
        }

        else if (sequence == TurnSequence::SQ_PRODUCE)
        {//no need to parse sequentially
            //initialize shared items
            land_control::init_land_all_shares(pLand);        
            RunOrder_AOComments<orders::Type::O_PRODUCE>(pLand);
            RunOrder_LandProduce(pLand);
        }
        else if (sequence == TurnSequence::SQ_BUILD) {
            RunOrder_AOComments<orders::Type::O_BUILD>(pLand);
            RunOrder_LandBuild(pLand);
        }
        else if (sequence == TurnSequence::SQ_ENTERTAIN) 
        {
            RunOrder_AOComments<orders::Type::O_ENTERTAIN>(pLand);
            RunOrder_LandEntertain(pLand, m_EconomyWork);
        }

        else if (sequence == TurnSequence::SQ_WORK)
        {
            RunOrder_AOComments<orders::Type::O_WORK>(pLand);
            RunOrder_LandWork(pLand, m_EconomyWork);
        }
        else if (sequence == TurnSequence::SQ_MAINTENANCE) 
        {
            RunOrder_AONames(pLand);
            //calculate silver for incoming units
            land_control::perform_on_each_unit_after_moving(pLand, [&](CUnit* unit) {
                if (unit != NULL && unit_control::of_player(unit))
                {
                    pLand->current_state_.economy_.maintenance_ += unit_control::get_upkeep(unit);
                    //pLand->current_state_.economy_.moving_in_ += std::max(unit_control::get_item_amount(unit, PRP_SILVER), (long)0);//it have to be zero if its below 0
                }
            });
        }
        else if (sequence == TurnSequence::SQ_TRANSPORT)
        {
            //RunOrder_AOComments<orders::Type::O_TRANSPORT>(pLand);
            //RunOrder_AOComments<orders::Type::O_DISTRIBUTE>(pLand);
            //RunOrder_LandTransport(pLand);
        }
        else if (sequence == TurnSequence::SQ_LAST) {
            long region_balance = pLand->current_state_.economy_.initial_amount_ +
                                  pLand->current_state_.economy_.tax_income_ +
                                  pLand->current_state_.economy_.claim_income_ +
                                  pLand->current_state_.economy_.sell_income_ + 
                                  pLand->current_state_.economy_.work_income_ +
                                  std::max(pLand->current_state_.economy_.moving_in_, (long)0) -
                                  pLand->current_state_.economy_.buy_expenses_ -
                                  pLand->current_state_.economy_.maintenance_ -
                                  std::max(pLand->current_state_.economy_.moving_out_, (long)0) - 
                                  pLand->current_state_.economy_.study_expenses_;
            if (region_balance < 0) {
                OrderError("Warning", pLand, nullptr, nullptr, "Region has silver balance below 0: "+std::to_string(region_balance)+" SILV");
            }
            std::list<CUnit*> no_monthlong_orders;
            CheckOrder_LandMonthlong(pLand, no_monthlong_orders);
        }

        // //call external events here
        // std::map<CLand*, std::vector<std::function<void(TurnSequence sequence)>>> bla = pLand->affections_.external_events();
        // for (auto pair : bla) {
        //     OrderError("Warning", pLand, nullptr, nullptr, std::string(pair.first->Name.GetData(), pair.first->Name.GetLength()) + " " + std::to_string(bla.size()));
        //     for (auto func : pair.second) {
        //         OrderError("Warning", pLand, nullptr, nullptr, "");
        //         func(sequence);
        //    }
        //        
        //}


    }   // phases loop
/*
    for (auto& pair : time_points)
    {
        std::chrono::microseconds duration;
        if (pair.first == 0)
            duration = duration_cast<microseconds>(pair.second - start);
        else if (pair.first % 10 == 0)
            duration = duration_cast<microseconds>(pair.second - time_points[pair.first - 10]);
        else
            duration = duration_cast<microseconds>(pair.second - time_points[pair.first - 1]);

        std::string message = "TIME ELAPSE PHASE ("+std::to_string(pair.first) + ") -- " + std::to_string(duration.count()) + " ms";
        OrderError("TIME", pLand, nullptr, message);
    }*/
    pLand->SetFlagsFromUnits();
    OrderErrFinalize();
}

void CAtlaParser::RunLandAutonames(CLand* land)
{
    // currently the only real dependance of AONames from land is LEAVE/ENTER logic
    // should be updated accordingly if it is going to be changed
    RunLandOrders(land, TurnSequence::SQ_LEAVE, TurnSequence::SQ_ENTER);
    RunOrder_AONames(land);
}

//-------------------------------------------------------------

int CAtlaParser::CountMenWithFlag(CLand * pLand, int unitFlag) const
{
    EValueType          type;
    long nmen;
    int headCount = 0;
    int skill;

    for (CUnit* pUnit : pLand->units_seq_)
    {
        if (pUnit->Flags & unitFlag)
        {
            if (pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal) && (nmen>0))
            {
                if (UNIT_FLAG_ENTERTAINING == (unsigned)unitFlag)
                {
                    if (pUnit->GetProperty("ENTE_", type, (const void *&)skill, eNormal))
                    {
                        nmen *= skill;
                    }
                }
                headCount += nmen;
            }
        }
    }
    return headCount;
}

//-------------------------------------------------------------

void CAtlaParser::DistributeSilver(CLand * pLand, int unitFlag, int silver, int menCount)
{
    EValueType          type;
    long nmen;

    int unitReceives;
    int skill;

    if (menCount == 0)
        return;

    for (CUnit* pUnit : pLand->units_seq_)
    {
        if (pUnit->Flags & unitFlag)
        {
            if (pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal) && (nmen>0))
            {
                if (UNIT_FLAG_ENTERTAINING == (unsigned)unitFlag)
                {
                    if (pUnit->GetProperty("ENTE_", type, (const void *&)skill, eNormal))
                    {
                        nmen *= skill;
                    }
                }
                unitReceives = (silver * nmen) / menCount;
                unit_control::modify_silver(pUnit, unitReceives, "distribution");
                silver -= unitReceives;
                menCount -= nmen;
                if (menCount == 0)
                    return;
            }
        }
    }
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandTaxPillage(CLand* land, bool apply_changes)
{
    land_control::Incomers taxers;
    std::vector<unit_control::UnitError> errors;
    land_control::get_land_taxers(land, taxers, errors, apply_changes);

    land->current_state_.economy_.tax_income_ = taxers.expected_income_;
    land->current_state_.tax_.requested_ = taxers.expected_income_;
    land->current_state_.tax_.requesters_amount_ = taxers.man_amount_;

    for (const auto& error : errors)
    {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
}

std::vector<orders::Type> get_monthlong_orders()
{
    std::vector<orders::Type> ret;
    std::vector<std::string> monthlong_orders = game_control::get_game_config<std::string>(SZ_SECT_COMMON, SZ_KEY_ORD_MONTH_LONG);
    for (const std::string& mon_order : monthlong_orders)
    {
        if (orders::types_mapping.find(mon_order) == orders::types_mapping.end())
        {
            gpApp->m_pAtlantis->OrderError("Error", nullptr, nullptr, nullptr, std::string("Unknown order in ")+SZ_KEY_ORD_MONTH_LONG + " group: " + mon_order);
            continue;
        }
        ret.push_back(orders::types_mapping[mon_order]);
    }    
    return ret;
}

std::set<orders::Type> get_duplicatable_orders()
{
    std::set<orders::Type> ret;
    std::vector<std::string> dup_orders = game_control::get_game_config<std::string>(SZ_SECT_COMMON, SZ_KEY_ORD_DUPLICATABLE);
    for (const std::string& dup_order : dup_orders)
    {
        if (orders::types_mapping.find(dup_order) == orders::types_mapping.end())
        {
            gpApp->m_pAtlantis->OrderError("Error", nullptr, nullptr, nullptr, std::string("Unknown order in ")+SZ_KEY_ORD_DUPLICATABLE + " group: " + dup_order);
            continue;
        }
        ret.insert(orders::types_mapping[dup_order]);
    }
    return ret;
}

void CAtlaParser::CheckOrder_LandMonthlong(CLand *land, std::list<CUnit*>& no_monthlong_orders)
{
    std::vector<unit_control::UnitError> errors;
    std::map<orders::Type, std::vector<std::shared_ptr<orders::Order>>> monthlong_collection;

    static std::vector<orders::Type> monthlong_orders = get_monthlong_orders();
    static std::set<orders::Type> duplicatable_orders = get_duplicatable_orders();
    
    land_control::perform_on_each_unit_after_moving(land, [&](CUnit* unit) { // perform_on_each_unit(land, [&](CUnit* unit) {
        if (!unit_control::of_player(unit))
            return;

        monthlong_collection.clear();

        for (const auto& ord_type : monthlong_orders)
            monthlong_collection[ord_type] = orders::control::retrieve_orders_by_type(ord_type, unit->orders_);

        size_t monthlong_orders_amount = std::accumulate(monthlong_collection.begin(), monthlong_collection.end(), (size_t)0, 
          [&](size_t acc, const std::pair<orders::Type, std::vector<std::shared_ptr<orders::Order>>>& orders) {
              if (duplicatable_orders.find(orders.first) == duplicatable_orders.end())
                  return acc + orders.second.size();
              else if (orders.second.size() > 0)
                  return acc + 1;
              else 
                  return acc;//no orders found
          });

        if (monthlong_orders_amount == 0) {
            no_monthlong_orders.push_back(unit);
            return;
        }

        if (monthlong_orders_amount > 1)
            errors.push_back({"Error", unit, nullptr, " duplicates monthlong orders"});

        for (const auto& monthlong_orders : monthlong_collection)
        {
            if (monthlong_orders.second.size() > 0)
            {
                if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_WORK))
                    unit->Flags |= UNIT_FLAG_WORKING;
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_ENTERTAIN))
                    unit->Flags |= UNIT_FLAG_ENTERTAINING;
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_PILLAGE)) {
                    unit->Flags |= UNIT_FLAG_PILLAGING;
                    land->Flags |= LAND_TAX_NEXT;
                }
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_TAX)) {
                    unit->Flags |= UNIT_FLAG_TAXING;
                    land->Flags |= LAND_TAX_NEXT;
                }                    
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_BUILD)) {
                    unit->Flags |= UNIT_FLAG_PRODUCING;
                    land->Flags |= LAND_TRADE_NEXT;
                }                  
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_PRODUCE)) {
                    unit->Flags |= UNIT_FLAG_PRODUCING;
                    land->Flags |= LAND_TRADE_NEXT;
                }                    
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_TEACH)) {
                    unit->Flags |= UNIT_FLAG_TEACHING;
                }   
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_STUDY)) {
                    unit->Flags |= UNIT_FLAG_STUDYING;
                }
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_MOVE))
                    unit->Flags |= UNIT_FLAG_MOVING;
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_ADVANCE))
                    unit->Flags |= UNIT_FLAG_MOVING;
                else if (orders::control::is_order_type(monthlong_orders.second[0], orders::Type::O_SAIL))
                    unit->Flags |= UNIT_FLAG_MOVING;
            }
        }
    });        

    for (const auto& error : errors)
    {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
}

void CAtlaParser::RunOrder_LandWork(CLand *land, bool apply_changes)
{
    land_control::Incomers workers;
    std::vector<unit_control::UnitError> errors;
    land_control::get_land_workers(land, workers, errors, apply_changes);

    land->current_state_.work_.requested_ = workers.expected_income_;
    land->current_state_.work_.requesters_amount_ = workers.man_amount_;
    land->current_state_.economy_.work_income_ += workers.expected_income_;

    for (const auto& error : errors)
    {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
}

void CAtlaParser::RunOrder_LandEntertain(CLand *land, bool apply_changes)
{
    land_control::Incomers entertainers;
    std::vector<unit_control::UnitError> errors;
    land_control::get_land_entertainers(land, entertainers, errors, apply_changes);

    land->current_state_.entertain_.requested_ = entertainers.expected_income_;
    land->current_state_.entertain_.requesters_amount_ = entertainers.man_amount_;
    land->current_state_.economy_.work_income_ += entertainers.expected_income_;

    for (const auto& error : errors)
    {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }    
}

//-------------------------------------------------------------


void CAtlaParser::RunOrder_Upkeep(CUnit * pUnit, int turns)
{
    long maintanance = unit_control::get_upkeep(pUnit);
    if (maintanance > 0)
        unit_control::modify_silver(pUnit, -maintanance * turns, "upkeep");
}

void CAtlaParser::RunOrder_Upkeep(CLand * land)
{
    land_control::perform_on_each_unit_after_moving(land, [](CUnit* unit) {
        if (unit_control::of_player(unit)) {
            long maintanance = unit_control::get_upkeep(unit);
            if (maintanance > 0)
                unit_control::modify_silver(unit, -maintanance, "upkeep");            
        }
    });
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandStudyTeach(CLand* land)
{
    std::vector<unit_control::UnitError> errors;
    std::unordered_map<long, land_control::Student> students_of_the_land;
    students_of_the_land = land_control::get_land_students(land, errors);

    //Economy
    for (const auto& student : students_of_the_land)
        if (unit_control::of_player(student.second.unit_))
            land->current_state_.economy_.study_expenses_ += student.second.skill_price_ * student.second.man_amount_;

    //teaching orders -- no need to wait for TurnSequence::Teach
    land_control::update_students_by_land_teachers(land, students_of_the_land, errors);
    for (auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
        

    //updating students
    for (auto& stud : students_of_the_land)
    {
        std::string skill;
        long study_lvl_goal;
        if (!orders::parser::specific::parse_study(stud.second.order_, skill, study_lvl_goal))
        {
            OrderError("Error", land, stud.second.unit_, stud.second.order_, "study order " + stud.second.order_->original_string_ + " is not valid!");
            continue;
        }

        stud.second.unit_->skills_[skill] += 30 + stud.second.days_of_teaching_;
        stud.second.unit_->skills_[skill] = std::min(stud.second.unit_->skills_[skill], stud.second.max_days_);
        
        //calculate property of teaching
        long room_for_teaching = std::max(stud.second.max_days_ - stud.second.cur_days_ - 30, (long)0);
        long space_for_teaching = std::min(room_for_teaching, (long)30);
        space_for_teaching = std::max(space_for_teaching - stud.second.days_of_teaching_, (long)0);                
        stud.second.unit_->monthlong_descr_ = std::to_string(space_for_teaching);
        while (stud.second.unit_->monthlong_descr_.size() < 4)
            stud.second.unit_->monthlong_descr_.insert(0, " ");

        stud.second.unit_->SetProperty(PRP_TEACHING, eLong, (const void*)space_for_teaching, eNormal);
    }
}

void CAtlaParser::RunOrder_LandAggression(CLand* land)
{
    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        if (orders::control::has_orders_with_type(orders::Type::O_ASSASSINATE, unit->orders_))
        {
            auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_ASSASSINATE, unit->orders_);
            if (orders.size() == 0)
                return;

            if (orders.size() > 1)
            {
                OrderError("Error", land, unit, nullptr, " more than 1 assassinate order");                        
                return;
            }
            long target_id;
            if (!orders::parser::specific::parse_assassinate(orders[0], target_id))
            {
                OrderError("Error", land, unit, orders[0], "assassinate: wrong format");
                return;
            }

            CUnit* target_unit = land_control::find_first_unit_if(land, [&](CUnit* unit) {
                return unit->Id == target_id;
            });
            if (target_unit == nullptr)
            {
                OrderError("Error", land, unit, orders[0], "assassinate: not existing target");
                return;
            }
            if (target_unit != nullptr && target_unit->FactionId == unit->FactionId)
            {
                OrderError("Error", land, unit, orders[0], "assassinate: target belongs to the same faction");
                return;
            }
            else 
            {
                std::string mess = "target " + std::to_string(target_id);
                unit->impact_description_.push_back("assassinate: " + mess);
                target_unit->impact_description_.push_back("Is going to be assassinated by " + std::string(unit->Name.GetData()));
                return;         
            }

        }
        if (orders::control::has_orders_with_type(orders::Type::O_ATTACK, unit->orders_))
        {
            std::vector<std::pair<std::shared_ptr<orders::Order>, std::vector<long>>> order_targets;//id, supress
            auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_ATTACK, unit->orders_);
            for (auto& order : orders)
            {
                std::vector<long> cur_targets;
                if (!orders::parser::specific::parse_attack(order, cur_targets))
                {
                    OrderError("Error", land, unit, order, "Couldn't parse attack order: " + order->original_string_);
                    continue;
                }
                order_targets.push_back({order, cur_targets});
            }
            for (auto& order_target : order_targets)
            {
                for (auto& target : order_target.second)
                {
                    CUnit* target_unit = land_control::find_first_unit_if(land, [&](CUnit* unit) {
                        return unit->Id == target;
                    });
                    if (target_unit == nullptr)
                    {
                        std::string mess = "attack: "+std::to_string(target)+"not existing target!";
                        OrderError("Error", land, unit, order_target.first, mess.c_str());
                        continue;
                    }
                    if (target_unit != nullptr && target_unit->FactionId == unit->FactionId)
                    {
                        std::string mess = "attack: "+std::to_string(target)+"belongs to the same faction!";
                        OrderError("Error", land, unit, order_target.first, mess.c_str());
                    }
                    else 
                    {
                        target_unit->impact_description_.push_back("Is attacked by " + std::string(unit->Name.GetData()));
                    }  
                }
            }
        }
        if (orders::control::has_orders_with_type(orders::Type::O_STEAL, unit->orders_))
        {
            auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_STEAL, unit->orders_);
            if (orders.size() == 0)
                return;
            if (orders.size() > 1)
            {
                OrderError("Error", land, unit, nullptr, "steal: more than 1 stealing order");
                return;
            }
            long target_id;
            std::string item;
            if (!orders::parser::specific::parse_steal(orders[0], target_id, item))
            {
                OrderError("Error", land, unit, orders[0], "steal: wrong format, should be 'STEAL XXX ITEM'!");
                return;
            }
            CUnit* target_unit = land_control::find_first_unit_if(land, [&](CUnit* unit) {
                return unit->Id == target_id;
            });
            if (target_unit == nullptr)
            {
                OrderError("Error", land, unit, orders[0], "steal: not existing target");
                return;
            }
            if (target_unit != nullptr && target_unit->FactionId == unit->FactionId)
            {
                OrderError("Error", land, unit, orders[0], "steal: target belongs to the same faction");
                return;
            } 
            else if (unit_control::get_item_amount(target_unit, item) == 0 && 
                     item_control::weight(item) > 0)
            {
                OrderError("Error", land, unit, orders[0], "steal: "+std::to_string(target_unit->Id)+" has no: " + item);
                return;
            }
            else 
            {
                std::string mess = "going to steal " + item + " from " + std::string(target_unit->Name.GetData());
                if (unit_control::get_item_amount(target_unit, item) == 0)
                    mess.append(", amount is unknown");

                unit->impact_description_.push_back("steal: " + mess);
                target_unit->impact_description_.push_back(item + " is going to be stolen by " + std::string(unit->Name.GetData()));
                return;            
            }
        }
    });
}

void CAtlaParser::RunCaravanAutomoveOrderGeneration(CLand* land)
{
    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        if (!unit_control::of_player(unit) || 
            unit_control::flags::is_moving(unit) ||
            unit->caravan_info_ == nullptr ||
            unit->caravan_info_->goal_land_ == nullptr)
            return;

        wxString Log;
        unit_control::MoveMode movemode = unit_control::get_move_state(unit);
        
        if (movemode.speed_ <= 0)
            return;
        
        bool noCross = true;
        if (!unit_control::flags::is_nocross(unit) && (movemode.swim_ == 1 || movemode.fly_ == 1))
            noCross = false;

        wxString route = RoutePlanner::GetRoute(land, unit->caravan_info_->goal_land_, movemode.speed_, RoutePlanner::ROUTE_MARKUP_TURN, noCross, Log);
        if (Log.size() > 0)
        {
            m_sOrderErrors << "For UNIT: " << unit->Id;
            m_sOrderErrors << Log.c_str();
        }
        if (!route.IsEmpty())
        {
            if (route.Length() > 66)
            {
                RoutePlanner::GetFirstMove(route);
            }

            route.Replace("_ ", "", true);
            route = wxString::Format("MOVE%s", route);
            orders::control::add_order_to_unit((const char *)route.ToUTF8(), unit);
        }
    });
}

template<orders::Type TYPE> void CAtlaParser::RunOrder_AOComments(CLand* land)
{
    //std::map<long, std::chrono::microseconds> time_points;

    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        if (!unit_control::of_player(unit))
            return;
        //auto start = std::chrono::high_resolution_clock::now();

        auto orders_by_type = orders::control::retrieve_orders_by_type(TYPE, unit->orders_);

        //get commented orders of the type
        if (TYPE != orders::Type::O_COMMENT) {
            auto commented_orders_of_type = orders::control::retrieve_orders_by_type(TYPE | orders::Type::O_COMMENT, unit->orders_);
            //extend the list of orders to parse on
            orders_by_type.insert(orders_by_type.end(), commented_orders_of_type.begin(), commented_orders_of_type.end());

        }
            

        //time_points[1] += duration_cast<microseconds>(std::chrono::high_resolution_clock::now() - start);

        //parse_line_to_order(const std::string& line)

        for (auto& order : orders_by_type)
        {
            orders::autoorders::AO_TYPES type = orders::autoorders::has_autoorders(order);
            if (type == orders::autoorders::AO_TYPES::AO_GET)
            {
                long amount;
                std::string item;
                if (!orders::autoorders::parse_get(order, amount, item))
                {
                    OrderError("error", land, unit, nullptr, "autoorders: couldn't parse get: " + order->comment_);
                    continue;
                }

                if (amount == 0)
                    continue;

                if (item == "SILV")
                    land->current_state_.economy_.claim_income_ += amount;
                unit_control::modify_item_by_reason(unit, item, amount, "by autoorders");
            }
            if (type == orders::autoorders::AO_TYPES::AO_CONDITION)
            {
                bool result;
                std::vector<unit_control::UnitError> errors;
                orders::autoorders::LogicAction action = land_control::check_conditional_logic(land, unit, order, errors, result);
                switch(action)
                {
                    case orders::autoorders::LogicAction::SWITCH_COMMENT: {
                            if (result)
                                orders::control::uncomment_order(order, unit);
                            else
                                orders::control::comment_order_out(order, unit);
                        };
                        break;
                    case orders::autoorders::LogicAction::DEL_COMMENT: {
                            if (result)
                                orders::control::uncomment_order(order, unit);
                            else 
                            {
                                if (strnicmp(order->original_string_.c_str(), "@;", 2) != 0 &&
                                    strnicmp(order->original_string_.c_str(), ";", 1) != 0)
                                {
                                    orders::control::remove_order(unit, order);
                                }
                            }
                        };
                        break;                        
                    case orders::autoorders::LogicAction::LOGIC_ERROR: {
                        if (result)
                            OrderError("Warning", land, unit, nullptr,  "autoorders: warning condition altered: " + order->comment_);
                        };
                        break;
                    case orders::autoorders::LogicAction::NONE:
                        break;
                }

                for (const auto& error : errors)
                    OrderError(error.type_, land, error.unit_, error.order_, error.message_);
            }
            if (type == orders::autoorders::AO_TYPES::AO_HELP)
            {
                OrderError("Warning", land, unit, nullptr, "HELP (How to use autoorders)");
                OrderError("Warning", land, unit, nullptr, "");
                OrderError("Warning", land, unit, nullptr, "Each comment of this group of commands will be evaluated exactly at phase when should be evaluated the order itself, before the order. For example: `move S S ;!GET 5 LBOW` will be parsed in next sequence, at MOVE phase it will add 5 LBOW to unit, and then will parse moving order. If the order consist of just comment, then it will be evaluated in the beginning of RunOrders.");
                OrderError("Warning", land, unit, nullptr, "All examples use `$`, but instead of `$` it is possile to use `!`.");
                OrderError("Warning", land, unit, nullptr, "List of acceptable autoorders:");
                OrderError("Warning", land, unit, nullptr, "$GET X ITEM:");
                OrderError("Warning", land, unit, nullptr, "    gives X items to the unit");
                OrderError("Warning", land, unit, nullptr, "$OWNER");
                OrderError("Warning", land, unit, nullptr, "    sets ownership of the building");
                OrderError("Warning", land, unit, nullptr, "$COND <condition>");
                OrderError("Warning", land, unit, nullptr, "    will evaluate condition and if its TRUE, then the entire order will be uncommented. If its FALSE, then the entire order will be commented. It is possible to use form `$COND_d` to get debug info (all the evaluations will be printed out)");
                OrderError("Warning", land, unit, nullptr, "$CONDEL <condition>");
                OrderError("Warning", land, unit, nullptr, "    same as COND, but will delete order if it was active and is going to be commented. It is possible to use form `$CONDEL_d` to get debug info (all the evaluations will be printed out)");
                OrderError("Warning", land, unit, nullptr, "$WARN <condition>");
                OrderError("Warning", land, unit, nullptr, "    will evaluate condition and if its TRUE, then there will be generated warning.  It is possible to use form `$WARN_d` to get debug info (all the evaluations will be printed out)");
                OrderError("Warning", land, unit, nullptr, "$HELP - this command.");
                OrderError("Warning", land, unit, nullptr, "");
                OrderError("Warning", land, unit, nullptr, "<condition> - is the key to understand the system. It consist of statements united by logical `&&` and `||`. Each statement is evaluated, so eventually the condition will have the value: true or false.");
                OrderError("Warning", land, unit, nullptr, "<condition> - each statement have starts with function. If function returns boolean (like `LOC[15,25]`), then its the statement itself. If function returns number (like `ITEM[MITH]`]), then it has one of next operands: `>`, `<`, `==`, `>=`, `<=`, `!=`, and then a number.");
                OrderError("Warning", land, unit, nullptr, "List of possible functions:");
                for (const auto& pair : autologic::function_descriptions())
                {
                    OrderError("Warning", land, unit, nullptr, pair.first + " -- " + pair.second);
                }


            }
        }
        //time_points[2] += duration_cast<microseconds>(std::chrono::high_resolution_clock::now() - start);
    });
/*
    for (auto& pair : time_points)
    {
        std::string message = "RunOrder_AOComments ELAPSE PHASE ("+std::to_string(pair.first) + ") -- " + std::to_string(pair.second.count()) + " ms";
        OrderError("RunOrder_AOComments", land, nullptr, message);
    }   */ 
}

void CAtlaParser::RunOrder_AONames(CLand* land)
{
    if (!game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_AUTONAMING))
        return;

    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        if (!unit_control::of_player(unit))
            return;

        //Update autoname if exists
        auto comment_orders = orders::control::retrieve_orders_by_type(orders::Type::O_COMMENT_AUTONAME, unit->orders_);
        if (comment_orders.size() > 0) {
            auto comment_order = comment_orders[comment_orders.size()-1];

            //autogenerate
            if (comment_order->comment_.find(" $C") != std::string::npos ||
                comment_order->comment_.find(" !C") != std::string::npos) {

                std::string new_client_name = "@;;" + autonaming::generate_unit_autoname(land, unit) + " $c";
                if (stricmp(&new_client_name[1], comment_order->comment_.c_str()) != 0)//[1] to ignore @ at comparation
                    orders::control::modify_order(unit, comment_order, new_client_name);
            }

            std::string name = comment_order->original_string_.substr(3);
            if (IS_NEW_UNIT(unit))
                name += std::string("(NEW ") + std::to_string((long)REVERSE_NEW_UNIT_ID(unit->Id)) + ")";
            unit->SetName(name.c_str());
        }

        auto naming_orders = orders::control::retrieve_orders_by_type(orders::Type::O_NAME, unit->orders_);
        for (auto& naming_order : naming_orders)
        {
            if (stricmp(naming_order->comment_.c_str(), ";$C") == 0 || 
                stricmp(naming_order->comment_.c_str(), ";!C") == 0)
                continue;
            if (naming_order->words_order_.size() > 1 && 
                naming_order->words_order_[1] == "UNIT" && 
                naming_order->original_string_[0] == '@')
            {
                std::string new_name = "@name unit \"" + autonaming::generate_unit_name(land, unit) + "\"";
                if (new_name != naming_order->original_string_)
                    orders::control::modify_order(unit, naming_order, new_name);
            }
        }
    });
}

//-------------------------------------------------------------

BOOL CAtlaParser::CheckResourcesForProduction(CUnit * pUnit, CLand * pLand, CStr & Error)
{
    BOOL                Ok = TRUE;
    TProdDetails        details;
    EValueType          type;
    const void        * value;
    long                nlvl  = 0;
    long                ntool = 0;
    //long                ncanproduce = 0;
    long                nmen  = 0;
    CStr                S;

    Error.Empty();
    if ((pUnit->Flags & UNIT_FLAG_PRODUCING) && !pUnit->ProducingItem.IsEmpty())
    {
        std::shared_ptr<TProdDetails> details = gpDataHelper->GetProdDetails (pUnit->ProducingItem.GetData());

        if (!pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal)  || (nmen<=0))
        {
            Error << " - There are no men in the unit!";
            Ok = FALSE;
        }

        if (details->skill_name_.empty() || details->per_month_<=0)
        {
            Error << " - Production requirements for item '" << pUnit->ProducingItem << "' are not configured! ";
            Ok = FALSE;
        }

        // check skill level
        S << details->skill_name_.c_str() << PRP_SKILL_POSTFIX;
        if (pUnit->GetProperty(S.GetData(), type, value, eNormal) && (eLong==type) )
        {
            nlvl = (long)value;
            if (nlvl < details->skill_level_)
            {
                Error << " - Skill " << details->skill_name_.c_str() << " level " << details->skill_level_ << " is required for production";
                Ok = FALSE;
            }
        }
        else
        {
            Error << " - Skill " << details->skill_name_.c_str() << " is required for production";
            Ok = FALSE;
        }


        if (!details->tool_name_.empty())
            if (!pUnit->GetProperty(details->tool_name_.c_str(), type, (const void *&)ntool, eNormal) || eLong!=type )
                ntool = 0;
        if (ntool > nmen)
            ntool = nmen;
    }

    return Ok;
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandBuild(CLand* land)
{


}

void CAtlaParser::RunOrder_LandProduce(CLand* land)
{
    std::vector<unit_control::UnitError> errors;
    std::vector<land_control::ProduceItem> out;
    land_control::get_land_producers(land, out, errors);
    for (auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }

    /*
    for (auto& product_request : out)
    {
        if (product_request.is_craft_)
        {
            std::shared_ptr<TProdDetails> prod_details = gpDataHelper->GetProdDetails(product_request.item_name_.c_str());
            for (const auto& req_resource : prod_details->req_resources_)
                land_control::get_land_shares(land, req_resource.first);
        }
    }*/

    for (auto& product_request : out)
    {
        if (!product_request.is_craft_)
        {//request from land
            long land_amount = land_control::get_resource(land->current_state_, product_request.item_name_);
            land_control::set_produced_items(land->current_state_, 
                                             product_request.item_name_, 
                                             product_request.items_amount_, //requested amount (in case of non-craft)
                                             product_request.items_amount_);//requested amount

            double leftovers(0.0);
            for (const std::pair<CUnit*, long>& producer : product_request.units_)
            {
                //or 1, or below 1 if we try to produce more than we can harvest
                double prod_coeficient = std::min(((double)land_amount / product_request.items_amount_), (double)1);

                //calculate production amount for current unit
                double intpart;
                leftovers += modf(prod_coeficient * producer.second, &intpart);
                if (leftovers >= 0.999999)
                {
                    intpart += 1;
                    leftovers -= 1.0;
                }
                unit_control::modify_item_by_produce(producer.first, product_request.item_name_, intpart);
            }
        }
        else
        {//its a craft
            for (const auto& producer : product_request.units_)
            {
                CUnit* unit = producer.first;
                long amount = producer.second;
                
                std::shared_ptr<TProdDetails> prod_details = gpDataHelper->GetProdDetails(product_request.item_name_.c_str());
                for (const auto& req_resource : prod_details->req_resources_)
                {//specify amount
                    long res_amount = 0;
                    if (!unit_control::flags::is_sharing(unit))
                        res_amount = unit_control::get_item_amount(unit, req_resource.first);

                    if (req_resource.second * amount > res_amount + land->current_state_.shared_items_[req_resource.first].first)
                    {
                        amount = (res_amount + land->current_state_.shared_items_[req_resource.first].first) / req_resource.second;
                        unit->impact_description_.push_back("produce: "+req_resource.first+" " +
                                    std::to_string(res_amount + land->current_state_.shared_items_[req_resource.first].first) + " is enough just for "+ 
                                    std::to_string(amount));
                    }
                }

                for (const auto& req_resource : prod_details->req_resources_)
                {//modify left shared resources
                    long req_material_amount = req_resource.second * amount;
                    if (req_material_amount > 0)
                    {
                        if (!unit_control::flags::is_sharing(unit)) 
                        {
                            long current_resource_amount = unit_control::get_item_amount(unit, req_resource.first);
                            if (current_resource_amount >= req_material_amount)
                            {
                                unit_control::modify_item_by_produce(unit, 
                                                                    product_request.item_name_,
                                                                    -req_material_amount);
                            }
                            else
                            {
                                unit_control::modify_item_by_produce(unit, 
                                                                    product_request.item_name_,
                                                                    -current_resource_amount);
                                req_material_amount -= current_resource_amount;
                                land->current_state_.shared_items_[req_resource.first].first -= req_material_amount;
                            }
                        }
                        else
                        {
                            land->current_state_.shared_items_[req_resource.first].first -= req_material_amount;
                        }
                    }
                }                    

                land_control::set_produced_items(land->current_state_, product_request.item_name_, amount, producer.second);
                unit_control::modify_item_by_produce(unit, 
                                                    product_request.item_name_,
                                                    amount);                     
            }
        }
    }
}

//-------------------------------------------------------------

BOOL CAtlaParser::GetItemAndAmountForGive(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params, CStr & Item, int & amount, const char * command, CUnit * pUnit2)
{
    BOOL                Ok = FALSE;
    CStr                S1  (32);
    char                ch;
    long                item_avail=0;
    EValueType          type;

    // UNIT
    // 15 SILV
    // ALL SILV
    // ALL SILV EXCEPT 15
    do
    {
        Item.Empty();
        amount = 0;

        params = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
        params = SkipSpaces(Item.GetToken(params, " \t", ch, TRIM_ALL));
        std::string codename, name, name_plural;
        gpApp->ResolveAliasItems(Item.GetData(), codename, name, name_plural);
        Item = codename.c_str();

        if (0 != stricmp("UNIT", S1.GetData()))
        {
            if (0 == stricmp("TAKE", command))
            {
                pUnit2->GetProperty(Item.GetData(), type, (const void*&)item_avail, eNormal);
                if (eLong!=type)
                {
                    SHOW_WARN_CONTINUE(" - Can not take that!");
                    break;
                }
            }
            else
            {
                if ( !pUnit->GetProperty(Item.GetData(), type, (const void*&)item_avail, eNormal) || (eLong!=type))
                {
                    SHOW_WARN_CONTINUE(" - Can not " << command << " that!");
                    break;
                }
            }
        }

        if (0 == stricmp("UNIT", S1.GetData()))
            Item = S1;
        else if (0 == stricmp("ALL", S1.GetData()))
        {
            amount = item_avail;
            params = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
            if (0==stricmp("EXCEPT", S1.GetData()))
            {
                params = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
                if (item_avail < atol(S1.GetData()))
                {
                    SHOW_WARN_CONTINUE(" - EXCEPT is too big. Use no more than " << item_avail << ".");
                    break;
                }
                amount -= atol(S1.GetData());
            }
        }
        else
            amount = atol(S1.GetData());

        if (amount < 0)
        {
            SHOW_WARN_CONTINUE(" - Can not " << command << " negative amount " << (long)amount);
            break;
        }

        if (item_avail < amount)
        {
            amount = item_avail;
            SHOW_WARN(" - Too many. " << command << " " << item_avail << " at most.");
        }

        Ok = TRUE;
    } while (FALSE);

    return Ok;
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_Withdraw(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params)
{
    CStr                Item, N;
    int                 amount;
    char                ch;

    do
    {
        // WITHDRAW 100 SILV
        params = N.GetToken(SkipSpaces(params), " \t", ch, TRIM_ALL);
        params = Item.GetToken(params, " \t", ch, TRIM_ALL);
        std::string codename, name, name_plural;
        gpApp->ResolveAliasItems(Item.GetData(), codename, name, name_plural);

        amount = atol(N.GetData()); // we allow negative amounts!

        unit_control::modify_item_by_reason(pUnit, codename, amount, "from withdrawal");
    } while (FALSE);

}

void CAtlaParser::RunOrder_LandFlags(CLand* land)
{
    std::vector<unit_control::UnitError> errors;
    land_control::apply_land_flags(land, errors);
    for (const auto& error : errors)
    {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
}

//-------------------------------------------------------------
void CAtlaParser::RunOrder_LandGive(CLand* land, CUnit* up_to)
{
    for (CUnit* unit : land->units_seq_)
    {
        if (up_to != nullptr && unit->Id == up_to->Id)
            return;

        if (!orders::control::has_orders_with_type(orders::Type::O_GIVE, unit->orders_) && 
            !orders::control::has_orders_with_type(orders::Type::O_TAKE, unit->orders_))
            continue;

        auto give_orders = orders::control::retrieve_orders_by_type(orders::Type::O_GIVE, unit->orders_);
        auto take_orders = orders::control::retrieve_orders_by_type(orders::Type::O_TAKE, unit->orders_);
        give_orders.insert(give_orders.end(), take_orders.begin(), take_orders.end());
        for (const auto& give_order : give_orders)
        {
            long target_id, target_faction_id, amount, except;
            std::string item_code;
            if (!orders::parser::specific::parse_give(give_order, target_id, target_faction_id, amount, item_code, except))
            {
                OrderError("Error", land, unit, give_order, "give/take: couldn't parse order: "+give_order->original_string_);
                continue;
            }

            if (target_id < 0)//NEW X
            {
                long x, y, z;
                land_control::get_land_coordinates(land->Id, x, y, z);
                if (target_faction_id == 0)//faction wasn't specified
                    target_id = NEW_UNIT_ID(x, y, z, unit->FactionId, abs(target_id));
                else
                    target_id = NEW_UNIT_ID(x, y, z, target_faction_id, abs(target_id));
            }

            CUnit* target_unit = nullptr;
            if (target_id > 0)
            {
                target_unit = land_control::find_first_unit_if(land, [&](CUnit* cur_unit) {
                    return cur_unit->Id == target_id;
                });
                if (target_unit == nullptr)
                {
                    OrderError("Error", land, unit, give_order, "give/take: can't locate target unit: "+give_order->original_string_);
                    continue;
                }
            }

            if (stricmp(item_code.c_str(), "UNIT") == 0)
            {
                if (target_unit == nullptr)
                    unit->impact_description_.push_back("give: drop off unit");
                else if (target_unit->FactionId == unit->FactionId)
                    OrderError("Error", land, unit, give_order, "give: transfer unit to yourself: "+give_order->original_string_);
                else {
                    unit->impact_description_.push_back("give: transfer unit to faction "+std::to_string(target_unit->FactionId));
                }                    
                continue;
            }

            CUnit* from_unit = unit;
            CUnit* to_unit = target_unit;
            if (give_order->type_ == orders::Type::O_TAKE) {
                from_unit = target_unit;
                to_unit = unit;
            }

            if (unit_control::get_item_amount(from_unit, item_code) == 0)
            {//can't find items in unit
                std::string another_name, another_plural;
                if (gpApp->ResolveAliasItems(item_code, item_code, another_name, another_plural))
                {
                    auto struct_parameters = game_control::get_game_config<std::string>(SZ_SECT_STRUCTS, another_name.c_str());
                    if (struct_parameters.size() > 0)
                    {//its a structure
                        //TODO
                        from_unit->impact_description_.push_back("tries to transfer " + std::to_string(amount) + 
                                                                 " " + another_name + " to unit "+std::to_string(to_unit->Id));
                        to_unit->impact_description_.push_back("tries to get " + std::to_string(amount) + 
                                                                 " " + another_name + " from unit "+std::to_string(from_unit->Id));
                    }
                    else
                        OrderError("Error", land, unit, give_order, "give/take: no items found: "+give_order->original_string_);
                }
                else
                    OrderError("Error", land, unit, give_order, "give/take: no items found: "+give_order->original_string_);
                //OrderError("Warning", land, unit, "give: if you try to give a ship, please, add it to ALIASES_ITEMS related to object in STRUCTURES (nut supported yet)");
                //OrderError("Warning", land, unit, "give: if you try to give item class, please, add it to UNIT_PROPERTY_GROUPS of your config (nut supported yet)");
                continue;
            }

            if (except >= 0)//ALL + optional(EXCEPT)
                amount = std::max(unit_control::get_item_amount(from_unit, item_code) - except, (long)0);

            if (amount > unit_control::get_item_amount(from_unit, item_code))
            {
                OrderError("Warning", land, unit, give_order, "give/take: has just "+std::to_string(unit_control::get_item_amount(from_unit, item_code))+
                    " instead of "+std::to_string(amount)+" of " + item_code);
                amount = std::min(amount, unit_control::get_item_amount(from_unit, item_code));
            }

            if (gpDataHelper->IsMan(item_code.c_str()))
            {
                unit_control::modify_man_from_unit(from_unit, to_unit, item_code, -amount);
                if (target_unit != nullptr)
                    unit_control::modify_man_from_unit(to_unit, from_unit, item_code, amount);
            }
            else
            {
                unit_control::modify_item_from_unit(from_unit, to_unit, item_code, -amount);
                if (target_unit != nullptr)
                    unit_control::modify_item_from_unit(to_unit, from_unit, item_code, amount);
            }
        }
    }    
}




//-------------------------------------------------------------

BOOL CAtlaParser::FindTargetsForSend(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char *& params, CUnit *& pUnit2, CLand *& pLand2)
{
    BOOL                Ok = FALSE;
    CBaseObject         Dummy;
    long                target_id;
    int                 X, Y, Z, X2, Y2, Z2, ID;
    int                 i;
    CStr                S1  (32);
    char                ch;

    /*
    SEND DIRECTION [dir] [quantity] [item]
    SEND DIRECTION [dir] UNIT [unit] [quantity] [item]
    SEND UNIT [unit] [quantity] [item]
    SEND UNIT [unit] ALL [item]
    SEND UNIT [unit] ALL [item] EXCEPT [quantity]
    SEND UNIT [unit] ALL [item class]
    */
    do
    {
        long x, y, z;
        land_control::get_land_coordinates(pLand->Id, x, y, z);

        params = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
        if (0==stricmp("DIRECTION", S1.GetData()))
        {
            pUnit2 = NULL;
            params = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
            LandIdToCoord(pLand->Id, X, Y, Z);

            for (i=0; i<(int)sizeof(Directions)/(int)sizeof(const char*); i++)
            {
                if (0==stricmp(S1.GetData(), Directions[i]))
                {
                    ExtrapolateLandCoord(X, Y, Z, i);
                    ID = LandCoordToId(X,Y, pLand->pPlane->Id);
                    pLand2 = GetLand(ID);
                    break;
                }
            }
            if (!pLand2)
                SHOW_WARN_CONTINUE(" - Can not find land in given direction");

            if (0==strnicmp("UNIT", params, 4))
            {
                params = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
                if (!GetTargetUnitId(x, y, z, params, pUnit->FactionId, target_id))
                    SHOW_WARN_CONTINUE(" - Invalid unit Id");
                if (target_id==pUnit->Id)
                    SHOW_WARN_CONTINUE(" - Giving to yourself");
                if (0 == target_id)
                    SHOW_WARN_CONTINUE(" - Invalid target unit");

                pUnit2 = this->global_find_unit(target_id);
                if (!pUnit2)
                    SHOW_WARN_CONTINUE(" - Invalid target unit");
            }
        }
        else if (0==stricmp("UNIT", S1.GetData()))
        {
            if (!GetTargetUnitId(x, y, z, params, pUnit->FactionId, target_id))
                SHOW_WARN_CONTINUE(" - Invalid unit Id");
            if (target_id==pUnit->Id)
                SHOW_WARN_CONTINUE(" - Giving to yourself");
            if (0 == target_id)
                SHOW_WARN_CONTINUE(" - Invalid target unit");

            pUnit2 = this->global_find_unit(target_id);
            if (!pUnit2)
                SHOW_WARN_CONTINUE(" - Invalid target unit");
            if (pUnit2)
                pLand2 = GetLand(pUnit2->LandId);
            if (pLand2)
            {
                //is it a neighbouring hex?
                LandIdToCoord(pLand->Id, X, Y, Z);
                LandIdToCoord(pLand2->Id, X2, Y2, Z2);
                if (Z!=Z2)
                    SHOW_WARN_CONTINUE(" - Target land on different plane");
                if (abs(Y-Y2)>2)
                    SHOW_WARN_CONTINUE(" - Target land is too far away");
                if (abs(X-X2)>1)
                {
                    // could be overlapping, check for it
                    if (pLand->pPlane->Width > 0)
                    {
                        if (X2 < X)
                        {
                            // X will be less
                            Z  = X2;
                            X2 = X;
                            X  = Z;
                        }
                        X += pLand->pPlane->Width;
                        if (abs(X-X2)>1)
                            SHOW_WARN_CONTINUE(" - Target land is too far away");
                    }
                    else
                        SHOW_WARN_CONTINUE(" - Target land is too far away");
                }
            }
        }
        else
            SHOW_WARN_CONTINUE(" - Invalid SEND command");

        Ok = TRUE;
    } while (FALSE);

    return Ok;
}

//-------------------------------------------------------------
//the function is obsolete
void CAtlaParser::RunOrder_Send(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params)
{
    CUnit             * pUnit2 = NULL;
    CLand             * pLand2 = NULL;
    CStr                Item;
    int                 amount, effweight;
    int                 price = 0;
    int                 WeatherMultiplier;
    long                unitmoney;

    EValueType          type;
    const void        * value;

    /*
    SEND DIRECTION [dir] [quantity] [item]
    SEND DIRECTION [dir] UNIT [unit] [quantity] [item]
    SEND UNIT [unit] [quantity] [item]
    SEND UNIT [unit] ALL [item]
    SEND UNIT [unit] ALL [item] EXCEPT [quantity]
    SEND UNIT [unit] ALL [item class]
    */
    do
    {
        // Find the target unit and land first
        if (!FindTargetsForSend(Line, ErrorLine, skiperror, pUnit, pLand, params, pUnit2, pLand2))
            return;

        if (!pLand2)
            SHOW_WARN_CONTINUE(" - Unable to locate target hex");

        if (GetItemAndAmountForGive(Line, ErrorLine, skiperror, pUnit, pLand, params, Item, amount, "send", NULL) )
        {
            if (!pUnit->GetProperty(Item.GetData(), type, value, eNormal) || (eLong!=type))
                SHOW_WARN_CONTINUE(" - Can not send " << Item);

            if (PE_OK!=pUnit->SetProperty(Item.GetData(), type, (const void*)((long)value-amount), eNormal))
                SHOW_WARN_CONTINUE(NOSET << BUG);

            long weights[5] = {0};
            game_control::get_item_weight(Item.GetData(), weights);

            // now we must pay the terrible price!
            WeatherMultiplier = 2;
            if (pLand2 && pLand2->WeatherWillBeGood)
                WeatherMultiplier = 1;

            effweight = weights[0] - weights[1];
            if (effweight<0)
                effweight = 0;
            price  = (int) (amount * floor(sqrt((double)effweight)) * WeatherMultiplier);

            if (!pUnit->GetProperty(PRP_SILVER, type, (const void *&)unitmoney, eNormal) )
            {
                unitmoney = 0;
                if (PE_OK!=pUnit->SetProperty(PRP_SILVER, type, (const void*)unitmoney, eBoth))
                    SHOW_WARN_CONTINUE(NOSETUNIT << pUnit->Id << BUG);
            }
            else if (eLong!=type)
                SHOW_WARN_CONTINUE(NOTNUMERIC << pUnit->Id << BUG);

            unitmoney -= price;
            if  (PE_OK!=pUnit->SetProperty(PRP_SILVER,   type, (const void *)unitmoney, eNormal))
                SHOW_WARN_CONTINUE(NOSET << BUG);
        }

    } while (FALSE);
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandBuy(CLand * land)
{
    std::vector<land_control::Trader> buyers;
    std::vector<unit_control::UnitError> errors;
    land_control::get_land_buys(land, buyers, errors);
    //long player_faction_id = game_control::get_game_config_val<long>("ATTITUDES", "PLAYER_FACTION_ID");

    for (const auto& buyer : buyers) 
    {
        land->current_state_.bought_items_[buyer.item_name_] += buyer.items_amount_;
        //Economy
        if (unit_control::of_player(buyer.unit_))
        {
            land->current_state_.economy_.buy_expenses_ += buyer.items_amount_ * buyer.market_price_;
        }            

        //Modification of state
        if (gpDataHelper->IsMan(buyer.item_name_.c_str()))
        {
            unit_control::modify_man_from_market(buyer.unit_, buyer.item_name_, buyer.items_amount_, buyer.market_price_);
        }
        else
        {
            unit_control::modify_item_from_market(buyer.unit_, buyer.item_name_, buyer.items_amount_, buyer.market_price_);
        } 

        //properties support:
        CStr                LandProp(32);
        MakeQualifiedPropertyName(PRP_SALE_PRICE_PREFIX, buyer.item_name_.c_str(), LandProp);
        CProductMarket sold_item = land_control::get_for_sale(land->current_state_, buyer.item_name_);
        if (PE_OK!=land->SetProperty(LandProp.GetData(), 
                                     eLong, 
                                     (const void *)(sold_item.item_.amount_ - buyer.items_amount_),  
                                     eNormal))
        {
            errors.push_back({"Error", buyer.unit_, nullptr, " - Can not set unit property"});
        }

        //should it be counted as production in the region
        if (game_control::get_game_config_val<long>("COMMON", "TRADE_ITEMS_AS_PROD") != 0)
        {
            if (gpDataHelper->IsTradeItem(buyer.item_name_.c_str()))
            {
                land_control::set_produced_items(land->current_state_, buyer.item_name_, buyer.items_amount_, buyer.items_amount_);
                land->Flags |= LAND_TRADE_NEXT;
            }
        }
    }    

    for (auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }

}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandSell(CLand * land)
{
    std::vector<land_control::Trader> sellers;
    std::vector<unit_control::UnitError> errors;
    land_control::get_land_sells(land, sellers, errors);
    //long player_faction_id = game_control::get_game_config_val<long>("ATTITUDES", "PLAYER_FACTION_ID");

    for (const auto& seller : sellers) 
    {
        land->current_state_.sold_items_[seller.item_name_] += seller.items_amount_;
        //Economy
        if (unit_control::of_player(seller.unit_))
        {
            land->current_state_.economy_.sell_income_ += seller.items_amount_ * seller.market_price_;
        }            

        //the sell
        if (gpDataHelper->IsMan(seller.item_name_.c_str()))
            unit_control::modify_man_from_market(seller.unit_, seller.item_name_, -seller.items_amount_, seller.market_price_);
        else
            unit_control::modify_item_from_market(seller.unit_, seller.item_name_, -seller.items_amount_, seller.market_price_);        

        //prorerties support for functionality based on properties
        CStr LandProp(32);
        MakeQualifiedPropertyName(PRP_WANTED_AMOUNT_PREFIX, seller.item_name_.c_str(), LandProp);
        CProductMarket wanted_item = land_control::get_wanted(land->current_state_, seller.item_name_);
        if (PE_OK!=land->SetProperty(LandProp.GetData(), 
                                    eLong, 
                                    (const void *)(wanted_item.item_.amount_ - seller.items_amount_),  
                                    eNormal))
            OrderError("Error", land, seller.unit_, nullptr, std::string(NOSET) + BUG);            
    }

    for (const auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
}

// void CAtlaParser::RunOrder_LandTransport(CLand* land) {

//     static long enabled = game_control::get_game_config_val<long>(SZ_SECT_COMMON, "QUARTERMASTERS_ENABLED");
//     if (!enabled)
//         return;

//     std::map<CLand*, std::map<CUnit*, std::vector<std::string>>> messages_per_unit;    

//     land_control::perform_on_each_unit_after_moving(land, [&](CUnit* unit) {
//         auto orders_transport = orders::control::retrieve_orders_by_type(orders::Type::O_TRANSPORT, unit->orders_);
        
//         for (auto order : orders_transport) {
//             long target_id(0), amount(0), except(0);
//             std::string item;
//             if (!orders::parser::specific::parse_transport(order, target_id, amount, item, except)) {
//                 OrderError("Error", land, unit, order, "transport: couldn't parse order: "+order->original_string_); 
//                 continue;
//             }

//             CUnit* target_unit = gpApp->m_pAtlantis->global_find_unit(target_id);
//             if (target_unit == nullptr) {
//                 OrderError("Error", land, unit, order, "transport: target unit doesn't exists: "+order->original_string_); 
//                 continue;
//             }

//             if (unit_control::structure_id(target_unit) == 0 || 
//                 unit_control::is_struct_owner(target_unit) == false) {
//                 OrderError("Error", land, unit, order, "transport: target unit is not an owner of a structure: "+order->original_string_); 
//                 continue;
//             }

//             CLand* target_land = gpApp->m_pAtlantis->GetLand(target_unit->LandId);
//             if (target_land == nullptr) {
//                 OrderError("Error", land, unit, order, "transport: target unit's land was not retrieved: "+order->original_string_); 
//                 continue;
//             }
//             CStruct* structure = land_control::get_struct(target_land, unit_control::structure_id(target_unit));
//             if (structure == nullptr) {
//                 OrderError("Error", land, unit, order, "transport: target unit's structure was not retrieved: "+order->original_string_); 
//                 continue;
//             }

//             auto buildings = game_control::get_game_config<std::string>(SZ_SECT_COMMON, "QUARTERMASTERS_BUILDINGS");
//             bool proper_building(false);
//             for (std::string& building : buildings) {
//                 if (stricmp(structure->type_.c_str(), building.c_str()) == 0)
//                     proper_building = true;
//             }

//             if (!proper_building) {
//                 OrderError("Error", land, unit, order, "transport: target unit's structure " + structure->type_ + " doesn't belong to QUARTERMASTERS_BUILDINGS"); 
//                 continue;
//             }

//             auto skills = game_control::get_game_config<std::string>(SZ_SECT_COMMON, "QUARTERMASTERS_SKILL");
//             bool proper_skill = skills.size() < 2;
//             std::string cur_skill;
//             for (unsigned i = 0; i < skills.size(); ++i) {
//                 if (i%2 == 0) {
//                     cur_skill = skills[i];
//                 }
//                 else {
//                     long cur_lvl = atol(skills[i].c_str());
//                     if (cur_lvl >= skills_control::get_skill_lvl_from_days(unit_control::get_current_skill_days(target_unit, cur_skill))) {
//                         proper_skill = true;
//                         break;
//                     }
//                 }
//             }

//             if (!proper_skill) {
//                 OrderError("Error", land, unit, order, "transport: target unit doesn't have a skill specified in QUARTERMASTERS_SKILL"); 
//                 continue;
//             }

//             if (amount == 0) {
//                 amount = std::max(unit_control::get_item_amount(unit, item) - except, amount);
//             }

//             //pretty naming
//             std::string long_name, long_name_plural;
//             gpApp->ResolveAliasItems(item, item, long_name, long_name_plural);
//             std::string item_description;
//             if (amount == 0)
//                 item_description = "all "+long_name_plural;
//             else if (amount == 1)
//                 item_description = "1 "+long_name;
//             else
//                 item_description = std::to_string(amount) + " " + long_name_plural;
            

//             messages_per_unit[target_land][target_unit].emplace_back("gets " + item_description + " from transport by " + 
//                                                     unit_control::compose_unit_name(unit));

//             unit->impact_description_.push_back("transports " + item_description + " to " + unit_control::compose_unit_name(target_unit));
//         }
        
        
//         auto orders_distribute = orders::control::retrieve_orders_by_type(orders::Type::O_DISTRIBUTE, unit->orders_);
        
//     });// perform_on_each_struct(land, [](CStruct* structure) {

//     CStr         land_name_cstr;
//     ComposeLandStrCoord(land, land_name_cstr);

//     std::string land_name = std::string(land->Name.GetData(), land->Name.GetLength()) + " " 
//                             + std::string(land_name_cstr.GetData(), land_name_cstr.GetLength());
//     for (auto& pair : messages_per_unit) {
//         pair.first->affections_.external_events()[land].push_back(QuartermasterAffectionFunc(land_name, std::move(pair.second)));
//     }

// }

void CAtlaParser::RunOrder_LandNameDescribe (CLand* land) {
    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_NAME, unit->orders_);
        for (const auto& order : orders)
        {
            std::string name;
            bool object(false);
            if (!orders::parser::specific::parse_namedescribe(order, name, object))
                OrderError("Error", land, unit, order, "name: couldn't parse order: "+order->original_string_);
        }

        orders = orders::control::retrieve_orders_by_type(orders::Type::O_DESCRIBE, unit->orders_);
        for (const auto& order : orders)
        {
            std::string name;
            bool object(false);
            //just checking validity
            if (!orders::parser::specific::parse_namedescribe(order, name, object)) {
                OrderError("Error", land, unit, order, "describe: couldn't parse order: "+order->original_string_);
            }
        }

    });
}

//-------------------------------------------------------------

namespace moving
{

    std::shared_ptr<orders::Order> get_moving_order(CUnit* unit, std::vector<unit_control::UnitError>& errors)
    {
        auto move_orders = orders::control::retrieve_orders_by_type(orders::Type::O_MOVE, unit->orders_);
        auto advance_orders = orders::control::retrieve_orders_by_type(orders::Type::O_ADVANCE, unit->orders_);
        auto sail_orders = orders::control::retrieve_orders_by_type(orders::Type::O_SAIL, unit->orders_);
        if (move_orders.size() + advance_orders.size() == 0)
            return nullptr;

        if (move_orders.size() + advance_orders.size() + sail_orders.size() > 1)
        {
            //multiple check should work well
            //errors.push_back({"Error", unit, "multiple move/advance/sail orders"});
            //return nullptr;
        }

        if (move_orders.size() > 0)
            return move_orders[0];
        else if (advance_orders.size() > 0)
            return advance_orders[0];
        return nullptr;
    }

    eDirection get_direction(const std::string& word)
    {
        for (size_t i=0; i<(int)sizeof(Directions)/(int)sizeof(const char*); i++)
        {
            if (stricmp(word.c_str(), Directions[i]) == 0)
                return (eDirection)(i%6);
        }
        return Center;
    }

    eDirection reverse_direction(eDirection dir)
    {
        switch (dir)
        {
            case North     : return South;
            case Northeast : return Southwest;
            case Southeast : return Northwest;
            case South     : return North;
            case Southwest : return Northeast;
            case Northwest : return Southeast;
            case Center    : return Center;//no use
        }
        return dir;
    }

    int normalize_x_coordinate(CPlane* plane, int x)
    {
        if (plane && plane->Width>0)
        {
            while (x < plane->WestEdge)
                x += plane->Width;
            while (x > plane->EastEdge)
                x -= plane->Width;
        }
        return x;
    }

    long get_next_hex_id(CPlane* plane, long hex_id, eDirection dir)
    {
        int x,y,z;
        LandIdToCoord(hex_id, x, y, z);  
        // Try to go in a direction on the map by assuming a simple grid.
        switch (dir)
        {
            case North     : y -= 2;     break;
            case Northeast : y--; x++;   break;
            case Southeast : y++; x++;   break;
            case South     : y += 2;     break;
            case Southwest : y++; x--;   break;
            case Northwest : y--; x--;   break;
            case Center:                 break;//no use
        }
        x = normalize_x_coordinate(plane, x);
        return LandCoordToId(x, y, z);      
    }

    bool is_exit_closed(CLand* land, eDirection dir)
    {
        if (!land) return false;
        if (land->xExit[dir] == EXIT_CLOSED)
            return true;
        return false;      
    }
}

//-------------------------------------------------------------
void CAtlaParser::RunOrder_LandMove(CLand* land)
{
    const int startMonth = (m_YearMon % 100) - 1;
    std::vector<unit_control::UnitError> errors;
    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        std::shared_ptr<orders::Order> order = moving::get_moving_order(unit, errors);
        if (order == nullptr)
            return;

        if (unit_control::get_item_amount_by_mask(unit, PRP_MEN) == 0)
        {
            errors.push_back({"Error", unit, order, "move: unit has no men"});
            return;
        }

        unit_control::MoveMode movemode = unit_control::get_move_state(unit);
        if (movemode.speed_ == 0)
        {
            for (size_t ord_index = 1; ord_index < order->words_order_.size(); ++ord_index)
            {//need to check that it is really going to move, and all the orders are not just PAUSE
                if (stricmp(order->words_order_[ord_index].c_str(), "P") != 0) 
                {
                    errors.push_back({"Error", unit, order, "move: unit can't move, probably the overweight"});
                    return;
                } else {
                    //in case of overload, but pause, we actually have movements, so it needs to define
                    //unit->movement_stop_ explicitly
                    unit->movements_.push_back(land->Id);
                }
            }
        }

        if (unit->caravan_info_ != nullptr)
        {//sanity check caravan speed
            switch(unit->caravan_info_->speed_) {
                case orders::CaravanSpeed::MOVE: {
                    if (!movemode.walk_ || movemode.speed_ < 2) 
                        errors.push_back({"Error", unit, order, "move: speed is below expected"});
                    break;
                }
                case orders::CaravanSpeed::RIDE: {
                    if (!movemode.ride_ || movemode.speed_ < 4) 
                        errors.push_back({"Error", unit, order, "move: speed is below expected"});
                    break;
                }
                case orders::CaravanSpeed::FLY: {
                    if (!movemode.fly_ || movemode.speed_ < 6) //TODO: flying speed fix to config
                        errors.push_back({"Error", unit, order, "move: speed is below expected"});
                    break;
                }
                case orders::CaravanSpeed::SWIM: {
                    if (!movemode.swim_) 
                        errors.push_back({"Error", unit, order, "move: expected swim, but it can't"});
                    break;
                }
                case orders::CaravanSpeed::SAIL: {
                    errors.push_back({"Error", unit, order, "move: expected sailing"});
                    break;
                }
                case orders::CaravanSpeed::UNDEFINED: {
                    errors.push_back({"Error", unit, order, "move: moving type is undefined due to an internal error"});
                    break;
                }
            }
        }
        long movepoints = movemode.speed_;
        long current_struct_id = unit_control::structure_id(unit);
        long hex_id = land->Id;
        long next_hex_id(0);
        CLand* cur_land = land;
        CLand* next_land = nullptr;
        unit->movement_stop_ = hex_id; //initial, even if can't move a step

        for (size_t ord_index = 1; ord_index < order->words_order_.size(); ++ord_index)
        {
            eDirection dir = moving::get_direction(order->words_order_[ord_index]);
            if (dir != Center)//not found
            {//parsing any direction order
                next_hex_id = moving::get_next_hex_id((cur_land ? cur_land->pPlane : nullptr), hex_id, dir);
                next_land = GetLand(next_hex_id);
                
                //exits check
                if (moving::is_exit_closed(cur_land, dir) || 
                    moving::is_exit_closed(next_land, moving::reverse_direction(dir)))
                    errors.push_back({"Warning", unit, order, "move: probably is going through the wall"});

                //water check
                if (next_land && land_control::is_water(next_land))
                {
                    if (unit_control::flags::is_nocross(unit))
                    {
                        errors.push_back({"Error", unit, order, "move: is going to step into ocean with `nocross 1`"});
                        break;
                    }
                    if (movemode.swim_ == 0 && movemode.fly_ == 0)
                    {
                        errors.push_back({"Error", unit, order, "move: is going to step into ocean being not swimming & not flying"});
                        break;
                    }
                }

                //calculate move points to verify stopping hex if order longer than unit can move
                if (next_land != nullptr)//no reason to calculate movepoints if there was lost information
                {
                    
                    int terrainCost = game_control::get_terrain_movement_cost(next_land->TerrainType.GetData());
                    long move_cost = unit_control::move_cost(terrainCost, 
                                                            land_control::is_bad_weather(next_land, startMonth), 
                                                            land_control::is_road_connected(cur_land, next_land, dir),
                                                            movemode);
                    if (movepoints >= move_cost)
                        unit->movement_stop_ = next_hex_id;

                    movepoints -= move_cost;

                    //logically perform the move
                    while(move_cost > 1)
                    {
                        unit->movements_.push_back(hex_id);
                        --move_cost;
                    }
                    unit->movements_.push_back(next_hex_id);
                    current_struct_id = 0;

                }
                else 
                {
                    if (movepoints >= 1)
                        unit->movement_stop_ = next_hex_id;
                    
                    movepoints -= 1;
                    
                    unit->impact_description_.push_back("move: through unknown territory, prediction of movepoints may be wrong");

                    //logically perform the move
                    unit->movements_.push_back(next_hex_id);
                    current_struct_id = 0;
                }
            }
            else if (stricmp(order->words_order_[ord_index].c_str(), "IN") == 0)
            {//parsing IN order
                if (cur_land == nullptr) {//specific check, because we are trying to change plane from unknown position
                    errors.push_back({"Error", unit, order, "move: going IN from unknown region"});
                    break;
                }

                if (current_struct_id == 0)
                {
                    errors.push_back({"Error", unit, order, "move: going IN without being in a structure"});
                    break;
                }
                CStruct* shaft = land_control::get_struct(cur_land, current_struct_id);
                if (shaft == nullptr)
                {
                    errors.push_back({"Error", unit, order, "move: didn't find specified structure, can't predict further"});
                    break;
                }
                if (!struct_control::flags::is_shaft(shaft))
                {
                    errors.push_back({"Error", unit, order, "move: specified structure is not a shaft, can't predict further"});
                    break;
                }
                if (!struct_control::has_link(shaft))
                {
                    errors.push_back({"Error", unit, order, "move: specified structure has no link, can't predict further"});
                    break;
                }

                next_land = GetLandFlexible(wxString::FromUTF8(shaft->original_description_.c_str()));
                if (next_land == nullptr)
                {
                    errors.push_back({"Error", unit, order, "move: couldn't deduct next region from shaft description, can't predict further"});
                    break;
                }

                next_hex_id = next_land->Id;

                //calculate move points to verify stopping hex if order longer than unit can move
                int terrainCost = game_control::get_terrain_movement_cost(next_land->TerrainType.GetData());
                long move_cost = unit_control::move_cost(terrainCost, 
                                                        land_control::is_bad_weather(next_land, startMonth), 
                                                        false,
                                                        movemode);
                if (movepoints >= move_cost)
                    unit->movement_stop_ = next_hex_id;
                movepoints -= move_cost;

                //logically perform the move
                while(move_cost > 1)
                {
                    unit->movements_.push_back(cur_land->Id);
                    --move_cost;
                }
                unit->movements_.push_back(next_hex_id);
                current_struct_id = 0;
            }
            else if (stricmp(order->words_order_[ord_index].c_str(), "P") == 0)
            {
                //calculate move points to verify stopping hex if order longer than unit can move
                if (movepoints >= 1)
                    unit->movement_stop_ = cur_land->Id;                
                movepoints -= 1;//even if we lost information, it's still valid
                unit->movements_.push_back(cur_land->Id);
                continue;
            }                
            else
            {//parsing number
                if (order->words_order_[ord_index].empty() || 
                    std::find_if(order->words_order_[ord_index].begin(), 
                                 order->words_order_[ord_index].end(), 
                                 [](unsigned char c) { 
                              return !std::isdigit(c); 
                            }) != order->words_order_[ord_index].end())
                {
                    errors.push_back({"Error", unit, order, "move: unknown moving order: "+order->words_order_[ord_index]});
                    break;
                }
                current_struct_id = atol(order->words_order_[ord_index].c_str());
                continue;//hex_id's and lands shouldn't be reset
            }

            //next region becomes current region for next cycle
            hex_id = next_hex_id;
            next_hex_id = 0;
            cur_land = next_land;
            next_land = nullptr;
        }

        //if we didn't use last steps, then fill movement as if it was "P"
        for (long i = 0; i < movepoints; ++i)
            unit->movements_.push_back(unit->movement_stop_);


        CLand* moving_stop_land = GetLand(unit->movement_stop_);
        CLand* moving_target_land = nullptr;
        if (unit->movements_.size() > 0)
            moving_target_land = GetLand(unit->movements_.back());

        land_control::moving::apply_moving(unit, land, moving_stop_land, moving_target_land);
        unit->impact_description_.push_back("moves; left movepoints: "+ std::to_string(movepoints));

        if (moving_stop_land != nullptr && land_control::is_water(moving_stop_land) && 
           !movemode.swim_ && unit_control::structure_id(unit) == 0)
            errors.push_back({"Warn", unit, order, "move: end in the ocean"});
    });

    for (auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
}

void CAtlaParser::RunOrder_LandSail(CLand* land)
{
    const int startMonth = (m_YearMon % 100) - 1;
    std::vector<unit_control::UnitError> errors;
    std::map<long, CUnit*> ships_and_owners;

    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_SAIL, unit->orders_);
        if (orders.size() == 0)
            return;

        if (orders.size() > 1)
        {//should be accepted if the order can multiply
            errors.push_back({"Error", unit, nullptr, "sail: multiple orders"});
            return;
        }

        std::shared_ptr<orders::Order> order = orders[0];

        long men_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
        if (men_amount == 0)
        {
            errors.push_back({"Error", unit, order, "sail: unit has no men"});
            return;
        }

        if (unit_control::structure_id(unit) == 0)
        {
            errors.push_back({"Error", unit, order, "sail: must be in a ship to issue SAIL order"});
            return;          
        }

        long sailing_lvl = skills_control::get_skill_lvl_from_days(unit_control::get_current_skill_days(unit, "SAIL"));
        if (sailing_lvl == 0)
        {
            errors.push_back({"Error", unit, order, "sail: need to know SAIL skill to sail"});
            return;
        }

        CStruct* ship  = land_control::get_struct(land, unit_control::structure_id(unit));
        if (ship == nullptr)
        {
            errors.push_back({"Error", unit, order, "sail: couldn't find ship's object"});
            return;
        }

        bool is_owner = unit_control::is_struct_owner(unit) || 
                        orders::autoorders::has_autoorders(order) == orders::autoorders::AO_TYPES::AO_OWNER;
        if (!is_owner && order->words_order_.size() > 1)
        {
            errors.push_back({"Error", unit, order, "sail: order direction being non-owner"});
            return;
        }

        ship->SailingPower += (men_amount*sailing_lvl);

        //continue just struct owners
        if (!is_owner)
            return;

        // check that settings for the structure were set well
        if (ship->travel_ == SHIP_TRAVEL::NONE ||
            ship->MinSailingPower == 0 ||
            ship->capacity_ == 0)
        {
            errors.push_back({"Error", unit, order, "sail: ship was not set up properly, please set up CAPACITY/SAILING_POWER/TRAVEL parameters"});
            return;
        }
        
        //capacity check
        if (ship->occupied_capacity_ > ship->capacity_)
        {
            errors.push_back({"Error", unit, order, "sail: can't sail due to overweight"});
            return;
        }
        
        long movepoints = ship->max_speed_;
        long hex_id = land->Id;
        long next_hex_id(0);
        CLand* cur_land = land;
        CLand* next_land = nullptr;
        unit->movement_stop_ = hex_id;

        for (size_t ord_index = 1; ord_index < order->words_order_.size(); ++ord_index)
        {
            //have to be agnostic to cur_land & next_land pointers
            eDirection dir = moving::get_direction(order->words_order_[ord_index]);
            if (dir != Center)//not found
            {//parsing any direction order
                next_hex_id = moving::get_next_hex_id((cur_land ? cur_land->pPlane : nullptr), hex_id, dir);
                next_land = GetLand(next_hex_id);
                
                //exits check
                if (moving::is_exit_closed(cur_land, dir) || 
                    moving::is_exit_closed(next_land, moving::reverse_direction(dir)))
                    errors.push_back({"Warning", unit, order, "sail: probably is going through the wall"});

                //land check
                if (ship->travel_ == SHIP_TRAVEL::SAIL && 
                    cur_land && next_land && 
                    !land_control::is_water(cur_land) && 
                    !land_control::is_water(next_land))
                {
                    errors.push_back({"Error", unit, order, "sail: is going to sail from land to land"});
                    break;
                }

                //calculate move points to verify stopping hex if order longer than unit can move
                if (next_land)
                {
                    long move_cost = land_control::is_bad_weather(next_land, startMonth) ? 2 : 1;
                    if (movepoints >= move_cost)
                        unit->movement_stop_ = next_hex_id;

                    movepoints -= move_cost;

                    while (move_cost > 1) {
                        unit->movements_.push_back(cur_land->Id);
                        --move_cost;
                    }
                    //logically perform the move
                    unit->movements_.push_back(next_hex_id);

                } 
                else 
                {
                    if (movepoints >= 1)
                        unit->movement_stop_ = next_hex_id;                  
                    movepoints -= 1;
                    unit->impact_description_.push_back("sails: through unknown terrotiry, movepoints count may be wrong");
                    //logically perform the move
                    unit->movements_.push_back(next_hex_id);
                }
            }
            else if (stricmp(order->words_order_[ord_index].c_str(), "P") == 0)
            {
                //calculate move points to verify stopping hex if order longer than unit can move
                if (movepoints >= 1)
                    unit->movement_stop_ = cur_land->Id;  

                movepoints -= 1;//even if we lost information, it's still valid
                unit->movements_.push_back(cur_land->Id);
                continue;
            }     
            else
            {
                errors.push_back({"Error", unit, order, "sail: unknown sailing order: "+order->words_order_[ord_index]});
                break;
            }

            //next region becomes current region for next cycle
            hex_id = next_hex_id;
            next_hex_id = 0;
            cur_land = next_land;
            next_land = nullptr;
        }

        for (long i = 0; i < movepoints; ++i)
            unit->movements_.push_back(unit->movement_stop_);        

        //CLand* moving_stop_land = GetLand(unit->movement_stop_);
        //CLand* moving_target_land = nullptr;
        //if (unit->movements_.size() > 0)
        //    moving_target_land = GetLand(unit->movements_.back());

        //land_control::moving::apply_moving(unit, land, moving_stop_land, moving_target_land);
        
        unit->impact_description_.push_back("sails; increased sailing power for: +" + std::to_string((men_amount*sailing_lvl)));
        if (unit_control::is_struct_owner(unit))
        {
            unit->impact_description_.push_back("sails; left movepoints: "+ std::to_string(movepoints));
            ships_and_owners[unit_control::structure_id(unit)] = unit;
        }
    });

    //update passangers & apply moving affection
    long cur_unit_struct_id;
    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        cur_unit_struct_id = unit_control::structure_id(unit);
        if (ships_and_owners.find(cur_unit_struct_id) != ships_and_owners.end())
        {
            unit->movement_stop_ = ships_and_owners[cur_unit_struct_id]->movement_stop_;
            unit->movements_ = ships_and_owners[cur_unit_struct_id]->movements_;
            
            CLand* moving_stop_land = GetLand(unit->movement_stop_);
            CLand* moving_target_land = nullptr;
            if (unit->movements_.size() > 0)
                moving_target_land = GetLand(unit->movements_.back());

            land_control::moving::apply_moving(unit, land, moving_stop_land, moving_target_land);
        }
    });

    for (auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.order_, error.message_);
    }
}

//-------------------------------------------------------------

#include <iostream>

void CAtlaParser::RunOrders(CLand * pLand, TurnSequence start_step, TurnSequence stop_step)
{
    int         i, n;
    CPlane    * pPlane;

    if (pLand)
    {
        RunLandOrders(pLand, start_step, stop_step);
        
        std::map<CLand*, std::vector<CUnit*>> incoming_units = pLand->affections_.incoming_units();
        for (auto pair : incoming_units) {
            if (pair.first == nullptr) {
                pLand->current_state_.run_orders_errors_.push_back({"Error", nullptr, "Incoming unit from non existing land"});
                continue;
            }
            RunLandOrders(pair.first, start_step, stop_step);
        }
    }
    else  // run orders for all lands
    {
        for (n=0; n<m_Planes.Count(); n++)
        {
            pPlane = (CPlane*)m_Planes.At(n);
            for (i=0; i<pPlane->Lands.Count(); i++)
            {
                pLand = (CLand*)pPlane->Lands.At(i);
                if (pLand) {
                    RunLandOrders(pLand, start_step, stop_step);
                }                    
            }
        }
    }
}

//-------------------------------------------------------------



/**logic of the function
 * inside one region we assume that giving items should be done by @give order
 * after that we fill all possible needs according to priority
 * 
 * iteration over regions happens from left to right, from top to bottom, so
 * if caravan is heading to already processed region, it will not take into account
 * fulfilled needs of the region. And vice versa, if it is heading to region which
 * was not processed, it may take needs.
 * 
 * it may happen that item will transfered to caravan, but not to local unit, 
 * if need of a unit in region where caravan is heading to has higher priority that
 * priority of need of local unit.
 * 
 * Thus, if you have a region wich requires finite amount of goods regular, it's recommended
 * to create there a NEED with high priority and specified amount of items, and create there
 * a storage: a NEED with low priority, which also SOURCE the same item.
 * Then caravan will still load items, even if need was fulfilled by another caravan (because target
 * region was already processed), if there will be delay, and caravan arrives in region with another 
 * caravan with same goods, they will be able to store exceeded amount of items, instead of moving them
 * back.
 */
BOOL CAtlaParser::ApplyDefaultOrders(BOOL EmptyOnly)
{
    //remove all autogenerated orders
    perform_on_each_land([](CLand* land){
        land_control::perform_on_each_unit(land, [](CUnit* unit) {
            orders::control::remove_orders_by_comment(unit, ";!AO");
            orders::control::remove_orders_by_comment(unit, ";!ao");
        });
    });

    //clearing and running all items
    RunOrders(NULL, TurnSequence::SQ_FIRST, TurnSequence::SQ_GIVE);
    
    //all source will give items to all needs according to priorities
    perform_on_each_land([&](CLand* land){
        ApplyLandDefaultOrders(land);
    });

    RunOrders(NULL);
    return TRUE;
}

void CAtlaParser::ApplyLandDefaultOrders(CLand* land)
{
    //collect all sources of region, including sources from caravans
    std::vector<orders::AutoSource> land_sources, caravan_sources;
    std::vector<orders::AutoRequirement> needs;
    orders::autoorders_caravan::get_land_autosources_and_autoneeds(land, land_sources, needs);
    orders::autoorders_caravan::get_land_caravan_autosources_and_autoneeds(land, caravan_sources, needs);

    if (needs.size() == 0)
        return;

    //sort needs by priority (in case of equal priority, caravans should go last)
    std::sort(needs.begin(), needs.end(), 
        [](const orders::AutoRequirement& req1, const orders::AutoRequirement& req2) {
            if (req1.priority_ == req2.priority_)
            {
                if (req1.unit_->caravan_info_ == nullptr && 
                    req2.unit_->caravan_info_ != nullptr)
                    return true;
                return false;
            }
            return req1.priority_ < req2.priority_;
    });   
    
    if (caravan_sources.size() > 0)//first unload caravans
    {
        while(orders::autoorders_caravan::distribute_autoorders(caravan_sources, needs))
        {}
    }

    if (land_sources.size() > 0)//now load caravans 
        orders::autoorders_caravan::distribute_autoorders(land_sources, needs);    
}

//-------------------------------------------------------------

int  CAtlaParser::SetUnitProperty(CUnit * pUnit, const char * name, EValueType type, const void * value, EPropertyType proptype)
{
    int       i;
    CStrInt * pSI;

    if (!m_UnitPropertyNames.Search((void*)name, i))
    {
        m_UnitPropertyNames.Insert(strdup(name));
        pSI = new CStrInt(name, (int)type);
        if (!m_UnitPropertyTypes.Insert(pSI))
            delete pSI;
    }
    return pUnit->SetProperty(name, type, value, proptype);
}

//-------------------------------------------------------------

int  CAtlaParser::SetLandProperty(CLand * pLand, const char * name, EValueType type, const void * value, EPropertyType proptype)
{
    int       i;
//    CStrInt * pSI;

    if (!m_LandPropertyNames.Search((void*)name, i))
    {
        m_LandPropertyNames.Insert(strdup(name));
//        pSI = new CStrInt(name, (int)type);
//        if (!m_LandPropertyTypes.Insert(pSI))
//            delete pSI;
    }
    return pLand->SetProperty(name, type, value, proptype);
}

//-------------------------------------------------------------

void WriteOneMageSkill(CStr & Line, const char * skill, CUnit * pUnit, const char * separator, int format)
{
    CStr                S;
    EValueType          type;
    const void        * value;
    long                nlvl=0, ndays=0;
    int                 n;

    Line << separator;


    S << skill << PRP_SKILL_POSTFIX;
    if (pUnit->GetProperty(S.GetData(), type, value, eNormal) && (eLong==type) )
        nlvl = (long)value;

    S.Empty();
    S << skill << PRP_SKILL_DAYS_POSTFIX;
    if (pUnit->GetProperty(S.GetData(), type, value, eNormal) && (eLong==type) )
        ndays = (long)value;

    switch (format)
    {
        case 0:  // Original decorated format
            if (nlvl>0)
            {
                Line << "_" << nlvl;
                for (n=1; n<=nlvl; n++)
                    ndays -= n*30;
                if (ndays<0)
                    ndays=0;
                n = ndays/30;

                while (n-- > 0)
                    Line << "+";
            }
            else
                Line << "_";

            break;

        case 1:  // just number of days
            Line << ndays;
            break;

        case 2:  // months and days
            if (ndays>0)
                Line << nlvl << "(" << ndays << ")";
            break;
    }
}

void CAtlaParser::WriteMagesCSV(const char * FName, BOOL vertical, const char * separator, int format)
{
    CBaseCollById       Mages;
    CUnit             * pUnit;
    int                 idx;
//    const char        * Foundations[3] = {"FORC_", "PATT_", "SPIR_"};
    EValueType          type;
    const void        * value;
    CStringSortColl     Skills;
    const char        * propname;
    int                 i, n, postlen;
    CStr                S, Line(64);
    CFileWriter         Dest;
    BOOL                IsMage;


    postlen = strlen(PRP_SKILL_POSTFIX);
    for (CUnit* pUnit : this->units_)
    {
        IsMage   = FALSE;
        i        = 0;
        propname = pUnit->GetPropertyName(i);
        while (propname)
        {
            if (gpDataHelper->IsRawMagicSkill(propname))
            {
                Mages.Insert(pUnit);
                IsMage = TRUE;
                break;
            }
            propname = pUnit->GetPropertyName(++i);
        }

        if (IsMage)
        {
            i        = 0;
            propname = pUnit->GetPropertyName(i);
            while (propname)
            {
                if (pUnit->GetProperty(propname, type, value, eNormal) && (eLong==type) )
                {
                    S = propname;
                    if (S.FindSubStrR(PRP_SKILL_POSTFIX) == S.GetLength()-postlen)
                    {
                        S.DelSubStr(S.GetLength()-postlen, postlen);
                        if (!Skills.Search((void*)S.GetData(), n))
                            Skills.Insert(strdup(S.GetData()));
                    }
                }

                propname = pUnit->GetPropertyName(++i);
            }
        }

    }


    if (Dest.Open(FName))
    {
        Line.Empty();
        if (vertical)
        {
            Line << "Skill";
            for (i=0; i<Mages.Count(); i++)
            {
                pUnit = (CUnit*)Mages.At(i);
                Line << separator << pUnit->Id << " " << pUnit->Name;
            }
            Line << EOL_FILE;
            Dest.WriteBuf(Line.GetData(), Line.GetLength());

            for (i=0; i<Skills.Count(); i++)
            {
                Line.Empty();
                Line << (const char *)Skills.At(i);
                for (idx=0; idx<Mages.Count(); idx++)
                {
                    pUnit = (CUnit*)Mages.At(idx);
                    WriteOneMageSkill(Line, (const char *)Skills.At(i), pUnit, separator, format);
                }
                Line << EOL_FILE;
                Dest.WriteBuf(Line.GetData(), Line.GetLength());
            }
        }
        else
        {
            Line << "Id" << separator << "Name";
            for (i=0; i<Skills.Count(); i++)
                Line << separator << (const char *)Skills.At(i);
            Line << EOL_FILE;
            Dest.WriteBuf(Line.GetData(), Line.GetLength());


            for (idx=0; idx<Mages.Count(); idx++)
            {
                pUnit = (CUnit*)Mages.At(idx);
                Line.Empty();
                Line << pUnit->Id << separator << pUnit->Name;
                for (i=0; i<Skills.Count(); i++)
                    WriteOneMageSkill(Line, (const char *)Skills.At(i), pUnit, separator, format);

                Line << EOL_FILE;
                Dest.WriteBuf(Line.GetData(), Line.GetLength());
            }
        }
        Dest.Close();
    }




    Mages.DeleteAll();
    Skills.FreeAll();
}

//-------------------------------------------------------------

void CAtlaParser::LookupAdvancedResourceVisibility(CUnit * pUnit, CLand * pLand)
{

/*

    const char        * propname;
    EValueType          type;
    const void        * value;
    CStr                S;
    int                 propidx, postlen;
    CLongColl           Levels;
    CBufColl            Resources;
    long                level;
    int                 i, idx;
    CItem               Dummy;
    CItem             * pProd;

    postlen = strlen(PRP_SKILL_POSTFIX);
    propidx = 0;
    propname = pUnit->GetPropertyName(propidx);
    while (propname)
    {
        if (pUnit->GetProperty(propname, type, value, eNormal) && (eLong==type) )
        {
            S = propname;
            if (S.FindSubStrR(PRP_SKILL_POSTFIX) == S.GetLength()-postlen)
            {
                S.DelSubStr(S.GetLength()-postlen, postlen);
                if (gpDataHelper->CanSeeAdvResources(S.GetData(), pLand->TerrainType.GetData(), Levels, Resources))
                {
                    level = (long)value;
                    for (i=0; i<Levels.Count(); i++)
                        if (level >= (long)Levels.At(i))
                        {
                            //possible logical error:
                            //Search actually uses first bytes ty compare.
                            //in the past it was Amount, now it is also amount.
                            Dummy.code_name_ = (const char *)Resources.At(i);
                            if (!pLand->Products.Search(&Dummy, idx))
                            {
                                pProd = new CItem;
                                pProd->amount_    = 0;
                                pProd->code_name_ = Dummy.code_name_;
                                pLand->Products.Insert(pProd);
                                land_control::add_resource(pLand->initial_state_, *pProd);
                            }
                        }
                }
            }
        }

        propname = pUnit->GetPropertyName(++propidx);
    }

    Levels.FreeAll();
    Resources.FreeAll();*/
}

//-------------------------------------------------------------

bool CAtlaParser::IsRoadConnected(CLand * pLand0, CLand * pLand1, int direction) const
{
    // Result only valid if land0 is connected to land1 in direction
    int road0 = SA_ROAD_N;
    int road1 = SA_ROAD_N;
    switch (direction)
    {
        case North     : road0 = SA_ROAD_N;  road1 = SA_ROAD_S;     break;
        case Northeast : road0 = SA_ROAD_NE; road1 = SA_ROAD_SW;    break;
        case Southeast : road0 = SA_ROAD_SE; road1 = SA_ROAD_NW;    break;
        case South     : road0 = SA_ROAD_S;  road1 = SA_ROAD_N;     break;
        case Southwest : road0 = SA_ROAD_SW; road1 = SA_ROAD_NE;    break;
        case Northwest : road0 = SA_ROAD_NW; road1 = SA_ROAD_SE;    break;
    }

    CStruct* road_structure = land_control::find_first_structure_if(pLand0, [&](CStruct* structure) {
        return (structure->Attr & road0) && !(structure->Attr & SA_ROAD_BAD);
    });
    if (road_structure == nullptr)
        return false;

    road_structure = land_control::find_first_structure_if(pLand1, [&](CStruct* structure) {
        return (structure->Attr & road1) && !(structure->Attr & SA_ROAD_BAD);
    });

    return road_structure != nullptr;
}

//-------------------------------------------------------------

// Month is zero-based 0-11
bool CAtlaParser::IsBadWeatherHex(CLand * pLand, int month) const
{
    int x, y, z;
    LandIdToCoord(pLand->Id, x, y, z);

    CPlane * pPlane = pLand->pPlane;
    if (pPlane && pPlane->TropicZoneMin < pPlane->TropicZoneMax)
    {
        // Weather is known
        if (y < pPlane->TropicZoneMin)
        {
            // Northern Hemisphere
            switch (month)
            {
                case 0: case 9: case 10: case 11:
                    return true;
            }
        }
        else if (y > pPlane->TropicZoneMax)
        {
            // Southern Hemisphere
            switch (month)
            {
                case 3: case 4: case 5: case 6:
                    return true;
            }
        }
        else
        {
            // Tropic Zone
            switch (month)
            {
                case 4: case 5: case 10: case 11:
                    return true;
            }
        }
    }
    return false;
}

//-------------------------------------------------------------

int CAtlaParser::GetMovementCost(int terrainCost, bool isBadWeather, bool hasRoad, int movementMode, bool noCross) const
{
    int MovementCost;

    if ((movementMode >= 6 && terrainCost < 999) || !noCross)
    {
        MovementCost = 1;
    }
    else
    {
        MovementCost = terrainCost;
    }

    if (isBadWeather)
    {
        MovementCost *= 2;
    }
    if (hasRoad && movementMode < 6)
    {
        MovementCost = (MovementCost + 1) / 2;
    }

    return MovementCost;
}

//-------------------------------------------------------------

CLand * CAtlaParser::GetLandFlexible(const wxString & description) const
{
    // Used for parsing a location entered by the user:
    //   "text text (coords)"
    //   "coords"
    //   "cityname"
    const wxString WorkStr = description.BeforeFirst(')').Trim().Trim(false);
    CLand * pLand = GetLand(WorkStr.AfterFirst('(').ToUTF8());

    if (!pLand)
    {
        pLand = GetLand(WorkStr.ToUTF8());
    }
    if (!pLand)
    {
        pLand = GetLandWithCity(WorkStr.BeforeFirst(' '));
    }
    return pLand;
}

//-------------------------------------------------------------

CLand * CAtlaParser::GetLandWithCity(const wxString & cityName) const
{
    // Try to find the city named by cityName.
    if (cityName.IsEmpty()) return NULL;

    int                np,nl;
    CPlane           * pPlane;
    CLand            * pLand;

    for (np=0; np<m_Planes.Count(); ++np)
    {
        pPlane = (CPlane*)m_Planes.At(np);
        for (nl=0; nl<pPlane->Lands.Count(); ++nl)
        {
            pLand    = (CLand*)pPlane->Lands.At(nl);
            if (cityName.CmpNoCase(wxString::FromUTF8(pLand->CityName.GetData())) == 0)
            {
                return pLand;
            }
        }
    }
    return NULL;
}

//-------------------------------------------------------------
