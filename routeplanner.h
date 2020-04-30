#ifndef ROUTEPLANNER_H_INCLUDED
#define ROUTEPLANNER_H_INCLUDED

class CLand;
#include "wx/string.h"


class RoutePlanner
{
    public:

        enum ROUTE_MARKUP {
            ROUTE_MARKUP_NONE
        ,   ROUTE_MARKUP_TURN
        ,   ROUTE_MARKUP_ALL
        };

        static wxString GetRoute(CLand * start, CLand * end, int movementMode, ROUTE_MARKUP, bool no_cross, wxString& Log);
        static bool tryUpdateRoute(CLand * pLandCurrent, CLand * pLandExit, int movementMode, int startMonth, wxString moveCommand, bool hasRoad, ROUTE_MARKUP, bool nocross);
        static wxString GetFirstMove(wxString Route); // Returns a route for the first turn from a given route.

};

#endif // ROUTEPLANNER_H_INCLUDED
