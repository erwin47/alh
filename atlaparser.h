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

#ifndef __AH_REP_PARSER_H__
#define __AH_REP_PARSER_H__

#include <map>
#include <list>

#include "data.h"
//#include "data_control.h"
#include "files.h"
#include "hash.h"


extern const char * Monthes[];

extern const char * EOL_MS   ;
extern const char * EOL_UNIX ;
extern const char * EOL_SCR  ;
extern const char * EOL_FILE ;

#define DEFAULT_PLANE "Overworld"

extern int   ExitFlags [];
extern int   EntryFlags[];

extern int Flags_NW_N_NE;
extern int Flags_N      ;
extern int Flags_SW_S_SE;
extern int Flags_S      ;


enum SHARE_TYPE {
    SHARE_BUY,
    SHARE_STUDY,
    SHARE_UPKEEP
};

typedef struct SAVE_HEX_OPTIONS_STRUCT
{
    BOOL   SaveStructs;
    BOOL   AlwaysSaveImmobStructs;
    BOOL   SaveUnits;
    BOOL   SaveResources;
    long   WriteTurnNo; // Add turn number atlaclient style
} SAVE_HEX_OPTIONS;

inline TurnSequence& operator++(TurnSequence& x) {
    return x = (TurnSequence)(std::underlying_type<TurnSequence>::type(x) + 1); 
}

inline TurnSequence& operator--(TurnSequence& x) {
    return x = (TurnSequence)(std::underlying_type<TurnSequence>::type(x) - 1); 
}

inline TurnSequence operator*(TurnSequence c) {
    return c;
}

inline TurnSequence begin(TurnSequence ) {
    return TurnSequence::SQ_FIRST;
}

inline TurnSequence end(TurnSequence ) {
    TurnSequence l=TurnSequence::SQ_LAST;
    return ++l;
}

class CAtlaParser
{
public:
    CAtlaParser();
    CAtlaParser(CGameDataHelper * pHelper);
    ~CAtlaParser();
    void       Clear();
    int        ParseRep(const char * FNameIn, BOOL Join, BOOL IsHistory);     // History is a rep!
    int        SaveOrders  (const char * FNameOut, const char * password, BOOL decorate, int factid);
    int        LoadOrders  (const char * FNameIn, int & FactionId);  // return an id of the order's faction
    void       RunOrders(CLand * pLand, TurnSequence start_step = TurnSequence::SQ_FIRST, TurnSequence stop_step = TurnSequence::SQ_LAST);
    void       RunLandOrders(CLand * pLand, TurnSequence beg_step = TurnSequence::SQ_FIRST, TurnSequence stop_step = TurnSequence::SQ_LAST);
    //! generate autonames as if RunOrders was executed, but just for AONaming
    void       RunLandAutonames(CLand * land);

    BOOL       ShareSilver(CUnit * pMainUnit);
    BOOL       GenOrdersTeach(CUnit * pMainUnit);
    BOOL       GenGiveEverything(CLand* land, CUnit * pFrom, const char * To);
    BOOL       DiscardJunkItems(CUnit * pUnit, const char * junk);
    BOOL       ApplyDefaultOrders(BOOL EmptyOnly);
    void       ApplyLandDefaultOrders(CLand* land);
    int        ParseCBDataFile(const char * FNameIn);
    void       WriteMagesCSV(const char * FName, BOOL vertical, const char * separator, int format);

    bool       IsLandExitClosed(CLand * pLand, int direction) const;
    CLand    * GetLandExit(CLand * pLand, int direction) const;
    CLand    * GetLand(int x, int y, int nPlane, BOOL AdjustForEdge=FALSE) const;
    CLand    * GetLand(long LandId) const;
    CLand    * GetLand(const char * landcoords) const; //  "48,52[,somewhere]"
    CLand    * GetLandFlexible(const wxString & description) const;
    CLand    * GetLandWithCity(const wxString & cityName) const;
    void       GetUnitList(CCollection * pResultColl, int x, int y, int z);
    void       CountMenForTheFaction(int FactionId);
    void       ComposeProductsLine(CLand * pLand, const char * eol, CStr & S);
    void       compose_products_detailed(CLand* land, std::stringstream& out);
    BOOL       LandStrCoordToId(const char * landcoords, long & id) const;
    int        NormalizeHexX(int NoX, CPlane *) const;
    void       ComposeLandStrCoord(CLand * pLand, CStr & LandStr);
    CFaction * GetFaction(int id);
    BOOL       SaveOneHex(CFileWriter & Dest, CLand * pLand, CPlane * pPlane, SAVE_HEX_OPTIONS * pOptions);
    long       SkillDaysToLevel(long days);
    CUnit *    SplitUnit(CUnit * pOrigUnit, long newId);
    wxString   getFullStrLandCoord(CLand *);
    BOOL       CheckResourcesForProduction(CUnit * pUnit, CLand * pLand, CStr & Error);

    void       ExtrapolateLandCoord(int &x, int &y, int z, int direction) const;

    void         CheckOrder_LandMonthlong  (CLand *land, std::list<CUnit*>& no_monthlong_orders);

    // Movement
    bool       IsRoadConnected(CLand *, CLand *, int direction) const;
    bool       IsBadWeatherHex(CLand * pLand, int month) const;
    int        GetMovementCost(int terrainCost, bool isBadWeather, bool hasRoad, int movementMode, bool noCross) const;
    
    int64_t           m_CrntFactionId;
    CStr              m_CrntFactionPwd;
    CLongColl         m_OurFactions;
    CBaseObject       m_Events;
    CBaseObject       m_SecurityEvents;
    CBaseObject       m_HexEvents;
    CBaseObject       m_Errors;
    CBaseColl         m_NewProducts;

    CBaseCollById     m_Factions;


    bool global_unit_exists(long unit_id) const {  return units_ids_.find(unit_id) != units_ids_.end();  }
    CUnit* global_find_unit(long unit_id) {  return global_unit_exists(unit_id) ? units_[units_ids_[unit_id]] : nullptr;  }

    std::unordered_map<long, size_t> units_ids_; //<Id, pos in units_>
    std::vector<CUnit*> units_;
    
    CBaseColl         m_Planes;
    long              m_YearMon;    // Current year/month accumulated for all loaded files
    CStringSortColl   m_UnitPropertyNames;
    CStrIntColl       m_UnitPropertyTypes;
    CStringSortColl   m_LandPropertyNames;
    //CStringSortColl   m_LandPropertyTypes;
    CBaseColl         m_Skills;
    CBaseColl         m_Items;
    CBaseColl         m_Objects;
//    CBaseCollByName   m_Battles;
    CBaseColl         m_Battles;
    CBaseCollById     m_Gates;

    long              m_nCurLine;
    long              m_GatesCount;
    int               m_ParseErr;
    BOOL              m_OrdersLoaded;
    CStr              m_FactionInfo;
    BOOL              m_ArcadiaSkills;

    bool              m_EconomyTaxPillage;
    bool              m_EconomyWork;

    const char * ReadPropertyName(const char * src, CStr & Name);

    void         OrderError(const std::string& type, CLand* land, CUnit* unit, 
                            const std::shared_ptr<orders::Order>& order, const std::string& Msg);

protected:
    int          ParseFactionInfo(BOOL GetNo, BOOL Join);
    int          ParseEvents(BOOL IsEvents=TRUE);
    int          ParseUnclSilver(CStr & Line);
    int          ParseAttitudes(CStr & Line, BOOL Join);
    int          ParseTerrain (CLand * pMotherLand, int ExitDir, CStr & FirstLine, BOOL FullMode, CLand ** ppParsedLand);
    int          AnalyzeTerrain(CLand * pMotherLand, CLand * pLand, BOOL IsExit, int ExitDir, CStr & Description);
    void         ComposeHexDescriptionForArnoGame(const char * olddescr, const char * newdescr, CStr & CompositeDescr);

    void         ParseWages(CLand * pLand, const char * str1, const char * str2);
    void         CheckExit(CPlane * pPlane, int Direction, CLand * pLandSrc, CLand * pLandExit);
    int          ParseUnit(CStr & FirstLine, BOOL Join);
    int          ParseStructure (CStr & FirstLine);
    int          ParseErrors();
    int          ParseLines(BOOL Join);
    BOOL         ParseOneUnitEvent(CStr & EventLine, BOOL IsEvent, int UnitId);
    BOOL         ParseOneLandEvent(CStr & EventLine, BOOL IsEvent);
    void         ParseOneMovementEvent(const char * params, const char * structid, const char * fullevent);
    int          ParseOneEvent(CStr & EventLine, BOOL IsEvent);
    void         ParseWeather(const char * src, CLand * pLand);
    int          ApplyLandFlags();
    int          SetLandFlag(const char * p, long flag);
    int          SetLandFlag(long LandId, long flag);
    int          ParseBattles();
    int          ParseSkills();
    int          ParseItems();
    int          ParseObjects();
    void         SetExitFlagsAndTropicZone();
    int          SetUnitProperty(CUnit * pUnit, const char * name, EValueType type, const void * value, EPropertyType proptype);
    int          SetLandProperty(CLand * pLand, const char * name, EValueType type, const void * value, EPropertyType proptype);
    int          LoadOrders  (CFileReader & F, int FactionId, BOOL GetComments);
    void         StoreBattle(CStr & Source);
    void         AnalyzeBattle(const char * src, CStr & Details);
    void         AnalyzeBattle_OneSide(const char * src, CStr & Details);
    const char * AnalyzeBattle_ParseUnit(const char * src, CUnit *& pUnit, BOOL & InFrontLine);
    void         AnalyzeBattle_SummarizeUnits(CBaseColl & Units, CStr & Details);
    void         SetShaftLinks();
    void         ApplySailingEvents();
    BOOL         GetTargetUnitId(long x, long y, long z, const char *& p, long FactionId, long & nId);
    int          ParseOneImportantEvent(CStr & EventLine);
    int          ParseImportantEvents();

    CUnit      * construct_or_retrieve_unit(long Id);
    CPlane     * MakePlane(const char * planename);

    BOOL         ReadNextLine(CStr & s);
    BOOL         ReadNextLineMerged(CStr & s);
    void         PutLineBack (CStr & s);

    void         GenericErr(int Severity, const char * Msg);
    
    void         OrderErrFinalize();

    // Order handlers and helpers
    void         RunOrder_Send             (CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params);
    void         RunOrder_Produce          (CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params);
    
    void         RunOrder_LandFlags        (CLand* land);
//!calculates give orders up to specified unit (or for all units if NULL)
    void         RunOrder_LandGive         (CLand* land, CUnit* up_to = NULL);
    void         RunOrder_LandSell         (CLand* land);
    void         RunOrder_LandBuy          (CLand* land);
    void         RunOrder_LandProduce      (CLand* land);
    void         RunOrder_LandBuild        (CLand* land);
    void         RunOrder_LandStudyTeach   (CLand* land);
    void         RunOrder_LandAggression   (CLand* land);//steal, assassinate, attack
    void         RunOrder_LandTaxPillage   (CLand* land, bool apply_changes);
    void         RunOrder_LandWork         (CLand *land, bool apply_changes);
    void         RunOrder_LandEntertain    (CLand *land, bool apply_changes);
    void         RunOrder_LandSail         (CLand* land);
    void         RunOrder_LandMove         (CLand* land);
    void         RunOrder_LandTransport    (CLand* land);
    void         RunOrder_LandNameDescribe (CLand* land);

    template<orders::Type TYPE> void RunOrder_AOComments(CLand* land);
    void         RunOrder_AONames          (CLand* land);
    void         RunCaravanAutomoveOrderGeneration(CLand* land);
    //void         RunOrder_Study            (CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params);
    BOOL         FindTargetsForSend        (CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char *& params, CUnit *& pUnit2, CLand *& pLand2);
    BOOL         GetItemAndAmountForGive   (CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params, CStr & Item, int & amount, const char * command, CUnit * pUnit2);
    void         RunOrder_Withdraw         (CStr & Line, CStr & ErrorLine, BOOL skiperror, CUnit * pUnit, CLand * pLand, const char * params);
    void         RunOrder_Upkeep           (CLand *);
    void         RunOrder_Upkeep           (CUnit *, int turns);
    void         DistributeSilver          (CLand *, int unitFlag, int silver, int menCount);
    int          CountMenWithFlag          (CLand *, int unitFlag) const;

    void         LookupAdvancedResourceVisibility(CUnit * pUnit, CLand * pLand);

    int          ParseCBHex   (const char * FirstLine);
    int          ParseCBStruct(const char * FirstLine);

    template<typename T>
    void perform_on_each_land(T func)
    {
        for (int n=0; n<m_Planes.Count(); n++)
        {
            CPlane* pPlane = (CPlane*)m_Planes.At(n);
            for (int i=0; i<pPlane->Lands.Count(); i++)
            {
                CLand* pLand = (CLand*)pPlane->Lands.At(i);
                if (pLand)
                    func(pLand);
            }
        }
    }    

    CFileReader    * m_pSource;

    CStringSortColl  m_TaxLandStrs;
    CStringSortColl  m_TradeLandStrs;
    CStringSortColl  m_BattleLandStrs;
    CLongSortColl    m_TradeUnitIds;
    CBaseCollByName  m_PlanesNamed;
    CBaseColl        m_LandsToBeLinked;
    CHashStrToLong   m_UnitFlagsHash;
    std::map<long, std::string> sailing_events_;//struct_id, collection of events

    CLand          * m_pCurLand   ;
    CStruct        * m_pCurStruct ;
    int              m_NextStructId;
    CStr             m_sOrderErrors;
    BOOL             m_JoiningRep;  // joining an allies' report
    BOOL             m_IsHistory;   // parsing history file
    long             m_CurYearMon; // Year/month for the file being loaded
    CStr             m_WeatherLine[8];
};



#endif

