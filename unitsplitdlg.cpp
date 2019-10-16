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

#include "wx/spinctrl.h"
#include "wx/dialog.h"


#include "cstr.h"
#include "collection.h"
#include "cfgfile.h"
#include "files.h"
#include "atlaparser.h"
#include "consts_ah.h"
#include "hash.h"
#include "ahapp.h"
#include "ahframe.h"

#include "unitsplitdlg.h"


#define SPACER_GENERIC 5
#define SPACER_NARROW  1

BEGIN_EVENT_TABLE(CUnitSplitDlg, wxDialog)
    EVT_BUTTON  (wxID_CANCEL  ,  CUnitSplitDlg::OnCancel)
    EVT_BUTTON  (wxID_OK      ,  CUnitSplitDlg::OnOk)
END_EVENT_TABLE()



//--------------------------------------------------------------------------

CUnitSplitDlg::CUnitSplitDlg(wxWindow *parent, CUnit * pUnit)
              :CResizableDlg( parent, wxT("Split Unit"), SZ_SECT_WND_SPLIT_UNIT_DLG )
{
    wxBoxSizer      * topsizer;
    wxBoxSizer      * sizer   ;
    //wxBoxSizer      * rowsizer;
    wxGridSizer     * gridsizer;
    wxStaticText    * st;
    int               rows, cols, idx;
    wxSpinCtrl      * pSpin;

    m_pUnit = pUnit;
    ScanProperties();

    topsizer = new wxBoxSizer( wxVERTICAL );
    m_btnOk         = new wxButton     (this, wxID_OK     , wxT("Set")    );
    m_btnCancel     = new wxButton     (this, wxID_CANCEL , wxT("Cancel") );
    m_spinUnitCount = new wxSpinCtrl   (this, -1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100000);
    m_textNewCommand= new wxTextCtrl  (this, -1, wxT(""), wxDefaultPosition, wxSize(225,150), wxTE_MULTILINE);

    sizer    = new wxBoxSizer( wxHORIZONTAL );
        sizer->Add(m_spinUnitCount , 0, wxALIGN_CENTER | wxALL , SPACER_GENERIC);
        st = new wxStaticText(this, -1, wxT("Number of new units to form"));
        sizer->Add(st              , 0, wxALIGN_CENTER | wxALL , SPACER_GENERIC);
    topsizer->Add(sizer        , 0, wxALL | wxGROW, SPACER_GENERIC );

    sizer    = new wxBoxSizer( wxHORIZONTAL );
        sizer->Add(m_textNewCommand , 1, wxGROW | wxALL , SPACER_GENERIC);
        st = new wxStaticText(this, -1, wxT("Orders for each new unit"));
        sizer->Add(st              , 0, wxALIGN_CENTER | wxALL , SPACER_GENERIC);
    topsizer->Add(sizer        , 1, wxALL | wxGROW, SPACER_GENERIC );

    st = new wxStaticText(this, -1, wxT("Give each new unit:"));
    topsizer->Add(st              , 0, wxALIGN_LEFT | wxALL , SPACER_GENERIC);

    cols = 2;
    rows = m_SplitControls.Count() / cols;
    while (rows > cols*5)
    {
        cols++;
        rows = m_SplitControls.Count() / cols;
    }
    while (rows*cols < m_SplitControls.Count())
        rows++;

    gridsizer = new wxGridSizer(rows, cols, 3, 3) ;
    for (idx=0; idx<m_SplitControls.Count(); idx++ )
    {
        pSpin = (wxSpinCtrl*)m_SplitControls.At(idx);

        sizer    = new wxBoxSizer( wxHORIZONTAL );
        st       = new wxStaticText(this, -1, pSpin->GetName());
        sizer->Add(st   , 1, wxALIGN_LEFT  | wxALL, SPACER_NARROW );
        sizer->Add(pSpin, 0, wxALL, SPACER_NARROW );

        gridsizer->Add(sizer, 1, wxGROW);
    }
    topsizer->Add(gridsizer, 0, wxALIGN_CENTER );

    sizer    = new wxBoxSizer( wxHORIZONTAL );
        sizer->Add(m_btnOk       , 0, wxALIGN_CENTER);
        sizer->Add(m_btnCancel   , 0, wxALIGN_CENTER | wxALL, SPACER_GENERIC);
    topsizer->Add(sizer, 0, wxALIGN_CENTER );

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_ALT, 13, wxID_OK);
    entries[1].Set(wxACCEL_CTRL, 13, wxID_OK);
    wxAcceleratorTable accel(2, entries);
    this->SetAcceleratorTable(accel);

    SetAutoLayout( TRUE );     // tell dialog to use sizer
    SetSizer( topsizer );      // actually set the sizer
    topsizer->Fit( this );            // set size to minimum size as calculated by the sizer
    topsizer->SetSizeHints( this );   // set size hints to honour mininum size}

    m_spinUnitCount->SetFocus();
    // m_btnOk->SetDefault(); // Commented because it steals 'enter/return' from the multiline wxTextCtrl.

    CResizableDlg::SetPos();
}


//--------------------------------------------------------------------------

CUnitSplitDlg::~CUnitSplitDlg()
{
    m_SplitControls.DeleteAll();
}

//--------------------------------------------------------------------------

void CUnitSplitDlg::ScanProperties()
{
    // make a spin control for each property

    int             propidx;
    CStr            propname;
    EValueType      type;
    const void    * value;
    wxSpinCtrl    * pSpin;

    propidx  = 0;
    propname = m_pUnit->GetPropertyName(propidx);
    while (!propname.IsEmpty())
    {
        if (m_pUnit->GetProperty(propname.GetData(), type, value, eOriginal) &&
            eLong==type &&
            !IsASkillRelatedProperty(propname.GetData()) &&
            0!=stricmp(PRP_SEQUENCE     , propname.GetData()) &&
            0!=stricmp(PRP_GUI_COLOR    , propname.GetData()) &&
            0!=stricmp(PRP_STRUCT_ID    , propname.GetData())&&
            0!=stricmp(PRP_FRIEND_OR_FOE, propname.GetData())
           )
        {
            pSpin = new wxSpinCtrl   (this, -1, wxT("1"));
            pSpin->SetRange(0, 0x7fffffff);
            pSpin->SetValue(0);
            pSpin->SetName(wxString::FromAscii(propname.GetData()));

            m_SplitControls.AtInsert(m_SplitControls.Count(), pSpin);
        }
        propname = m_pUnit->GetPropertyName(++propidx);
    }
}

//--------------------------------------------------------------------------

void CUnitSplitDlg::OnOk(wxCommandEvent& event)
{
    CLand * pLand;
    int     id,i,idx;
    CStr    S, Cmd;
    wxSpinCtrl * pSpin;
    wxString     sBoo;
    const char * p;

    pLand = gpApp->m_pAtlantis->GetLand(m_pUnit->LandId);
    if (pLand)
        id = pLand->GetNextNewUnitNo();
    else
        id = 1;

    sBoo = m_textNewCommand->GetValue();
    p = sBoo.mb_str();
    while (p && *p)
    {
        p = SkipSpaces(S.GetToken(p, '\n', TRIM_ALL));
        Cmd << S << EOL_SCR;
    }

    m_pUnit->Orders.TrimRight(TRIM_ALL);
    if (!m_pUnit->Orders.IsEmpty())
        m_pUnit->Orders << EOL_SCR;

    for (i=0; i<m_spinUnitCount->GetValue(); i++)
    {
        CUnit * pUnitNew = gpApp->m_pAtlantis->SplitUnit(m_pUnit, id+i);
        if (pUnitNew)
            pUnitNew->Orders << Cmd;

        for (idx=0; idx<m_SplitControls.Count(); idx++ )
        {
            pSpin = (wxSpinCtrl*)m_SplitControls.At(idx);

            if (pSpin->GetValue() > 0)
                m_pUnit->Orders << "GIVE NEW " << (long)(id+i) << " " << (long)pSpin->GetValue() << " " << pSpin->GetName().mb_str() << EOL_SCR;
        }
    }

    if (pLand)
        gpApp->m_pAtlantis->RunOrders(pLand);

    StoreSize();
    EndModal(wxID_OK);
}

//--------------------------------------------------------------------------

void CUnitSplitDlg::OnCancel(wxCommandEvent& event)
{
    StoreSize();
    EndModal(wxID_CANCEL);
}

//--------------------------------------------------------------------------
#include <wx/statline.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
namespace unit_control
{
    namespace flags
    {
        bool is_behind(CUnit* unit) {  return unit->Flags & UNIT_FLAG_BEHIND;  }
        bool is_guard(CUnit* unit) {  return unit->Flags & UNIT_FLAG_GUARDING;  }
        bool is_hold(CUnit* unit) {  return unit->Flags & UNIT_FLAG_HOLDING;  }
        bool is_noaid(CUnit* unit) {  return unit->Flags & UNIT_FLAG_RECEIVING_NO_AID;  }
        bool is_avoid(CUnit* unit) {  return unit->Flags & UNIT_FLAG_AVOIDING;  }
        bool is_nocross(CUnit* unit) {  return unit->Flags & UNIT_FLAG_NO_CROSS_WATER;  }
        bool is_reveal(CUnit* unit, std::string flag) {
            for(size_t i = 0; i < flag.size(); ++i)
                flag[i] = std::tolower(flag[i]);
            if (!flag.compare("unit"))
                return unit->Flags & UNIT_FLAG_REVEALING_UNIT;
            if (!flag.compare("faction"))
                return unit->Flags & UNIT_FLAG_REVEALING_FACTION;
            return false;
        }
        bool is_spoils(CUnit* unit, const std::string flag) {
            // not implemented
            return false;
        }

        bool is_consume(CUnit* unit, std::string flag) {
            for(size_t i = 0; i < flag.size(); ++i)
                flag[i] = std::tolower(flag[i]);
            if (!flag.compare("unit"))
                return unit->Flags & UNIT_FLAG_CONSUMING_UNIT;
            if (!flag.compare("faction"))
                return unit->Flags & UNIT_FLAG_CONSUMING_FACTION;
            return false;
        }

    }

};

namespace game_control
{
    namespace details
    {
        std::vector<Skill> get_skills_()
        {
            std::vector<Skill> ret;
            const char  * szName;
            const char  * szValue;
            int sectidx = gpApp->GetSectionFirst(SZ_SECT_SKILLS, szName, szValue);
            while (sectidx >= 0)
            {
                Skill temp;
                std::string name(szName);
                if (name.find(" ") == std::string::npos || 
                    name.find("[") == std::string::npos || 
                    name.find("]") == std::string::npos )
                    continue;
                temp.short_name_ = name.substr(name.find("["), name.find("]") - name.find("[")); 
                temp.long_name_ = name.substr(0, name.find(" "));
                temp.study_price_ = gpDataHelper->GetStudyCost(temp.long_name_.c_str());
                ret.push_back(temp);
                sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_SKILLS, szName, szValue);
            }
            return ret;
        }
    }
    const std::vector<Skill>& get_skills()
    {
        static std::vector<Skill> skills_list = details::get_skills_();
        return skills_list;
    }
}

BEGIN_EVENT_TABLE(CCreateNewUnit, wxDialog)
    EVT_BUTTON  (wxID_CANCEL  ,  CCreateNewUnit::OnCancel)
    EVT_BUTTON  (wxID_OK      ,  CCreateNewUnit::OnOk)
    EVT_SPINCTRL(-1           ,  CCreateNewUnit::onAnySpinUpdate)
    EVT_COMBOBOX(-1           ,  CCreateNewUnit::onAnyComboBoxUpdate)
    EVT_CHECKBOX(-1           ,  CCreateNewUnit::onAnyComboBoxUpdate)
END_EVENT_TABLE()

CCreateNewUnit::CCreateNewUnit(wxWindow *parent, CUnit * pUnit, CLand* pLand) : 
    CResizableDlg( parent, wxT("Create new unit"), "WINDOW_SPLIT_UNIT_DLG" ), 
    unit_(pUnit),
    land_(pLand)
{
    wxBoxSizer* temp_sizer;

    //buttons are horizontal sizered
    wxBoxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );    
    buttonsizer->Add(new wxButton(this, wxID_OK, wxT("Ok")), 1, wxALIGN_CENTER | wxALL);
    buttonsizer->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")), 1, wxALIGN_CENTER | wxALL);
    buttonsizer->Add(new wxStaticText(this, -1, wxT("  Copies: ")), 0, wxALL);
    spin_copies_amount_ = new wxSpinCtrl(this, -1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    buttonsizer->Add(spin_copies_amount_, 0, wxALL);
    //unit sizer consist of unit naming, buying, studying and silver sections
    wxBoxSizer* unitsizer = new wxBoxSizer( wxVERTICAL );

    //name unit
    wxBoxSizer* unit_names_sizer = new wxBoxSizer( wxVERTICAL );
    spin_new_num_alias_ = new wxSpinCtrl(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    spin_new_num_alias_->SetValue(pLand->GetNextNewUnitNo());
    text_name_ = new wxTextCtrl(this, -1, wxT(""));
    text_loc_description_ = new wxTextCtrl(this, -1, wxT(""));
    text_description_ = new wxTextCtrl(this, -1, wxT(""));

    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Alias(num):")), 0, wxALL);
    temp_sizer->Add(spin_new_num_alias_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Name: ")), 0, wxALL);
    temp_sizer->Add(text_name_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Local descr: ")), 0, wxALL);
    temp_sizer->Add(text_loc_description_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Description: ")), 0, wxALL);
    temp_sizer->Add(text_description_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    unitsizer->Add(unit_names_sizer, 0, 0);
    unitsizer->Add(new wxStaticLine(this), 0, 0);

    //buy units
    wxBoxSizer* unit_buy_sizer = new wxBoxSizer( wxVERTICAL );
    spin_buy_units_amount_ = new wxSpinCtrl(this, -1, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    combobox_buy_units_type_ = new wxComboBox(this, -1, wxT("buying"), wxDefaultPosition, wxDefaultSize, 0, NULL);
    for (auto& item : pLand->for_sale_)
    {
        std::string temp = item.second.long_name_ + std::string(": ") +
            std::to_string(item.second.amount_) + std::string(" at $") + std::to_string(item.second.price_);
        combobox_buy_units_type_->Append( temp );
        sale_products_.insert({temp, item.second});
    }
    if (combobox_buy_units_type_->GetCount() > 0)
        combobox_buy_units_type_->SetSelection(0);

    flag_buy_repeating_ = new wxCheckBox(this, -1, "repeating");
    flag_buy_all_ = new wxCheckBox(this, -1, "buy all");

    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(spin_buy_units_amount_, 0, wxALL);
    temp_sizer->Add(combobox_buy_units_type_, 1, wxALL);
    unit_buy_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(flag_buy_repeating_, 0, wxALL);
    temp_sizer->Add(flag_buy_all_, 0, wxALL);
    unit_buy_sizer->Add(temp_sizer, 0, wxALL);
    unitsizer->Add(unit_buy_sizer, 0, wxALL);
    unitsizer->Add(new wxStaticLine(this), 1, 0);

    //study order
    wxBoxSizer* unit_study_sizer = new wxBoxSizer( wxVERTICAL );
    flag_check_study_ = new wxCheckBox(this, -1, "");
    combobox_skills_ = new wxComboBox(this, -1, wxT("studying"), wxDefaultPosition, wxDefaultSize, 0, NULL);
    const std::vector<game_control::Skill>& skills = game_control::get_skills();
    for (const game_control::Skill& skill : skills)
        combobox_skills_->Append(skill.long_name_);
    if (combobox_skills_->GetCount() > 0)
        combobox_skills_->SetSelection(0);

    flag_study_repeating_ = new wxCheckBox(this, -1, "repeating");;
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(flag_check_study_, 0, wxALL);
    temp_sizer->Add(combobox_skills_, 1, wxALL);
    unit_study_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(flag_study_repeating_, 0, wxALL);
    unit_study_sizer->Add(temp_sizer, 0, wxALL);
    unitsizer->Add(unit_study_sizer, 0, wxALL);
    unitsizer->Add(new wxStaticLine(this), 1, 0);

    //receive silver section
    wxBoxSizer* unit_silver_sizer = new wxBoxSizer( wxVERTICAL );
    spin_silver_amount_ = new wxSpinCtrl(this, -1, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    combobox_units_ = new wxComboBox(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL);

    EValueType type;
    const void* value;
    std::vector<CUnit*> local_units;
    land_control::get_units_if(land_, local_units, [this](CUnit* unit) {
        return unit->FactionId == this->unit_->FactionId;
    });
    for (CUnit* unit : local_units)
    {
        value = 0;
        unit->GetProperty(PRP_SILVER, type, value, eNormal);

        std::string unit_and_silver = std::string(unit->Name.GetData()) + "(" + 
            std::to_string(unit->Id) + ") " + std::string(unit->pFaction->Name.GetData()) + 
            " (max: " + std::to_string((long)value) + ")";
        
        silver_holders_.insert({unit_and_silver, unit->Id});
        combobox_units_->Append(unit_and_silver);
    }

    flag_receive_silver_repeating_ = new wxCheckBox(this, -1, "repeating");
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Receive: ")), 0, wxALL);
    temp_sizer->Add(spin_silver_amount_, 0, wxALL);
    temp_sizer->Add(flag_receive_silver_repeating_, 0, wxALL);
    unit_silver_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("From: ")), 0, wxALL);
    temp_sizer->Add(combobox_units_, 0, wxALL);
    unit_silver_sizer->Add(temp_sizer, 1, wxALL);
    unitsizer->Add(unit_silver_sizer, 0, wxALL);
    unitsizer->Add(new wxStaticLine(this), 1, 0);

    //Expenses part
    wxBoxSizer* unit_expenses_sizer = new wxBoxSizer( wxVERTICAL );
    expenses_buying_ = new wxStaticText(this, -1, wxT("0"));
    expenses_studying_ = new wxStaticText(this, -1, wxT("0"));
    expenses_all_ = new wxStaticText(this, -1, wxT("0")); 
    unit_expenses_sizer->Add(new wxStaticText(this, -1, wxT("Expenses: ")), 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("    For buying: ")), 0, wxALL);
    temp_sizer->Add(expenses_buying_, 0, wxALL);
    unit_expenses_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("    For studying: ")), 0, wxALL);
    temp_sizer->Add(expenses_studying_, 0, wxALL);    
    unit_expenses_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("    Sum: ")), 0, wxALL);
    temp_sizer->Add(expenses_all_, 0, wxALL);
    unit_expenses_sizer->Add(temp_sizer, 0, wxALL);
    unitsizer->Add(unit_expenses_sizer, 0, wxALL);
    unitsizer->Add(new wxStaticLine(this), 1, 0);

    ////////////////////////////////////////////////////
    //flag section init
    flag_behind_ = new wxCheckBox(this, -1, "behind");
    flag_behind_->SetValue(unit_control::flags::is_behind(unit_));
    flag_avoid_ = new wxCheckBox(this, -1, "avoid");
    flag_avoid_->SetValue(unit_control::flags::is_avoid(unit_));
    flag_hold_ = new wxCheckBox(this, -1, "hold");
    flag_hold_->SetValue(unit_control::flags::is_hold(unit_));
    flag_noaid_ = new wxCheckBox(this, -1, "noaid");
    flag_noaid_->SetValue(unit_control::flags::is_noaid(unit_));
    flag_guard_ = new wxCheckBox(this, -1, "guard");
    flag_guard_->SetValue(unit_control::flags::is_guard(unit_));
    flag_nocross_ =  new wxCheckBox(this, -1, "nocross");
    flag_nocross_->SetValue(unit_control::flags::is_nocross(unit_));

    wxArrayString buf;
    buf.Add(wxT("none"));
    buf.Add(wxT("unit"));
    buf.Add(wxT("faction"));
    radiobox_flag_reveal_ = new wxRadioBox(this, -1, wxT("reveal"), wxDefaultPosition,
                        wxDefaultSize, buf, 1);
    if (unit_control::flags::is_reveal(unit_, "unit"))
        radiobox_flag_reveal_->SetSelection(1);
    else if (unit_control::flags::is_reveal(unit_, "faction"))
        radiobox_flag_reveal_->SetSelection(2);
    
    buf.clear();
    buf.Add(wxT("none"));
    buf.Add(wxT("walk"));
    buf.Add(wxT("ride"));    
    buf.Add(wxT("fly"));
    buf.Add(wxT("all"));
    radiobox_flag_spoils_ = new wxRadioBox(this, -1, wxT("spoils"), wxDefaultPosition,
                        wxDefaultSize, buf, 1);
    
    buf.clear();
    buf.Add(wxT("none"));
    buf.Add(wxT("unit"));
    buf.Add(wxT("faction"));
    radiobox_flag_consume_ = new wxRadioBox(this, -1, wxT("consume"), wxDefaultPosition,
                        wxDefaultSize, buf, 1);
    if (unit_control::flags::is_consume(unit_, "unit"))
        radiobox_flag_consume_->SetSelection(1);
    else if (unit_control::flags::is_consume(unit_, "faction"))
        radiobox_flag_consume_->SetSelection(2);                        

    //flag section represent
    wxBoxSizer* flagsizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer->Add(flag_behind_, 0, wxALL);
    temp_sizer->Add(flag_avoid_, 0, wxALL);
    temp_sizer->Add(flag_hold_, 0, wxALL);
    temp_sizer->Add(flag_noaid_, 0, wxALL);
    temp_sizer->Add(flag_guard_, 0, wxALL);
    temp_sizer->Add(flag_nocross_, 0, wxALL);
    temp_sizer->Add(radiobox_flag_consume_, 0, wxALL);
    flagsizer->Add(temp_sizer, 0, wxALL);
    flagsizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, 0);
    
    temp_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer->Add(radiobox_flag_reveal_, 0, wxALL);
    temp_sizer->Add(radiobox_flag_spoils_, 0, wxALL);    
    flagsizer->Add(temp_sizer, 0, wxALL);

    //main section represent
    wxBoxSizer* mainsizer = new wxBoxSizer( wxHORIZONTAL );
    mainsizer->Add(unitsizer, 1, wxALL | wxEXPAND);
    mainsizer->Add(flagsizer, 0, wxALL | wxEXPAND);

    //full section represent
    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );
    topsizer->Add(mainsizer, 1, wxALL);
    topsizer->Add(buttonsizer, 0, wxALL);

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_ALT, 13, wxID_OK);
    entries[1].Set(wxACCEL_CTRL, 13, wxID_OK);
    wxAcceleratorTable accel(2, entries);
    this->SetAcceleratorTable(accel);

    SetAutoLayout( TRUE );     // tell dialog to use sizer
    SetSizer( topsizer );      // actually set the sizer
    topsizer->Fit( this );            // set size to minimum size as calculated by the sizer
    topsizer->SetSizeHints( this );   // set size hints to honour mininum size}        
    CResizableDlg::SetPos();
}

CCreateNewUnit::~CCreateNewUnit() 
{

}

void CCreateNewUnit::UpdateExpences()
{
    try 
    {
        std::string buy_unit_type(combobox_buy_units_type_->GetValue().mb_str());
        CProductMarket product = sale_products_.at(buy_unit_type);
        long buying_expences = spin_buy_units_amount_->GetValue() * product.price_;

        long studying_expences = 0;
        if (flag_check_study_->IsChecked())
        {
            const std::vector<game_control::Skill>& skills = game_control::get_skills();
            std::string study_skill(combobox_skills_->GetValue().mb_str());
            for (const auto& skill : skills)
            {
                if (!skill.long_name_.compare(study_skill.c_str()))
                {
                    studying_expences = skill.study_price_;
                    break;
                }
            }            
        }
        expenses_buying_->SetLabel(std::to_string(buying_expences));
        expenses_studying_->SetLabel(std::to_string(studying_expences));
        expenses_all_->SetLabel(std::to_string(buying_expences + studying_expences));
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

void CCreateNewUnit::OnCancel       (wxCommandEvent& event)
{
    StoreSize();
    EndModal(wxID_CANCEL);
}
void CCreateNewUnit::OnOk           (wxCommandEvent& event)
{
    std::stringstream unit_order;
    unit_order << "behind " << (flag_behind_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "avoid " << (flag_avoid_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "hold " << (flag_hold_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "noaid " << (flag_noaid_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "guard " << (flag_guard_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "nocross " << (flag_nocross_->IsChecked()? 1 : 0) << std::endl;
    std::string temp_flag = radiobox_flag_reveal_->GetString(radiobox_flag_reveal_->GetSelection()).ToStdString();
    unit_order << "reveal " << (temp_flag.compare("none") ? temp_flag : "") << std::endl;
    temp_flag = radiobox_flag_spoils_->GetString(radiobox_flag_spoils_->GetSelection()).ToStdString();
    unit_order << "spoils " << temp_flag << std::endl;
    temp_flag = radiobox_flag_consume_->GetString(radiobox_flag_consume_->GetSelection()).ToStdString();
    unit_order << "consume " << (temp_flag.compare("none") ? temp_flag : "") << std::endl;

    std::string unit_name(text_name_->GetValue().ToStdString());
    if (!unit_name.empty())
        unit_order << "name unit \"" << unit_name << "\"" << std::endl;

    std::string unit_local_description(text_loc_description_->GetValue().mb_str());
    if (!unit_local_description.empty())
        unit_order << "@;;" << unit_local_description << std::endl;

    std::string unit_description(text_description_->GetValue().mb_str());
    if (!unit_description.empty())
        unit_order << "describe unit \"" << unit_description << "\"" << std::endl;

    int buy_units = spin_buy_units_amount_->GetValue();
    if (buy_units > 0)
    {
        std::string combo_buy_unit(combobox_buy_units_type_->GetValue().mb_str());
        CProductMarket item = sale_products_[combo_buy_unit];
        if (flag_buy_repeating_->IsChecked())
            unit_order << "@";
        if (flag_buy_all_->IsChecked())
            unit_order << "buy all " << item.short_name_ << std::endl;
        else
            unit_order << "buy " << buy_units << " " << item.short_name_ << std::endl;
    }

    if (flag_check_study_->IsChecked())
    {
        if (flag_study_repeating_->IsChecked())
            unit_order << "@";
        unit_order << "study " << combobox_skills_->GetValue() << std::endl;
    }

    int new_unit_id = spin_new_num_alias_->GetValue();
    int rec_silver = spin_silver_amount_->GetValue();
    if (rec_silver > 0) 
    {
        std::string unit_name = std::string(combobox_units_->GetValue().mb_str());
        int unit_id = silver_holders_[unit_name];
        std::vector<CUnit*> units;
        land_control::get_units_if(land_, units, [&unit_id](CUnit* unit) {
            return unit->Id == unit_id;
        });
        if (units.size() > 0) 
        {
            std::stringstream giver_orders;
            giver_orders << std::endl << "give NEW " << new_unit_id << " " << rec_silver << " SILV" << std::endl;
            units[0]->Orders << giver_orders.str().c_str();
        }
    }

    unit_->Orders.TrimRight(TRIM_ALL);
    if (!unit_->Orders.IsEmpty())
        unit_->Orders << EOL_SCR;

    for (size_t i=0; i<spin_copies_amount_->GetValue(); i++)
    {
        CUnit * pUnitNew = gpApp->m_pAtlantis->SplitUnit(unit_, new_unit_id+i);
        if (pUnitNew)
            pUnitNew->Orders << unit_order.str().c_str();
    }

    gpApp->m_pAtlantis->RunOrders(land_);

    StoreSize();
    EndModal(wxID_OK);
}
void CCreateNewUnit::onAnySpinUpdate(wxSpinEvent& event)
{
    UpdateExpences();
}
void CCreateNewUnit::onAnyComboBoxUpdate(wxCommandEvent& event)
{
    UpdateExpences();
}
