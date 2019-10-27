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
4) silv and SILV are duplicated %) resolve the issue
6) Get item by pressing first letter
*/
class CReceiveDlg : public CResizableDlg
{
    CUnit* unit_;
    CLand* land_;

    wxComboBox* combobox_item_types_;
    wxSpinCtrl* spin_items_amount_;
    wxComboBox* combobox_units_;
    wxCheckBox* order_repeating_;

    std::map<std::string, CUnit*> unit_name_to_unit_;

public:
    CReceiveDlg(wxWindow *parent, CUnit * pUnit, CLand* pLand);
    ~CReceiveDlg();

private:
    void init_item_types_combobox();

    std::set<std::string> get_item_types_list(CUnit* unit, CLand* land) const;
    std::vector<std::string> get_units_with_item(const std::string& item_type, CUnit* unit, CLand* land);

    //Events
    void OnItemChosen   (wxCommandEvent& event);
    void OnMax          (wxCommandEvent& event);
    void OnOk           (wxCommandEvent& event);
    void OnAdd          (wxCommandEvent& event);
    void OnCancel       (wxCommandEvent& event);

};

#endif 
