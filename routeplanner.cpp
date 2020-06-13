#include "routeplanner.h"

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
#include "data_control.h"

wxString RoutePlanner::GetRoute(CLand * start, CLand * end, int movementMode, ROUTE_MARKUP markup, bool nocross, wxString& Log)
{
    extern const char * Directions[]; // Definition is in atlaparser.cpp

    int bestSolution = 999; // measured in movement points required.
    std::list<CLand *> ListHex;
    std::list<CLand *> ListHexCleanup;
    CLand * pLandCurrent;
    CLand * pLandExit;
    std::list<CLand *>::iterator HexOld;
    bool EraseLandFromList;
    //wxString Log;

    int x, y, x0, y0, z, i;

    const int startMonth = gpApp->m_pAtlantis->m_YearMon % 100 - 1;

    ListHex.push_back(start);

    // Reset all regions
    for (int n=0; n<gpApp->m_pAtlantis->m_Planes.Count(); n++)
    {
        CPlane * pPlane = (CPlane*)gpApp->m_pAtlantis->m_Planes.At(n);
        for (i=0; i<pPlane->Lands.Count(); i++)
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
                for (i=0; i<6; i++)
                {
                    pLandExit = gpApp->m_pAtlantis->GetLandExit(pLandCurrent, i);
                    if (pLandExit && ((pLandCurrent->Flags&LAND_VISITED) || gpApp->m_pAtlantis->GetLandExit(pLandExit, (i+3)%6)))
                    {
                        LandIdToCoord(pLandExit->Id, x, y, z);
                        {
                            const bool hasRoad = gpApp->m_pAtlantis->IsRoadConnected(pLandCurrent, pLandExit, i);
                            Log << "    Found Exit to Land (" << x << " " << y << ")" << "\n";

                            if (tryUpdateRoute(pLandCurrent, pLandExit, movementMode, startMonth, Directions[i + 6], hasRoad, markup, nocross))
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

                land_control::perform_on_each_struct(pLandCurrent, [&](CStruct* structure) {
                    if (struct_control::flags::is_shaft(structure))
                    {
                        Log << "      Found Shaft\n";
                        if (struct_control::has_link(structure))
                        {
                            pLandExit = gpApp->m_pAtlantis->GetLandFlexible(structure->original_description_.c_str());
                            if (pLandExit)
                            {
                                Log << "      Found connection to landExit\n";
                                if (tryUpdateRoute(pLandCurrent, pLandExit, movementMode, startMonth, wxString::Format("%ld IN", structure->Id), false, markup, nocross))
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
                    }
                });
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
    if (end->TotalMovementCost > movementMode)
    {
        Route = wxString::Format("; %d turns", (end->TotalMovementCost + movementMode - 1)/movementMode);
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


bool RoutePlanner::tryUpdateRoute(CLand * pLandCurrent, CLand * pLandExit, const int movementMode, 
                                  const int startMonth, const wxString moveCommand, const bool hasRoad, 
                                  ROUTE_MARKUP markup, bool nocross)
{
    wxString route_markers;
    // Land Found, now determine whether this route is faster than the old route.
    int oldTurn = (pLandCurrent->TotalMovementCost + movementMode - 1) / movementMode;
    if (oldTurn == 0) oldTurn = 1;
    int currentMonth = (startMonth + oldTurn - 1) % 12;

    int terrainCost = gpApp->m_pAtlantis->GetTerrainMovementCost(pLandExit->TerrainType.GetData());
    if (nocross && land_control::is_water(pLandExit))
    {
        terrainCost = 999;
    }

    int isBadWeather = gpApp->m_pAtlantis->IsBadWeatherHex(pLandExit, currentMonth);

    if (markup == ROUTE_MARKUP_ALL)
    {
        if (isBadWeather) route_markers += wxT("w");
        if (hasRoad) route_markers += wxT("r");
    }

    int MovementCost = gpApp->m_pAtlantis->GetMovementCost(terrainCost, isBadWeather, hasRoad, movementMode, true);
    int newMovementCost = pLandCurrent->TotalMovementCost + MovementCost;
    int newTurn = (newMovementCost + movementMode - 1) / movementMode;
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

        MovementCost = gpApp->m_pAtlantis->GetMovementCost(terrainCost, isBadWeather, hasRoad, movementMode, true);
        if (MovementCost > movementMode)
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
                        MovementCost += movementMode;
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
        newMovementCost = oldTurn * movementMode + MovementCost;
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
