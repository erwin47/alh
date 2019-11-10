#ifndef AH_RECEIVE_DIALOG_INCL
#define AH_RECEIVE_DIALOG_INCL

#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "ah_control.h"
#include "ahframe.h"
#include <set>



/*
TODO: 
1) add button "Add" which will add possibility to put few orders in a line.
It will need a text space to store there orderes.
After each such order amounts and accessible item types have to be recalculated.
6) Get item by pressing first letter on a keyboard, when window is opened.
*/
class CReceiveDlg : public CResizableDlg
{
    CUnit* unit_;
    CLand* land_;

    wxComboBox* combobox_item_types_;
    wxSpinCtrl* spin_items_amount_;
    wxComboBox* combobox_units_;
    wxCheckBox* order_repeating_;
    wxCheckBox* use_order_take_;

    std::map<std::string, CUnit*> unit_name_to_unit_;
    std::map<std::string, std::string> long_to_short_item_names_;

public:
    CReceiveDlg(wxWindow *parent, CUnit * pUnit, CLand* pLand);
    ~CReceiveDlg();

private:
    void init_item_types_combobox();
    std::string compose_give_order(CUnit* to_whom, long amount, const std::string& item);
    std::string compose_give_comment(CUnit* from_whom, long amount, const std::string& item);
    std::string compose_take_order(CUnit* from_whom, long amount, const std::string& item);
    std::string compose_take_comment(CUnit* fo_whom, long amount, const std::string& item);
    void set_order(CUnit* unit, const std::string& order);

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
