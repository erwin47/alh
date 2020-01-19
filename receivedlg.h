#ifndef AH_RECEIVE_DIALOG_INCL
#define AH_RECEIVE_DIALOG_INCL

#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "ah_control.h"
#include "ahframe.h"
#include <set>


#define RECEIVE_DLG_SETTINGS        "RECEIVE_DLG_SETTINGS"
#define SZ_RECDLG_GROUPS            "REC_DLG_GROUPS"

/*
TODO: 
1) add button "Add" which will add possibility to put few orders in a line.
It will need a text space to store there orderes.
After each such order amounts and accessible item types have to be recalculated.
6) Get item by pressing first letter on a keyboard, when window is opened.
*/
struct ItemUnitPair
{
    long amount_;
    std::string item_code_;
    CUnit* unit_;
};

class CReceiveDlg : public CResizableDlg
{
    CUnit* unit_;
    CLand* land_;

    wxComboBox* combobox_item_types_;
    wxSpinCtrl* spin_items_amount_;
    wxComboBox* combobox_units_;
    wxCheckBox* order_repeating_;
    wxCheckBox* use_order_take_;

    std::map<std::string, ItemUnitPair> unit_name_to_unit_;
    std::map<std::string, std::string> long_to_short_item_names_;

    const std::string FROM_ALL_ = "-=FROM ALL=-";
    std::map<std::string, std::vector<std::string>> groups_; 

public:
    CReceiveDlg(wxWindow *parent, CUnit * pUnit, CLand* pLand);
    ~CReceiveDlg();

private:
    void init_item_types_combobox();
    std::string compose_take_order(CUnit* from_whom, long amount, const std::string& item);
    void perform_take(CUnit* giving_one, CUnit* receiving_one, long amount, const std::string& item_code_name);
    void perform_give(CUnit* giving_one, CUnit* receiving_one, long amount, const std::string& item_code_name);

    std::set<CItem> get_item_types_list(CUnit* unit, CLand* land) const;
    std::vector<std::string> get_units_with_item(const std::string& item_type, CUnit* unit, CLand* land);

    //Events
    void OnItemChosen   (wxCommandEvent& event);
    void OnMax          (wxCommandEvent& event);
    void OnOk           (wxCommandEvent& event);
    void OnAdd          (wxCommandEvent& event);
    void OnCancel       (wxCommandEvent& event);

};

#endif 
