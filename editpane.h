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

#ifndef __AH_EDIT_PANE_INCL__
#define __AH_EDIT_PANE_INCL__

#include <wx/wx.h>
#include <wx/textctrl.h>
class CStr;

class CEditPane : public wxPanel
{
public:
    CEditPane(wxWindow *parent, const wxString &header, BOOL editable, int WhichFont);
    virtual     ~CEditPane();

    virtual void Update();
    virtual void Init();
    void         SetSource(CStr * pSource, BOOL * pChanged);
    void         SetSource(const std::string& source, BOOL * pChanged);
    virtual void ApplyFonts();
    BOOL         SaveModifications();
    virtual void         OnKillFocus(wxFocusEvent& event);
    void         OnMouseDClick();
    void         SetReadOnly(BOOL ReadOnly);
    void         GetValue(CStr & value);
    void         SetHeader(const std::string& new_header);

    wxTextCtrl   * m_pEditor;
    bool        evt_text_event_;
    int test_;


protected :
    void         OnSize      (wxSizeEvent & event);

    CStr         * m_pSource;
    BOOL         * m_pChanged;
    wxStaticText * m_pHeader;
    int            m_HdrHeight;
    int            m_WhichFont;
    wxColour       m_ColorNormal;
    wxColour       m_ColorReadOnly;

    //DECLARE_EVENT_TABLE()
};

class CUnitOrderEditPane : public CEditPane
{
    CUnit*          unit_;
    bool            unit_order_pane_modified_;

public:
    CUnitOrderEditPane(wxWindow *parent, const wxString &header, BOOL editable, int WhichFont);
    virtual     ~CUnitOrderEditPane();
 
    CUnit*       change_representing_unit(CUnit* unit);

    void OnOrderModified(wxCommandEvent& event);
    virtual void         OnKillFocus(wxFocusEvent& event);
};

#endif
