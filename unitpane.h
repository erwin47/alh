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

#ifndef __AH_UNIT_PANE_INCL__
#define __AH_UNIT_PANE_INCL__

enum ScoutType {
    SCOUT_SIMPLE
,   SCOUT_MOVE
,   SCOUT_OBSERVER
,   SCOUT_STEALTH
,   SCOUT_GUARD
};

#include <functional>

bool default_unit_filter(CUnit* unit);

class CUnitPane: public CListPane
{
public:
    CUnitPane(wxWindow *parent, wxWindowID id = list_units_hex);
    virtual ~CUnitPane() {}
    virtual void Init(CAhFrame * pParentFrame, const char * szConfigSection, const char * szConfigSectionHdr);
    virtual void Done();
    void         Update(CLand * pLand, std::function<bool(CUnit* unit)> filter = default_unit_filter);
    virtual void ApplyFonts();
    CUnit      * GetUnit(long index);
    virtual void Sort();
    void         SelectUnit(long UnitId);
    void         SelectNextUnit();
    void         SelectPrevUnit();

    CUnit*       GetSelectedUnit();

    void         LoadUnitListHdr();
    void         SaveUnitListHdr();
    void         ReloadHdr(const char * szConfigSectionHdr);


    TPropertyHolderColl * m_pUnits;
    CLand               * m_pCurLand;

    void UpdateCells();

    long         selected_unit_id_;//! 
    bool         is_filtered_;//! is used for Ctrl+G interface filtering out units by a chosen rule

protected:
    void         OnSelected(wxListEvent& event);
    void         OnColClicked(wxListEvent& event);
    void         OnIdle(wxIdleEvent& event);
    void         OnRClick(wxListEvent& event);
    bool         CreateScout(CUnit *, ScoutType);

    CAhFrame            * m_pFrame;
    //CCollection         * m_pFactions;

    CStr                  m_sConfigSection;
    CStr                  m_sConfigSectionHdr;

    int                   m_ColClicked;

public:

    void OnPopupMenuShareSilv         (wxCommandEvent& event);
    void OnPopupMenuTeach             (wxCommandEvent& event);
    void OnPopupMenuSplit             (wxCommandEvent& event);
    void OnPopupMenuCreateNew         (wxCommandEvent& event);
    void OnPopupMenuReceiveItems      (wxCommandEvent& event);
    void OnPopupFilterByItems         (wxCommandEvent& event);
    void OnPopupSellAll               (wxCommandEvent& event);
    void OnPopupShareAsCaravan        (wxCommandEvent& event);
    void OnPopupPaintCaravanRoadmap   (wxCommandEvent& event);
    void OnPopupMenuDiscardJunk       (wxCommandEvent& event);
    void OnPopupMenuDetectSpies       (wxCommandEvent& event);
    void OnPopupMenuGiveEverything    (wxCommandEvent& event);
    void OnPopupMenuAddUnitToTracking (wxCommandEvent& event);
    void OnPopupMenuUnitFlags         (wxCommandEvent& event);
    void OnPopupMenuIssueOrders       (wxCommandEvent& event);

    void OnPopupMenuScoutSimple       (wxCommandEvent& event);
    void OnPopupMenuScoutMove         (wxCommandEvent& event);
    void OnPopupMenuScoutObserver     (wxCommandEvent& event);
    void OnPopupMenuScoutStealth      (wxCommandEvent& event);
    void OnPopupMenuScoutGuard        (wxCommandEvent& event);


    DECLARE_EVENT_TABLE()
};

#endif
