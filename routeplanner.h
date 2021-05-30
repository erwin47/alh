#ifndef ROUTEPLANNER_H_INCLUDED
#define ROUTEPLANNER_H_INCLUDED

class CLand;
class CUnit;

#include "wx/string.h"


enum class MovementMode {
    Walk,
    Ride,
    Fly,
    Sail,
};

class MovementSetup {
    public:
        int speed{ 2 }; // default atlantis: walk=2, ride=4, fly=6. With mani and windchime, fly=8. Sailing can have varying speeds.
        MovementMode mode{ MovementMode::Walk };
        bool noCross{ true };
        bool canSwim{ false };
};

class RoutePlanner
{
    public:
        inline static int SpeedWalk{ 2 };
        inline static int SpeedRide{ 4 };
        inline static int SpeedFly{ 6 };

        enum ROUTE_MARKUP {
            ROUTE_MARKUP_NONE
        ,   ROUTE_MARKUP_TURN
        ,   ROUTE_MARKUP_ALL
        };
        static bool IsMageOrApprentice(CUnit *);
        static int GetSwinLevel(CUnit *);
        static MovementSetup GetMovementSetup(CUnit * pUnit, long order);
        static int GetMovementCost(int terrainCost, bool isBadWeather, bool hasRoad, MovementSetup);

        static wxString GetRoute(CLand * start, CLand * end, MovementSetup, ROUTE_MARKUP);
        static bool tryUpdateRoute(CLand * pLandCurrent, CLand * pLandExit, MovementSetup, int startMonth, wxString moveCommand, bool hasRoad, ROUTE_MARKUP);
        static wxString GetFirstMove(wxString Route); // Returns a route for the first turn from a given route.

};

#endif // ROUTEPLANNER_H_INCLUDED
