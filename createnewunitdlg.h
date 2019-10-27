#ifndef AH_CREATE_NEW_UNIT_DIALOG_INCL
#define AH_CREATE_NEW_UNIT_DIALOG_INCL

#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "data.h"
#include "data_control.h"
#include "ah_control.h"
#include "ahframe.h"

class CCreateNewUnit : public CResizableDlg
{
    CUnit* unit_;
    CLand* land_;
    std::unordered_map<std::string, CProductMarket> sale_products_;
    std::map<std::string, CUnit*> silver_holders_;

    //naming section
    wxSpinCtrl* spin_new_num_alias_;
    wxTextCtrl* text_name_;
    wxTextCtrl* text_loc_description_;
    wxTextCtrl* text_description_;

    //buy section
    wxSpinCtrl* spin_buy_units_amount_;
    wxComboBox* combobox_buy_units_type_;
    wxCheckBox* flag_buy_repeating_;
    wxCheckBox* flag_buy_all_;

    //study section
    wxCheckBox* flag_check_study_;
    wxComboBox* combobox_skills_;
    wxCheckBox* flag_study_repeating_;

    //receive silver section
    wxSpinCtrl* spin_silver_amount_;
    wxComboBox* combobox_units_;
    wxButton*   button_give_all_;
    wxCheckBox* flag_receive_silver_repeating_;

    //maintenance section
    wxSpinCtrl* spin_maintenance_turns_;

    //Expenses section
    wxStaticText* expenses_buying_; 
    wxStaticText* expenses_studying_; 
    wxStaticText* expenses_maintenance_; 
    wxStaticText* expenses_all_;

    //flags section
    wxCheckBox* flag_behind_;
    wxCheckBox* flag_avoid_;
    wxCheckBox* flag_hold_;
    wxCheckBox* flag_noaid_;
    wxCheckBox* flag_guard_;
    wxCheckBox* flag_nocross_;

    wxRadioBox* radiobox_flag_reveal_;
    wxRadioBox* radiobox_flag_spoils_;
    wxRadioBox* radiobox_flag_consume_;

    //Copies
    wxSpinCtrl* spin_copies_amount_;

public:
    //flags consist of command and it's possible values
    CCreateNewUnit(wxWindow *parent, CUnit * pUnit, CLand* pLand);
    ~CCreateNewUnit();

private:
    void InitializeButtons();
    wxBoxSizer* ButtonsToForm();

    void InitializeNaming(CLand* land);
    wxBoxSizer* NamingToForm();

    void InitializeBuyItems(CLand* land);
    wxBoxSizer* BuyItemsToForm();

    void InitializeStudy();
    wxBoxSizer* StudyToForm();

    void InitializeRecvSilver(int faction_id, CLand* land);
    wxBoxSizer* RecvSilverToForm();

    void InitializeMaintanence();
    wxBoxSizer* MaintanenceToForm();

    void InitializeExpences();
    wxBoxSizer* ExpencesToForm();

    void InitializeFlags(CUnit* unit);
    wxBoxSizer* FlagsToForm();

    void UpdateExpences();

    //Events
    void OnCancel       (wxCommandEvent& event);
    void OnOk           (wxCommandEvent& event);
    void onAnySpinUpdate(wxSpinEvent& event);
    void onAnyComboBoxUpdate(wxCommandEvent& event);
    void onGiveAllButton(wxCommandEvent & event);

    DECLARE_EVENT_TABLE()
};

#endif
