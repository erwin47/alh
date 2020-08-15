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

#ifndef __AH_UTIL_HEX_FILTER_DIALOG_INCL__
#define __AH_UTIL_HEX_FILTER_DIALOG_INCL__


#define HEX_SIMPLE_FLTR_COUNT 3

extern const char * HEX_FILTER_OPERATION[];


class CHexFilterDlg : public CResizableDlg
{
public:
    CHexFilterDlg(wxWindow *parent, const char * szConfigSection);


    wxComboBox    * m_cbProperty[HEX_SIMPLE_FLTR_COUNT];
    wxComboBox    * m_cbCompare [HEX_SIMPLE_FLTR_COUNT];
    wxTextCtrl    * m_tcValue   [HEX_SIMPLE_FLTR_COUNT];

    CStr            m_TrackingGroup;

private:
    void Init();
    void Save();
    BOOL IsValid();
    void Load        (const char * szConfigSection);
    void LoadSetCombo(const char * setselect);
    void Reload      (const char * setname);
    void EnableBoxes (BOOL bOldBoxes);

    void OnButton       (wxCommandEvent& event);
    void OnRadioButton  (wxCommandEvent& event);

    void OnTextChange   (wxCommandEvent& event);
    void OnSelectChange (wxCommandEvent& event);

//    void OnSetNameChange(wxCommandEvent& event);
//    void OnSetNameSelect(wxCommandEvent& event);
    void OnBoxesChange  (wxCommandEvent& event);

    CStr         m_sControllingConfig;
    CStr         m_sCurrentSection;
    CStr         m_sSavedConfigSelected;
    BOOL         m_IsSaving;
    time_t       m_lastselect;

    wxComboBox    * m_cbSetName;
    wxRadioButton * m_rbUseBoxes;
    wxRadioButton * m_rbUsePython;
    wxTextCtrl    * m_tcFilterText;
    //wxCheckBox    * m_chUseSelectedHexes;
    //wxCheckBox    * m_chDisplayOnMap;
    wxButton   * m_btnSet   ;
    wxButton   * m_btnRemove;
    wxButton   * m_btnCancel;
//    wxButton   * m_btnTracking;
    wxButton   * m_btnHelp;
    wxColour     m_ColorNormal;
    wxColour     m_ColorReadOnly;

    BOOL         m_bReady;

    DECLARE_EVENT_TABLE();
};

//===================


class CHexFilterAutologicDlg : public CResizableDlg
{
    //example part
    wxComboBox*                 example_box_;
    wxStaticText*               example_description_;
    std::map<std::string, std::string>  examples_;//function, description

    //user defined part
    std::vector<wxTextCtrl*>    descriptions_;
    std::vector<wxTextCtrl*>    fields_;
    std::vector<wxTextCtrl*>    result_field_;
    std::vector<wxButton*>      buttons_;

    //bottom part
    wxButton*                   clear_button_;
    wxButton*                   cancel_button_; 

    std::string                 result_;
    std::string                 result_filter_;

    wxBoxSizer*                 topsizer_;

    //void add_row();
    //void remove_row();
    
public:
    CHexFilterAutologicDlg(wxWindow *parent, const std::map<std::string, std::string>& examples);
    ~CHexFilterAutologicDlg();

    void OnApply(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void OnExampleChosen(wxCommandEvent& event);
    
    // it needs to create specific panel to be able to add items dynamically
    //void OnAddRow(wxCommandEvent& event);
    //void OnRemRow(wxCommandEvent& event);

    inline const std::string& get_result() const {  return result_;  }
    inline const std::string& get_result_filter() const {  return result_filter_;  }
};

#endif
