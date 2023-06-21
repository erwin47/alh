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

#include "cstr.h"
#include "collection.h"
#include "cfgfile.h"
#include "files.h"
#include "atlaparser.h"
#include "data.h"
#include "hash.h"

#include "objs.h"
#include "ahapp.h"
#include "editpane.h"

#include "data_control.h"

//--------------------------------------------------------------------


class CEditorForPane : public wxTextCtrl
{
public:
    CEditorForPane(CEditPane * parent);

protected :
    void         OnKillFocus(wxFocusEvent& event);
    void         OnMouseEvent(wxMouseEvent& event);

    CEditPane * m_pParent;

    //DECLARE_EVENT_TABLE()
};

//--------------------------------------------------------------------

//BEGIN_EVENT_TABLE(CEditorForPane, wxTextCtrl)    
//    EVT_LEFT_DCLICK      (    CEditorForPane::OnMouseEvent     )
//    EVT_KILL_FOCUS       (    CEditorForPane::OnKillFocus      )
//END_EVENT_TABLE()


//--------------------------------------------------------------------

CEditorForPane::CEditorForPane(CEditPane * parent)
               :wxTextCtrl(parent, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE)
{
    m_pParent = parent;

    this->Bind(wxEVT_LEFT_DCLICK, &CEditorForPane::OnMouseEvent, this);
    this->Bind(wxEVT_KILL_FOCUS, &CEditorForPane::OnKillFocus, this);
}

//--------------------------------------------------------------------

void CEditorForPane::OnKillFocus(wxFocusEvent& event)
{
    m_pParent->OnKillFocus(event);
    event.Skip();
}

void CEditorForPane::OnMouseEvent(wxMouseEvent& event)
{
    // maybe it can be handled in the parent control, but just to be sure
    m_pParent->OnMouseDClick();
    event.Skip();
}

//====================================================================


//BEGIN_EVENT_TABLE(CEditPane, wxPanel)
//    EVT_SIZE             (                      CEditPane::OnSize           )
//END_EVENT_TABLE()




//--------------------------------------------------------------------

CEditPane::CEditPane(wxWindow* parent, const wxString& header, BOOL editable, int WhichFont)
          : wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize )
{
    m_pSource       = NULL;
    m_pChanged      = NULL;

    m_pHeader       = new wxStaticText(this, -1, header, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE );
    m_HdrHeight     = 0;
    m_WhichFont     = WhichFont;
    m_pEditor       = new CEditorForPane(this);
    evt_text_event_ = false;
    m_ColorNormal   = m_pEditor->GetBackgroundColour() ;

    m_ColorReadOnly.Set(APPLY_COLOR_DELTA(m_ColorNormal.Red()),
                        APPLY_COLOR_DELTA(m_ColorNormal.Green()),
                        APPLY_COLOR_DELTA(m_ColorNormal.Blue()));

    SetReadOnly(!editable);

    this->Bind(wxEVT_SIZE, &CEditPane::OnSize, this);
}

CEditPane::~CEditPane()
{
}


//--------------------------------------------------------------------

void CEditPane::Init()
{
    ApplyFonts();
}

//--------------------------------------------------------------------

void CEditPane::SetSource(CStr * pSource, BOOL * pChanged)
{
    m_pSource   = pSource;
    m_pChanged  = pChanged;
    m_pEditor->SetValue(pSource?wxString::FromUTF8(pSource->GetData()):wxT(""));
}

void CEditPane::SetSource(const std::string& source, BOOL * pChanged)
{
    if (m_pSource)
        m_pSource->SetStr(source.c_str());
    m_pChanged  = pChanged;
    m_pEditor->SetValue(source);
}

//--------------------------------------------------------------------

void CEditPane::Update()
{
}

//--------------------------------------------------------------------

BOOL CEditPane::SaveModifications()
{
    if (m_pEditor->IsModified())
    {
        if (m_pSource)
            m_pSource->SetStr(m_pEditor->GetValue().mb_str());
        if (m_pChanged)
            *m_pChanged = TRUE;
        m_pEditor->DiscardEdits();

		if (!m_pSource && !m_pChanged)
			return FALSE;

        return TRUE;
    }
    else
        return FALSE;
}

//--------------------------------------------------------------------

void CEditPane::GetValue(CStr & value)
{
    value.SetStr(m_pEditor->GetValue().mb_str());
}

void CEditPane::SetHeader(const std::string& new_header)
{
    m_pHeader->SetLabel(new_header);
}

//--------------------------------------------------------------------

void CEditPane::ApplyFonts()
{
    m_pEditor->SetFont(*gpApp->m_Fonts[m_WhichFont]);

    if (m_pHeader)
    {
        wxCoord           w, h, descent, ext;

        m_pHeader->SetFont(*gpApp->m_Fonts[FONT_EDIT_HDR]);
        m_pHeader->GetTextExtent(wxT("A"), &w, &h, &descent, &ext);

        m_HdrHeight = h+2;
    }
}

//--------------------------------------------------------------------

void CEditPane::SetReadOnly(BOOL ReadOnly)
{
    if (m_pEditor)
    {
        m_pEditor->SetEditable(!ReadOnly);
        m_pEditor->SetBackgroundColour(ReadOnly?m_ColorReadOnly:m_ColorNormal);
    }
}

//--------------------------------------------------------------------

void CEditPane::OnKillFocus(wxFocusEvent& )
{
    test_ = 5;
}

//--------------------------------------------------------------------

void CEditPane::OnSize(wxSizeEvent& event)
{
    wxSize size = event.GetSize();

    if (m_pHeader && (m_HdrHeight>0))
        m_pHeader->SetSize(0, 0, size.x, m_HdrHeight, wxSIZE_ALLOW_MINUS_ONE);

    m_pEditor->SetSize(0, m_HdrHeight, size.x, size.y-m_HdrHeight, wxSIZE_ALLOW_MINUS_ONE);

}

//--------------------------------------------------------------------

void CEditPane::OnMouseDClick()
{
    gpApp->EditPaneDClicked(this);
}

//===========================================

CUnitOrderEditPane::CUnitOrderEditPane(wxWindow *parent, const wxString &header, BOOL editable, int WhichFont)
          :CEditPane(parent, header, editable, WhichFont )
{
    unit_ = NULL;
    unit_order_pane_modified_ = false;
#ifdef __APPLE__
    m_pEditor->OSXDisableAllSmartSubstitutions();
#endif
    m_pEditor->Bind(wxEVT_TEXT, &CUnitOrderEditPane::OnOrderModified, this);
    //m_pEditor->Bind(wxEVT_KILL_FOCUS, &CUnitOrderEditPane::OnOrderModified, this);

}

void CUnitOrderEditPane::OnOrderModified(wxCommandEvent& event)
{
    evt_text_event_ = true;
    event.Skip();
    //EVT_KILL_FOCUS
 /*   if (unit_ != NULL && m_pEditor->IsModified())
    {
        unit_order_pane_modified_ = true;
        //unit_->Orders.SetStr(m_pEditor->GetValue().mb_str());
        //unit_->orders_ = orders::parser::parse_lines_to_orders(std::string(unit_->Orders.GetData(), unit_->Orders.GetLength()));
        //m_pEditor->DiscardEdits();

        //need to rerun orders for at least current land and land where it goes:
        //CLand* land = land_control::get_land(unit_->LandId);
        //gpApp->m_pAtlantis->RunLandOrders(land);
    }*/
}

CUnitOrderEditPane::~CUnitOrderEditPane()
{    
}

CUnit* CUnitOrderEditPane::change_representing_unit(CUnit* unit)
{
    //compose new unit orders label
    int land_x, land_y, land_z;
    LandIdToCoord(unit->LandId, land_x, land_y, land_z);
    std::string label = unit_control::compose_unit_name(unit) + " in (" + std::to_string(land_x);
    label += "," + std::to_string(land_y) + "," + std::to_string(land_z) + ")";
    this->m_pHeader->SetLabel(label);
    CUnit* prev_unit = unit_;
    if (m_pEditor->IsModified() && evt_text_event_ && unit_ != NULL)
    {
        unit_->Orders.SetStr(m_pEditor->GetValue().mb_str());
        unit_->orders_ = orders::parser::parse_lines_to_orders(std::string(unit_->Orders.GetData(), unit_->Orders.GetLength()));
        unit_->orders_.is_modified_ = true;
        gpApp->orders_changed(true);
        evt_text_event_ = false;
        m_pEditor->DiscardEdits();
    }
    unit_ = unit;
    m_pEditor->ChangeValue(unit_ ? wxString::FromUTF8(unit_->Orders.GetData()) : wxT(""));
    return prev_unit;
}

void CUnitOrderEditPane::OnKillFocus(wxFocusEvent& )
{
    test_ = 25;
}


