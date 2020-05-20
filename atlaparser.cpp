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
    m_EconomyShareAfterBuy = false;
    m_EconomyWork     = false;
    m_EconomyMaintainanceCosts = false;
    m_EconomyShareMaintainance = false;

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
    m_Units.FreeAll();

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

    int          i, idx;
    const char * s;
    CBaseObject  Dummy;
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
        Dummy.Id = (long)m_TradeUnitIds.At(i);
        if (m_Units.Search(&Dummy, idx))
        {
            pUnit = (CUnit*)m_Units.At(idx);
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
    CBaseObject* pSailEvent;

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
        pUnit = MakeUnit(UnitId);
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
        else if (0==stricmp("has"    , Buf.GetData()) && EventLine.FindSubStr("stolen")>=0 ||
                 0==stricmp("is"     , Buf.GetData()) && EventLine.FindSubStr("caught")>=0  ||
                 0==stricmp("steals" , Buf.GetData()) ||
                 0==stricmp("is"     , Buf.GetData()) && EventLine.FindSubStr("forbidden")>=0 ||
                 0==stricmp("forbids", Buf.GetData()) && EventLine.FindSubStr("entry")>=0
                )
        {
            BOOL show = TRUE;
            if (0==stricmp("steals", Buf.GetData()) &&
                0==atol(gpDataHelper->GetConfString(SZ_SECT_COMMON, SZ_KEY_SHOW_STEALS)))
                show = FALSE;
            if (show)
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

    attitudes[ATT_FRIEND1] = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_FRIEND1);
    attitudes[ATT_FRIEND2] = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_FRIEND2);
    attitudes[ATT_NEUTRAL] = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_NEUTRAL);
    attitudes[ATT_ENEMY] = gpDataHelper->GetConfString(SZ_SECT_ATTITUDES, SZ_ATT_ENEMY);

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
    CItem      * pProd;
    int          delpos = 0;
    int          dellen = 0;
    int          idx;
    BOOL         TerrainPassed = FALSE;
    BOOL         ProductsWereEmpty;
    BOOL         Valid;

    SectType = eMain;

    ProductsWereEmpty = (0==pLand->Products.Count());
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
                        pProd = new CItem;
                        pProd->amount_ = n1;
                        CStr LongName, ShortName;
                        p = LongName.GetToken(p, '[', TRIM_ALL);
                        p = ShortName.GetToken(p, ']', TRIM_ALL);
                        pProd->code_name_ = std::string(ShortName.GetData(), ShortName.GetLength());

                        if (pLand->Products.Search(pProd, idx))
                            pLand->Products.AtFree(idx);
                        else
                            if (!m_IsHistory && !ProductsWereEmpty)
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
                        pLand->Products.Insert(pProd);
                        land_control::add_resource(pLand->initial_state_, *pProd);

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
            pLand->Products.FreeAll();
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
                        pStruct->capacity_, pStruct->MinSailingPower, pStruct->Attr))
                    OrderError("Error", pLand, nullptr, "Couldn't parse Gate data");

                land_control::structures::add_structure(pLand, pLand->initial_state_, pStruct);
                //pLand->AddNewStruct(pStruct);

                p = SkipSpaces(p);
                p = SkipSpaces(S.GetToken(p, ' '));  // S = of
                p = S.GetToken(p, ' ');  // S = 35
                m_GatesCount = atol(S.GetData());
                DoBreak = FALSE;


                if (m_Gates.Search(pGate, idx))
                {
                    pGate = (CBaseObject*)m_Gates.At(idx);
                }
                else
                {
                    pGate = new CBaseObject;
                    pGate->Id = no;
                    m_Gates.Insert(pGate);
                }
                ComposeLandStrCoord(pLand, sCoord);
                pGate->Description.Format("Gate % 4d. ", no);
                pGate->Description << pLand->TerrainType << " (" << sCoord << ")";
                pGate->Name        = pGate->Description;
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

CUnit * CAtlaParser::MakeUnit(long Id)
{
    CBaseObject  Dummy;
    int          idx;
    CUnit      * pUnit;

    Dummy.Id = Id;
    if (m_Units.Search(&Dummy, idx))
    {
        pUnit    = (CUnit*)m_Units.At(idx);
    }
    else
    {
        pUnit = new CUnit;
        pUnit->Id = Id;
        m_Units.Insert(pUnit);
    }

    return pUnit;
}

//----------------------------------------------------------------------

/*
void CAtlaParser::MakeUnitAndGetOld(long Id, CUnit *& pUnit, CUnit *& pUnitOld, int & idxold)
{
    CBaseObject  Dummy;

    Dummy.Id = Id;
    if (m_Units.Search(&Dummy, idxold))
        pUnitOld = (CUnit*)m_Units.At(idxold);
    else
        pUnitOld = NULL;

    pUnit = new CUnit;
    pUnit->Id = Id;
}
*/

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
    int           attitude;
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

    pUnit = MakeUnit(n1);
    if (m_pCurLand)
        m_pCurLand->AddUnit(pUnit);
    if (0 == strnicmp(SkipSpaces(UnitPrefix.GetData()), HDR_UNIT_OWN, sizeof(HDR_UNIT_OWN)-1) ||
        UnitPrefix.GetLength() + UnitText.GetLength() > pUnit->Description.GetLength())
    {
        pUnit->Description = UnitPrefix;
        pUnit->Description << UnitText;
        pUnit->Name = S1;
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
    if(pUnit->IsOurs && (!Join)) // set own units to ATT_FRIEND2
    {
        attitude = ATT_FRIEND2;
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

                            //So, is it a leader?
                            //if (S1.FindSubStr(SZ_LEADER) >=0 )
                                //SetUnitProperty(pUnit, PRP_LEADER, eCharPtr, SZ_LEADER, eBoth);
                            //if (S1.FindSubStr(SZ_HERO) >=0 )
                                //SetUnitProperty(pUnit, PRP_LEADER, eCharPtr, SZ_HERO, eBoth);
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
    LookupAdvancedResourceVisibility(pUnit, m_pCurLand);

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
                                 pStruct->Id, pStruct->name_, pStruct->type_, pStruct->fleet_ships_);

    //pStruct->Name = pStruct->name_.c_str();
    for (const auto& subship : pStruct->fleet_ships_)
    {
        long load(0), spower(0);
        std::string code, name, longname;
        game_control::get_struct_attributes(subship.first, load, spower, pStruct->Attr);
        pStruct->capacity_ += load * subship.second;
        pStruct->MinSailingPower += spower * subship.second;
    }

    if (pStruct->fleet_ships_.size() == 0)
    {//for non-mobile it needs to load flags
        game_control::get_struct_attributes(pStruct->type_, pStruct->capacity_, pStruct->MinSailingPower, pStruct->Attr);
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
    CLand* pLand;
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

    CLand * pLand;
    int     i;

    pLand = GetLand(x, y, z);
    if (pLand)
        for (i=0; i<pLand->Units.Count(); i++)
            pResultColl->Insert(pLand->Units.At(i));
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
    int        i;

    CItem * pProd;

    if (pLand->Products.Count() > 0)
    {
        S << "  Products:";
        for (i=0; i<pLand->Products.Count(); i++)
        {
            pProd = (CItem*)pLand->Products.At(i);

            if (pProd->amount_>1)
            {
                std::string name, plural;
                gpApp->ResolveAliasItems(pProd->code_name_, pProd->code_name_, name, plural);
                S << " " << pProd->amount_ << " " << plural.c_str() << " ["<< pProd->code_name_.c_str() << "]";
            }
            else
            {
                std::string name, plural;
                gpApp->ResolveAliasItems(pProd->code_name_, pProd->code_name_, name, plural);
                S << " " << pProd->amount_ << " " << name.c_str() << " ["<< pProd->code_name_.c_str() << "]";
            }

            if (pLand->Products.Count()-1 == i)
                S << ".";
            else
            {
                S << ",";
            }
        }

        S << eol;
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
            out << " (attempt: " << land->current_state_.produced_items_[code] << ")";
        }
    }
    out << std::endl;

    std::vector<CItem> crafted;
    for (const auto& production : land->current_state_.produced_items_)
    {
        if (std::find_if(land->current_state_.resources_.begin(),
                     land->current_state_.resources_.end(), [&](const CItem& item) {
                return production.first == item.code_name_;
            }) == land->current_state_.resources_.end())
        {
            crafted.push_back({production.second, production.first});
            //out << std::endl << "    " << production.second << " " << production.first;
        }
    }

    if (crafted.size() > 0)
    {
        out << "  Crafts:";
        for (auto& craft : crafted)
        {
            if (!gpApp->ResolveAliasItems(craft.code_name_, code, name, plural))
            {
                code = craft.code_name_;
                name = craft.code_name_;
                plural = craft.code_name_;
            }
            if (craft.amount_ > 1)
                out << std::endl << "    " << craft.amount_ << " " << plural;
            else
                out << std::endl << "    " << craft.amount_ << " " << name;            
        }
    }
    out << std::endl;
}

//-------------------------------------------------------------

BOOL CAtlaParser::SaveOneHex(CFileWriter & Dest, CLand * pLand, CPlane * pPlane, SAVE_HEX_OPTIONS * pOptions)
{
    CLand            * pLandExit;
    int                i;
    int                g;

    CStr               sLine (128);
    CStr               sExits(128);
    CStruct          * pStruct;
    const char       * p;
    BOOL               IsLinked;
    BOOL               TurnNoMarkerWritten = FALSE;
    CUnit            * pUnit;
    EValueType         type;
    const void       * value;
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
        for (i=0; i<pLand->UnitsSeq.Count(); i++)
        {
            pUnit = (CUnit *)pLand->UnitsSeq.At(i);
            if (!IS_NEW_UNIT(pUnit) && unit_control::structure_id(pUnit) == 0)//!pUnit->GetProperty(PRP_STRUCT_ID, type, value, eOriginal) )
            {
                sLine << pUnit->Description;
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
    int           i, n, x;
    CUnit       * pUnit;
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
                for (x=0; x<pLand->Units.Count(); x++)
                {
                    pUnit = (CUnit*)pLand->Units.At(x);
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
                            OrderError("Warning", pLand, pUnit, s.ToStdString());
                        }
                    }
                }

                bool LandDecorationShown = false;
                for (x=0; x<pLand->Units.Count(); x++)
                {
                    pUnit = (CUnit*)pLand->Units.At(x);
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
                    OrderError("Warning", pLand, 0, s.ToStdString());
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
    else
        err = ERR_FOPEN;

    return err;
}

//-------------------------------------------------------------

// count number of men for the specified faction in every hex.
// store as a property

void CAtlaParser::CountMenForTheFaction(int FactionId)
{
    int           nPlane;
    int           nLand;
    int           nUnit;
    CPlane      * pPlane;
    CLand       * pLand;
    CUnit       * pUnit;
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

            for (nUnit=0; nUnit<pLand->Units.Count(); nUnit++)
            {
                pUnit = (CUnit*)pLand->Units.At(nUnit);
                if (FactionId == pUnit->FactionId)
                {
                    if (pUnit->GetProperty(PRP_MEN, type, (const void *&)x, eNormal) && (eLong==type))
                        nMen+=x;
                }
            }
            pLand->SetProperty(PRP_SEL_FACT_MEN, eLong, (void*)nMen, eBoth);
        }
    }

}

//-------------------------------------------------------------

CUnit * CAtlaParser::SplitUnit(CUnit * pOrigUnit, long newId)
{
    CUnit         Dummy;
    int           idx;
    CUnit       * pUnitNew;

    Dummy.Id = NEW_UNIT_ID(newId, pOrigUnit->FactionId);

    CLand * pLand = GetLand(pOrigUnit->LandId);
    wxASSERT(pLand);

    if (pLand->Units.Search(&Dummy, idx))
        pUnitNew = (CUnit*)pLand->Units.At(idx);
    else
    {
        CStr            propname;
        EValueType      type;
        const void    * value;

        pUnitNew = new CUnit;
        pUnitNew->Id = Dummy.Id;
        pUnitNew->FactionId = pOrigUnit->FactionId;
        pUnitNew->pFaction = pOrigUnit->pFaction;
        pUnitNew->Flags = pOrigUnit->Flags;
        pUnitNew->FlagsOrg = pOrigUnit->FlagsOrg;
        pUnitNew->IsOurs = pOrigUnit->IsOurs;
        pUnitNew->Name << "NEW " << newId;
        pUnitNew->Description << "Created by " << pOrigUnit->Name << " (" << pOrigUnit->Id << ")";

        // Copy persistent settings from the creating unit
        if (pOrigUnit->GetProperty(PRP_FRIEND_OR_FOE, type, value, eNormal) && eLong==type)
            SetUnitProperty(pUnitNew,PRP_FRIEND_OR_FOE, type, value, eNormal);
        
        unit_control::set_structure(pUnitNew, unit_control::structure_id(pOrigUnit), false);
        if (pOrigUnit->GetProperty(PRP_STRUCT_ID, type, value, eOriginal) && eLong==type)
            SetUnitProperty(pUnitNew,PRP_STRUCT_ID, type, value, eBoth);
        if (pOrigUnit->GetProperty(PRP_STRUCT_NAME, type, value, eOriginal) && eCharPtr==type)
            SetUnitProperty(pUnitNew,PRP_STRUCT_NAME, type, value, eBoth);
        // add new unit to the land of pOrigUnit
        pLand->AddUnit(pUnitNew);
    }
    return pUnitNew;
}

//-------------------------------------------------------------

int CAtlaParser::LoadOrders  (CFileReader & F, int FactionId, BOOL GetComments)
{
    CStr          Line(64), S(64), No(32);
    const char  * p;
    CPlane      * pPlane;
    CLand       * pLand;
    CUnit       * pUnit;
    CUnit       * pOrigUnit;
    CUnit         Dummy;
    int           idx;
    char          ch;

    m_CrntFactionPwd.Empty();
    for (int nPlane=0; nPlane<m_Planes.Count(); nPlane++)
    {
        pPlane = (CPlane*)m_Planes.At(nPlane);
        for (int nLand=0; nLand<pPlane->Lands.Count(); nLand++)
        {
            pLand = (CLand*)pPlane->Lands.At(nLand);
            pLand->DeleteAllNewUnits(FactionId);
            for (int nUnit=0; nUnit<pLand->Units.Count(); nUnit++)
            {
                pUnit = (CUnit*)pLand->Units.At(nUnit);
                if (FactionId == pUnit->FactionId)
                {
                    pUnit->Orders.Empty();
                }
            }
        }
    }

    pUnit = NULL;
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
            Dummy.Id = atol(No.GetData());
            if (m_Units.Search(&Dummy, idx))
                pUnit = (CUnit*)m_Units.At(idx);
            else
                pUnit = NULL;
        }
        else if (0==stricmp(S.GetData(),"form") && !pOrigUnit)
        {
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
    int           idx;

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
                Dummy.Id = atol(No.GetData());
                if (m_Units.Search(&Dummy, idx))
                {
                    pUnit = (CUnit*)m_Units.At(idx);
                    if (pUnit->FactionId > 0)
                    {
                        FactionId = pUnit->FactionId; // just take the first unit
                        break;
                    }
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


void CAtlaParser::OrderError(const std::string& type, CLand* land, CUnit* unit, const std::string& Msg)
{
    CStr         S(32);
    CStr         prefix;

    std::string land_name;
    if (land != nullptr)
    {
        CStr         land_name_cstr;
        ComposeLandStrCoord(land, land_name_cstr);
        land_name = std::string(land_name_cstr.GetData(), land_name_cstr.GetLength()) + " ";
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
    OrderError("Error", pLand, pUnit, Line.GetData()); \
    return Changed;                         \
}


BOOL CAtlaParser::ShareSilver(CUnit * pMainUnit)
{
    CLand             * pLand = NULL;
    CUnit             * pUnit;
    int                 idx;
    long                unitmoney;
    long                mainmoney;
    long                n;
    EValueType          type;
    CStr                Line(32);
    BOOL                Changed = FALSE;

    do
    {
        if (!pMainUnit || IS_NEW_UNIT(pMainUnit) || !pMainUnit->IsOurs  )
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

        mainmoney == unit_control::get_item_amount(pMainUnit, PRP_SILVER);

        if (mainmoney<=0)
            break;


        for (idx=0; idx<pLand->Units.Count(); idx++)
        {
            pUnit = (CUnit*)pLand->Units.At(idx);
            if (!pUnit->IsOurs)
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

BOOL CAtlaParser::GenGiveEverything(CUnit * pFrom, const char * To)
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
    long                x;
    CUnit               Dummy;
    CStr                S;
    const char        * p;
    long                n1;

    do
    {
        if (!pFrom || IS_NEW_UNIT(pFrom) || !pFrom->IsOurs || !To || !*To )
            break;

        pLand = GetLand(pFrom->LandId);
        if (!pLand)
            break;

        p  = To;
        if (!GetTargetUnitId(p, pFrom->FactionId, n1))
        {
            S << "Invalid unit id " << To;
            OrderError("Error", pLand, pFrom, S.GetData());
            break;
        }
        Dummy.Id = n1;
        if (n1 != 0 && !pLand->Units.Search(&Dummy, i) )
        {
            S << "Can not find unit " << To;
            OrderError("Error", pLand, pFrom, S.GetData());
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
    int                 idx;
    CStr                Line(32);

    if (!pMainUnit || !pMainUnit->IsOurs)
        return FALSE;

    CLand* pLand = GetLand(pMainUnit->LandId);
    if (!pLand)
        return FALSE;

    const char* val = gpApp->GetConfig(SZ_SECT_COMMON, "PEASANT_CAN_TEACH");
    int peasant_can_teach = 0;
    if (val != NULL)
        peasant_can_teach = atoi(val);

    EValueType          type;
    const void        * value;
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
        OrderError(error.type_, pLand, error.unit_, error.message_);
    }        

    BOOL changed = FALSE;
    for (const auto& stud : students)
    {
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
            
            //why does it return true for skills which this unit doesn't have??
            //skill.append(PRP_SKILL_POSTFIX);
            //pMainUnit->GetProperty(skill.c_str(), type, (const void *&)teacher_skill_lvl, eNormal);

            //long teachers_amount;
            //pMainUnit->GetProperty(PRP_MEN, type, (const void *&)teachers_amount, eNormal);

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
                pMainUnit->Orders << ";" << skill.c_str() << " " << stud.second.cur_days_ << "(" << stud.second.max_days_ << ")";
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

    if (!pUnit || IS_NEW_UNIT(pUnit) || !pUnit->IsOurs )
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

BOOL CAtlaParser::DetectSpies(CUnit * pUnit, long lonum, long hinum, long amount)
{

#define DETECT_ONE_SPY                                                         \
{                                                                              \
    pUnit->Orders << "GIVE " << no << " " << amount << " SILV ;ne"  << EOL_SCR;\
    Changed = TRUE;                                                            \
    no++;                                                                      \
}

    CLand             * pLand = NULL;
    BOOL                Changed = FALSE;
    long                no;
    int                 idx = 0;
    CUnit             * pNext;

    if (!pUnit || IS_NEW_UNIT(pUnit) || !pUnit->IsOurs)
        return FALSE;

    pLand = GetLand(pUnit->LandId);
    if (!pLand)
        return FALSE;

    RunLandOrders(pLand); // just in case...

    pUnit->Orders.TrimRight(TRIM_ALL);
    if (!pUnit->Orders.IsEmpty())
        pUnit->Orders << EOL_SCR ;

    no = lonum;
    while (no<=hinum)
    {
        pNext = (CUnit*)m_Units.At(idx);

        if (pNext)
        {
            while (no<pNext->Id)
                DETECT_ONE_SPY
            if (no == pNext->Id)
                no++;
            idx++;
        }
        else
            DETECT_ONE_SPY
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

BOOL CAtlaParser::GetTargetUnitId(const char *& p, long FactionId, long & nId)
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
            nId = NEW_UNIT_ID(atol(Y.GetData()), atol(X.GetData()));
        else
            nId = 0;
        return TRUE;
    }
    if (0==stricmp("NEW", N1.GetData()))
    {
        p = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));
        if (!N1.IsInteger())
            return FALSE;
        nId = NEW_UNIT_ID(atol(N1.GetData()), FactionId);
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
        OrderError("Warning", pLand, pUnit, ErrorLine.GetData()); \
    }                                                \
    continue;                                        \
}

#define SHOW_WARN(msg)                               \
{                                                    \
    if (!skiperror)                                  \
    {                                                \
        ErrorLine.Empty();                           \
        ErrorLine << Line << msg;                    \
        OrderError("Warning", pLand, pUnit, ErrorLine.GetData()); \
    }                                                \
}

#define SHOW_WARN_BREAK(msg)                         \
{                                                    \
    if (!skiperror)                                  \
    {                                                \
        ErrorLine.Empty();                           \
        ErrorLine << Line << msg;                    \
        OrderError("Warning", pLand, pUnit, ErrorLine.GetData()); \
        break;                                       \
    }                                                \
}

void CAtlaParser::RunPseudoComment(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand,
                                   const char * src, TurnSequence sequence, wxString & destination)
{
    CStr            Command;
    CStr            S;
    char            ch;
    const char    * p;
    EValueType      type;
    const void    * value;

    do
    {
        p = SkipSpaces(Command.GetToken(SkipSpaces(src), " \t", ch, TRIM_ALL));

        // it must be sequenced just like the real commands!
        if (TurnSequence::SQ_CLAIM == sequence && (0==stricmp(Command.GetData(), "$GET")))  // do it in CLAIM so it will affect new units too
        {
            // GET 100 silv
            Command = src;
            RunOrder_Withdraw(Command, S, FALSE, pUnit, pLand, p);
        }
        else if (TurnSequence::SQ_CLAIM == sequence && (0==stricmp(Command.GetData(), "$TRAVEL")))
        {
            CStr token;
            p = SkipSpaces(token.GetToken(p, " ;\t", ch, TRIM_ALL));
            p = token.GetData();
            if (0==stricmp(p, "walk"))
            {
                pUnit->reqMovementSpeed = 2;
            }
            else if (0==stricmp(p, "ride"))
            {
                pUnit->reqMovementSpeed = 4;
            }
            else if (0==stricmp(p, "fly"))
            {
                pUnit->reqMovementSpeed = 6;
            }
            else
                SHOW_WARN_CONTINUE(" - Invalid parameter");
        }
        else if (TurnSequence::SQ_MOVE==sequence && (0==stricmp(Command.GetData(), "$MOVE")))
        {
            destination = wxString::FromUTF8(p);
        }
        else if (TurnSequence::SQ_LAST == sequence && (0==stricmp(Command.GetData(), "$checkzero")))
        {
            CStr itemname;
            p = SkipSpaces(itemname.GetToken(p, " ;\t", ch, TRIM_ALL));

            long itemCount = 0;
            if (pUnit->GetProperty(itemname.GetData(), type, value, eNormal) && (eLong==type) )
                itemCount = (long)value;
            if (itemCount > 0) SHOW_WARN_CONTINUE(" - has items");
        }
        else if (TurnSequence::SQ_LAST == sequence && (0==stricmp(Command.GetData(), "$checkmax")))
        {
            CStr reqItemname;
            p = SkipSpaces(reqItemname.GetToken(p, " ;\t", ch, TRIM_ALL));
            CStr reqItemcount;
            p = SkipSpaces(reqItemcount.GetToken(p, " ;\t", ch, TRIM_ALL));
            long reqItems = atoi(reqItemcount.GetData());

            long itemCount = 0;
            if (pUnit->GetProperty(reqItemname.GetData(), type, value, eNormal) && (eLong==type) )
                itemCount = (long)value;
            if (itemCount > reqItems) SHOW_WARN_CONTINUE(" - has too many");
        }
        else if (TurnSequence::SQ_LAST == sequence && (0==stricmp(Command.GetData(), "$UPKEEP")))
        {
            Command = src;
            int turns = atol(p);
            if (turns < 1) turns = 1;
            RunOrder_Upkeep(pUnit, turns);
        }

    }
    while (false);
}


//#include <chrono> 
//using namespace std::chrono; 

void CAtlaParser::RunLandOrders(CLand * pLand, TurnSequence beg_step, TurnSequence stop_step)
{
    CBaseObject         Dummy;
    CUnit             * pUnit;
    CUnit             * pUnitNew;
    CUnit             * pUnitMaster;
    CUnit             * pUnit2;
    CStruct           * pStruct;
    CStr                Line(32);
    CStr                Cmd (32);
    CStr                S   (32);
    CStr                S1  (32);
    CStr                N1  (32);
    CStr                ErrorLine(32);
    const char        * p;
    const char        * src;
    long                n1;
    long                unitmoney;
    int                 mainidx;
    //TurnSequence        sequence;
    BOOL                errors = FALSE;
    int                 idx;
    EValueType          type;
    const void        * value;
    int                 X, Y, Z;    // current coordinates for moving unit
    int                 XN=0, YN=0;     // saved current coords while new unit is processed
    int                 LocA3, LocA3N=0; // support for ArcadiaIII sailing
    char                ch;
    //BOOL                TeachCheckGlb = (BOOL)atoi(sCheckTeach?sCheckTeach:gpDataHelper->GetConfString(SZ_SECT_COMMON, SZ_KEY_CHECK_TEACH_LVL));
    BOOL                skiperror = false;
    int                 nNestingMode; // Shar1 Support for TURN/ENDTURN
    long                order;

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

            // AutoOrders initialization & sanity check section
            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {
                //caravan sanity check
                if (unit->IsOurs && unit_control::init_caravan(unit))
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
                if (unit->IsOurs)
                    pLand->current_state_.economy_.initial_amount_ += unit_control::get_item_amount(unit, PRP_SILVER);

                std::vector<std::shared_ptr<orders::Order>> errors = orders::control::retrieve_orders_by_type(orders::Type::O_ERROR, unit->orders_);
                for (auto& order : errors)
                {
                    OrderError("error", pLand, unit, order->comment_ + ": " + order->original_string_);
                }
            });
            RunOrder_AOComments<orders::Type::O_COMMENT>(pLand);
            CheckOrder_LandWork(pLand);
        }

        if (sequence == TurnSequence::SQ_FORM)
        {            
        }
        if (sequence == TurnSequence::SQ_CLAIM)
        {
            //RunOrder_AOComments(pLand, );
            RunOrder_LandFlags(pLand);
        }
        if (sequence == TurnSequence::SQ_LEAVE)
        {
        }
        if (sequence == TurnSequence::SQ_ENTER)
        {
        }
        if (sequence == TurnSequence::SQ_PROMOTE)
        {
        }
        if (sequence == TurnSequence::SQ_STEAL) //for SQ_ATTACK, SQ_ASSASSINATE
        {
            RunOrder_AOComments<orders::Type::O_STEAL>(pLand);
            RunOrder_AOComments<orders::Type::O_ATTACK>(pLand);
            RunOrder_AOComments<orders::Type::O_ASSASSINATE>(pLand);
            RunOrder_LandAggression(pLand);
        } 
        if (sequence == TurnSequence::SQ_GIVE)
        {
            
            RunOrder_AOComments<orders::Type::O_GIVE>(pLand);
            RunOrder_AOComments<orders::Type::O_TAKE>(pLand);
            RunOrder_LandGive(pLand);
        } 
        if (sequence == TurnSequence::SQ_JOIN) {}
        if (sequence == TurnSequence::SQ_EXCHANGE) {}
        if (sequence == TurnSequence::SQ_PILLAGE) {}

        if (sequence == TurnSequence::SQ_TAX)
        {
            RunOrder_AOComments<orders::Type::O_TAX>(pLand);
            RunOrder_AOComments<orders::Type::O_PILLAGE>(pLand);
            RunOrder_LandTaxPillage(pLand, m_EconomyTaxPillage);
        }
        if (sequence == TurnSequence::SQ_CAST) {}

        if (sequence == TurnSequence::SQ_SELL)
        {//no need to parse sequentially
            RunOrder_AOComments<orders::Type::O_SELL>(pLand);
            RunOrder_LandSell(pLand);
        }

        if (sequence == TurnSequence::SQ_BUY)
        {//no need to parse sequentially
            RunOrder_AOComments<orders::Type::O_BUY>(pLand);
            RunOrder_LandBuy(pLand);
        }
        if (sequence == TurnSequence::SQ_FORGET) {}
        if (sequence == TurnSequence::SQ_WITHDRAW) {}
        if (sequence == TurnSequence::SQ_SAIL) {
            RunOrder_AOComments<orders::Type::O_SAIL>(pLand);
        }
        if (sequence == TurnSequence::SQ_MOVE) {
            RunOrder_AOComments<orders::Type::O_MOVE>(pLand);
            RunOrder_AOComments<orders::Type::O_ADVANCE>(pLand);
        }
        if (sequence == TurnSequence::SQ_TEACH) {}

        if (sequence == TurnSequence::SQ_STUDY)
        {//no need to parse sequentially
            RunOrder_AOComments<orders::Type::O_STUDY>(pLand);
            RunOrder_LandStudyTeach(pLand);
        }

        if (sequence == TurnSequence::SQ_PRODUCE)
        {//no need to parse sequentially
            RunOrder_AOComments<orders::Type::O_PRODUCE>(pLand);
            pLand->current_state_.produced_items_.clear();
            RunOrder_LandProduce(pLand);
        }
        if (sequence == TurnSequence::SQ_BUILD) {}
        if (sequence == TurnSequence::SQ_ENTERTAIN) 
        {
            //m_EconomyShareAfterBuy
            RunOrder_LandEntertain(pLand, m_EconomyWork);
        }

        if (sequence == TurnSequence::SQ_WORK)
        {
            RunOrder_LandWork(pLand, m_EconomyWork);
        }
        if (sequence == TurnSequence::SQ_MAINTENANCE) 
        {
            RunOrder_AONames(pLand);
        }

        for (mainidx=0; mainidx<pLand->UnitsSeq.Count(); mainidx++)
        {
            nNestingMode = 0; // Shar1 Support for TURN/ENDTURN
            pUnitMaster = NULL;
            bool isSimCmd = false;
            pUnit       = (CUnit*)pLand->UnitsSeq.At(mainidx);
            CLand* destination_land = nullptr;
            if (pUnit->caravan_info_ != nullptr)
                destination_land = pUnit->caravan_info_->goal_land_;

            LandIdToCoord(pLand->Id, X, Y, Z);
            LocA3 = NO_LOCATION;

            src         = pUnit->Orders.GetData();
            while (src && *src)
            {
                src  = Line.GetToken(src, '\n', TRIM_ALL);

                // execute all lines starting with @;;
                p = strstr(Line.GetData(), "@;;");
                if (p && p == Line.GetData())
                {
                    isSimCmd = true;
                    Line.DelSubStr(0, 3);
                    pUnit->Name = Line;
                    if (IS_NEW_UNIT(pUnit))
                        pUnit->Name << "(NEW " << (long)REVERSE_NEW_UNIT_ID(pUnit->Id) << ")";
                    continue;
                }
                else
                {
                    p = strstr(Line.GetData(), ";;");
                    if (p)
                    {
                        wxString destination;
                        p+=2;
                        RunPseudoComment(Line, ErrorLine, skiperror, pUnit, pLand, p, sequence, destination);
                        if (destination_land == nullptr)
                        {//if is alrady defined, it's a caravan, do not define
                            destination_land = GetLandFlexible(destination);
                        }
                            
                    }
                    isSimCmd = false;
                }

                while (Line.GetData()[0] == ' ')
                {
                    Line.DelSubStr(0, 1);
                }

                // trim all the comments
                p = strchr(Line.GetData(), ';');
                if (p)
                {
                    S1 = SkipSpaces(++p);
                    S1.TrimRight(TRIM_ALL);
                    skiperror = (0==stricmp(S1.GetData(),"ne") || 0==stricmp(S1.GetData(),"$ne"));
                    if (!nNestingMode)
                    {
                        wxString destination;
                        RunPseudoComment(Line, ErrorLine, skiperror, pUnit, pLand, S1.GetData(), sequence, destination);
                        if (destination_land == nullptr)
                        {//if is alrady defined, it's a caravan, do not define
                            destination_land = GetLandFlexible(destination);
                        }
                            
                    }
                    Line.DelSubStr(p-Line.GetData()-1, Line.GetLength() - (p-Line.GetData()-1) );
                }
                else
                    skiperror = FALSE;

                p    = SkipSpaces(Cmd.GetToken(Line.GetData(), " \t", ch, TRIM_ALL));
                if ('@'==Cmd.GetData()[0])
                {
                    Cmd.DelCh(0);
                    if (Cmd.IsEmpty()) // I do not know if putting spaces after '@' is legal... anyway
                        p    = SkipSpaces(Cmd.GetToken(p, " \t", ch, TRIM_ALL));
                }
                if ('!'==Cmd.GetData()[0])
                {
                    Cmd.DelCh(0);
                    if (Cmd.IsEmpty()) // I do not know if putting spaces after '@' is legal... anyway
                        p    = SkipSpaces(Cmd.GetToken(p, " \t", ch, TRIM_ALL));
                    skiperror = TRUE;
                }

                if (Cmd.IsEmpty())
                    continue;
                if (!gpDataHelper->GetOrderId(Cmd.GetData(), order))
                {
                    if (TurnSequence::SQ_FIRST == sequence)
                        SHOW_WARN_CONTINUE(" - unknown order")
                    else
                        continue;
                }

                // Shar1 Support for TURN/ENDTURN - Start
                if (nNestingMode)
                {
                    // I don't know if Atlantis support nested TURN/ENDTURN tags
                    // By now, ALH won't support them. But it can be changed easily,
                    // changing the nNestingMode into a turnLevel (integer)
                    if (O_TURN == order || O_TEMPLATE == order || O_ALL == order)
                    {
                        SHOW_WARN_CONTINUE(" - Nesting TURN, TEMPLATE and such orders are not allowed");
                    }
                    else if (nNestingMode+1 == order)
                    {
                        nNestingMode = 0;
                    }
                    else
                    {
                        // here we can do something special!
                    }
                    continue;
                }

                switch (order)
                {
                    case O_TURN:
                    case O_TEMPLATE:
                    case O_ALL:
                        nNestingMode = order;
                        break;

                    case O_ENDTURN:
                    case O_ENDTEMPLATE:
                    case O_ENDALL:
                        SHOW_WARN_CONTINUE(" - ENDXXX without XXX");
                        break;                        // Shar1 Support for TURN/ENDTURN - End

                    case O_GIVEIF:
                        if (TurnSequence::SQ_GIVE == sequence)
                            //RunOrder_Give(Line, ErrorLine, skiperror, pUnit, pLand, p, TRUE);
                        break;

                    case O_TAKE:
                        if (TurnSequence::SQ_GIVE == sequence)
                            RunOrder_Take(Line, ErrorLine, skiperror, pUnit, pLand, p, TRUE);
                        break;

                    case O_SEND:
                        if (TurnSequence::SQ_FORGET == sequence) // send is after buy
                            RunOrder_Send(Line, ErrorLine, skiperror, pUnit, pLand, p);
                        break;

                    case O_WITHDRAW:
                        if (TurnSequence::SQ_WITHDRAW == sequence)
                            RunOrder_Withdraw(Line, ErrorLine, skiperror, pUnit, pLand, p);
                        break;

                    case O_RECRUIT:
                        if (TurnSequence::SQ_FORGET == sequence)
                        {
                            // do recruiting here
                            p        = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));
                            Dummy.Id = atol(N1.GetData());
                            if (!pLand->Units.Search(&Dummy, idx))
                                SHOW_WARN_CONTINUE(" - Can not find unit " << N1);
                        }
                        break;

                    case O_FORM:
                        p        = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));
                        n1       = NEW_UNIT_ID(atol(N1.GetData()), pUnit->FactionId);
                        Dummy.Id = n1;

                        if (TurnSequence::SQ_FORM==sequence)
                        {
                            SHOW_WARN_CONTINUE(" - Do not manually enter a FORM command - use the split context-menu or Create New Unit dialog");
                            break;
                            if (!IS_NEW_UNIT_ID(n1))
                                SHOW_WARN_CONTINUE(" - Invalid new unit number!");
                            if (pLand->Units.Search(&Dummy, idx))
                                SHOW_WARN_CONTINUE(" - Unit already exists!");
                            pUnitNew             = new CUnit;
                            pUnitNew->Id         = n1;
                            pUnitNew->FactionId  = pUnit->FactionId;
                            pUnitNew->pFaction   = pUnit->pFaction;
                            pUnitNew->Flags      = pUnit->Flags;
                            pUnitNew->IsOurs     = TRUE;
                            pUnitNew->Name        << "NEW " << N1;
                            pUnitNew->Description << "Created by " << pUnit->Name << " (" << pUnit->Id << ")";

                            // set attitude:
                            int attitude = gpDataHelper->GetAttitudeForFaction(pUnit->FactionId);
                            SetUnitProperty(pUnitNew,PRP_FRIEND_OR_FOE,eLong,(void *) attitude,eNormal);

                            unit_control::set_structure(pUnitNew, unit_control::structure_id(pUnit), false);
                            if (pUnit->GetProperty(PRP_STRUCT_ID, type, value, eOriginal) )
                                pUnitNew->SetProperty(PRP_STRUCT_ID, type, value, eBoth);
                            if (pUnit->GetProperty(PRP_STRUCT_NAME, type, value, eOriginal) )
                                pUnitNew->SetProperty(PRP_STRUCT_NAME, type, value, eBoth);

                            if (!pLand->AddUnit(pUnitNew))
                                SHOW_WARN_CONTINUE(" - Can not create new unit! " << BUG);
                        }
                        else
                        {
                            break;
                            if (!IS_NEW_UNIT_ID(n1))
                                continue;
                            if (pUnitMaster)
                                SHOW_WARN_CONTINUE(" - Sorry, we do not support new units creating units :(");
                            pUnitMaster       = pUnit;
                            if (!pLand->Units.Search(&Dummy, idx))
                                SHOW_WARN_CONTINUE(" - Can not find new unit! " << BUG);
                            pUnit = (CUnit*)pLand->Units.At(idx);
                            XN     = X;
                            YN     = Y;
                            LocA3N = LocA3;
                            LandIdToCoord(pLand->Id, X, Y, Z);
                            LocA3 = NO_LOCATION;
                        }
                        break;

                    case O_ENDFORM:
                        //what is going on here ??
                        if (TurnSequence::SQ_FORM!=sequence)
                        {
                            if (!pUnitMaster)
                                if (TurnSequence::SQ_CLAIM == sequence)
                                {
                                    SHOW_WARN_CONTINUE(" - FORM was not called!");
                                }
                                else
                                    continue;

                            //if  (TurnSequence::SQ_TEACH==sequence)  // have to call it here or new units teaching will not be calculated
                            //    OrderProcess_Teach(skiperror, pUnit);

                            pUnit       = pUnitMaster;
                            pUnitMaster = NULL;
                            X     = XN;
                            Y     = YN;
                            LocA3 = LocA3N;
                        }
                        break;

                    case O_NAME:
                        if (TurnSequence::SQ_CLAIM==sequence)
                            RunOrder_Name(Line, ErrorLine, skiperror, pUnit, pLand, p);
                        break;

                    case O_CLAIM:
                        if (TurnSequence::SQ_CLAIM==sequence)
                        {
                            p        = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));
                            n1       = atol(N1.GetData());
                            unit_control::modify_silver(pUnit, n1, "claiming");
                        }
                        break;


                    case O_LEAVE:
                        if (TurnSequence::SQ_LEAVE==sequence)
                        {
                            long struct_id = unit_control::structure_id(pUnit);
                            if (struct_id > 0)
                            {
                                pStruct  = land_control::get_struct(pLand, struct_id);
                                if (pStruct && pStruct->OwnerUnitId == pUnit->Id)
                                    pStruct->OwnerUnitId = 0;
                            }

                            unit_control::set_structure(pUnit, 0, false);
                            //pUnit->DelProperty(PRP_STRUCT_NAME);
                            //pUnit->DelProperty(PRP_STRUCT_OWNER);
                            //pUnit->DelProperty(PRP_STRUCT_ID);

                            if ( (PE_OK!=pUnit->SetProperty(PRP_STRUCT_NAME,  eCharPtr, "", eNormal)) ||
                                (PE_OK!=pUnit->SetProperty(PRP_STRUCT_OWNER, eCharPtr, "", eNormal)) ||
                                (PE_OK!=pUnit->SetProperty(PRP_STRUCT_ID,    eLong,  0, eNormal)) )
                                SHOW_WARN_CONTINUE(NOSETUNIT << pUnit->Id << BUG);
                            //TODO: set new owner HERE
                        }
                        break;


                    case O_ENTER:
                        if (TurnSequence::SQ_ENTER==sequence)
                        {
                            p        = SkipSpaces(N1.GetToken(p, " \t", ch, TRIM_ALL));
                            n1       = atol(N1.GetData());
                            pStruct  = land_control::get_struct(pLand, n1);
                            if (pStruct)
                            {
                                unit_control::set_structure(pUnit, pStruct->Id, pStruct->OwnerUnitId == 0);
                                if (0==pStruct->OwnerUnitId)
                                {
                                    pStruct->OwnerUnitId = pUnit->Id;
                                    S1 = YES;
                                }
                                else
                                    S1.Empty();
                                    
                                if ( (PE_OK!=pUnit->SetProperty(PRP_STRUCT_ID,  eLong, 0,                  eNormal)) ||
                                    (PE_OK!=pUnit->SetProperty(PRP_STRUCT_ID,  eLong, (void*)pStruct->Id, eNormal)) ||
                                    (PE_OK!=pUnit->SetProperty(PRP_STRUCT_NAME,  eCharPtr, "",                      eNormal)) ||
                                    (PE_OK!=pUnit->SetProperty(PRP_STRUCT_NAME,  eCharPtr, pStruct->name_.c_str(), eNormal)) ||
                                    (PE_OK!=pUnit->SetProperty(PRP_STRUCT_OWNER, eCharPtr, S1.GetData(), eNormal)) )
                                    SHOW_WARN_CONTINUE(NOSETUNIT << pUnit->Id << BUG);
                            }
                            else
                                SHOW_WARN_CONTINUE(" - Invalid structure number " << n1);
                        }
                        break;


                    case O_PROMOTE:
                        if (TurnSequence::SQ_PROMOTE==sequence)
                            RunOrder_Promote(Line, ErrorLine, skiperror, pUnit, pLand, p);
                        break;


                    case O_MOVE:
                    case O_SAIL:
                    case O_ADVANCE:
                        if (TurnSequence::SQ_MOVE==sequence)
                        {
                            if (isSimCmd)
                            {//I have no idea what is the Sim.
                                destination_land = GetLandFlexible(wxString::FromUTF8(p));
                            }
                            else
                            {
                                RunOrder_Move(Line, ErrorLine, skiperror, pUnit, pLand, p, X, Y, LocA3, order);
                            }
                        }
                        break;

                    case O_PILLAGE:
                        //if (TurnSequence::SQ_PILLAGE ==sequence )
                            //pUnit->Flags |=  UNIT_FLAG_PILLAGING;
                        break;

                    case O_TAX:
                        //if (TurnSequence::SQ_TAX ==sequence )
                            //pUnit->Flags |=  UNIT_FLAG_TAXING;
                        break;

                    case O_ENTERTAIN:
                        //if (TurnSequence::SQ_ENTERTAIN ==sequence )
                            //pUnit->Flags |=  UNIT_FLAG_ENTERTAINING;
                        break;

                    case O_WORK:
                        //if (TurnSequence::SQ_WORK ==sequence)
                            //pUnit->Flags |=  UNIT_FLAG_WORKING;
                        break;

                    case O_BUILD:
                        if (TurnSequence::SQ_BUILD==sequence)
                            pUnit->Flags |= UNIT_FLAG_PRODUCING;
                        break;

                    case O_PRODUCE:
                        //if (TurnSequence::SQ_PRODUCE==sequence)
                        //    RunOrder_Produce(Line, ErrorLine, skiperror, pUnit, pLand, p);
                        break;

                    default:
                        break;
                }//switch (order)

                // copy commands to the new unit
                if (pUnitMaster && (TurnSequence::SQ_LAST==sequence) && (O_FORM != order) && !errors)
                    pUnit->Orders << Line << EOL_SCR;

            } // commands loop


            //if(TurnSequence::SQ_TEACH==sequence)  // we have to collect all the students, then teach
            //{
            //    OrderProcess_Teach(skiperror, pUnit);
            //}
                

            if (TurnSequence::SQ_MOVE==sequence && !pUnit->pMovement && destination_land != nullptr)
            {
                int movementMode;
                bool noCross;
                wxString Log;
                GetMovementMode(pUnit, movementMode, noCross, O_MOVE);
                wxString route = RoutePlanner::GetRoute(pLand, destination_land, movementMode, RoutePlanner::ROUTE_MARKUP_TURN, noCross, Log);
                if (Log.size() > 0)
                {
                    m_sOrderErrors << Log.c_str();
                }
                if (!route.IsEmpty())
                {
                    if (route.Length() > 66)
                    {
                        // only supply first move
                        RoutePlanner::GetFirstMove(route);
                    }

                    route.Replace("_ ", "", true);
                    RunOrder_Move(Line, ErrorLine, skiperror, pUnit, pLand, (const char *)route.ToUTF8(), X, Y, LocA3, O_MOVE);
                    route = wxString::Format("MOVE%s", route);

                    pUnit->Orders.TrimRight(TRIM_ALL);
                    pUnit->Orders << EOL_SCR;
                    pUnit->Orders << (const char *)route.ToUTF8();
                    Line = (const char *)route.ToUTF8();
                }
            }
            //if (TurnSequence::SQ_MAX-1==sequence)
            //{
            //    // set land flags in the last sequence, so all unit flags are already applied
            //    if ((pUnit->Flags & UNIT_FLAG_TAXING) && !(pUnit->Flags & UNIT_FLAG_GIVEN))
            //        pLand->Flags |= LAND_TAX_NEXT;
            //    if ((pUnit->Flags & UNIT_FLAG_PRODUCING) && !(pUnit->Flags & UNIT_FLAG_GIVEN))
            //        pLand->Flags |= LAND_TRADE_NEXT;
            //}

        }   // units loop
        if (TurnSequence::SQ_BUY==sequence)
        {
            if (m_EconomyShareAfterBuy)
            {
                RunOrder_ShareSilver(Line, ErrorLine, skiperror, pLand, SHARE_BUY, wxT("purchases"));
            }
        }
        if (TurnSequence::SQ_STUDY==sequence)
        {
            if (m_EconomyShareMaintainance)
            {
                RunOrder_ShareSilver(Line, ErrorLine, skiperror, pLand, SHARE_STUDY, wxT("study"));
            }
        }
        if (TurnSequence::SQ_MAINTENANCE==sequence)
        {
            if (m_EconomyMaintainanceCosts)
            {
                RunOrder_Upkeep(pLand);
            }
            if (m_EconomyShareMaintainance)
            {
                RunOrder_ShareSilver(Line, ErrorLine, skiperror, pLand, SHARE_UPKEEP, wxT("maintainance"));
            }

            //calculate silver for incoming units
            CBaseColl           arriving_units;
            CUnit*              unit_temp;
            gpApp->GetUnitsMovingIntoHex(pLand->Id, arriving_units);
            for (int i=0; i<arriving_units.Count(); ++i)
            {
                unit_temp = (CUnit*)arriving_units.At(i);
                if (unit_temp != NULL)
                {
                    pLand->current_state_.economy_.maintenance_ += unit_control::get_upkeep(unit_temp);
                    pLand->current_state_.economy_.moving_in_ += unit_control::get_item_amount(unit_temp, PRP_SILVER);
                }
            }

            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {
                if (!unit->IsOurs)
                    return;

                long men_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                if (men_amount > 0) 
                {
                    //moving out
                    if (unit->pMovement != NULL && unit->pMovement->Count() > 0)
                        pLand->current_state_.economy_.moving_out_ += unit_control::get_item_amount(unit, PRP_SILVER);
                    else 
                    {//maintenance
                        pLand->current_state_.economy_.maintenance_ += unit_control::get_upkeep(unit);
                    }
                }
            });        

        }
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

    land_control::structures::update_struct_weights(pLand);
    pLand->SetFlagsFromUnits();
    OrderErrFinalize();
}

//-------------------------------------------------------------

int CAtlaParser::CountMenWithFlag(CLand * pLand, int unitFlag) const
{
    EValueType          type;
    CUnit * pUnit;
    long nmen;
    int headCount = 0;
    int skill;

    for (int unitidx=0; unitidx<pLand->UnitsSeq.Count(); ++unitidx)
    {
        pUnit = (CUnit*)pLand->UnitsSeq.At(unitidx);
        if (pUnit->Flags & unitFlag)
        {
            if (pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal) && (nmen>0))
            {
                if (UNIT_FLAG_ENTERTAINING == unitFlag)
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
    CUnit * pUnit;
    long nmen;

    int unitReceives;
    int skill;

    if (menCount == 0)
        return;

    for (int unitidx=0; unitidx<pLand->UnitsSeq.Count(); ++unitidx)
    {
        pUnit = (CUnit*)pLand->UnitsSeq.At(unitidx);
        if (pUnit->Flags & unitFlag)
        {
            if (pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal) && (nmen>0))
            {
                if (UNIT_FLAG_ENTERTAINING == unitFlag)
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
        OrderError(error.type_, land, error.unit_, error.message_);
    }
}

void CAtlaParser::CheckOrder_LandWork(CLand *land)
{
    std::vector<unit_control::UnitError> errors;
    land_control::check_land_workers(land, errors);
    for (const auto& error : errors)
    {
        OrderError(error.type_, land, error.unit_, error.message_);
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
        OrderError(error.type_, land, error.unit_, error.message_);
    }
}

void CAtlaParser::CheckOrder_LandEntertain(CLand *land)
{

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
        OrderError(error.type_, land, error.unit_, error.message_);
    }    
}

//-------------------------------------------------------------


void CAtlaParser::RunOrder_Upkeep(CUnit * pUnit, int turns)
{
    EValueType          type;
    long nmen;
    int Maintainance;
    const char * leadership;

    if (pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal) && (nmen>0))
    {
        if (unit_control::is_leader(pUnit))
            Maintainance = nmen * game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_LEADER);
        else
            Maintainance = nmen * game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_PEASANT);

        unit_control::modify_silver(pUnit, -Maintainance * turns, "upkeep");
    }
}

void CAtlaParser::RunOrder_Upkeep(CLand * pLand)
{
    CUnit * pUnit;

    for (int unitidx=0; unitidx<pLand->UnitsSeq.Count(); ++unitidx)
    {
        pUnit = (CUnit*)pLand->UnitsSeq.At(unitidx);
        if (pUnit->FactionId == m_CrntFactionId)
        {
            RunOrder_Upkeep(pUnit, 1);
        }
    }
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_ShareSilver (CStr & LineOrig, CStr & ErrorLine, BOOL skiperror, CLand * pLand, SHARE_TYPE shareType, wxString shareName)
{
    EValueType          type;
    CUnit * pUnit;
    long silverNeeded = 0;
    long unitSilver;
    long shareSilver;
    long silverNeededOrig;
    // CStr ErrorLine;
    CStr Line;
    CUnit * displayUnit = NULL;

    // Share silver between units which have a shortage.
    // Normally only silver is shared between units with the share flag set.
    // When shareAll is set silver will be shared by all units (such as for upkeep costs).

    for (int unitidx=0; unitidx<pLand->UnitsSeq.Count(); ++unitidx)
    {
        pUnit = (CUnit*)pLand->UnitsSeq.At(unitidx);
        if (pUnit->IsOurs && !IS_NEW_UNIT(pUnit))
        {
            displayUnit = pUnit;
        }
        if (shareType == SHARE_BUY || !pUnit->pMovement || pUnit->pMovement->Count() == 0)
        if (pUnit->GetProperty(PRP_SILVER, type, (const void *&)unitSilver, eNormal))
        {
            if (unitSilver < 0)
            {
                silverNeeded -= unitSilver;
            }
        }
    }

    if (!silverNeeded) return;
    silverNeededOrig = silverNeeded;

    for (int unitidx=0; unitidx<pLand->UnitsSeq.Count(); ++unitidx)
    {
        pUnit = (CUnit*)pLand->UnitsSeq.At(unitidx);
        if (!pUnit->IsOurs) continue;
        if (shareType == SHARE_BUY || !pUnit->pMovement || pUnit->pMovement->Count() == 0)
        if (shareType == SHARE_UPKEEP || pUnit->Flags & UNIT_FLAG_SHARING)
        {
            unitSilver = unit_control::get_item_amount(pUnit, PRP_SILVER);
            if (unitSilver > 0)
            {
                shareSilver = std::min(unitSilver, silverNeeded);
                silverNeeded -= shareSilver;
                unit_control::modify_silver(pUnit, -shareSilver, "sharing");
                if (!silverNeeded)
                    break;
            }
        }
    }

    long silverAvailable = silverNeededOrig - silverNeeded;
    long shortage = silverNeeded;

    for (int unitidx=0; unitidx<pLand->UnitsSeq.Count(); ++unitidx)
    {
        pUnit = (CUnit*)pLand->UnitsSeq.At(unitidx);
        if (!pUnit->IsOurs) continue;
        if (shareType == SHARE_BUY || !pUnit->pMovement || pUnit->pMovement->Count() == 0)
        if (pUnit->GetProperty(PRP_SILVER, type, (const void *&)unitSilver, eNormal))
        {
            if (unitSilver < 0)
            {
                shareSilver = std::min(-unitSilver, silverAvailable);
                silverAvailable -= shareSilver;
                unitSilver += shareSilver;
                unit_control::modify_silver(pUnit, shareSilver, "sharing");
            }
        }
    }

    if (shortage > 0)
    {
        if (displayUnit) pUnit = displayUnit;
        CStr sCoord;
        ComposeLandStrCoord(pLand, sCoord);
        wxString message = wxT(" - ") + wxString::FromUTF8(sCoord.GetData());
        message += wxString::Format(" - Shortage of %d silver for %s.", shortage, shareName);
        SHOW_WARN(message.ToUTF8());
    }
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandStudyTeach(CLand* land)
{
    std::vector<unit_control::UnitError> errors;
    std::unordered_map<long, land_control::Student> students_of_the_land;
    students_of_the_land = land_control::get_land_students(land, errors);

    //Economy
    for (const auto& student : students_of_the_land)
        land->current_state_.economy_.study_expenses_ += student.second.skill_price_ * student.second.man_amount_;

    //teaching orders -- no need to wait for TurnSequence::Teach
    land_control::update_students_by_land_teachers(land, students_of_the_land, errors);
    for (auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.message_);
    }
        

    //updating students
    for (auto& stud : students_of_the_land)
    {
        std::string skill;
        long study_lvl_goal;
        if (!orders::parser::specific::parse_study(stud.second.order_, skill, study_lvl_goal))
        {
            OrderError("Error", land, stud.second.unit_, "study order " + stud.second.order_->original_string_ + " is not valid!");
            continue;
        }

        stud.second.unit_->skills_[skill] += 30 + stud.second.days_of_teaching_;
        stud.second.unit_->skills_[skill] = std::min(stud.second.unit_->skills_[skill], stud.second.max_days_);
        
        //calculate property of teaching
        long room_for_teaching = std::max(stud.second.max_days_ - stud.second.cur_days_ - 30, (long)0);
        long space_for_teaching = std::min(room_for_teaching, (long)30);
        space_for_teaching = std::max(space_for_teaching - stud.second.days_of_teaching_, (long)0);                
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
                OrderError("Error", land, unit, " more than 1 assassinate order");                        
                return;
            }
            long target_id;
            if (!orders::parser::specific::parse_assassinate(orders[0], target_id))
            {
                OrderError("Error", land, unit, "assassinate: wrong format");
                return;
            }

            CUnit* target_unit = land_control::find_first_unit_if(land, [&](CUnit* unit) {
                return unit->Id == target_id;
            });
            if (target_unit == nullptr)
            {
                OrderError("Error", land, unit, "assassinate: not existing target");
                return;
            }
            if (target_unit != nullptr && target_unit->FactionId == unit->FactionId)
            {
                OrderError("Error", land, unit, "assassinate: target belongs to the same faction");
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
            std::vector<long> targets;
            auto orders = orders::control::retrieve_orders_by_type(orders::Type::O_ATTACK, unit->orders_);
            for (auto& order : orders)
            {
                if (!orders::parser::specific::parse_attack(order, targets))
                {
                    OrderError("Error", land, unit, "Couldn't parse attack order: " + order->original_string_);
                    continue;
                }
            }
            for (auto& target : targets)
            {
                CUnit* target_unit = land_control::find_first_unit_if(land, [&](CUnit* unit) {
                    return unit->Id == target;
                });
                if (target_unit == nullptr)
                {
                    std::string mess = "attack: "+std::to_string(target)+"not existing target!";
                    OrderError("Error", land, unit, mess.c_str());
                    continue;
                }
                if (target_unit != nullptr && target_unit->FactionId == unit->FactionId)
                {
                    std::string mess = "attack: "+std::to_string(target)+"belongs to the same faction!";
                    OrderError("Error", land, unit, mess.c_str());
                }
                else 
                {
                    target_unit->impact_description_.push_back("Is attacked by " + std::string(unit->Name.GetData()));
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
                OrderError("Error", land, unit, "steal: more than 1 stealing order");
                return;
            }
            long target_id;
            std::string item;
            if (!orders::parser::specific::parse_steal(orders[0], target_id, item))
            {
                OrderError("Error", land, unit, "steal: wrong format, should be 'STEAL XXX ITEM'!");
                return;
            }
            CUnit* target_unit = land_control::find_first_unit_if(land, [&](CUnit* unit) {
                return unit->Id == target_id;
            });
            if (target_unit == nullptr)
            {
                OrderError("Error", land, unit, "steal: not existing target");
                return;
            }
            if (target_unit != nullptr && target_unit->FactionId == unit->FactionId)
            {
                OrderError("Error", land, unit, "steal: target belongs to the same faction");
                return;
            } 
            else if (unit_control::get_item_amount(target_unit, item) == 0 && 
                     item_control::weight(item) > 0)
            {
                OrderError("Error", land, unit, "steal: "+std::to_string(target_unit->Id)+" has no: " + item);
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
template<orders::Type TYPE> void CAtlaParser::RunOrder_AOComments(CLand* land)
{
    //std::map<long, std::chrono::microseconds> time_points;

    land_control::perform_on_each_unit(land, [&](CUnit* unit) {
        //auto start = std::chrono::high_resolution_clock::now();

        auto orders_by_type = orders::control::retrieve_orders_by_type(TYPE, unit->orders_);

        //time_points[0] += duration_cast<microseconds>(std::chrono::high_resolution_clock::now() - start);
        //need to find comments which are commented out actual orders
        
        auto order_comments = orders::control::retrieve_orders_by_type(orders::Type::O_COMMENT, unit->orders_);
        for (auto& order_comment : order_comments)
        {
            auto it = order_comment->comment_.begin();
            while (it < order_comment->comment_.end() && (*it == '@' || *it == ';'))
                ++it;
            
            auto it_end = it;
            while (it_end < order_comment->comment_.end() && std::isalpha(*it_end))
                ++it_end;
            
            if (orders::types_mapping[std::string(it, it_end)] == TYPE)
                orders_by_type.push_back(order_comment);
        }

        /*auto order_comments = orders::control::retrieve_orders_by_type(orders::Type::O_COMMENT, unit->orders_);
        for (auto& order_comment : order_comments)
        {
            auto it = order_comment->comment_.begin();
            while (it < order_comment->comment_.end() && (*it == '@' || *it == ';'))
                ++it;
            
            auto real_order = orders::parser::parse_line_to_order(std::string(it, order_comment->comment_.end()));
            if (real_order->type_ == order_type)
            {
                orders_by_type.push_back(order_comment);
            }
        }*/

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
                    OrderError("error", land, unit, "autoorders: couldn't parse get: " + order->comment_);
                    continue;
                }

                if (amount == 0)
                    continue;

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
                    case orders::autoorders::LogicAction::ERROR: {
                        if (result)
                            OrderError("Warning", land, unit, "autoorders: warning condition altered: " + order->comment_);
                        };
                        break;
                    case orders::autoorders::LogicAction::NONE:
                        break;
                }

                for (const auto& error : errors)
                    OrderError(error.type_, land, error.unit_, error.message_);
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
        auto comment_orders = orders::control::retrieve_orders_by_type(orders::Type::O_COMMENT_AUTONAME, unit->orders_);
        for (auto& comment_order : comment_orders)
        {
            std::string new_name = "@;;" + autonaming::generate_unit_autoname(land, unit);
            if (stricmp(&new_name[1], comment_order->comment_.c_str()) != 0)//[1] to ignore @ at comparation
            {
                comment_order->comment_.insert(0, "%DEL%");
                orders::control::remove_orders_by_comment(unit, "%DEL%");
                orders::control::add_order_to_unit(new_name, unit);
            }
        }
    });
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_Name(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params)
{
    CStr  What, Name, NewName;

    params = SkipSpaces(What.GetToken(params, ' ', TRIM_ALL));
    params = Name.GetToken(params, ' ', TRIM_ALL);

    if (0==stricmp(What.GetData(), "unit"))
    {
        if (IS_NEW_UNIT(pUnit))
        {
            NewName << Name << " - " << "NEW " << (long)REVERSE_NEW_UNIT_ID(pUnit->Id);
            Name = NewName;
        }
        pUnit->SetName(Name.GetData());
    }
}


//-------------------------------------------------------------

BOOL CAtlaParser::CheckResourcesForProduction(CUnit * pUnit, CLand * pLand, CStr & Error)
{
    BOOL                Ok = TRUE;
    unsigned int        x;
    int                 i;
    TProdDetails        details;
    CUnit             * pSharer;
    const char        * propname;
    BOOL                SharerFound;
    EValueType          type;
    const void        * value;
    long                nlvl  = 0;
    long                ntool = 0;
    long                ncanproduce = 0;
    long                nres  = 0;
    long                nrequired = 0;
    long                nmen  = 0;
    int                 no;
    CStr                S;

    Error.Empty();
    if ((pUnit->Flags & UNIT_FLAG_PRODUCING) && !pUnit->ProducingItem.IsEmpty())
    {
        gpDataHelper->GetProdDetails (pUnit->ProducingItem.GetData(), details);

        if (!pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal)  || (nmen<=0))
        {
            Error << " - There are no men in the unit!";
            Ok = FALSE;
        }

        if (details.skill_name_.empty() || details.per_month_<=0)
        {
            Error << " - Production requirements for item '" << pUnit->ProducingItem << "' are not configured! ";
            Ok = FALSE;
        }

        // check skill level
        S << details.skill_name_.c_str() << PRP_SKILL_POSTFIX;
        if (pUnit->GetProperty(S.GetData(), type, value, eNormal) && (eLong==type) )
        {
            nlvl = (long)value;
            if (nlvl < details.skill_level_)
            {
                Error << " - Skill " << details.skill_name_.c_str() << " level " << details.skill_level_ << " is required for production";
                Ok = FALSE;
            }
        }
        else
        {
            Error << " - Skill " << details.skill_name_.c_str() << " is required for production";
            Ok = FALSE;
        }


        if (!details.tool_name_.empty())
            if (!pUnit->GetProperty(details.tool_name_.c_str(), type, (const void *&)ntool, eNormal) || eLong!=type )
                ntool = 0;
        if (ntool > nmen)
            ntool = nmen;

        ncanproduce = (long)((((double)nmen)*nlvl + ntool*details.tool_plus_) / details.per_month_);

        /*
        for (x=0; x<sizeof(details.resname)/sizeof(*details.resname); x++)
        {
            SharerFound = FALSE;
            if (!details.resname[x].IsEmpty())
            {
                if (!pUnit->GetProperty(details.resname[x].GetData(), type, (const void *&)nres, eNormal) || eLong!=type )
                    nres = 0;

                // how many do we need?
                nrequired = (ncanproduce * details.resamt[x] );

                if (nrequired > nres)
                {
                    // Now check if there are our units SHARING the resource.
                    // We will not give a damn if they have enough or not though :)
                    // since we do not know how does server disribute the resources
                    for (i=0; i<pLand->Units.Count(); i++)
                    {
                        pSharer = (CUnit*)pLand->Units.At(i);
                        if (pSharer->FactionId == pUnit->FactionId && (pSharer->Flags & UNIT_FLAG_SHARING))
                        {
                            no = 0;
                            propname = pSharer->GetPropertyName(no);
                            while (propname)
                            {
                                if (0==stricmp(propname, details.resname[x].GetData()))
                                {
                                    SharerFound = TRUE;
                                    break;
                                }

                                propname = pSharer->GetPropertyName(++no);
                            }
                        }
                    }

                    if (!SharerFound)
                    {
                        if (0==nres)
                            Error << " - " << details.resname[x] << " needed for production!";
                        else
                            Error << " - " << (nrequired - nres) << " more units of " << details.resname[x] << " needed for production at full capacity (" << ncanproduce << ")!";
                        Ok = FALSE;
                    }
                }
            }
        }*/
    }

    return Ok;
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandProduce(CLand* land)
{
    std::vector<CUnit*> producers;
    land_control::get_units_if(land, producers, [](CUnit* unit) {
        return orders::control::has_orders_with_type(orders::Type::O_PRODUCE, unit->orders_);
    });

    struct ProducerInfo
    {
        CUnit* producer_;
        long item_amount_;
    };

    std::map<std::string, std::vector<ProducerInfo>> land_producers_info;//collection for each resource
    for (CUnit* producer : producers)
    {//check requirements and predict
        long man_amount = unit_control::get_item_amount_by_mask(producer, PRP_MEN);
        if (man_amount <= 0)
        {
            OrderError("Warning", land, producer, "produce: no man in the unit!");
        }

        auto produce_orders = orders::control::retrieve_orders_by_type(orders::Type::O_PRODUCE, producer->orders_);
        if (produce_orders.size() > 1)
        {//check amount of produce orders
            OrderError("Error", land, producer, "produce: more than one production order!");
            continue;
        }
        std::string item;
        long goal_amount;
        if (!orders::parser::specific::parse_produce(produce_orders[0], item, goal_amount))
        {
            OrderError("Error", land, producer, "produce: wrong format!");
            continue;
        }

        TProdDetails prod_details;
        gpDataHelper->GetProdDetails(item.c_str(), prod_details);
        if (prod_details.skill_name_.empty() || prod_details.per_month_<=0)
        {//check details settings
            std::string mess = "produce: production requirements for '" + item + "' are not configured!";
            OrderError("Warning", land, producer, mess);
            continue;
        }

        long skill_days = unit_control::get_current_skill_days(producer, prod_details.skill_name_);
        long skill_lvl = skills_control::get_skill_lvl_from_days(skill_days);
        if (skill_lvl < prod_details.skill_level_)
        {//check skill requirements
            std::string mess = "produce: skill '" + prod_details.skill_name_ +
                "' (" + std::to_string(prod_details.skill_level_) + ") is required to produce";
            OrderError("Error", land, producer, mess);
            continue;
        }

        long tools_plus(0);
        if (!prod_details.tool_name_.empty())
        {
            long tool_amount = unit_control::get_item_amount(producer, prod_details.tool_name_);
            tool_amount = std::min(tool_amount, man_amount);
            tools_plus = tool_amount * prod_details.tool_plus_;
        }

        //how many this unit can produce
        long basic_produce_power = (long)((man_amount*skill_lvl + tools_plus)/prod_details.per_month_);
        
        if (goal_amount > 0)//restriction by goal
            basic_produce_power = std::min(goal_amount, basic_produce_power);
        //produce from existing resources (if requires)
        for (const auto& req_resource : prod_details.req_resources_)
        {
            long producer_has = unit_control::get_item_amount(producer, req_resource.first);
            if (producer_has / req_resource.second < (long)(basic_produce_power))
            {//restriction by existing resources
                basic_produce_power = (long)(producer_has / req_resource.second);
                producer->impact_description_.push_back("produce: "+req_resource.first+" " +
                    std::to_string(producer_has) + " is enough just for "+ 
                    std::to_string(basic_produce_power));
            }
        }

        producer->Flags |= UNIT_FLAG_PRODUCING;
        land_control::set_produced_items(land->current_state_, item, basic_produce_power);
        land_producers_info[item].push_back({producer, basic_produce_power});
    }

    //resolve land request (we had to collect all requests before calculation)
    for (auto& res_to_producers : land_producers_info)
    {
        if (std::find_if(land->current_state_.resources_.begin(), 
                         land->current_state_.resources_.end(), 
                         [&](const CItem& item) {
                return item.code_name_ == res_to_producers.first;
            }) != land->current_state_.resources_.end())
        {//it's a request to land resources
            long land_amount = land_control::get_resource(land->current_state_, res_to_producers.first);
            long requested_amount = land_control::get_produced_items(land->current_state_, res_to_producers.first);

            //fill producers description regarding their requests
            double leftovers(0.0);
            std::sort(res_to_producers.second.begin(), res_to_producers.second.end(), 
            [](const ProducerInfo& item1, const ProducerInfo& item2){
                return item1.item_amount_ > item2.item_amount_;
            });
            for (const ProducerInfo& prod_info : res_to_producers.second)
            {
                if (land_amount < requested_amount)
                {
                    double intpart;
                    leftovers += modf(((double)land_amount / requested_amount) * prod_info.item_amount_, &intpart);
                    //prod_info.item_amount_ = intpart;
                    if (leftovers >= 0.999999)
                    {
                        intpart += 1;
                        leftovers -= 1.0;
                    }
                    unit_control::modify_item_by_produce(prod_info.producer_, res_to_producers.first, intpart);
                }
                else
                    unit_control::modify_item_by_produce(prod_info.producer_, res_to_producers.first, prod_info.item_amount_);
            }
        }
        else
        {//it's a production of items from other items not touching land 
         //or request to lands resource which doesnt presented here
            TProdDetails prod_details;
            gpDataHelper->GetProdDetails(res_to_producers.first.c_str(), prod_details);

            for (const ProducerInfo& prod_info : res_to_producers.second)
            {
                if (prod_details.req_resources_.size() == 0)
                {
                    std::string mess = "produce: '" + res_to_producers.first +
                        "' is not a craft, and not presented among land resources";
                    OrderError("Warning", land, prod_info.producer_, mess);
                    continue;                
                }

                unit_control::modify_item_by_produce(prod_info.producer_, 
                                                     res_to_producers.first, 
                                                     prod_info.item_amount_);

                for (const auto& req_resource : prod_details.req_resources_)
                {
                    unit_control::modify_item_by_produce(prod_info.producer_, 
                                                            req_resource.first, 
                                                            (long)(-prod_info.item_amount_ * req_resource.second));
                }
            }
        }
    }
}

void CAtlaParser::RunOrder_Produce(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params)
{
    /*
    TProdDetails        details;
    const void        * value;
    EValueType          type;
    long                nlvl  = 0;
    long                nmen  = 0;
    CStr                S(32), Product(32), Error;

    do
    {
        params  = ReadPropertyName(params, Product);
        pUnit->Flags        |= UNIT_FLAG_PRODUCING;
        pUnit->ProducingItem = gpDataHelper->ResolveAlias(Product.GetData());
        Product = pUnit->ProducingItem;
        gpDataHelper->GetProdDetails (Product.GetData(), details);

        if (!pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal)  || (nmen<=0))
            SHOW_WARN_CONTINUE(" - There are no men in the unit!");

        if (details.skillname.IsEmpty() || details.months<=0)
            SHOW_WARN_CONTINUE(" - Production requirements for item '" << Product << "' are not configured! ");

        // check skill level
        S << details.skillname << PRP_SKILL_POSTFIX;
        if (pUnit->GetProperty(S.GetData(), type, value, eNormal) && (eLong==type) )
        {
            nlvl = (long)value;
            if (nlvl < details.skilllevel)
                SHOW_WARN_CONTINUE(" - Skill " << details.skillname << " level " << details.skilllevel << " is required for production");
        }
        else
            SHOW_WARN_CONTINUE(" - Skill " << details.skillname << " is required for production");

        // check required resources
        if (gpDataHelper->ImmediateProdCheck())
            if (!CheckResourcesForProduction(pUnit, pLand, Error))
                SHOW_WARN_CONTINUE(Error.GetData());

    } while (FALSE);*/
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
    EValueType          type;
    const void        * value;

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
        OrderError(error.type_, land, error.unit_, error.message_);
    }
}

//-------------------------------------------------------------
void CAtlaParser::RunOrder_LandGive(CLand* land, CUnit* up_to)
{
    for (size_t i = 0; i < land->UnitsSeq.Count(); i++)
    {
        CUnit* unit = (CUnit*)land->UnitsSeq.At(i);
        if (up_to != nullptr && unit->Id == up_to->Id)
            return;

        if (!orders::control::has_orders_with_type(orders::Type::O_GIVE, unit->orders_))
            continue;

        auto give_orders = orders::control::retrieve_orders_by_type(orders::Type::O_GIVE, unit->orders_);
        for (const auto& give_order : give_orders)
        {
            long target_id, target_faction_id, amount, except;
            std::string item_code;
            if (!orders::parser::specific::parse_give(give_order, target_id, target_faction_id, amount, item_code, except))
            {
                OrderError("Error", land, unit, "give: couldn't parse order: "+give_order->original_string_);
                continue;
            }

            if (target_id < 0)//NEW X
            {
                if (target_faction_id == 0)//faction wasn't specified
                    target_id = NEW_UNIT_ID(abs(target_id), unit->FactionId);
                else
                    target_id = NEW_UNIT_ID(abs(target_id), target_faction_id);
            }

            CUnit* target_unit = nullptr;
            if (target_id > 0)
            {
                target_unit = land_control::find_first_unit_if(land, [&](CUnit* cur_unit) {
                    return cur_unit->Id == target_id;
                });
                if (target_unit == nullptr)
                {
                    OrderError("Error", land, unit, "give: can't locate target unit: "+give_order->original_string_);
                    continue;
                }
            }

            if (stricmp(item_code.c_str(), "UNIT") == 0)
            {
                if (target_unit == nullptr)
                    unit->impact_description_.push_back("give: drop off unit");
                else if (target_unit->FactionId == unit->FactionId)
                    OrderError("Error", land, unit, "give: transfer unit to yourself: "+give_order->original_string_);
                else 
                    unit->impact_description_.push_back("give: transfer unit to faction "+std::to_string(target_unit->FactionId));
                continue;
            }

            if (unit_control::get_item_amount(unit, item_code) == 0)
            {//can't find items in unit
                std::string another_name, another_plural;
                if (gpApp->ResolveAliasItems(item_code, item_code, another_name, another_plural))
                {
                    auto struct_parameters = game_control::get_game_config<std::string>(SZ_SECT_STRUCTS, another_name.c_str());
                    if (struct_parameters.size() > 0)
                    {//its a structure
                        //TODO
                    }
                }
                OrderError("Error", land, unit, "give: no items to give in order: "+give_order->original_string_);
                OrderError("Warning", land, unit, "give: if you try to give a ship, please, add it to ALIASES_ITEMS related to object in STRUCTURES (nut supported yet)");
                OrderError("Warning", land, unit, "give: if you try to give item class, please, add it to UNIT_PROPERTY_GROUPS of your config (nut supported yet)");
                continue;
            }

            if (except >= 0)//ALL + optional(EXCEPT)
                amount = std::max(unit_control::get_item_amount(unit, item_code) - except, (long)0);

            if (amount > unit_control::get_item_amount(unit, item_code))
            {
                OrderError("Warning", land, unit, "give: "+std::to_string(unit_control::get_item_amount(unit, item_code))+
                    " instead of "+std::to_string(amount)+", no more items");
                amount = std::min(amount, unit_control::get_item_amount(unit, item_code));
            }

            if (gpDataHelper->IsMan(item_code.c_str()))
            {
                unit_control::modify_man_from_unit(unit, target_unit, item_code, -amount);
                if (target_unit != nullptr)
                    unit_control::modify_man_from_unit(target_unit, unit, item_code, amount);
            }
            else
            {
                unit_control::modify_item_from_unit(unit, target_unit, item_code, -amount);
                if (target_unit != nullptr)
                    unit_control::modify_item_from_unit(target_unit, unit, item_code, amount);
            }
        }
    }    
}




//-------------------------------------------------------------

BOOL CAtlaParser::FindTargetsForSend(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char *& params, CUnit *& pUnit2, CLand *& pLand2)
{
    BOOL                Ok = FALSE;
    CBaseObject         Dummy;
    int                 idx;
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
                if (!GetTargetUnitId(params, pUnit->FactionId, target_id))
                    SHOW_WARN_CONTINUE(" - Invalid unit Id");
                if (target_id==pUnit->Id)
                    SHOW_WARN_CONTINUE(" - Giving to yourself");
                if (0 == target_id)
                    SHOW_WARN_CONTINUE(" - Invalid target unit");
                Dummy.Id = target_id;
                if (pLand2->Units.Search(&Dummy, idx))
                    pUnit2 = (CUnit*)m_Units.At(idx);
                if (!pUnit2)
                    SHOW_WARN_CONTINUE(" - Invalid target unit");
            }
        }
        else if (0==stricmp("UNIT", S1.GetData()))
        {
            if (!GetTargetUnitId(params, pUnit->FactionId, target_id))
                SHOW_WARN_CONTINUE(" - Invalid unit Id");
            if (target_id==pUnit->Id)
                SHOW_WARN_CONTINUE(" - Giving to yourself");
            if (0 == target_id)
                SHOW_WARN_CONTINUE(" - Invalid target unit");

            Dummy.Id = target_id;
            if (m_Units.Search(&Dummy, idx))
                pUnit2 = (CUnit*)m_Units.At(idx);
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

void CAtlaParser::RunOrder_Take(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params, BOOL IgnoreMissingTarget)
{
    EValueType          type;
    long                n1;
    CBaseObject         Dummy;
    int                 idx;
    CUnit             * pUnit2 = NULL;
    const void        * value;
    const void        * value2;

//    int               * weights;    // calculate weight change while giving
//    const char       ** movenames;
//    int                 movecount;

    CStr                Item;
    int                 amount;
    char                ch;

    do
    {
        // TAKE FROM <TARGET> (<AMOUNT>|ALL) <ITEM> [EXCEPT <AMOUNT>]
        // Where:
        //   <TARGET> is
        //     <UNIT ID>                          existing unit
        //     NEW <UNIT ID>                      new own unit
        //     FACTION <FACTION ID> NEW <UNIT ID> other faction new unit

        // Remove FROM token from the params
        params = SkipSpaces(Item.GetToken(params, " \t", ch, TRIM_ALL));

        if (0!=stricmp("FROM", Item.GetData()))
            SHOW_WARN_CONTINUE(" - Invalid TARGET command");

        if (!GetTargetUnitId(params, pUnit->FactionId, n1))
            SHOW_WARN_CONTINUE(" - Invalid unit id");
        if (n1==pUnit->Id)
        {
            SHOW_WARN_CONTINUE(" - Taking from yourself");
        }            
        if (0!=n1)
        {
            Dummy.Id = n1;
            if (pLand->Units.Search(&Dummy, idx))
                pUnit2 = (CUnit*)pLand->Units.At(idx);
            else
            {
                SHOW_WARN_CONTINUE(" - Can not locate target unit");
            }
                
        }
        else
            SHOW_WARN_CONTINUE(" - Invalid unit id");

        if (pUnit2 && pUnit->FactionId!=pUnit2->FactionId)
            SHOW_WARN_CONTINUE(" - Target unit must belong to the same faction");

        if (pUnit2)
        {
            if (GetItemAndAmountForGive(Line, ErrorLine, skiperror, pUnit, pLand, params, Item, amount, "take", pUnit2) )
            {
                if (0==stricmp("UNIT", Item.GetData()))
                    SHOW_WARN_CONTINUE(" - Taking unit is not allowed");

                if (!pUnit2->GetProperty(Item.GetData(), type, value, eNormal) || (eLong!=type))
                    SHOW_WARN_CONTINUE(" - Can not take " << Item);

                if (gpDataHelper->IsMan(Item.GetData()))
                {
                    unit_control::modify_man_from_unit(pUnit2, pUnit, Item.GetData(), -amount);
                    unit_control::modify_man_from_unit(pUnit, pUnit2, Item.GetData(), amount);                    
                }
                else
                {
                    unit_control::modify_item_from_unit(pUnit2, pUnit, Item.GetData(), -amount);
                    unit_control::modify_item_from_unit(pUnit, pUnit2, Item.GetData(), amount);
                } 
                // check how giving men affects skills
                AdjustSkillsAfterGivingMen(pUnit2, pUnit, Item, amount);
            }
        }

    } while (FALSE);
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

    int               * weights;    // calculate weight change while giving
    const char       ** movenames;
    int                 movecount;


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

            if ( gpDataHelper->GetItemWeights(Item.GetData(), weights, movenames, movecount) )
            {

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
                pUnit->CalcWeightsAndMovement();
//                pUnit ->AddWeight(-amount, weights, movenames, movecount);
            }
        }

    } while (FALSE);
}


//-------------------------------------------------------------

void CAtlaParser::AdjustSkillsAfterGivingMen(CUnit * pUnitGive, CUnit * pUnitTake, CStr & item, long AmountGiven)
{
    int                 idx;
    const char        * propname_days;
    CStringSortColl     SkillNames(32);
    int                 postlen;
    CStr                S, BasePropName, Prop, PropDays, PropStudy;
    CUnit             * tmpunits[2] = {pUnitGive, pUnitTake};
    int                 i;
    char              * p;
    long                dayssrc, daystarg, newdays, newskill, mentarg, newdaysexp, newskillexp;
    EValueType          type;
    BOOL                bWasSetGive, bWasSetTake;
    const void        * valueGive = NULL;
    const void        * valueTake = NULL;

    if (!gpDataHelper->IsMan(item.GetData()))
        return;

    if ( !pUnitTake->GetProperty(PRP_MEN, type, (const void*&)mentarg, eNormal) || eLong!=type || mentarg <= 0)
    {
        LOG_ERR(ERR_UNKNOWN, "There must be men in the unit!");
        return;
    }

    // Adjust leadership here so we do not need to make another func

    if (pUnitGive->GetProperty(PRP_LEADER, type, valueGive, eNormal) && eCharPtr!=type)
    {
        S = "Wrong property type ";  S << BUG;
        OrderError("Warning", nullptr, pUnitGive,  S.GetData());
        return;
    }
    if (!pUnitTake->GetProperty(PRP_LEADER, type, valueTake, eNormal))
    {
        pUnitTake->SetProperty(PRP_LEADER, eCharPtr, "", eBoth);
    }
    else if (eCharPtr!=type)
    {
        S = "Wrong property type ";  S << BUG;
        OrderError("Warning", nullptr, pUnitTake,  S.GetData());
        return;
    }

    if (valueGive && !valueTake && 0==mentarg-AmountGiven)
    {
        // giving to new unit
        pUnitTake->SetProperty(PRP_LEADER, eCharPtr, valueGive, eNormal);
    }
    // else we do not give a damn!




    // make big list of skills
    postlen  = strlen(PRP_SKILL_DAYS_POSTFIX);
    for (i=0; i<2; i++)
    {
        idx      = 0;
        propname_days = tmpunits[i]->GetPropertyName(idx);
        while (propname_days)
        {
            S = propname_days;
            if (S.FindSubStrR(PRP_SKILL_DAYS_POSTFIX) == S.GetLength()-postlen)
            {
                p = strdup(propname_days);
                if (!SkillNames.Insert(p))
                    free(p);
            }
            propname_days = tmpunits[i]->GetPropertyName(++idx);
        }
    }

    // now handle each skill
    for (i=0; i<SkillNames.Count(); i++)
    {
        propname_days = (const char*)SkillNames.At(i);
        BasePropName = propname_days;
        BasePropName.DelSubStr(BasePropName.GetLength()-postlen, postlen);


        // Generic skill properties

        Prop     = BasePropName;  Prop     << PRP_SKILL_POSTFIX;
        PropDays = BasePropName;  PropDays << PRP_SKILL_DAYS_POSTFIX;
        if ( !pUnitGive->GetProperty(PropDays.GetData(), type, (const void*&)dayssrc, eNormal) )
            dayssrc = 0;
        if ( !pUnitTake->GetProperty(PropDays.GetData(), type, (const void*&)daystarg, eNormal) )
        {
            pUnitTake->SetProperty(PropDays.GetData(), eLong, (const void*)0L, eNormal);
            pUnitTake->SetProperty(Prop.GetData()    , eLong, (const void*)0L, eNormal);
            daystarg = 0;
        }

        newdays  = (daystarg*(mentarg-AmountGiven) + dayssrc*AmountGiven)/mentarg;
        newskill = SkillDaysToLevel(newdays);

        pUnitTake->SetProperty(PropDays.GetData() , eLong, (const void*)newdays , eNormal);
        pUnitTake->SetProperty(Prop.GetData()     , eLong, (const void*)newskill, eNormal);

        // Arcadia skill properties

        Prop      = BasePropName;  Prop      << PRP_SKILL_EXPERIENCE_POSTFIX;
        PropDays  = BasePropName;  PropDays  << PRP_SKILL_DAYS_EXPERIENCE_POSTFIX;
        PropStudy = BasePropName;  PropStudy << PRP_SKILL_STUDY_POSTFIX;

        bWasSetGive = pUnitGive->GetProperty(PropDays.GetData(), type, (const void*&)dayssrc, eNormal);
        bWasSetTake = pUnitTake->GetProperty(PropDays.GetData(), type, (const void*&)daystarg, eNormal);

        if (bWasSetGive || bWasSetTake)
        {
            // It is Arcadia indeed!

            PropStudy = BasePropName; PropStudy << PRP_SKILL_STUDY_POSTFIX;

            if ( !bWasSetGive )
                dayssrc = 0;
            if ( !bWasSetTake )
            {
                pUnitTake->SetProperty(PropDays.GetData() , eLong, (const void*)0L, eNormal);
                pUnitTake->SetProperty(Prop.GetData()     , eLong, (const void*)0L, eNormal);
                pUnitTake->SetProperty(PropStudy.GetData(), eLong, (const void*)0L, eNormal);
                daystarg = 0;
            }
            pUnitTake->SetProperty(PropStudy.GetData(), eLong, (const void*)newskill, eNormal);  //PRP_SKILL_STUDY_POSTFIX;

            newdaysexp  = (daystarg*(mentarg-AmountGiven) + dayssrc*AmountGiven)/mentarg;
            newskillexp = SkillDaysToLevel(newdaysexp);

            pUnitTake->SetProperty(PropDays.GetData() , eLong, (const void*)newdaysexp , eNormal);  //PRP_SKILL_DAYS_EXPERIENCE_POSTFIX
            pUnitTake->SetProperty(Prop.GetData()     , eLong, (const void*)newskillexp, eNormal);  //PRP_SKILL_EXPERIENCE_POSTFIX

            newskill = newskill + newskillexp;
            Prop     = BasePropName;  Prop     << PRP_SKILL_POSTFIX;
            pUnitTake->SetProperty(Prop.GetData()     , eLong, (const void*)newskill, eNormal);  //PRP_SKILL_POSTFIX
        }
    }


    SkillNames.FreeAll();
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandBuy(CLand * land)
{
    std::vector<land_control::Trader> buyers;
    std::vector<unit_control::UnitError> errors;
    land_control::get_land_buys(land, buyers, errors);
    long player_faction_id = game_control::get_game_config_val<long>("ATTITUDES", "PLAYER_FACTION_ID");

    for (const auto& buyer : buyers) 
    {
        //Economy
        if (buyer.unit_->FactionId == player_faction_id)
        {
            land->current_state_.bought_items_[buyer.item_name_] += buyer.items_amount_;
            land->current_state_.economy_.buy_expenses_ += buyer.items_amount_ * buyer.market_price_;
        }            

        //Modification of state
        if (gpDataHelper->IsMan(buyer.item_name_.c_str()))
        {
            //if (unit_control::get_item_amount(buyer.unit_, "LEAD") > 0)
            //{
            //    SetUnitProperty(buyer.unit_, PRP_LEADER, eCharPtr, SZ_LEADER, eNormal);
            //}
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
            errors.push_back({"Error", buyer.unit_, " - Can not set unit property"});
        }

        //should it be counted as production in the region
        if (game_control::get_game_config_val<long>("COMMON", "TRADE_ITEMS_AS_PROD") != 0)
        {
            if (gpDataHelper->IsTradeItem(buyer.item_name_.c_str()))
            {
                land_control::set_produced_items(land->current_state_, buyer.item_name_, buyer.items_amount_);
                buyer.unit_->Flags |= UNIT_FLAG_PRODUCING;
            }
        }
        buyer.unit_->CalcWeightsAndMovement();
    }    

    for (auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.message_);
    }

}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_LandSell(CLand * land)
{
    std::vector<land_control::Trader> sellers;
    std::vector<unit_control::UnitError> errors;
    land_control::get_land_sells(land, sellers, errors);
    long player_faction_id = game_control::get_game_config_val<long>("ATTITUDES", "PLAYER_FACTION_ID");

    for (const auto& seller : sellers) {
        //Economy
        if (seller.unit_->FactionId == player_faction_id)
        {
            land->current_state_.sold_items_[seller.item_name_] += seller.items_amount_;
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
            OrderError("Error", land, seller.unit_, std::string(NOSET) + BUG);            
        seller.unit_->CalcWeightsAndMovement();  
    }

    for (const auto& error : errors) {
        OrderError(error.type_, land, error.unit_, error.message_);
    }
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_Promote(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params)
{
    long                n1;
    long                id1=0, id2=0;
    int                 idx;
    CBaseObject         Dummy;
    CUnit             * pUnit2;
    EValueType          type;
    CStruct           * pStruct;

    do
    {
        if (!GetTargetUnitId(params, pUnit->FactionId, n1))
            SHOW_WARN_CONTINUE(" - Invalid unit Id");

        Dummy.Id = n1;
        if (pLand->Units.Search(&Dummy, idx))
        {
            pUnit2 = (CUnit*)pLand->Units.At(idx);
            params      = NULL;

            long struct_id = unit_control::structure_id(pUnit);
            if (struct_id == 0)
            {
                OrderError("Error", pLand, pUnit, "promote: units isn't in structure");
                return;
            }
            if (!unit_control::is_struct_owner(pUnit))
            {
                OrderError("Error", pLand, pUnit, "promote: units isn't owner of structure");
                return;
            }
            if (struct_id != unit_control::structure_id(pUnit2))
            {
                OrderError("Error", pLand, pUnit, "promote: target unit isn't in the same structure");
                return;
            }
            unit_control::set_structure(pUnit, struct_id, false);
            unit_control::set_structure(pUnit2, struct_id, true);

            if   (PE_OK!=pUnit ->SetProperty(PRP_STRUCT_OWNER, eCharPtr, "",  eNormal))
                SHOW_WARN_CONTINUE(NOSETUNIT << pUnit->Id << BUG);

            if ( (PE_OK!=pUnit2->SetProperty(PRP_STRUCT_OWNER, eCharPtr, "",  eNormal)) ||
                 (PE_OK!=pUnit2->SetProperty(PRP_STRUCT_OWNER, eCharPtr, YES, eNormal)) )
                SHOW_WARN_CONTINUE(NOSETUNIT << pUnit2->Id << BUG);

            pStruct = land_control::get_struct(pLand, id1);
            if (pStruct)
                pStruct->OwnerUnitId = pUnit2->Id;
        }
        else
            SHOW_WARN_CONTINUE(" - Can not find unit " << n1)
    } while (FALSE);
}

//-------------------------------------------------------------

void CAtlaParser::GetMovementMode(CUnit * pUnit, int & movementMode, bool & noCross, long order) const
{
    EValueType          type;
    const void        * value;

    movementMode = 0;
    noCross = true;

    if (pUnit->GetProperty(PRP_MOVEMENT, type, value, eNormal) && eCharPtr==type)
    {
        const char * movementModeStr = (const char *)value;
        if (strstr(movementModeStr, "Swim"))
            noCross = false;

        if (strstr(movementModeStr, "Fly"))
        {
            movementMode = 6;
            noCross = unit_control::flags::is_nocross(pUnit);
        }
        else
        if (strstr(movementModeStr, "Ride"))
            movementMode = 4;
        else
        if (strstr(movementModeStr, "Walk"))
            movementMode = 2;
        else
            movementMode = 2;
    }

    if (O_SAIL == order)
    {
        noCross = false;
        movementMode = 4;
    }
}

//-------------------------------------------------------------

void CAtlaParser::RunOrder_Move(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params, int & X, int & Y, int & LocA3, long order)
{
    int                 i, idx=0;
    CStr                S1;
    char                ch;
    EValueType          type;
    long                n1;
    CStruct           * pStruct;
    CStr                sErr(32), S;
    CBaseObject         Dummy;
    long                skill, nmen, structid;
    const void        * value;

    CLand             * pLandExit = NULL;
    CLand             * pLandCurrent = pLand;
    long                hexId = 0;
    long                newHexId = 0;
    long                currentStruct = 0;
    int                 totalMovementCost = 0;
    bool                pathCanBeTraced = true;
    int                 movementMode = 0;
    const int           startMonth = (m_YearMon % 100) - 1;
    bool                noCross = true;
    bool                throughWall = false;

    GetMovementMode(pUnit, movementMode, noCross, order);

    do
    {
        // should we do Arcadia III handling?
        if (O_SAIL == order &&
            unit_control::structure_id(pUnit) > 0)
        {
            pStruct  = land_control::get_struct(pLand, unit_control::structure_id(pUnit));
        }

        // Determine whether the unit is currently in a structure
        currentStruct = unit_control::structure_id(pUnit);

        while (params)
        {
            params        = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
            for (i=0; i<(int)sizeof(Directions)/(int)sizeof(const char*); i++)
            {
                if (0==stricmp(S1.GetData(), Directions[i]))
                {
                    // In this loop pLandCurrent is set, or hexId is set.
                    if (pLandCurrent)
                    {
                        hexId = pLandCurrent->Id;
                    }
                    if (pLandCurrent)
                    {
                        pLandExit = GetLandExit(pLandCurrent, i%6);
                        if (pLandExit)
                        {
                            newHexId = pLandExit->Id;
                        }
                        else if (IsLandExitClosed(pLandCurrent, i%6))
                        {
                            throughWall = true;
                        }
                    }
                    if (!pLandExit)
                    {
                        // Going into unknown terrain.
                        int x,y,z;
                        LandIdToCoord(hexId, x, y, z);
                        ExtrapolateLandCoord(x, y, z, i%6);
                        newHexId = LandCoordToId(x, y, z);
                    }
                    if (newHexId)
                    {
                        if (!pUnit->pMovement)
                            pUnit->pMovement = new CLongColl;

                        pUnit->pMovement->Insert((void*)newHexId);
                        currentStruct = 0;
                        pLandExit = GetLand(newHexId);
                    }
                    if (pLandExit && IsLandExitClosed(pLandExit, (i+3)%6) && (pLandExit->FindExit(hexId) < 0))
                    {
                        throughWall = true;
                    }
                    if (pLandExit && pLandCurrent && pathCanBeTraced)
                    {
                        int terrainCost = GetTerrainMovementCost(wxString::FromUTF8(pLandExit->TerrainType.GetData()));
                        if (noCross && terrainCost == 999)
                        {
                            pathCanBeTraced = false;
                            SHOW_WARN(" - Trying to move into the ocean!");
                        }
                        else
                        {
                            bool weather = IsBadWeatherHex(pLandExit, startMonth);
                            bool road = IsRoadConnected(pLandCurrent, pLandExit, i%6);
                            int cost = GetMovementCost(terrainCost, weather, road, movementMode, noCross);
                            totalMovementCost += cost;
                        }
                    }
                    else totalMovementCost += 1;

                    pLandCurrent = pLandExit;
                    hexId = newHexId;
                    newHexId = 0;
                    pLandExit = NULL;
                    if (throughWall)
                        SHOW_WARN_CONTINUE(" - Hexes are not connected - going through a wall perhaps!");

                    break;
                }
            }
            if (0==stricmp(S1.GetData(), "IN"))
            {
                // Find structure, and validate that it is a shaft.
                // Determine where it leads to.
                if (0 == currentStruct)
                {
                    SHOW_WARN_CONTINUE(" - Going IN without being in a structure!");
                }
                else  if (pLandCurrent && pathCanBeTraced)
                {
                    if (0==(pLandCurrent->Flags&LAND_VISITED))
                    {
                        SHOW_WARN_CONTINUE(" - Going through shaft in unvisited hex!");
                    }
                    else if (pLandCurrent->current_state_.structures_.size() < currentStruct)
                    {
                        SHOW_WARN_CONTINUE(" - No such structure!");
                    }
                    else
                    {
                        CStruct * pStruct = land_control::get_struct(pLandCurrent, currentStruct);
                        if (pStruct->Attr & SA_SHAFT)
                        {
                            if (pStruct->original_description_.find("links to (") != std::string::npos)
                            {
                                pLandExit = GetLandFlexible(wxString::FromUTF8(pStruct->original_description_.c_str()));
                                if (pLandExit)
                                {
                                    if (!pUnit->pMovement)
                                        pUnit->pMovement = new CLongColl;
                                    pUnit->pMovement->Insert((void*)pLandExit->Id);
                                    currentStruct = 0;
                                    if (pathCanBeTraced && pLandExit && pLandCurrent)
                                    {
                                        int terrainCost = GetTerrainMovementCost(wxString::FromUTF8(pLandExit->TerrainType.GetData()));
                                        bool weather = IsBadWeatherHex(pLandExit, startMonth);
                                        bool road = false;
                                        int cost = GetMovementCost(terrainCost, weather, road, movementMode, noCross);
                                        totalMovementCost += cost;
                                    }
                                    int z;
                                    LandIdToCoord(pLandExit->Id, X, Y, z);
                                    pLandCurrent = pLandExit;
                                }
                                else
                                {
                                    totalMovementCost += 1;
                                    pathCanBeTraced = false;
                                    SHOW_WARN_CONTINUE(" - Cannot find hex at other side of shaft, description broken!");
                                }
                            }
                            else
                            {
                                totalMovementCost += 1;
                                pathCanBeTraced = false;
                                if (!pUnit->pMovement)
                                    pUnit->pMovement = new CLongColl;
                                pUnit->pMovement->Insert((void*)pLandCurrent->Id);
                                // SHOW_WARN_CONTINUE(" - Proceeding through unknown shaft!");
                            }
                        }
                        else
                        {
                            totalMovementCost += 1;
                            pathCanBeTraced = false;
                            SHOW_WARN_CONTINUE(" - Structure is not a shaft!");
                        }
                    }
                }
                else totalMovementCost += 1;
            }
            else
            {
                // Try to parse a structure number.
                int structNumber = atoi(S1.GetData());
                if (structNumber > 0 && structNumber < 10000)
                    currentStruct = structNumber;
            }
        }

    SomeChecks:
        if (pUnit->reqMovementSpeed > 0)
        {
            if (movementMode < pUnit->reqMovementSpeed)
            {
                wxString msg = wxString::Format(wxT(" - Unit is moving at slower speed than specified! [%d/%d]"), movementMode, pUnit->reqMovementSpeed);
                SHOW_WARN_CONTINUE(msg.ToUTF8());
            }
        }
        else if (totalMovementCost > movementMode && gpDataHelper->ShowMoveWarnings())
        {
            wxString msg = wxString::Format(wxT(" - Unit is moving further than it can! [%d/%d]"), totalMovementCost, movementMode);
            SHOW_WARN_CONTINUE(msg.ToUTF8());
        }
        if (!pUnit->GetProperty(PRP_MEN, type, (const void *&)nmen, eNormal) || (eLong!=type) || (nmen<=0))
            SHOW_WARN_CONTINUE(" - There are no men in the unit!");

        if (O_SAIL == order)
        {
            // ship's sailing power
            if (unit_control::structure_id(pUnit) == 0)
                SHOW_WARN_CONTINUE(" - Must be in a ship to issue SAIL order!");

            S = "SAIL"; S << PRP_SKILL_POSTFIX;
            if (!pUnit->GetProperty(S.GetData(), type, (const void *&)skill, eNormal) || (eLong!=type) || (skill<=0))
                SHOW_WARN_CONTINUE(" - Needs SAIL skill!");

            pStruct  = land_control::get_struct(pLand, unit_control::structure_id(pUnit));
            if (pStruct == nullptr)
            {
                SHOW_WARN_CONTINUE(" - Invalid Struct Id " << BUG);
            }                
            else
            {
                pStruct->SailingPower += (nmen*skill);
            }                
        }
        else
        {
            if (gpDataHelper->ShowMoveWarnings())
            {
                pUnit->CheckWeight(sErr);
                if (!sErr.IsEmpty())
                    SHOW_WARN_CONTINUE(sErr);
            }
        }
    } while (FALSE);
}

//-------------------------------------------------------------


    static struct
    {
        eDirection   Location;
        eDirection   Direction;
        int          dX;
        int          dY;
        eDirection   TargetLoc;
    } NextLandLoc[] =
    {
        { North    , North     ,  0, -2, South     },
        { North    , Northeast ,  1, -1, Northwest },
        { North    , Southeast ,  0,  0, Northeast },
        { North    , Southwest ,  0,  0, Northwest },
        { North    , Northwest , -1, -1, Northeast },

        { Northeast, North     ,  0, -2, Southeast },
        { Northeast, Northeast ,  1, -1, Southwest },
        { Northeast, Southeast ,  1,  1, North     },
        { Northeast, South     ,  0,  0, Southeast },
        { Northeast, Northwest ,  0,  0, North     },

        { Southeast, North     ,  0,  0, Northeast },
        { Southeast, Northeast ,  1, -1, South     },
        { Southeast, Southeast ,  1,  1, Northwest },
        { Southeast, South     ,  0,  2, Northeast },
        { Southeast, Southwest ,  0,  0, South     },

        { South    , Northeast ,  0,  0, Southeast },
        { South    , Southeast ,  1,  1, Southwest },
        { South    , South     ,  0,  2, North     },
        { South    , Southwest , -1,  1, Southeast },
        { South    , Northwest ,  0,  0, Southwest },

        { Southwest, North     ,  0,  0, Northwest },
        { Southwest, Southeast ,  0,  0, South     },
        { Southwest, South     ,  0,  2, Northwest },
        { Southwest, Southwest , -1,  1, Northeast },
        { Southwest, Northwest , -1, -1, South     },

        { Northwest, North     ,  0, -2, Southwest },
        { Northwest, Northeast ,  0,  0, North     },
        { Northwest, South     ,  0,  0, Southwest },
        { Northwest, Southwest , -1,  1, North     },
        { Northwest, Northwest , -1, -1, Southeast },

        { Center   , North     ,  0, -2, South     },
        { Center   , Northeast ,  1, -1, Southwest },
        { Center   , Southeast ,  1,  1, Northwest },
        { Center   , South     ,  0,  2, North     },
        { Center   , Southwest , -1,  1, Northeast },
        { Center   , Northwest , -1, -1, Southeast }
    };


// Special SAILing for Arcadia III.

void CAtlaParser::RunOrder_SailAIII(CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params, int & X, int & Y, int & LocA3)
{
    int                 ID;
    int                 i, j;
    CStr                S1;
    char                ch;
    eDirection          Dir;
    BOOL                GoodSail;
    CStr                SailPassed;
    CLand             * pNewLand;


    while (params && *params)
    {
        GoodSail = FALSE;

        params        = SkipSpaces(S1.GetToken(params, " \t", ch, TRIM_ALL));
        for (i=0; i<(int)sizeof(Directions)/(int)sizeof(const char*); i++)
            if (0==stricmp(S1.GetData(), Directions[i]))
            {
                Dir = (eDirection)(i%6);

                for (j=0; j<(int)sizeof(NextLandLoc)/(int)sizeof(*NextLandLoc); j++)
                    if ( NextLandLoc[j].Location == LocA3 && NextLandLoc[j].Direction == Dir )
                    {
                        X += NextLandLoc[j].dX;
                        Y += NextLandLoc[j].dY;
                        LocA3 = NextLandLoc[j].TargetLoc;
                        GoodSail = TRUE;
                        break;
                    }

                if (!GoodSail)
                    break;

                X = NormalizeHexX(X, pLand->pPlane);
                ID = LandCoordToId(X,Y, pLand->pPlane->Id);

                pNewLand = GetLand(ID);
                if (!pNewLand || (pNewLand && 0 == stricmp("Ocean", pNewLand->TerrainType.GetData()) ||
             0 == stricmp("Lake", pNewLand->TerrainType.GetData()) ) )
                    LocA3 = Center;

                if (!pUnit->pMovement)
                    pUnit->pMovement = new CLongColl;
                pUnit->pMovement->Insert((void*)ID);

                if (!pUnit->pMoveA3Points)
                    pUnit->pMoveA3Points = new CLongColl;
                pUnit->pMoveA3Points->Insert((void*)LocA3);

                break;
            }

        if (!GoodSail)
            SHOW_WARN_CONTINUE(" - Can not sail " << SailPassed << S1);
        SailPassed << S1 << ' ';
    }
}

//-------------------------------------------------------------

void CAtlaParser::OrderProcess_Teach(BOOL skiperror, CUnit * pUnit)
{
    /*
    long          nstud = 0; // students count
    long          n1, n2;
    CUnit       * pUnit2;
    double        teach;
    CStr          ErrorLine(32);
    CStr          Line;
    EValueType    type;
    int           i;
    const char  * leadership;

    do
    {
        if ( pUnit->pStudents && (pUnit->pStudents->Count() > 0)  )
        {
            if (!pUnit->StudyingSkill.IsEmpty())
            {
                BOOL ItsOk = FALSE;
                // is it a freaking hero?
                if (m_ArcadiaSkills &&
                    pUnit->GetProperty(PRP_LEADER, type, (const void *&)leadership, eNormal) &&
                    eCharPtr==type &&
                    0 == stricmp(leadership, SZ_HERO))
                    ItsOk = TRUE;

                if (!ItsOk)
                    SHOW_WARN_CONTINUE(" - can not teach and study at the same time");
            }

            for (i=0; i<pUnit->pStudents->Count(); i++)
            {

                pUnit2 = (CUnit*)pUnit->pStudents->At(i);
                if (!pUnit2->GetProperty(PRP_MEN, type, (const void *&)n2, eNormal) )
                    n2 = 0; // should not happen
                nstud += n2;
            }
            if (!pUnit->GetProperty(PRP_MEN, type, (const void *&)n1, eNormal) )
                n1 = 0; // unlikely, too

            // n1 - number of teachers
            // nstud - number of students

            if (nstud>0 && n1>0)
            {
                pUnit ->Teaching = (double)nstud/n1;
                teach = (double)n1*STUDENTS_PER_TEACHER/nstud*30;

                for (i=0; i<pUnit->pStudents->Count(); i++)
                {
                    pUnit2           = (CUnit*)pUnit->pStudents->At(i);
                    pUnit2->Teaching += teach;
                }
            }
        }
    }
    while (FALSE);*/
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
    }
    else  // run orders for all lands
    {
        for (n=0; n<m_Planes.Count(); n++)
        {
            pPlane = (CPlane*)m_Planes.At(n);
            for (i=0; i<pPlane->Lands.Count(); i++)
            {
                pLand = (CLand*)pPlane->Lands.At(i);
                if (pLand)
                    RunLandOrders(pLand, start_step, stop_step);
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
    //RunOrders(NULL);
    
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
    for (idx=0; idx<m_Units.Count(); idx++)
    {
        pUnit = (CUnit*)m_Units.At(idx);

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
                                land_control::add_resource(pLand->current_state_, *pProd);
                            }
                        }
                }
            }
        }

        propname = pUnit->GetPropertyName(++propidx);
    }

    Levels.FreeAll();
    Resources.FreeAll();
}

//-------------------------------------------------------------

int CAtlaParser::GetTerrainMovementCost(wxString Terrain) const
{
    int MovementCost = 2;

    std::map<wxString, int>::const_iterator i = TerrainMovementCost.find(Terrain.Lower());
    if (i != TerrainMovementCost.end())
    {
        MovementCost = i->second;
    }
    return MovementCost;
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
