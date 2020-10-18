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
#include "wx/dialog.h"


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
#include "utildlgs.h"
#include "hexfilterdlg.h"
#include "mappane.h"
#include "ah_control.h"


#define SPACER_GENERIC 5

#define ID_BTN_TRACK     wxID_HIGHEST + 10
#define ID_CB_SET_NAME   wxID_HIGHEST + 11
#define ID_BTN_HELP      wxID_HIGHEST + 12
#define ID_TC_TEXT       wxID_HIGHEST + 13

const char * HEX_FILTER_OPERATION[] = {">", ">=", "=", "<=", "<", "<>"};


BEGIN_EVENT_TABLE(CHexFilterDlg, wxDialog)
    EVT_BUTTON      (-1,             CHexFilterDlg::OnButton)
    EVT_RADIOBUTTON (-1,             CHexFilterDlg::OnRadioButton)
    EVT_COMBOBOX    (-1,             CHexFilterDlg::OnSelectChange)
    EVT_TEXT        (-1,             CHexFilterDlg::OnTextChange)
END_EVENT_TABLE()

//--------------------------------------------------------------------------

CHexFilterDlg::CHexFilterDlg(wxWindow *parent, const char * szConfigSection)
               :CResizableDlg( parent, wxT("Find Hexes"), SZ_SECT_WND_HEX_FLTR_DLG),
                m_bReady(0)
{
//    CMapPane   * pMapPane   = (CMapPane  * )gpApp->m_Panes[AH_PANE_MAP];

    wxBoxSizer * topsizer;
    wxBoxSizer * sizer   ;
    wxBoxSizer * rowsizer;
    wxFlexGridSizer * gridsizer;
    int          count;
    CStr         sConfSet;
    wxStaticText * stSetName;

    m_IsSaving           = FALSE;
    m_lastselect         = 0;
    m_sControllingConfig = szConfigSection;


    topsizer = new wxBoxSizer( wxVERTICAL );

    m_btnSet        = new wxButton     (this, wxID_OK     , wxT("Set")    );
    m_btnRemove     = new wxButton     (this, wxID_NO     , wxT("Clear")  );
    m_btnCancel     = new wxButton     (this, wxID_CANCEL , wxT("Cancel") );
//    m_btnTracking   = new wxButton     (this, ID_BTN_TRACK, wxT("Tracking") );
    m_btnHelp       = new wxButton     (this, ID_BTN_HELP , wxT("Help") );
    m_cbSetName     = new wxComboBox   (this, ID_CB_SET_NAME);
    m_rbUseBoxes    = new wxRadioButton(this, -1, wxT("Boxes"));
    m_rbUsePython   = new wxRadioButton(this, -1, wxT("Python") );
    m_rbUsePython->Enable(false);
    m_tcFilterText  = new wxTextCtrl   (this, ID_TC_TEXT, wxT(""), wxDefaultPosition, wxSize(100,50), wxTE_MULTILINE);
//    m_chDisplayOnMap= new wxCheckBox(this, -1, wxT("Mark results on the map"));
//    m_chUseSelectedHexes = new wxCheckBox(this, -1, wxT("In the selected hexes only"));
    stSetName       = new wxStaticText (this, -1, wxT("Filter name:"));

    //if (!pMapPane->HaveSelection())
    //    m_chUseSelectedHexes->Enable(FALSE);

    sizer    = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(stSetName       , 0, wxALIGN_CENTER | wxALL , SPACER_GENERIC);
    sizer->Add(m_cbSetName     , 1, wxALIGN_CENTER | wxALL , SPACER_GENERIC);
    topsizer->Add(sizer        , 0, wxALL | wxGROW, SPACER_GENERIC );


    gridsizer = new wxFlexGridSizer(2,2,3,3) ;
    gridsizer->AddGrowableCol(1);
    gridsizer->AddGrowableRow(1);

    rowsizer = new wxBoxSizer( wxVERTICAL );
    gridsizer->Add(m_rbUseBoxes, 0, wxALIGN_LEFT | wxALL, SPACER_GENERIC);

    for (count=0; count < HEX_SIMPLE_FLTR_COUNT; count++)
    {
        //wxSize WiderSize(wxDefaultSize.GetWidth()*2, wxDefaultSize.GetHeight());
        m_cbProperty[count] = new wxComboBox(this, -1); //, "", wxDefaultPosition,  WiderSize);
        m_cbCompare [count] = new wxComboBox(this, -1); //, "", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY );
        m_tcValue   [count] = new wxTextCtrl(this, -1);

        if (count>0)
            rowsizer->Add(new wxStaticText(this, -1, wxT("AND")), 0, wxALIGN_LEFT | wxLEFT, SPACER_GENERIC );

        sizer    = new wxBoxSizer( wxHORIZONTAL );
        sizer->Add(m_cbProperty[count], 2, wxALL | wxGROW, SPACER_GENERIC);
        sizer->Add(m_cbCompare [count], 0, wxALIGN_CENTER | wxALL, SPACER_GENERIC);
        sizer->Add(m_tcValue   [count], 3, wxALL | wxGROW, SPACER_GENERIC);
        rowsizer->Add(sizer, 1, wxGROW );
    }
    gridsizer->Add(rowsizer, 1, wxALIGN_LEFT | wxGROW );


    gridsizer->Add(m_rbUsePython   , 0, wxALIGN_LEFT | wxALL, SPACER_GENERIC);
    gridsizer->Add(m_tcFilterText, 1, wxGROW | wxALL, SPACER_GENERIC);
    topsizer->Add(gridsizer, 1, wxGROW);

    //sizer    = new wxBoxSizer( wxHORIZONTAL );
    //sizer->Add(m_chDisplayOnMap, 0, wxALIGN_LEFT | wxALL, SPACER_GENERIC);
    //sizer->Add(m_chUseSelectedHexes , 0, wxALIGN_LEFT | wxALL, SPACER_GENERIC);
    //topsizer->Add(sizer, 0, wxALIGN_LEFT );

    sizer    = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(m_btnSet     , 0, wxALIGN_CENTER);
    sizer->Add(m_btnRemove  , 0, wxALIGN_CENTER | wxALL, SPACER_GENERIC);
//    sizer->Add(m_btnTracking, 0, wxALIGN_CENTER | wxALL, SPACER_GENERIC);
    sizer->Add(m_btnCancel  , 0, wxALIGN_CENTER | wxALL, SPACER_GENERIC);
    sizer->Add(m_btnHelp    , 0, wxALIGN_CENTER | wxALL, SPACER_GENERIC);
    topsizer->Add(sizer, 0, wxALIGN_CENTER );

    SetAutoLayout( TRUE );     // tell dialog to use sizer
    SetSizer( topsizer );      // actually set the sizer
    topsizer->Fit( this );            // set size to minimum size as calculated by the sizer
    topsizer->SetSizeHints( this );   // set size hints to honour mininum size}

    m_cbProperty[0]->SetFocus();
    m_btnSet->SetDefault();


    m_ColorNormal   = m_tcFilterText->GetBackgroundColour() ;
    m_ColorReadOnly.Set(APPLY_COLOR_DELTA(m_ColorNormal.Red()),
                        APPLY_COLOR_DELTA(m_ColorNormal.Green()),
                        APPLY_COLOR_DELTA(m_ColorNormal.Blue()));




    sConfSet = gpApp->GetConfig(szConfigSection, SZ_KEY_FLTR_SET);
    sConfSet.TrimRight(TRIM_ALL);
    if (sConfSet.IsEmpty())
        sConfSet << SZ_SECT_HEX_FILTER << "Default";
    LoadSetCombo(sConfSet.GetData());
    Init();
    Load(sConfSet.GetData());

    CResizableDlg::SetSize();

    m_bReady = TRUE;
}

//--------------------------------------------------------------------------

void CHexFilterDlg::LoadSetCombo(const char * setselect)
{
    const char * setsection;
    int          setnameoffs;
    int          x=0, i=0;

    m_IsSaving = TRUE;

    setnameoffs = strlen(SZ_SECT_HEX_FILTER);
    setsection  = gpApp->GetNextSectionName(CONFIG_FILE_CONFIG, SZ_SECT_HEX_FILTER);
    while (setsection)
    {
        if (0!=strnicmp(setsection, SZ_SECT_HEX_FILTER, setnameoffs))
            break;
        m_cbSetName->Append(wxString::FromAscii(&setsection[setnameoffs]));
        if (0==stricmp(setsection, setselect))
            x = i;
        i++;
        setsection = gpApp->GetNextSectionName(CONFIG_FILE_CONFIG, setsection);
    }
    if (0==i)
        m_cbSetName->Append(wxString::FromAscii(&setselect[setnameoffs]));
    m_cbSetName->SetSelection(x);

    m_IsSaving = FALSE;
}

//--------------------------------------------------------------------------

void CHexFilterDlg::Init()
{
    int          count;
    int          i;
    const char * item;
    CStr         S;

    for (count=0; count < HEX_SIMPLE_FLTR_COUNT; count++)
    {
        m_cbProperty[count]->Append(wxT(""));
        m_cbProperty[count]->Append(wxT("REG[NAME]"));
        m_cbProperty[count]->Append(wxT("RES[MITH]"));
        m_cbProperty[count]->Append(wxT("ITEM[MITH]"));
        m_cbProperty[count]->Append(wxT("SKILL[ARMO]"));
        m_cbProperty[count]->Append(wxT("LOC_ITEM[MITH]"));
        m_cbProperty[count]->Append(wxT("SELL_AMOUNT[MITH]"));
        m_cbProperty[count]->Append(wxT("SELL_PRICE[MITH]"));
        m_cbProperty[count]->Append(wxT("BUY_AMOUNT[MITH]"));
        m_cbProperty[count]->Append(wxT("BUY_PRICE[MITH]"));

        /*for (i=0; i<gpApp->m_pAtlantis->m_LandPropertyNames.Count(); i++)
        {
            item = (const char *) gpApp->m_pAtlantis->m_LandPropertyNames.At(i);
            m_cbProperty[count]->Append(wxString::FromAscii(item));
        }*/

        m_cbCompare [count]->Append(wxT(""));
        for (i=0; (unsigned)i<sizeof(HEX_FILTER_OPERATION)/sizeof(*HEX_FILTER_OPERATION); i++)
        {
            item = HEX_FILTER_OPERATION[i];
            m_cbCompare[count]->Append(wxString::FromAscii(item));
        }
    }
}

//--------------------------------------------------------------------------

void CHexFilterDlg::Load(const char * szConfigSection)
{
    int          count;
    int          i;
    CStr         ConfigKey;
    const char * selvalue;
    int          selidx;
    CStr         S;

    m_sCurrentSection = szConfigSection;

    for (count=0; count < HEX_SIMPLE_FLTR_COUNT; count++)
    {
        ConfigKey.Format("%s%d", SZ_KEY_HEX_FLTR_PROPERTY, count);
        selvalue = gpApp->GetConfig(szConfigSection, ConfigKey.GetData());
        selidx   = 0;
        for (i=0; i<(int)m_cbProperty[count]->GetCount(); i++)
            if (0==stricmp(m_cbProperty[count]->GetString(i).mb_str(), selvalue))
            {
                selidx = i;
                break;
            }
        m_cbProperty[count]->SetSelection(selidx);

        ConfigKey.Format("%s%d", SZ_KEY_HEX_FLTR_COMPARE , count);
        m_cbCompare [count]->SetValue(wxString::FromAscii(gpApp->GetConfig(szConfigSection, ConfigKey.GetData())));
        selvalue = gpApp->GetConfig(szConfigSection, ConfigKey.GetData());
        selidx   = 0;
        for (i=0; i<(int)m_cbCompare[count]->GetCount(); i++)
            if (0==stricmp(m_cbCompare[count]->GetString(i).mb_str(), selvalue))
            {
                selidx = i;
                break;
            }
        m_cbCompare[count]->SetSelection(selidx);

        ConfigKey.Format("%s%d", SZ_KEY_HEX_FLTR_VALUE   , count);
        m_tcValue   [count]->SetValue(wxString::FromAscii(SkipSpaces(gpApp->GetConfig(szConfigSection, ConfigKey.GetData()))) );
    }

    m_tcFilterText->SetValue(wxString::FromAscii(gpApp->GetConfig(szConfigSection, SZ_KEY_HEX_FLTR_PYTHON_CODE)));

    S = gpApp->GetConfig(szConfigSection, SZ_KEY_HEX_FLTR_SOURCE);
    if (0==stricmp(S.GetData(), SZ_KEY_HEX_FLTR_SOURCE_PYTHON))
    {
        m_rbUsePython->SetValue(TRUE);
        EnableBoxes(FALSE);
    }
    else
    {
        m_rbUseBoxes->SetValue(TRUE);
        EnableBoxes(TRUE);
    }

    //S = gpApp->GetConfig(szConfigSection, SZ_KEY_HEX_FLTR_SELECTED_HEXES);
    //m_sSavedConfigSelected = S;
    //if ( m_chUseSelectedHexes->IsEnabled() )
    //    m_chUseSelectedHexes->SetValue(atol(S.GetData()) != 0);

    //S = gpApp->GetConfig(szConfigSection, SZ_KEY_HEX_FLTR_SHOW_ON_MAP);
    //m_chDisplayOnMap->SetValue(atol(S.GetData()) != 0);
}

//--------------------------------------------------------------------------

BOOL CHexFilterDlg::IsValid()
{
    CStr         S1, S2;
    int          count;
    BOOL         isvalid = FALSE;

    if (m_rbUseBoxes->GetValue())
        for (count=0; count < HEX_SIMPLE_FLTR_COUNT; count++)
        {
            S1 = m_cbProperty[count]->GetValue().mb_str();   S1.TrimRight(TRIM_ALL);
            S2 = m_cbCompare[count]->GetValue().mb_str();    S2.TrimRight(TRIM_ALL);
            if (!S1.IsEmpty() && !S2.IsEmpty())
                isvalid = TRUE;
        }
    else
    {
        S1 = m_tcFilterText->GetValue().mb_str();
        if (!S1.IsEmpty() )
            isvalid = TRUE;
    }
    return isvalid;
}

//--------------------------------------------------------------------------

void CHexFilterDlg::Save()
{
    int          count;
    CStr         ConfigKey;
    CStr         SetName;
    CStr         S;
    BOOL         found = FALSE;

    m_IsSaving = TRUE;

    gpApp->RemoveSection(m_sCurrentSection.GetData());

    if (IsValid())
    {
        SetName = m_sCurrentSection;
        SetName.DelSubStr(0, strlen(SZ_SECT_HEX_FILTER));

        for (count=0; count<(int)m_cbSetName->GetCount(); count++)
            if (0==stricmp(m_cbSetName->GetString(count).mb_str(), SetName.GetData()))
            {
                found = TRUE;
                break;
            }
        if (!found)
            m_cbSetName->Append(wxString::FromAscii(SetName.GetData()));


        for (count=0; count < HEX_SIMPLE_FLTR_COUNT; count++)
        {
            ConfigKey.Format("%s%d", SZ_KEY_HEX_FLTR_PROPERTY, count);
            gpApp->SetConfig(m_sCurrentSection.GetData(), ConfigKey.GetData(), m_cbProperty[count]->GetValue().mb_str());

            ConfigKey.Format("%s%d", SZ_KEY_HEX_FLTR_COMPARE , count);
            gpApp->SetConfig(m_sCurrentSection.GetData(), ConfigKey.GetData(), m_cbCompare[count]->GetValue().mb_str());

            ConfigKey.Format("%s%d", SZ_KEY_HEX_FLTR_VALUE   , count);
            gpApp->SetConfig(m_sCurrentSection.GetData(), ConfigKey.GetData(), m_tcValue[count]->GetValue().mb_str());
        }

        gpApp->SetConfig(m_sCurrentSection.GetData(), SZ_KEY_HEX_FLTR_PYTHON_CODE, m_tcFilterText->GetValue().mb_str());
        gpApp->SetConfig(m_sCurrentSection.GetData(), SZ_KEY_HEX_FLTR_SOURCE, m_rbUsePython->GetValue() ? SZ_KEY_HEX_FLTR_SOURCE_PYTHON : "");

        //S = m_sSavedConfigSelected;
        //if ( m_chUseSelectedHexes->IsEnabled() )
        //    S = m_chUseSelectedHexes->GetValue()?"1":"0";
        //gpApp->SetConfig(m_sCurrentSection.GetData(), SZ_KEY_HEX_FLTR_SELECTED_HEXES, S.GetData());
        //gpApp->SetConfig(m_sCurrentSection.GetData(), SZ_KEY_HEX_FLTR_SHOW_ON_MAP, m_chDisplayOnMap->GetValue()?"1":"0");

    }

    m_IsSaving = FALSE;
}

//--------------------------------------------------------------------------

void CHexFilterDlg::Reload(const char * setname)
{
    CStr  Sect;

    if (m_IsSaving)
        return;

    Save();
    Sect << SZ_SECT_HEX_FILTER << setname;
    Load(Sect.GetData());
}

//--------------------------------------------------------------------------

void CHexFilterDlg::EnableBoxes (BOOL bOldBoxes)
{
    int i;

    m_tcFilterText->Enable(!bOldBoxes);

    for (i=0; i < HEX_SIMPLE_FLTR_COUNT; i++)
    {
        m_cbProperty[i]->Enable(bOldBoxes);
        m_cbCompare [i]->Enable(bOldBoxes);
        m_tcValue   [i]->Enable(bOldBoxes);
    }

}

//--------------------------------------------------------------------------


void CHexFilterDlg::OnTextChange   (wxCommandEvent& event)
{
    wxObject * object = event.GetEventObject();

    if (object == m_cbSetName)
    {
        if (m_lastselect < time(NULL))
            Reload(m_cbSetName->GetValue().mb_str()); // returns the old value on windoze when value is selected
    }
    else
        OnBoxesChange(event);
}

//--------------------------------------------------------------------------

void CHexFilterDlg::OnSelectChange (wxCommandEvent& event)
{
    wxObject * object = event.GetEventObject();

    if (object == m_cbSetName)
    {
        Reload(m_cbSetName->GetString(m_cbSetName->GetSelection()).mb_str());
        m_lastselect = time(NULL);
    }
    else
        OnBoxesChange(event);
}

//--------------------------------------------------------------------------

void CHexFilterDlg::OnBoxesChange  (wxCommandEvent& event)
{
    wxObject * object = event.GetEventObject();

    if (!m_bReady || object == m_tcFilterText)
        return;

    if (m_rbUseBoxes->GetValue())
    {
        int i;
        CStr s, s1, s2, s3;

        for (i=0; i < HEX_SIMPLE_FLTR_COUNT; i++)
        {
            s1 = m_cbProperty[i]->GetValue().mb_str();
            s2 = m_cbCompare [i]->GetValue().mb_str();
            s3 = m_tcValue   [i]->GetValue().mb_str();
            if (!s1.IsEmpty() || !s2.IsEmpty() || !s3.IsEmpty())
            {
                // adjust 'equal to'
                s2.TrimLeft();
                s2.TrimRight();
                if (0 == strcmp(s2.GetData(), "="))
                    s2 = "==";

                // quote strings
                s1.TrimLeft();
                s1.TrimRight();

                CStrInt  * pSI, SI(s1.GetData(), 0);
                int        idx;
                EValueType type;

                if (gpApp->m_pAtlantis->m_UnitPropertyTypes.Search(&SI, idx))
                {
                    pSI = (CStrInt*)gpApp->m_pAtlantis->m_UnitPropertyTypes.At(idx);
                    type = (EValueType)pSI->m_value;
                    if (eCharPtr == type)
                    {
                        s3.InsStr("\"", 0, 1);
                        s3 << '\"';
                    }
                }

                if (!s.IsEmpty())
                    s << " and ";
                s << s1 << s2 << s3;
            }
        }
        m_tcFilterText->SetValue(wxString::FromAscii(s.GetData()));
    }
}

//--------------------------------------------------------------------------

void CHexFilterDlg::OnRadioButton  (wxCommandEvent& event)
{
    wxObject * object = event.GetEventObject();

    EnableBoxes(object == m_rbUseBoxes);
}

//--------------------------------------------------------------------------

void CHexFilterDlg::OnButton(wxCommandEvent& event)
{
    wxObject * object = event.GetEventObject();
    int        i;

    m_TrackingGroup.Empty();
    if (object == m_btnRemove)
    {
        for (i=0; i < HEX_SIMPLE_FLTR_COUNT; i++)
        {
            m_cbProperty[i]->SetValue(wxT(""));
            m_cbCompare [i]->SetValue(wxT(""));
            m_tcValue   [i]->SetValue(wxT(""));
        }
        m_tcFilterText->SetValue(wxT(""));
    }
    //else if (object == m_btnTracking)
    //{
    //    int          sectidx;
    //    CStr         S;
    //    const char * szName;
    //    const char * szValue;

    //    sectidx = gpApp->GetSectionFirst(SZ_SECT_UNIT_TRACKING, szName, szValue);
    //    while (sectidx >= 0)
    //    {
    //        if (!S.IsEmpty())
    //            S << ",";
    //        S << szName;
    //        sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_UNIT_TRACKING, szName, szValue);
    //    }
    //    if (S.IsEmpty())
    //        S = "Default";

    //    CComboboxDlg dlg(this, "Show a tracking group", "Select a group to load units from.", S.GetData());
    //    if (wxID_OK == dlg.ShowModal())
    //    {
    //        m_TrackingGroup = dlg.m_Choice;

    //        StoreSize();
    //        EndModal(wxID_OK);
    //    }
    //}
    else if (object == m_btnSet)
    {
        Save();
        gpApp->SetConfig(m_sControllingConfig.GetData(), SZ_KEY_FLTR_SET, m_sCurrentSection.GetData());
        StoreSize();
        EndModal(wxID_OK);
    }
    else if (object == m_btnCancel)
    {
        StoreSize();
        EndModal(wxID_CANCEL);
    }
    else if (object == m_btnHelp)
    {
        wxMessageBox(wxT("\n"
                     "- Filter names can not be modified.\n"
                     "- To add a filter type in the name for it and set conditions.\n"
                     "- To delete a filter remove conditions from it.")
                     );
    }
}

#include "data_control.h"

CHexFilterAutologicDlg::CHexFilterAutologicDlg(wxWindow *parent, const std::map<std::string, std::string>& examples)
            :CResizableDlg(parent, wxT("Find Hexes Autologic"), "HEX_FILTER_AUTOLOGIC_DLG"), examples_(examples)
{
    topsizer_ = new wxBoxSizer( wxVERTICAL );
    long layers = game_control::get_game_config_val<long>("HEX_FILTER_AUTOLOGIC_DLG", "AMOUNT");
    layers = std::max<long>(5, layers);

    // Example part
    example_box_ = new wxComboBox(this, -1);
    example_description_ = new wxStaticText(this, -1, wxT("Manual modification of [HEX_FILTER_AUTOLOGIC_DLG] `AMOUNT` in settings\n allows to set up any amount of rows for current window"));
    example_box_->Append(wxT(""));
    for (const auto& pair: examples_)
    {
        example_box_->Append(pair.first);
    }
    example_box_->Bind(wxEVT_COMBOBOX, &CHexFilterAutologicDlg::OnExampleChosen, this); 
    wxBoxSizer* example_sizer = new wxBoxSizer( wxVERTICAL );
    example_sizer->Add(new wxStaticText(this, -1, wxT("Function examples:")), 0, wxALL);
    example_sizer->Add(example_box_, 0, wxALL | wxEXPAND);
    example_sizer->Add(example_description_, 1, wxALL | wxEXPAND);
    example_sizer->AddSpacer(20);
    topsizer_->Add(example_sizer, 0, wxALL | wxEXPAND);

    //Configuration part
    topsizer_->Add(new wxStaticText(this, -1, wxT("Filters:")), 0, wxALL);
    descriptions_.resize(layers);
    fields_.resize(layers);
    result_field_.resize(layers);
    buttons_.resize(layers);
    for (long i = 0; i < layers; ++i)
    {
        std::string descr = "FIELD"+std::to_string(i)+"_DESCR";
        std::string filter = "FIELD"+std::to_string(i)+"_FILTER";
        std::string res_filter = "FIELD"+std::to_string(i)+"_RESULT_FILTER";
        descr = game_control::get_game_config_val<std::string>("HEX_FILTER_AUTOLOGIC_DLG", descr.c_str());
        filter = game_control::get_game_config_val<std::string>("HEX_FILTER_AUTOLOGIC_DLG", filter.c_str());
        res_filter = game_control::get_game_config_val<std::string>("HEX_FILTER_AUTOLOGIC_DLG", res_filter.c_str());

        wxBoxSizer* evaluation_sizer = new wxBoxSizer( wxHORIZONTAL );
        wxBoxSizer* field_sizer = new wxBoxSizer( wxVERTICAL );
        descriptions_[i] = new wxTextCtrl(this, -1, descr.c_str(), wxDefaultPosition);
        fields_[i] = new wxTextCtrl(this, -1, filter.c_str(), wxDefaultPosition, wxSize(225,50), wxTE_MULTILINE);
        result_field_[i] = new wxTextCtrl(this, -1, res_filter.c_str(), wxDefaultPosition);
        wxBoxSizer* result_filter_sizer = new wxBoxSizer( wxHORIZONTAL );
        result_filter_sizer->Add(new wxStaticText(this, -1, wxT("Filter result: ")), 0, wxALL);
        result_filter_sizer->Add(result_field_[i], 1, wxALL);
        wxBoxSizer* description_sizer = new wxBoxSizer( wxHORIZONTAL );
        description_sizer->Add(new wxStaticText(this, -1, wxT("Name: ")), 0, wxALL);
        description_sizer->Add(descriptions_[i], 1, wxALL);


        if (i == 0)
            fields_[i]->SetFocus();
        field_sizer->Add(description_sizer, 0, wxALL | wxEXPAND);
        field_sizer->Add(fields_[i], 1, wxALL | wxEXPAND);
        field_sizer->Add(result_filter_sizer, 0, wxALL | wxEXPAND);

        buttons_[i] = new wxButton(this, wxID_OK, wxT("Apply To Map"));
        buttons_[i]->Bind(wxEVT_BUTTON, &CHexFilterAutologicDlg::OnApply, this);
        evaluation_sizer->Add(field_sizer, 1, wxALL | wxEXPAND);
        evaluation_sizer->Add(buttons_[i], 0, wxALIGN_CENTER | wxALL);
        topsizer_->Add(evaluation_sizer, 1, wxALL | wxEXPAND);
    }
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* button = new wxButton(this, wxID_NO, wxT("Clear Map"));
    button->Bind(wxEVT_BUTTON, &CHexFilterAutologicDlg::OnClear, this);
    button_sizer->Add(button, 1, wxALIGN_CENTER | wxALL);
    button = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    button->Bind(wxEVT_BUTTON, &CHexFilterAutologicDlg::OnCancel, this);
    button_sizer->Add(button, 1, wxALIGN_CENTER | wxALL);
    button = new wxButton(this, wxID_ABOUT, wxT("Help"));
    button->Bind(wxEVT_BUTTON, &CHexFilterAutologicDlg::OnHelp, this);
    button_sizer->Add(button, 1, wxALIGN_CENTER | wxALL);

    topsizer_->Add(button_sizer, 0, wxALIGN_CENTER | wxALL);

    SetAutoLayout( TRUE );     // tell dialog to use sizer
    SetSizer( topsizer_ );      // actually set the sizer
    topsizer_->Fit( this );            // set size to minimum size as calculated by the sizer
    topsizer_->SetSizeHints( this );   // set size hints to honour mininum size}        
    CResizableDlg::SetPos();
}

CHexFilterAutologicDlg::~CHexFilterAutologicDlg()
{
    for (size_t i = 0; i < fields_.size(); ++i)
    {
        std::string descr = "FIELD"+std::to_string(i)+"_DESCR";
        std::string filter = "FIELD"+std::to_string(i)+"_FILTER";
        std::string res_filter = "FIELD"+std::to_string(i)+"_RESULT_FILTER";
        gpApp->SetConfig("HEX_FILTER_AUTOLOGIC_DLG", descr.c_str(), descriptions_[i]->GetValue().mb_str());
        gpApp->SetConfig("HEX_FILTER_AUTOLOGIC_DLG", filter.c_str(), fields_[i]->GetValue().mb_str());
        gpApp->SetConfig("HEX_FILTER_AUTOLOGIC_DLG", res_filter.c_str(), result_field_[i]->GetValue().mb_str());
    }

}

void CHexFilterAutologicDlg::OnApply(wxCommandEvent& event)
{
    wxObject * object = event.GetEventObject();
    result_.clear();
    result_filter_.clear();
    for (long i = 0; i < buttons_.size(); ++i)
    {
        if (object == buttons_[i])
        {
            result_ = fields_[i]->GetValue().mb_str();
            result_filter_ = result_field_[i]->GetValue().mb_str();
            break;
        }
    }
    StoreSize();    
    EndModal(wxID_OK);
}

void CHexFilterAutologicDlg::OnClear(wxCommandEvent& event)
{
    result_.clear();
    result_filter_.clear();
    StoreSize();
    EndModal(wxID_NO);
}

void CHexFilterAutologicDlg::OnCancel(wxCommandEvent& event)
{
    StoreSize();
    EndModal(wxID_CANCEL);
}

void CHexFilterAutologicDlg::OnHelp(wxCommandEvent& event)
{
    wxMessageBox(wxT("Use examples of functions with `&&` and `||` to generate your filter.\nUse `Filter result` to filter out resulting amount. Example body of `Filter result`:\n    ` < 50` -- to print out just results with amount < 50"));
    return;
}
void CHexFilterAutologicDlg::OnExampleChosen(wxCommandEvent& event)
{
    std::string chosen_function = example_box_->GetValue().ToStdString();
    examples_[chosen_function];
    example_description_->SetLabel(examples_[chosen_function].c_str());
}
/*
void CHexFilterAutologicDlg::OnAddRow(wxCommandEvent& event)
{

}

void CHexFilterAutologicDlg::OnRemRow(wxCommandEvent& event)
{

}*/
