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


BEGIN_EVENT_TABLE(CUnitPane, wxListCtrl)
    EVT_LIST_ITEM_SELECTED   (list_units_hex, CUnitPane::OnSelected)
    EVT_LIST_COL_CLICK       (list_units_hex, CUnitPane::OnColClicked)
    EVT_LIST_ITEM_RIGHT_CLICK(list_units_hex, CUnitPane::OnRClick)
    EVT_IDLE                 (CUnitPane::OnIdle)

    EVT_MENU             (menu_Popup_ShareSilv     , CUnitPane::OnPopupMenuShareSilv      )
    EVT_MENU             (menu_Popup_Teach         , CUnitPane::OnPopupMenuTeach          )
    EVT_MENU             (menu_Popup_Split         , CUnitPane::OnPopupMenuSplit          )
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
          :CListPane(parent, id, wxLC_REPORT)
{
    m_pUnits   = NULL;
    m_pCurLand = NULL;
    m_ColClicked = -1;
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

void CUnitPane::Update(CLand * pLand)
{
    int               i;
    CUnit           * pUnit;
    eSelMode          selmode = sel_by_no;
    long              seldata = 0;
    BOOL              FullUpdate = (pLand != m_pCurLand); // if not full mode, refresh new units only
    wxListItem        info;
    CBaseColl         ArrivingUnits;
    long              GuiColor;


    // It is a must, since some locations pointed to by stored pointers may be invalid at the moment.
    // Namely all new units are deleted when orders are processed.
    m_pUnits->DeleteAll();
//    m_pFactions->DeleteAll();

    if (!FullUpdate)
        for (i=GetItemCount()-1; i>=0; i--)
        {
            info.m_itemId = i;
            info.m_col    = 0;
            info.m_mask   = wxLIST_MASK_DATA;
            GetItem(info);
        }

    if (pLand)
    {
        for (i=0; i<pLand->Units.Count(); i++)
        {
            pUnit = (CUnit*)pLand->Units.At(i);
            if (pUnit && pUnit->pMovement)
                GuiColor = 1;
            else
                GuiColor = 0;
            pUnit->SetProperty(PRP_GUI_COLOR, eLong, (void*)GuiColor, eBoth);
            m_pUnits->AtInsert(m_pUnits->Count(), pUnit);
        }
        m_pUnits->SetSortMode(m_SortKey, NUM_SORTS);

        gpApp->GetUnitsMovingIntoHex(pLand->Id, ArrivingUnits);
        for (i=0; i<ArrivingUnits.Count(); ++i)
        {
            pUnit = (CUnit*)ArrivingUnits.At(i);
            if (pUnit->LandId != pLand->Id)
            {
                GuiColor = 2;
                pUnit->SetProperty(PRP_GUI_COLOR, eLong, (void*)GuiColor, eBoth);
                m_pUnits->AtInsert(m_pUnits->Count(), pUnit);
            }
        }

        if (pLand->guiUnit)
        {
            seldata = pLand->guiUnit;
            selmode = sel_by_id;
        }
    }
    m_pCurLand = pLand;

    SetData(selmode, seldata, FullUpdate);

    if ((0==m_pUnits->Count()) || !FullUpdate) // otherwise will be called from OnSelected()
        gpApp->OnUnitHexSelectionChange(GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED));
}

//--------------------------------------------------------------------------

void CUnitPane::SelectUnit(long UnitId)
{
    int               i;
    wxListItem        info;

    for (i=GetItemCount()-1; i>=0; i--)
    {
        info.m_itemId = i;
        info.m_col    = 0;
        info.m_mask   = wxLIST_MASK_DATA;
        GetItem(info);
        if ((int)info.m_data==UnitId)
        {
            SetItemState(i, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
            EnsureVisible(i);
        }
        else
            SetItemState(i, 0, wxLIST_STATE_SELECTED);
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
    x = m_pLayout ? (m_pLayout->Count()+20) : 100;
    for (i=x; i>=0; i--)
        DeleteColumn(i);

    LoadUnitListHdr();

    if (m_pCurLand && m_pCurLand->guiUnit)
    {
        seldata = m_pCurLand->guiUnit;
        selmode = sel_by_id;
    }

    SetData(selmode, seldata, TRUE);
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
    CEditPane  * pOrders;
    CUnit      * pUnit = GetUnit(event.m_itemIndex);
    bool         changed = false;
    int          idx, i;

    if (pUnit)
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            changed = pOrders->SaveModifications();

        if (changed && m_pCurLand)
        {
            CBaseObject Dummy;
            CUnit * pOldUnit = NULL;

            Dummy.Id = m_pCurLand->guiUnit;
            if (m_pCurLand->Units.Search(&Dummy, idx))
                pOldUnit = (CUnit*)m_pCurLand->Units.At(idx);
            else
            {
                CBaseColl         ArrivingUnits;

                gpApp->GetUnitsMovingIntoHex(m_pCurLand->Id, ArrivingUnits);
                for (i=0; i<ArrivingUnits.Count(); ++i)
                {
                    pOldUnit = (CUnit*)ArrivingUnits.At(i);
                    if (pOldUnit->Id == m_pCurLand->guiUnit)
                    {
                        CLand * pEditedLand = gpApp->m_pAtlantis->GetLand(pOldUnit->LandId);
                        if (pEditedLand)
                        {
                            gpApp->m_pAtlantis->RunOrders(pEditedLand);
                            if (m_pCurLand != pEditedLand)
                                gpApp->m_pAtlantis->RunOrders(m_pCurLand);
                        }
                        break;
                    }
                }
            }
        }

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        if (changed)
        {
            gpApp->EditPaneChanged(pOrders);
        }
//        Update(m_pCurLand);
        gpApp->OnUnitHexSelectionChange(event.m_itemIndex);
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
    CUnit      * pUnit = GetUnit(idx);
    int          x=0,y;

    int          width, height;

    int nItems = GetSelectedItemCount();


    wxDisplaySize(&width, &height);
    y = event.GetPoint().y;
    ClientToScreen(&x, &y);
    if (height-y < 150)
        y = height-150;
    ScreenToClient(&x, &y);

    if (nItems>1)
    {
        // multiple units
        menu.Append(menu_Popup_IssueOrders     , wxT("Issue orders"));
        menu.Append(menu_Popup_UnitFlags       , wxT("Set custom flags")    );
        menu.Append(menu_Popup_AddToTracking   , wxT("Add to a tracking group"));

        PopupMenu( &menu, event.GetPoint().x, y);
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
                }
                menu.Append(menu_Popup_Teach         , wxT("Teach")             );
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

            PopupMenu( &menu, event.GetPoint().x, y);
        }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuTeach (wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit)
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        gpApp->SetOrdersChanged(gpApp->m_pAtlantis->GenOrdersTeach(pUnit)
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuSplit(wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        // do it here

        CUnitSplitDlg dlg(this, pUnit);
        if (wxID_OK == dlg.ShowModal()) // it will modify unit's orders
            gpApp->SetOrdersChanged(TRUE);

        Update(m_pCurLand);
    }
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

    gpApp->m_pAtlantis->ReadPropertyName(m_pCurLand->PeasantRace.GetData(), race);

    if (race.IsEmpty())
        return false; // When there is no race to buy, we cannot make a scout.

    if (race.FindSubStr("ORCS") != -1) race = "ORC";

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
    if (! (pUnit->FlagsOrg & UNIT_FLAG_REVEALING_FACTION)) newUnitOrders << wxT("reveal faction\n");
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
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        if (CreateScout(pUnit, SCOUT_SIMPLE))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutMove(wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        if (CreateScout(pUnit, SCOUT_MOVE))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutObserver(wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        if (CreateScout(pUnit, SCOUT_OBSERVER))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutStealth(wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        if (CreateScout(pUnit, SCOUT_STEALTH))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuScoutGuard(wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit && !IS_NEW_UNIT(pUnit))
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        if (CreateScout(pUnit, SCOUT_GUARD))
            Update(m_pCurLand);
    }
}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuShareSilv  (wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    if (pUnit)
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        gpApp->SetOrdersChanged(gpApp->m_pAtlantis->ShareSilver(pUnit)
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }
}


//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuGiveEverything (wxCommandEvent& event)
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;
    wxString     N;

    if (pUnit)
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        N = wxGetTextFromUser(wxT("Give everything to unit"), wxT("Confirm"));

        gpApp->SetOrdersChanged(gpApp->m_pAtlantis->GenGiveEverything(pUnit, N.mb_str())
                               || gpApp->GetOrdersChanged());
        Update(m_pCurLand);
    }

}

//--------------------------------------------------------------------------

void CUnitPane::OnPopupMenuDiscardJunk(wxCommandEvent& WXUNUSED(event))
{
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;

    // For new units: delete the entire unit
    // For normal units: issue orders to give away all useless items
    if (pUnit)
    {
        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

        if (IS_NEW_UNIT(pUnit))
        {
            m_pCurLand->RemoveUnit(pUnit);
            DeleteItem(idx);
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
    long         idx   = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    CUnit      * pUnit = GetUnit(idx);
    CEditPane  * pOrders;
    BOOL         DoCheck;

    if (pUnit)
    {
        DoCheck = atol(gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_SPY_DETECT_WARNING));
        if (DoCheck &&
            wxYES != wxMessageBox(wxT("Really generate orders for spy detection?  It might freeze the program on Linux!"), wxT("Confirm"), wxYES_NO, NULL))
            return;

        pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
        if (pOrders)
            pOrders->SaveModifications();

        if (m_pCurLand)
            m_pCurLand->guiUnit = pUnit->Id;

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
    CEditPane  * pOrders;
    BOOL         Changed = FALSE;
    CGetTextDlg  dlg(this, "Order", "Orders for the selected units");

    if (wxID_OK != dlg.ShowModal())
        return;
    dlg.m_Text.TrimRight(TRIM_ALL);
    if (dlg.m_Text.IsEmpty())
        return;


    pOrders = (CEditPane*)gpApp->m_Panes[AH_PANE_UNIT_COMMANDS];
    if (pOrders)
        pOrders->SaveModifications();

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
