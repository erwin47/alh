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

#include <sstream>

#include <wx/listctrl.h>
#include <wx/version.h>

#include "ahapp.h"
#include "objs.h"
//#include "data.h"
#include "consts_ah.h"
#include <codecvt>

#include "listpane.h"
#include "data_control.h"

//------------------------------------------------------------------------

CListPane::CListPane(wxWindow *parent, wxWindowID id, long style)
          :wxListCtrl()
{
    m_pParent        = parent;
    m_pLayout        = NULL;
    m_pData          = NULL;

    //is_selection_automatic_ = false;

    // Disable the SystemTheme before calling Create
    //   This prevents gaps in the backgroundcolor of selected rows.
    #if wxCHECK_VERSION(3, 1, 0)
      this->EnableSystemTheme(false);
    #endif
    Create(parent, id, wxDefaultPosition, wxDefaultSize, style);

    for (int i=0; i<NUM_SORTS; i++)
        m_SortKey[i] = NULL;
    m_SortKey[NUM_SORTS-1] = PRP_ID;
    Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CListPane::OnListLostFocus), NULL);
    SetSingleStyle(wxLC_VRULES, false);
}

void CListPane::OnListLostFocus(wxFocusEvent& event)
{
    // Inhibite change of selected item color when a list loses focus
    event.Skip(false);
}

//-----------------------------------------------------------------------

CListPane::~CListPane()
{
    for (int i=0; i<NUM_SORTS-1; i++)
    {
        if (m_SortKey[i])
            free((void*)m_SortKey[i]);
        m_SortKey[i] = NULL;
    }
}

//-----------------------------------------------------------------------

void CListPane::ApplyFonts()
{
}

//-----------------------------------------------------------------------

void CListPane::SetSortName(unsigned short key, const char * sortname)
{
    int               i;
    CListLayoutItem * p;
    wxListItem        item;
    CStr              S;
    int               width;

    if (!sortname)
        return; // can not set it
    if (key >= NUM_SORTS-1 )
        return;


    if (m_SortKey[key] && 0==stricmp(m_SortKey[key], sortname))
        return; // no need to set

    if (m_SortKey[key])
    {
        // reset old sort caption
        if (m_pLayout)
            for (i=0; i<m_pLayout->Count(); i++)
            {
                p = (CListLayoutItem*)m_pLayout->At(i);
                if (0==stricmp(p->m_Name, m_SortKey[key]))
                {
                    width  = GetColumnWidth(i);
                    item.m_mask = wxLIST_MASK_TEXT;
                    item.SetText(wxString::FromAscii(p->m_Caption));
                    SetColumn(i, item);
                    SetColumnWidth(i, width);
                }
            }

        free((void*)m_SortKey[key]);
    }

    m_SortKey[key] = strdup(sortname);

    // set new sort caption
    for (key=0; key<NUM_SORTS-1; key++)
        if (m_pLayout)
            for (i=0; i<m_pLayout->Count(); i++)
            {
                p = (CListLayoutItem*)m_pLayout->At(i);
                if (m_SortKey[key] && (0==stricmp(p->m_Name, m_SortKey[key])))
                {
                    S.Format("%d ", key+1);
                    S << p->m_Caption;
                    width  = GetColumnWidth(i);
                    item.m_mask   = wxLIST_MASK_TEXT;
                    item.SetText(wxString::FromAscii(S.GetData()));
                    SetColumn(i, item);
                    SetColumnWidth(i, width);
                }
            }

}

//-----------------------------------------------------------------------

const char * CListPane::GetSortName(unsigned short key)
{
    if (key >= NUM_SORTS-1 )
        return NULL;
    return m_SortKey[key];
}

//------------------------------------------------------------------------

void CListPane::SetLayout()
{
    int               i;
    CListLayoutItem * p;

    DeleteAllColumns();

    for (i=0; i < NUM_SORTS-1; i++)
        if (m_SortKey[i])
        {
            free((void*)m_SortKey[i]);
            m_SortKey[i] = NULL;
        }


    if (m_pLayout)
        for (i=0; i<m_pLayout->Count(); i++)
        {
            p = (CListLayoutItem*)m_pLayout->At(i);

            InsertColumn(i, wxString::FromAscii(p->m_Caption), (p->m_Flags&LIST_FLAG_ALIGN_RIGHT)?wxLIST_FORMAT_RIGHT:wxLIST_FORMAT_LEFT, p->m_Width);
        }
    Refresh();
}

//------------------------------------------------------------------------

void CListPane::SetData(eSelMode selmode, long seldata, BOOL FullUpdate)
{
    int                row, col;
    TPropertyHolder  * dataitem;
    CListLayoutItem  * layoutitem;
    const void       * value;
    EValueType         valuetype;
    wxListItem         info;
    int                selno = -1;

    if (FullUpdate)
        DeleteAllItems();

    info.SetMask(wxLIST_MASK_TEXT);
    if (m_pData && m_pLayout)
        for (row=0; row<m_pData->Count(); row++)
        {
            dataitem = (TPropertyHolder*)m_pData->At(row);
            info.SetId(row);//m_itemId = row;
            for (col=0; col<m_pLayout->Count(); col++)
            {
                layoutitem = (CListLayoutItem*)m_pLayout->At(col);

                info.SetText("");
                info.SetColumn(col);

                CUnit* unit = dynamic_cast<CUnit*>(dataitem);
                if (unit != nullptr && 
                    stricmp(layoutitem->m_Name, PRP_FLAGS_STANDARD) == 0)
                {
                    
                    std::stringstream ss;
                    if (unit->Flags & UNIT_FLAG_GUARDING         )  ss << 'g';
                    if (unit->Flags & UNIT_FLAG_AVOIDING         )  ss << 'a';
                    if (unit->Flags & UNIT_FLAG_BEHIND           )  ss << 'b';
                    if (unit->Flags & UNIT_FLAG_REVEALING_UNIT   )  ss << "rU";
                    else if (unit->Flags & UNIT_FLAG_REVEALING_FACTION)  ss << "rF";
                    if (unit->Flags & UNIT_FLAG_HOLDING          )  ss << 'h';
                    if (unit->Flags & UNIT_FLAG_RECEIVING_NO_AID )  ss << 'i';
                    if (unit->Flags & UNIT_FLAG_CONSUMING_UNIT   )  ss << "cU";
                    else if (unit->Flags & UNIT_FLAG_CONSUMING_FACTION)  ss << "cF";
                    if (unit->Flags & UNIT_FLAG_NO_CROSS_WATER   )  ss << 'x';
                    if (unit->Flags & UNIT_FLAG_SPOILS_NONE)  ss << "sN"; 
                    if (unit->Flags & UNIT_FLAG_SPOILS_WALK)  ss << "sW";
                    if (unit->Flags & UNIT_FLAG_SPOILS_RIDE)  ss << "sR";
                    if (unit->Flags & UNIT_FLAG_SPOILS_FLY)  ss << "sF";
                    if (unit->Flags & UNIT_FLAG_SPOILS_SWIM)  ss << "sS";
                    if (unit->Flags & UNIT_FLAG_SPOILS_SAIL)  ss << "sL";
                    if (unit->Flags & UNIT_FLAG_SHARING )  ss << 'z';
                    wxString temp(ss.str());
                    info.SetText(temp);
                }
                else if (unit != nullptr && 
                    stricmp(layoutitem->m_Name, PRP_MONTHLONG_ACTION) == 0)
                {
                    std::stringstream ss;
                    if (unit_control::flags::is_pillaging(unit)  )  ss << "$$" << unit->monthlong_descr_;
                    if (unit_control::flags::is_taxing(unit)     )  ss << '$' << unit->monthlong_descr_;
                    if (unit->Flags & UNIT_FLAG_PRODUCING        )  ss << 'p' << unit->monthlong_descr_;
                    if (unit_control::flags::is_entertaining(unit)) ss << 'E' << unit->monthlong_descr_;
                    if (unit->Flags & UNIT_FLAG_STUDYING         )  ss << 'S' << unit->monthlong_descr_;
                    if (unit_control::flags::is_teaching(unit)   )  ss << 'T' << unit->monthlong_descr_;        
                    if (unit_control::flags::is_working(unit)    )  ss << 'W' << unit->monthlong_descr_;
                    if (unit_control::flags::is_moving(unit)     )  ss << 'M' << unit->monthlong_descr_;
                    wxString temp(ss.str());
                    info.SetText(temp);                    
                }
                else if (unit != nullptr && 
                    stricmp(layoutitem->m_Name, "weight") == 0)
                {
                    long weights[5] = {0};
                    unit_control::get_weights(unit, weights);
                    wxString res;
                    res << weights[0];
                    info.SetText(res);    
                }
                else if (unit != nullptr && 
                    stricmp(layoutitem->m_Name, "weight_walk") == 0)
                {
                    long weights[5] = {0};
                    unit_control::get_weights(unit, weights);
                    wxString res;
                    res << weights[1] - weights[0];
                    info.SetText(res);    
                }
                else if (unit != nullptr && 
                    stricmp(layoutitem->m_Name, "weight_ride") == 0)
                {
                    long weights[5] = {0};
                    unit_control::get_weights(unit, weights);
                     wxString res;
                    res << weights[2] - weights[0];
                    info.SetText(res);    
                }
                else if (unit != nullptr && 
                    stricmp(layoutitem->m_Name, "weight_fly") == 0)
                {
                    long weights[5] = {0};
                    unit_control::get_weights(unit, weights);
                    wxString res;
                    res << weights[3] - weights[0];
                    info.SetText(res);    
                }
                else if (unit != nullptr && 
                    stricmp(layoutitem->m_Name, "weight_swim") == 0)
                {
                    long weights[5] = {0};
                    unit_control::get_weights(unit, weights);
                    wxString res;
                    res << weights[4] - weights[0];
                    info.SetText(res);     
                }
                else if (dataitem->GetProperty(layoutitem->m_Name, valuetype, value ))
                {
                    switch (valuetype)
                    {
                    case eLong:
                        if (((long)value<0) && (0==stricmp(PRP_ID, layoutitem->m_Name)))
                            value = 0; // mask negative ids of new units
                        info.SetText(std::to_string((long)value));
                        break;
                    case eCharPtr:
                        info.SetText(std::string((const char*)value));
                        break;
                    default:
                        break;
                    }
                }

                info.m_format = (layoutitem->m_Flags&LIST_FLAG_ALIGN_RIGHT)?wxLIST_FORMAT_RIGHT:wxLIST_FORMAT_LEFT;
                info.m_mask  |= wxLIST_MASK_FORMAT;

                if (dataitem->GetProperty( PRP_ID, valuetype, value ) && (eLong==valuetype))
                {
                    info.m_data  = (long)value;
                    info.m_mask |= wxLIST_MASK_DATA;
                    if ((sel_by_id==selmode) && (selno<0) && ((long)value==seldata))
                        selno = row;
                }

                if ( (0==col) && (row>=GetItemCount()) )
                {
                    if ((sel_by_no==selmode) && (selno<0) && (row==seldata))
                        selno = row;

                    InsertItem(info);

                    info.m_mask   = wxLIST_MASK_TEXT;
                }
                else
                    SetItem(info);
            }
            if (dataitem->GetProperty(PRP_GUI_COLOR, valuetype, value ) && (eLong==valuetype))
            {
                wxColour backgroundColor;
                switch ((long)value)
                {
                    case 1:
                        StrToColor(&backgroundColor, gpApp->GetConfig(SZ_SECT_COLORS, SZ_UNIT_MOVING_OUT));
                    break;
                    case 2:
                        StrToColor(&backgroundColor, gpApp->GetConfig(SZ_SECT_COLORS, SZ_UNIT_ARRIVING));
                    break;
                    case 3:
                        StrToColor(&backgroundColor, gpApp->GetConfig(SZ_SECT_COLORS, SZ_UNIT_GUARDING));
                    break;
                    case 4:
                        StrToColor(&backgroundColor, gpApp->GetConfig(SZ_SECT_COLORS, SZ_UNIT_HAS_ERRORS));
                    break;
                    case 5:
                        StrToColor(&backgroundColor, gpApp->GetConfig(SZ_SECT_COLORS, SZ_UNIT_OWNS_BUILDING));
                    break;
                    case 6:
                        StrToColor(&backgroundColor, gpApp->GetConfig(SZ_SECT_COLORS, SZ_UNIT_END_MOVEMENT_ORDER));
                    break;
                    default:
                        backgroundColor = wxColor("#FFFFFF");
                }
                SetItemBackgroundColour(row, backgroundColor);
            }
        }

    while (m_pData->Count() < GetItemCount())
        DeleteItem(m_pData->Count());

    if (selno>=0)
    {
        //this->is_selection_automatic_ = true;
        SetItemState(selno, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        EnsureVisible(selno);
    }
}

//------------------------------------------------------------------------

void CListPane::Sort()
{
    long         idx;
    long         data=0;
    wxListItem   info;

    idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (idx>=0)
    {
        info.SetId(idx);
        info.m_mask  = wxLIST_MASK_DATA;
        GetItem(info);
        data = info.m_data;
    }

    if (m_pData )
    {
        m_pData->SetSortMode(m_SortKey, NUM_SORTS);
        SetData(sel_by_id, data, TRUE);
    }
}

//------------------------------------------------------------------------

