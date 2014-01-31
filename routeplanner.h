#ifndef ROUTEPLANNER_H_INCLUDED
#define ROUTEPLANNER_H_INCLUDED

class CLand;
#include "wx/string.h"


class RoutePlanner
{
    public:

        wxString GetRoute(CLand * start, CLand * end, int movementMode);
        bool tryUpdateRoute(CLand * pLandCurrent, CLand * pLandExit, int movementMode, int startMonth, wxString moveCommand, bool hasRoad);
        wxString GetFirstMove(wxString Route); // Returns a route for the first turn from a given route.

};

#endif // ROUTEPLANNER_H_INCLUDED
