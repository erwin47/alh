#include "routeplanner.h"

#include <algorithm>
#include <list>

#include "ahapp.h"
#include "atlaparser.h"
#include "cfgfile.h"
#include "collection.h"
#include "consts.h"
#include "consts_ah.h"
#include "cstr.h"
#include "files.h"
#include "hash.h"
#include "objs.h"
#include "stdhdr.h"

int RoutePlanner::GetMovementCost(int terrainCost, bool isBadWeather, bool hasRoad, MovementSetup setup)
{
    int cost{ 1 };

    switch (setup.mode) {
    case MovementMode::Walk:
    case MovementMode::Ride:
        if (!setup.canSwim || terrainCost < 999)
        {
            cost = terrainCost;
        }
        break;

    case MovementMode::Fly:
        if (terrainCost >= 999 && setup.noCross)
            cost = terrainCost;
        break;

    case MovementMode::Sail:
        cost = 1;
    }
    
    if (isBadWeather)
    {
        cost *= 2;
    }
    if (hasRoad && cost < 6)
    {
        cost = (cost + 1) / 2;
    }
    return cost;
}

int RoutePlanner::GetSwinLevel(CUnit* pUnit)
{
    EValueType          type;
    const void* value;

    int swin{ 0 };

    if (pUnit->GetProperty("SWIN_", type, value, eNormal) && (eLong == type))
    {
        // Mages can directly use their SWIN skill
        swin = (int)value;
    }

    if (pUnit->GetProperty("WCHM", type, value, eNormal) && (eLong == type))
    {
        const int WindChimeSWIN = 2;
        if (value > 0 && IsMageOrApprentice(pUnit))
        {
            swin = std::max(swin, WindChimeSWIN);
        }
    }
    return swin;
}


bool RoutePlanner::IsMageOrApprentice(CUnit * pUnit)
{
    std::vector<wxString> skills = { "PATT", "SPIR", "FORC", "MANI" };

    EValueType          type;
    const void* value;

    int level = 0;

    for (auto skill : skills)
    {
        wxString property = skill + PRP_SKILL_POSTFIX;
        if (pUnit->GetProperty(property.c_str(), type, value, eNormal) && (eLong == type))
            level = std::max((int)value, level);
    }
    return level;
}


MovementSetup RoutePlanner::GetMovementSetup(CUnit* pUnit, long order)
{
    EValueType          type;
    const void* value;

    MovementSetup setup;

    if (O_SAIL == order)
    {
        setup.mode = MovementMode::Sail;
        setup.noCross = false;
        setup.speed = 4;
    }
    else if (pUnit->GetProperty(PRP_MOVEMENT, type, value, eNormal) && eCharPtr == type)
    {
        const char* movementModeStr = (const char*)value;
        if (strstr(movementModeStr, "Swim"))
        {
            setup.noCross = (pUnit->Flags & UNIT_FLAG_NO_CROSS_WATER);
            setup.canSwim = true;
        }

        if (strstr(movementModeStr, "Walk"))
        {
            setup.mode = MovementMode::Walk;
            setup.speed = SpeedWalk;
        }
        else if (strstr(movementModeStr, "Ride"))
        {
            setup.mode = MovementMode::Ride;
            setup.speed = SpeedRide;
        }
        else if (strstr(movementModeStr, "Fly"))
        {
            setup.mode = MovementMode::Fly;
            setup.noCross = (pUnit->Flags & UNIT_FLAG_NO_CROSS_WATER);
            setup.speed = SpeedFly;

            if (GetSwinLevel(pUnit))
                setup.speed += 2;
        }
    }
    return setup;
}


wxString RoutePlanner::GetRoute(CLand * start, CLand * end, MovementSetup setup, ROUTE_MARKUP markup)
{
    extern const char * Directions[]; // Definition is in atlaparser.cpp

    int bestSolution = 999; // measured in movement points required.
    std::list<CLand *> ListHex;
    std::list<CLand *> ListHexCleanup;
    CLand * pLandCurrent;
    CLand * pLandExit;
    std::list<CLand *>::iterator HexOld;
    bool EraseLandFromList;
    wxString Log;

    int x, y, x0, y0, z;

    const int startMonth = gpApp->m_pAtlantis->m_YearMon % 100 - 1;

    ListHex.push_back(start);

    // Reset all regions
    for (int n=0; n<gpApp->m_pAtlantis->m_Planes.Count(); n++)
    {
        CPlane * pPlane = (CPlane*)gpApp->m_pAtlantis->m_Planes.At(n);
        for (int i=0; i<pPlane->Lands.Count(); i++)
        {
            CLand * pLand = (CLand*)pPlane->Lands.At(i);
            if (pLand)
                pLand->TotalMovementCost = 999;
        }
    }


    start->ArrivedFromHexId = 0;
    start->TotalMovementCost = 0;
    start->MoveDirection = "";
    end->ArrivedFromHexId = 0;
    end->TotalMovementCost = 999;
    end->MoveDirection = "";

    // Do passes based on movement cost, and process hexes which equal mp.
    for (int mp=0; mp < bestSolution; ++mp)
    {
        Log << "Starting MP=" << mp << "\n";
        for (std::list<CLand *>::iterator hexi = ListHex.begin(); hexi != ListHex.end(); )
        {
            EraseLandFromList = false;
            pLandCurrent = *hexi;
            if (pLandCurrent->TotalMovementCost == mp)
            {
                EraseLandFromList = true;
                LandIdToCoord(pLandCurrent->Id, x0, y0, z);
                Log << "  Checking Land (" << x0 << " " << y0 << ")" << "\n";
                // Loop through the exits
                for (int i=0; i<6; i++)
                {
                    pLandExit = gpApp->m_pAtlantis->GetLandExit(pLandCurrent, i);
                    if (pLandExit && ((pLandCurrent->Flags&LAND_VISITED) || gpApp->m_pAtlantis->GetLandExit(pLandExit, (i+3)%6)))
                    {
                        LandIdToCoord(pLandExit->Id, x, y, z);
                        {
                            const bool hasRoad = gpApp->m_pAtlantis->IsRoadConnected(pLandCurrent, pLandExit, i);
                            Log << "    Found Exit to Land (" << x << " " << y << ")" << "\n";

                            if (tryUpdateRoute(pLandCurrent, pLandExit, setup, startMonth, Directions[i + 6], hasRoad, markup))
                            {
                                ListHex.push_back(pLandExit);
                                if (pLandExit->Id == end->Id && bestSolution > pLandExit->TotalMovementCost)
                                {
                                    bestSolution = pLandExit->TotalMovementCost;
                                    Log << "BestSolution set to " << bestSolution << "\n";
                                }
                                Log << "      Adding Route to Land (" << x << "," << y << ") with MP " << pLandExit->TotalMovementCost << "\n";
                            }
                        }
                    }
                }
                // Try the shafts
                CStruct    * pStruct;
                for (int j=0; j<pLandCurrent->Structs.Count(); j++)
                {
                    pStruct = (CStruct*)pLandCurrent->Structs.At(j);
                    if (pStruct->Attr & SA_SHAFT  )
                    {
                        Log << "      Found Shaft\n";
                        if (pStruct->Description.FindSubStr("links to (") >= 0)
                        {
                            pLandExit = gpApp->m_pAtlantis->GetLandFlexible(pStruct->Description.GetData());
                            if (pLandExit)
                            {
                                Log << "      Found connection to landExit\n";
                                if (tryUpdateRoute(pLandCurrent, pLandExit, setup, startMonth, wxString::Format("%ld IN", pStruct->Id), false, markup))
                                {
                                    ListHex.push_back(pLandExit);
                                    LandIdToCoord(pLandExit->Id, x, y, z);
                                    if (pLandExit->Id == end->Id && bestSolution > pLandExit->TotalMovementCost)
                                    {
                                        bestSolution = pLandExit->TotalMovementCost;
                                        Log << "BestSolution set to " << bestSolution << "\n";
                                    }
                                    Log << "      Adding Route through shaft to Land (" << x << "," << y << "," << z << ") with MP " << pLandExit->TotalMovementCost << "\n";
                                }
                            }
                        }
                        break;
                    }
                }
            }
            HexOld = hexi;
            ++hexi;
            if (EraseLandFromList)
            {
                ListHex.erase(HexOld);
                ListHexCleanup.push_back(pLandCurrent);
            }
        }
    }

    wxString Route;
    if (end->TotalMovementCost > setup.speed)
    {
        Route = wxString::Format("; %d turns", (end->TotalMovementCost + setup.speed - 1) / setup.speed);
    }

    if (end->ArrivedFromHexId)
    {
        for (int i = 0; (i < 99 && end) ; ++i)
        {
            Route = wxString(end->MoveDirection.GetData()) + " " + Route;
            end = gpApp->m_pAtlantis->GetLand(end->ArrivedFromHexId);
        }
    }
    else if (end->TotalMovementCost >= 999)
    {
        Route = wxEmptyString;
    }

    // Reset Data for next usage
    start->TotalMovementCost = 999;
    for (std::list<CLand *>::iterator hexi = ListHex.begin(); hexi != ListHex.end(); ++hexi)
    {
        pLandCurrent = *hexi;
        pLandCurrent->TotalMovementCost = 999;
    }
    for (std::list<CLand *>::iterator hexi = ListHexCleanup.begin(); hexi != ListHexCleanup.end(); ++hexi)
    {
        pLandCurrent = *hexi;
        pLandCurrent->TotalMovementCost = 999;
    }
    return Route;
}


bool RoutePlanner::tryUpdateRoute(CLand * pLandCurrent, CLand * pLandExit, MovementSetup setup, const int startMonth, const wxString moveCommand, const bool hasRoad, ROUTE_MARKUP markup)
{
    wxString route_markers;
    // Land Found, now determine whether this route is faster than the old route.
    int oldTurn = (pLandCurrent->TotalMovementCost + setup.speed - 1) / setup.speed;
    if (oldTurn == 0) oldTurn = 1;
    int currentMonth = (startMonth + oldTurn - 1) % 12;

    const int terrainCost = gpApp->m_pAtlantis->GetTerrainMovementCost(pLandExit->TerrainType.GetData());

    bool isBadWeather = gpApp->m_pAtlantis->IsBadWeatherHex(pLandExit, currentMonth);

    if (markup == ROUTE_MARKUP_ALL)
    {
        if (isBadWeather) route_markers += wxT("w");
        if (hasRoad) route_markers += wxT("r");
    }

    int MovementCost = GetMovementCost(terrainCost, isBadWeather, hasRoad, setup);
    int newMovementCost = pLandCurrent->TotalMovementCost + MovementCost;
    int newTurn = (newMovementCost + setup.speed - 1) / setup.speed;
    if (newTurn > oldTurn)
    {
        // Switching to a new turn, lose excess movement points first, change of weather possible
        if (markup == ROUTE_MARKUP_ALL || markup == ROUTE_MARKUP_TURN)
        {
            route_markers = wxT("_ ");
        }
        currentMonth = (startMonth + oldTurn) % 12;

        // Recalculate cost for weather in new turn, it may have changed
        isBadWeather = gpApp->m_pAtlantis->IsBadWeatherHex(pLandExit, currentMonth);

        if (markup == ROUTE_MARKUP_ALL)
        {
            if (isBadWeather) route_markers += wxT("w");
            if (hasRoad) route_markers += wxT("r");
        }

        MovementCost = GetMovementCost(terrainCost, isBadWeather, hasRoad, setup);
        if (MovementCost > setup.speed)
        {
            if (isBadWeather)
            {
                if (markup == ROUTE_MARKUP_ALL || markup == ROUTE_MARKUP_TURN)
                {
                    route_markers = wxT("_ _ ");
                }
                for (int mon = 1; mon < 5; ++mon)
                {
                    if (gpApp->m_pAtlantis->IsBadWeatherHex(pLandExit, (currentMonth + mon) % 12))
                    {
                        MovementCost += setup.speed;
                        if (markup == ROUTE_MARKUP_ALL || markup == ROUTE_MARKUP_TURN)
                        {
                            route_markers += wxT("_ ");
                        }
                    }
                    else break;
                }
            }
            else
            {
                MovementCost = 999;
            }
        }
        newMovementCost = oldTurn * setup.speed + MovementCost;
    }
    if (pLandExit->TotalMovementCost > newMovementCost)
    {
        pLandExit->TotalMovementCost = newMovementCost;
        pLandExit->ArrivedFromHexId = pLandCurrent->Id;
        pLandExit->MoveDirection = route_markers + moveCommand;
        return true;
    }
    return false;
}

wxString RoutePlanner::GetFirstMove(wxString Route)
{
    Route = Route.BeforeFirst(';') + "_";
    wxString Move = Route.BeforeFirst('_');
    Move.Replace("r", "");
    Move.Replace("w", "");
    Move.Trim();
    Move.Trim(false);
    return Move;
}
