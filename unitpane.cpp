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

#include "stdhdr.h"

#include "wx/listctrl.h"
#include "wx/spinctrl.h"

#include "cstr.h"
#include "collection.h"
#include "cfgfile.h"
#include "files.h"
#include "atlaparser.h"
#include "consts.h"
#include "consts_ah.h"
#include "hash.h"

#include "ahapp.h"
#include "ahframe.h"
#include "listpane.h"
#include "unitpane.h"
#include "editpane.h"
#include "utildlgs.h"
#include "unitsplitdlg.h"
#include "flagsdlg.h"
#include "createnewunitdlg.h"
#include "receivedlg.h"


BEGIN_EVENT_TABLE(CUnitPane, wxListCtrl)
    EVT_LIST_ITEM_SELECTED   (list_units_hex, CUnitPane::OnSelected)
    EVT_LIST_COL_CLICK       (list_units_hex, CUnitPane::OnColClicked)
    EVT_LIST_ITEM_RIGHT_CLICK(list_units_hex, CUnitPane::OnRClick)
    EVT_IDLE                 (CUnitPane::OnIdle)

    EVT_MENU             (menu_Popup_ShareSilv     , CUnitPane::OnPopupMenuShareSilv      )
    EVT_MENU             (menu_Popup_Teach         , CUnitPane::OnPopupMenuTeach          )
    EVT_MENU             (menu_Popup_Split         , CUnitPane::OnPopupMenuSplit          )
    EVT_MENU             (menu_Popup_Create_New    , CUnitPane::OnPopupMenuCreateNew      )
    EVT_MENU             (menu_Popup_ReceiveItems  , CUnitPane::OnPopupMenuReceiveItems   )
    EVT_MENU             (menu_Popup_FilterByItems , CUnitPane::OnPopupFilterByItems      )
    EVT_MENU             (menu_Popup_DiscardJunk   , CUnitPane::OnPopupMenuDiscardJunk    )
    EVT_MENU             (menu_Popup_DetectSpies   , CUnitPane::OnPopupMenuDetectSpies    )
    EVT_MENU             (menu_Popup_GiveEverything, CUnitPane::OnPopupMenuGiveEverything )

    EVT_MENU             (menu_Popup_ScoutSimple   , CUnitPane::OnPopupMenuScoutSimple    )
    EVT_MENU             (menu_Popup_ScoutMove     , CUnitPane::OnPopupMenuScoutMove      )
    EVT_MENU             (menu_Popup_ScoutObserver , CUnitPane::OnPopupMenuScoutObserver  )
    EVT_MENU             (menu_Popup_ScoutStealth  , CUnitPane::OnPopupMenuScoutStealth   )
    EVT_MENU             (menu_Popup_ScoutGuard    , CUnitPane::OnPopupMenuScoutGuard     )

    EVT_MENU             (menu_Popup_AddToTracking , CUnitPane::OnPopupMenuAddUnitToTracking)
    EVT_MENU             (menu_Popup_UnitFlags     , CUnitPane::OnPopupMenuUnitFlags      )
    EVT_MENU             (menu_Popup_IssueOrders   , CUnitPane::OnPopupMenuIssueOrders    )


END_EVENT_TABLE()



//--------------------------------------------------------------------------

CUnitPane::CUnitPane(wxWindow *parent, wxWindowID id)
          :CListPane(parent, id, wxLC_REPORT | wxLC_SINGLE_SEL)
{
    m_pUnits   = NULL;
    m_pCurLand = NULL;
    m_ColClicked = -1;

    is_filtered_ = false;
    ApplyFonts();
}

//--------------------------------------------------------------------------

void CUnitPane::Init(CAhFrame * pParentFrame, const char * szConfigSection, const char * szConfigSectionHdr)
{
    m_pFrame            = pParentFrame;
    m_sConfigSection    = szConfigSection;
    m_sConfigSectionHdr = szConfigSectionHdr;
    m_pLayout           = new CListLayout;              // this one will hold the objects for real
    m_pUnits            = new TPropertyHolderColl(128); // only references
    m_pData             = m_pUnits;                     // units are more obvious when using from outside
    LoadUnitListHdr();
}

//--------------------------------------------------------------------------

void CUnitPane::Done()
{
    SaveUnitListHdr();
    DeleteAllItems();

    if (m_pLayout)
    {
        m_pLayout->FreeAll();
        delete m_pLayout;
        m_pLayout = NULL;
    }

    if (m_pUnits)
    {
        m_pUnits->DeleteAll();
        delete m_pUnits;
        m_pUnits = NULL;
        m_pData  = NULL;
    }
}

//--------------------------------------------------------------------------

bool default_unit_filter(CUnit* unit) {   return true;  }

void CUnitPane::Update(CLand * pLand, std::function<bool(CUnit* unit)> filter)
{
    int               i;
    CUnit           * pUnit;
    //eSelMode          selmode = sel_by_no;
    //long              seldata = 0;
    //BOOL              FullUpdate = (pLand != m_pCurLand); // if not full mode, refresh new units only
    //wxListItem        info;
    //CBaseColl         ArrivingUnits;
    long              GuiColor;

    CLand* prev_land = m_pCurLand;
    m_pCurLand = pLand;
    
    selected_unit_id_ = -1;

    if (prev_land == m_pCurLand)
    {
        pUnit = GetSelectedUnit();
        if (pUnit != nullptr)
            selected_unit_id_ = pUnit->Id;
    }

    //because we don't update windows, except current unit's list
    //if (gpApp->getLayout() == AH_LAYOUT_1_WIN_ONE_DESCR)
    //    FullUpdate = FALSE;
    // It is a must, since some locations pointed to by stored pointers may be invalid at the moment.
    // Namely all new units are deleted when orders are processed.
    if (!this->is_filtered_)
        m_pUnits->DeleteAll();//we shouldn't delete all, if we are already filtered, we just want to continue
                              //with list of units which we have

    /*if (!FullUpdate)
        for (i=GetItemCount()-1; i>=0; i--)
        {
            info.m_itemId = i;
            info.m_col    = 0;
            info.m_mask   = wxLIST_MASK_DATA;
            GetItem(info);
        }*/

    if (!this->is_filtered_ && pLand)
    {
        std::set<long> already_listed_units;
        for (i=0; i<pLand->Units.Count(); i++)
        {
            pUnit = (CUnit*)pLand->Units.At(i);
            if (pUnit && !filter(pUnit)) {
                this->is_filtered_ = true;
                //FullUpdate = true;
                continue;
            }              

            if (pUnit && pUnit->movements_.size() > 0 || pUnit->movement_stop_ == pLand->Id)
                already_listed_units.insert(pUnit->Id);

            if (pUnit && pUnit->has_error_) {//pUnit->Flags & UNIT_FLAG_HAS_ERROR) {
                GuiColor = 4;//light red, for units with errors
            }
            else if (pUnit && pUnit->movement_stop_ != 0 && pUnit->movement_stop_ != pLand->Id)
                GuiColor = 1;
            else if (pUnit && (pUnit->Flags & UNIT_FLAG_GUARDING))
                GuiColor = 3;
            else if (pUnit && unit_control::is_struct_owner(pUnit))
                GuiColor = 5;//color of owning building
            else
                GuiColor = 0;
            pUnit->SetProperty(PRP_GUI_COLOR, eLong, (void*)GuiColor, eBoth);
            m_pUnits->AtInsert(m_pUnits->Count(), pUnit);
        }
        land_control::moving::perform_on_each_incoming_unit(pLand, [&](CUnit* unit) {
            if (already_listed_units.find(unit->Id) != already_listed_units.end())
                return;

            if (unit && !filter(unit)) {
                this->is_filtered_ = true;
                //FullUpdate = true;
                return;
            }
                

            already_listed_units.insert(unit->Id);
            GuiColor = 2;
            unit->SetProperty(PRP_GUI_COLOR, eLong, (void*)GuiColor, eBoth);
            m_pUnits->AtInsert(m_pUnits->Count(), unit);
        });

        land_control::moving::perform_on_each_going_to_come_unit(pLand, [&](CUnit* unit) {
            if (already_listed_units.find(unit->Id) != already_listed_units.end())
                return;

            if (unit && !filter(unit)) {
                this->is_filtered_ = true;
                //FullUpdate = true;//
                return;
            }

            already_listed_units.insert(unit->Id);
            GuiColor = 6;
            unit->SetProperty(PRP_GUI_COLOR, eLong, (void*)GuiColor, eBoth);
            m_pUnits->AtInsert(m_pUnits->Count(), unit);
        });

        m_pUnits->SetSortMode(m_SortKey, NUM_SORTS);

        //if (pLand->guiUnit)
        //{
        //    seldata = pLand->guiUnit;
        //    selmode = sel_by_id;
        //}
    }
    //if (prev_land != pLand) {
    //    selected_unit_id_ = -1;
        //SetData(no_selection, 0, true);
    //}
    SetData(sel_by_id, selected_unit_id_, prev_land != m_pCurLand);
        

    /*if (prev_land != pLand) {
        long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
        if (item >= 0)
            this->SetItemState(item, 0, wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED);
    }*/

    //We don't actually want to update windows, except unit's list
    //if ((0==m_pUnits->Count()) || !FullUpdate) // otherwise will be called from OnSelected()
    //    gpApp->OnUnitHexSelectionChange(GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED));
}

//--------------------------------------------------------------------------

void CUnitPane::SelectUnit(long UnitId)
{
    int               i;
    wxListItem        info;

    selected_unit_id_ = -1;
    for (i=GetItemCount()-1; i>=0; i--)
    {
        info.m_itemId = i;
        info.m_col    = 0;
        info.m_mask   = wxLIST_MASK_DATA;
        GetItem(info);
        if ((int)info.m_data==UnitId)
        {
            selected_unit_id_ = UnitId;
            SetItemState(i, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
            EnsureVisible(i);
            this->SetFocus();
        }
        else {
            SetItemState(i, 0, wxLIST_STATE_SELECTED);            
        }
            
    }
}

//--------------------------------------------------------------------------

void CUnitPane::Sort()
{
    long              idx;
    long              data=0;
    wxListItem        info;

    idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (idx>=0)
    {
        info.SetId(idx);
        info.m_mask  = wxLIST_MASK_DATA;
        GetItem(info);
        data = info.m_data;
    }
    m_pUnits->SetSortMode(m_SortKey, NUM_SORTS);
    SetData(sel_by_id, data, TRUE);
}

//--------------------------------------------------------------------------

void CUnitPane::ApplyFonts()
{
    SetFont(*gpApp->m_Fonts[FONT_UNIT_LIST]);
}

//--------------------------------------------------------------------------

void CUnitPane::LoadUnitListHdr()
{
    CListLayoutItem * pLI;
    int               i;
    const char      * szName;
    const char      * szValue;
    CStr              S(32);
    int               width;
    unsigned long     flags;

    m_pLayout->FreeAll();
    i = gpApp->GetSectionFirst(m_sConfigSectionHdr.GetData(), szName, szValue);
    while (i >= 0)
    {
        szValue = S.GetToken(szValue, ',');  width = atol(S.GetData());
        szValue = S.GetToken(szValue, ',');  flags = atol(S.GetData());
        szValue = S.GetToken(szValue, ',');
        while (szValue && (*szValue<=' '))
            szValue++;
        if ( width>0 && szValue && strlen(szValue)>0 )
        {
            pLI = new CListLayoutItem(S.GetData(), szValue, width, flags);
            m_pLayout->Insert(pLI);
        }
        i = gpApp->GetSectionNext(i, m_sConfigSectionHdr.GetData(), szName, szValue);
    }

    SetLayout();
    SetSortName(0,  gpApp->GetConfig(m_sConfigSection.GetData(), SZ_KEY_SORT1));
    SetSortName(1,  gpApp->GetConfig(m_sConfigSection.GetData(), SZ_KEY_SORT2));
    SetSortName(2,  gpApp->GetConfig(m_sConfigSection.GetData(), SZ_KEY_SORT3));
    Sort();
}

//--------------------------------------------------------------------------

void CUnitPane::ReloadHdr(const char * szConfigSectionHdr)
{
    eSelMode          selmode = sel_by_no;
    long              seldata = 0;
    int               i, x;

    m_sConfigSectionHdr = szConfigSectionHdr;

    DeleteAllItems();
    DeleteAllColumns();
    /*if (m_pLayout != nullptr)
    {
        x = m_pLayout->Count()-1; //why +20 ??? : (m_pLayout->Count()+20) : 100;
        for (i=x; i>=0; i--)
            DeleteColumn(i);
    }*/
    LoadUnitListHdr();

    if (m_pCurLand && selected_unit_id_ > 0)
        SetData(sel_by_id, selected_unit_id_, true);
    else 
        SetData(sel_by_no, 0, true);

    
}

//--------------------------------------------------------------------------

void CUnitPane::SaveUnitListHdr()
{
    CListLayoutItem * pLI;
    int               i;
    CStr              Key;
    CStr              Val;

    if (m_pLayout)
    {
        // we are naming items dynamically, so remove them first!
        gpApp->RemoveSection(m_sConfigSectionHdr.GetData());

        for (i=0; i<m_pLayout->Count(); i++)
        {
            pLI = (CListLayoutItem*)m_pLayout->At(i);


            Key.Format("%03d", i);
            Val.Format("%d, %lu, %s, %s", GetColumnWidth(i), pLI->m_Flags, pLI->m_Name, pLI->m_Caption);
            gpApp->SetConfig(m_sConfigSectionHdr.GetData(), Key.GetData(), Val.GetData());
        }

        gpApp->SetConfig(m_sConfigSection.GetData(), SZ_KEY_SORT1, GetSortName(0 ) );
        gpApp->SetConfig(m_sConfigSection.GetData(), SZ_KEY_SORT2, GetSortName(1 ) );
        gpApp->SetConfig(m_sConfigSection.GetData(), SZ_KEY_SORT3, GetSortName(2 ) );
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnSelected(wxListEvent& event)
{
    //if (is_selection_automatic_) {
    //    is_selection_automatic_ = false;
    //    return;
    //}

    CUnitOrderEditPane* pOrders;
    CUnit      * pUnit = GetUnit(event.m_itemIndex);
    bool         changed = false;
    int          idx, i;

    if (pUnit)
    {
        selected_unit_id_ = pUnit->Id;
//        if (m_pCurLand)
//            m_pCurLand->guiUnit = pUnit->Id;
        if (gpApp->OnUnitHexSelectionChange(event.m_itemIndex))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

CUnit * CUnitPane::GetUnit(long index)
{
    wxListItem   info;
    CUnit      * pUnit = NULL;

    info.m_itemId = index;
    info.m_col    = 0;
    info.m_mask   = wxLIST_MASK_DATA;
    if (GetItem(info))
    {
        pUnit = (CUnit*)m_pUnits->At(info.m_itemId);
        wxASSERT(pUnit && (pUnit->Id == (long)info.m_data));  // just make sure we've got the right unit
    }

    return pUnit;
}

//--------------------------------------------------------------------------

void CUnitPane::SelectNextUnit()
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (idx < GetItemCount()-1)
    {
        CUnit      * pUnit = GetUnit(idx+1);
        if (pUnit)
            SelectUnit(pUnit->Id);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::SelectPrevUnit()
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (idx>0)
    {
        CUnit      * pUnit = GetUnit(idx-1);
        if (pUnit)
            SelectUnit(pUnit->Id);
    }
}

CUnit* CUnitPane::GetSelectedUnit() 
{
    long idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (idx >= 0)
    {
        CUnit* unit = GetUnit(idx);
        selected_unit_id_ = unit->Id;
        return unit;
    }
    selected_unit_id_ = -1;
    return nullptr;
}

//--------------------------------------------------------------------------

void CUnitPane::OnColClicked(wxListEvent& event)
{
    // Have to do it in idle sinse the dialog requires double clicking otherwise
    m_ColClicked = event.m_col;
}

//--------------------------------------------------------------------------

void CUnitPane::OnIdle(wxIdleEvent& event)
{
    // Set sorting
    if (m_ColClicked>=0)
    {
        CListLayoutItem * p;
        int               col = m_ColClicked;

        m_ColClicked = -1;
        p = (CListLayoutItem*)m_pLayout->At(col);
        if (p)
        {
            wxString choice, message=wxString::FromAscii(p->m_Caption), caption=wxT("Set sort order");
            wxString choices[NUM_SORTS-1];

            choices[0]=wxT("primary");
            choices[1]=wxT("secondary");
            choices[2]=wxT("tertiary");

            choice = wxGetSingleChoice(message, caption, NUM_SORTS-1, choices, m_pParent);

            if (!choice.IsEmpty())
            {
                int key;
                if (0==stricmp(choice.mb_str(), "primary"))
                    key = 0;
                else if (0==stricmp(choice.mb_str(), "secondary"))
                    key = 1;
                else
                    key = 2;
                SetSortName(key, p->m_Name);
                Sort();
            }
        }
    }

    //CListPane::OnIdle(event);
    event.Skip();
}

//--------------------------------------------------------------------------

void CUnitPane::OnRClick(wxListEvent& event)
{
    wxMenu       menu;
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (idx == -1)
        return;
    CUnit      * pUnit = GetUnit(idx);

    int nItems = GetSelectedItemCount();

    if (nItems>1)
    {
        // multiple units
        menu.Append(menu_Popup_IssueOrders     , wxT("Issue orders"));
        menu.Append(menu_Popup_UnitFlags       , wxT("Set custom flags")    );
        menu.Append(menu_Popup_AddToTracking   , wxT("Add to a tracking group"));

        PopupMenu( &menu, event.GetPoint().x, event.GetPoint().y);
    }
    else
        if (pUnit)
        {
            // single unit
            if (pUnit->IsOurs)
            {
                if (!IS_NEW_UNIT(pUnit))
                {
                    menu.Append(menu_Popup_ShareSilv     , wxT("Share SILV")        );
                    menu.Append(menu_Popup_Split         , wxT("Split")             );
                    menu.Append(menu_Popup_Create_New    , wxT("Create new unit (Ctrl+F)")   );
                }
                menu.Append(menu_Popup_Teach         , wxT("Teach")             );
                menu.Append(menu_Popup_ReceiveItems  , wxT("Receive (Ctrl+R)")             );
                menu.Append(menu_Popup_FilterByItems  , wxT("Filter units (Ctrl+G)")  );
                if (IS_NEW_UNIT(pUnit))
                {
                    menu.Append(menu_Popup_DiscardJunk   , wxT("Discard This Unit") );
                }
                if (!IS_NEW_UNIT(pUnit))
                {
                    menu.Append(menu_Popup_DiscardJunk   , wxT("Discard junk items"));
                    menu.Append(menu_Popup_GiveEverything, wxT("Give everything")   );

                    wxMenu * menuScouts = new wxMenu();
                    menuScouts->Append(menu_Popup_ScoutSimple , wxT("Scout"));
                    menuScouts->Append(menu_Popup_ScoutMove   , wxT("Scout North"));
                    menuScouts->Append(menu_Popup_ScoutObserver, wxT("Scout Observer"));
                    menuScouts->Append(menu_Popup_ScoutStealth, wxT("Scout Stealth"));
                    menuScouts->Append(menu_Popup_ScoutGuard  , wxT("Scout Guard"));

                    menu.AppendSubMenu(menuScouts, wxT("Create"), wxT("Create a new unit with a simple task"));
                    // menu.Append(menu_Popup_DetectSpies   , wxT("Detect spies")      );
                }
            }
            menu.Append(menu_Popup_UnitFlags       , wxT("Set custom flags")    );
            menu.Append(menu_Popup_AddToTracking   , wxT("Add to a tracking group"));

            PopupMenu( &menu, event.GetPoint().x, event.GetPoint().y);
        }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuTeach (wxCommandEvent& event)
{
    CUnitOrderEditPane  * pOrders;

    CUnit      * pUnit = GetSelectedUnit();
    if (pUnit)
    {
        gpApp->SetOrdersChanged(gpApp->m_pAtlantis->GenOrdersTeach(pUnit)
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuSplit(wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        CUnitSplitDlg dlg(this, pUnit);
        if (wxID_OK == dlg.ShowModal()) // it will modify unit's orders
            gpApp->SetOrdersChanged(TRUE);

        Update(m_pCurLand);
    }
}

void CUnitPane::OnPopupMenuCreateNew(wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CLand* pLand = gpApp->m_pAtlantis->GetLand(pUnit->LandId);
    CUnitOrderEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        //if (m_pCurLand)
        //    m_pCurLand->guiUnit = pUnit->Id;

        //gpApp->m_pAtlantis->RunLandOrders(pLand, TurnSequence::SQ_FIRST, TurnSequence::SQ_GIVE);
        CCreateNewUnit dlg(this, pUnit, pLand);
        if (wxID_OK == dlg.ShowModal()) // it will modify unit's orders
            gpApp->SetOrdersChanged(TRUE);

        gpApp->m_pAtlantis->RunLandOrders(pLand);
        Update(m_pCurLand);
    }
}

void CUnitPane::OnPopupFilterByItems(wxCommandEvent& event)
{
    long idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (idx < 0)
        return;
    CUnit* pUnit = GetUnit(idx);
    if (!pUnit)
        return;

    CLand* pLand = gpApp->m_pAtlantis->GetLand(pUnit->LandId);
    CItemChooseDlg dlg(this, pUnit, pLand);

    if (wxID_OK == dlg.ShowModal()) // it will modify unit's orders
    {
        struct UpdateFilter {
            std::string code_;
            CUnit* issuing_unit_;
            bool operator()(CUnit* unit) {
                if (unit->Id == issuing_unit_->Id)
                    return true;
                if (unit_control::get_item_amount(unit, code_, true) > 0)
                    return true;
                return false;
            }
        };
        
        this->is_filtered_ = false;
        UpdateFilter filter;
        filter.code_ = dlg.chosen_code_;
        filter.issuing_unit_ = pUnit;
        Update(m_pCurLand, filter);
        this->is_filtered_ = true;
    }
    else
    {
        this->is_filtered_ = false;
        Update(m_pCurLand);
    }
    //gpApp->m_pAtlantis->RunLandOrders(pLand);
}

void CUnitPane::OnPopupMenuReceiveItems(wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (idx < 0)
        return;
    CUnit* pUnit = GetUnit(idx);
    if (!pUnit)
        return;

    CLand* pLand = gpApp->m_pAtlantis->GetLand(pUnit->LandId);

    CReceiveDlg dlg(this, pUnit, pLand);
    if (wxID_OK == dlg.ShowModal()) // it will modify unit's orders
        gpApp->SetOrdersChanged(TRUE);

    gpApp->m_pAtlantis->RunLandOrders(pLand);
    Update(m_pCurLand);
}

//--------------------------------------------------------------------------

bool CUnitPane::CreateScout(CUnit * pUnit, ScoutType scoutType)
{
    CStr         race(32), racemarket;
    EValueType   type;
    long         peritem;
    wxString    newUnitOrders;
    int         newUnitId;

    // Create a scout in the current hex, and issue orders for the new unit and the originating unit.
    // m_pCurLand must be set to the hex the unit is in.

    if (!m_pCurLand) return false;

    newUnitOrders << wxT("name unit scout\n");

    gpApp->m_pAtlantis->ReadPropertyName(m_pCurLand->current_state_.peasant_race_.c_str(), race);

    if (race.IsEmpty())
        return false; // When there is no race to buy, we cannot make a scout.

    if (race.FindSubStr("ORCS") != -1) race = "ORC";
    if (race.FindSubStr("HUMANS") != -1) race = "MAN";

    peritem = 0;
    MakeQualifiedPropertyName(PRP_SALE_PRICE_PREFIX, race.GetData(), racemarket);
    m_pCurLand->GetProperty(racemarket.GetData(), type, (const void *&)peritem, eNormal);

    newUnitOrders << wxString::Format("buy 1 %s\n", wxString::FromAscii(race.GetData()).Lower());

    if (   pUnit->FlagsOrg & UNIT_FLAG_TAXING)             newUnitOrders << wxT("autotax 0\n");
    if (! (pUnit->FlagsOrg & UNIT_FLAG_AVOIDING))          newUnitOrders << wxT("avoid 1\n");
    if (! (pUnit->FlagsOrg & UNIT_FLAG_BEHIND))            newUnitOrders << wxT("behind 1\n");
    if (   pUnit->FlagsOrg & UNIT_FLAG_CONSUMING_UNIT)     newUnitOrders << wxT("consume\n");
    if (   pUnit->FlagsOrg & UNIT_FLAG_CONSUMING_FACTION)  newUnitOrders << wxT("consume\n");
    if (   pUnit->FlagsOrg & UNIT_FLAG_GUARDING)           newUnitOrders << wxT("guard 0\n");
    if (   pUnit->FlagsOrg & UNIT_FLAG_HOLDING)            newUnitOrders << wxT("hold 0\n");
    if (   pUnit->FlagsOrg & UNIT_FLAG_RECEIVING_NO_AID)   newUnitOrders << wxT("noaid 0\n");
    if (   pUnit->FlagsOrg & UNIT_FLAG_NO_CROSS_WATER)     newUnitOrders << wxT("nocross 0\n");
    if (scoutType == SCOUT_STEALTH)
    {
        if ((pUnit->FlagsOrg & UNIT_FLAG_REVEALING_FACTION) || (pUnit->FlagsOrg & UNIT_FLAG_REVEALING_UNIT)) newUnitOrders << wxT("reveal\n");
    }
    else if (! (pUnit->FlagsOrg & UNIT_FLAG_REVEALING_FACTION)) newUnitOrders << wxT("reveal faction\n");
    if (   pUnit->FlagsOrg & UNIT_FLAG_SHARING)            newUnitOrders << wxT("share 0\n");
    newUnitOrders << wxT("spoils none\n");

    newUnitId = m_pCurLand->GetNextNewUnitNo();

    switch (scoutType)
    {
        case SCOUT_SIMPLE:
            newUnitOrders << wxT("@work\n");
        break;
        case SCOUT_MOVE:
            peritem += 10;
            newUnitOrders << wxT("MOVE N\n");
            newUnitOrders << wxT("TURN\n@work\nENDTURN\n");
        break;
        case SCOUT_OBSERVER:
            peritem += 60;
            newUnitOrders << wxT("STUDY OBSERVATION\n");
            newUnitOrders << wxT("@;; name unit \"scout observer\"\n");
            newUnitOrders << wxT("TURN\n@work\nENDTURN\n");
        break;
        case SCOUT_STEALTH:
            peritem += 60;
            newUnitOrders << wxT("STUDY STEALTH\n");
            newUnitOrders << wxT("@;; name unit \"scout stealth\"\n");
            newUnitOrders << wxT("TURN\n@work\nENDTURN\n");
        break;
        case SCOUT_GUARD:
            peritem += 20;
            newUnitOrders << wxT("STUDY COMBAT\n");
            newUnitOrders << wxT("@;; name unit guard\n");
            newUnitOrders << wxT("TURN\navoid 0\nhold 1\n@guard 1\n@work\nENDTURN\n");
        default: ;
    }

    CUnit * pUnitNew = gpApp->m_pAtlantis->SplitUnit(pUnit, newUnitId);
    if (pUnitNew)
        pUnitNew->Orders << newUnitOrders.ToUTF8();

    pUnit->Orders.TrimRight(TRIM_ALL);
    if (!pUnit->Orders.IsEmpty())
        pUnit->Orders << EOL_SCR;

    if (peritem)
    {
        pUnit->Orders << "GIVE NEW " << (long)(newUnitId) << " " << (long)peritem << " SILV" << EOL_SCR;
    }

    if (m_pCurLand)
        gpApp->m_pAtlantis->RunOrders(m_pCurLand);

    gpApp->SetOrdersChanged(TRUE);

    return true;
}

void CUnitPane::OnPopupMenuScoutSimple(wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        if (CreateScout(pUnit, SCOUT_SIMPLE))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutMove(wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        if (CreateScout(pUnit, SCOUT_MOVE))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutObserver(wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        if (CreateScout(pUnit, SCOUT_OBSERVER))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutStealth(wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        if (CreateScout(pUnit, SCOUT_STEALTH))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutGuard(wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        if (CreateScout(pUnit, SCOUT_GUARD))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuShareSilv  (wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    if (pUnit)
    {
        gpApp->SetOrdersChanged(gpApp->m_pAtlantis->ShareSilver(pUnit)
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }
}


//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuGiveEverything (wxCommandEvent& event)
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;
    wxString     N;

    if (pUnit)
    {
        N = wxGetTextFromUser(wxT("Give everything to unit"), wxT("Confirm"));

        gpApp->SetOrdersChanged(gpApp->m_pAtlantis->GenGiveEverything(pUnit, N.mb_str())
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }

}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuDiscardJunk(wxCommandEvent& WXUNUSED(event))
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;

    // For new units: delete the entire unit
    // For normal units: issue orders to give away all useless items
    if (pUnit)
    {
        if (IS_NEW_UNIT(pUnit))
        {
            CLand * pLand = gpApp->m_pAtlantis->GetLand(pUnit->LandId);
            pLand->RemoveUnit(pUnit);
            delete pUnit;
            gpApp->SetOrdersChanged(true);
        }
        else
            gpApp->SetOrdersChanged(gpApp->m_pAtlantis->DiscardJunkItems(pUnit, gpApp->GetConfig(SZ_SECT_UNITPROP_GROUPS, PRP_JUNK_ITEMS))
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuDetectSpies(wxCommandEvent& WXUNUSED(event))
{
    CUnit* pUnit = GetSelectedUnit();
    CUnitOrderEditPane  * pOrders;
    BOOL         DoCheck;

    if (pUnit)
    {
        DoCheck = atol(gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_SPY_DETECT_WARNING));
        if (DoCheck &&
            wxYES != wxMessageBox(wxT("Really generate orders for spy detection?  It might freeze the program on Linux!"), wxT("Confirm"), wxYES_NO, NULL))
            return;

        gpApp->SetOrdersChanged(gpApp->m_pAtlantis->DetectSpies(pUnit,
                                                                atol(gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_SPY_DETECT_LO)),
                                                                atol(gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_SPY_DETECT_HI)),
                                                                atol(gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_SPY_DETECT_AMT)))
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuAddUnitToTracking (wxCommandEvent& WXUNUSED(event))
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    int          sectidx;
    CStr         S;
    const char * szName;
    const char * szValue;
    BOOL         found = FALSE;
    BOOL         ManyUnits = (GetSelectedItemCount() > 1);

    sectidx = gpApp->GetSectionFirst(SZ_SECT_UNIT_TRACKING, szName, szValue);
    while (sectidx >= 0)
    {
        if (!S.IsEmpty())
            S << ",";
        S << szName;
        sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_UNIT_TRACKING, szName, szValue);
    }
    if (S.IsEmpty())
        S = "Default";


    if (pUnit || ManyUnits)
    {
        CComboboxDlg dlg(this, "Add unit to a tracking group", "Select a group to add unit to.\nTo create a new group, just type in it's name.", S.GetData());
        if (wxID_OK == dlg.ShowModal())
        {

            idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            while (idx>=0)
            {
                pUnit = GetUnit(idx);
                found = FALSE;

                szValue = gpApp->GetConfig(SZ_SECT_UNIT_TRACKING, dlg.m_Choice.GetData());
                while (szValue && *szValue)
                {
                    szValue = S.GetToken(szValue, ',');
                    if (atol(S.GetData()) == pUnit->Id)
                    {
                        found = TRUE;
                        break;
                    }
                }
                if (found)
                    wxMessageBox(wxT("The unit is already in the group."));
                else
                {
                    S = gpApp->GetConfig(SZ_SECT_UNIT_TRACKING, dlg.m_Choice.GetData());
                    S.TrimRight(TRIM_ALL);
                    if (!S.IsEmpty())
                        S << ",";
                    S << pUnit->Id;
                    gpApp->SetConfig(SZ_SECT_UNIT_TRACKING, dlg.m_Choice.GetData(), S.GetData());
                }

                idx   = GetNextItem(idx, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
        }
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuUnitFlags (wxCommandEvent& WXUNUSED(event))
{
    long         idx;
    CUnit      * pUnit;
    unsigned int flags = 0;
    BOOL         ManyUnits = (GetSelectedItemCount() > 1);

    if (!ManyUnits)
    {
        idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        pUnit = GetUnit(idx);
        if (pUnit)
            flags = pUnit->Flags;
    }

    CUnitFlagsDlg dlg(this, ManyUnits?eManyUnits:eThisUnit, flags & UNIT_CUSTOM_FLAG_MASK);
    int rc = dlg.ShowModal();
    idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (idx>=0)
    {
        pUnit = GetUnit(idx);

        switch (rc)
        {
        case wxID_OK:
            pUnit->Flags    &= ~UNIT_CUSTOM_FLAG_MASK;
            pUnit->FlagsOrg &= ~UNIT_CUSTOM_FLAG_MASK;
            pUnit->Flags    |= (dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsOrg |= (dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsLast = ~pUnit->Flags;
            break;
        case ID_BTN_SET_ALL_UNIT:
            pUnit->Flags    |= (dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsOrg |= (dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsLast = ~pUnit->Flags;
            break;
        case ID_BTN_RMV_ALL_UNIT:
            pUnit->Flags    &= ~(dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsOrg &= ~(dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsLast = ~pUnit->Flags;
            break;
        }
        idx   = GetNextItem(idx, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
//    Update(m_pCurLand);
    SetData(sel_by_no, -1, FALSE);

}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuIssueOrders(wxCommandEvent& event)
{
    long         idx;
    CUnit      * pUnit;
    CUnitOrderEditPane  * pOrders;
    BOOL         Changed = FALSE;
    CGetTextDlg  dlg(this, "Order", "Orders for the selected units");

    if (wxID_OK != dlg.ShowModal())
        return;
    dlg.m_Text.TrimRight(TRIM_ALL);
    if (dlg.m_Text.IsEmpty())
        return;

    idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (idx>=0)
    {
        pUnit = GetUnit(idx);
        if (pUnit->IsOurs)
        {
            Changed = TRUE;
            pUnit->Orders.TrimRight(TRIM_ALL);
            if (!pUnit->Orders.IsEmpty())
                pUnit->Orders << EOL_SCR;
            pUnit->Orders << dlg.m_Text;
        }
        idx   = GetNextItem(idx, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    if (Changed)
    {
        gpApp->SetOrdersChanged(TRUE);
        gpApp->m_pAtlantis->RunOrders(m_pCurLand);
        Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------
