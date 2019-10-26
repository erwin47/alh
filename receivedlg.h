#ifndef AH_RECEIVE_DIALOG_INCL
#define AH_RECEIVE_DIALOG_INCL

#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "ah_control.h"
#include "ahframe.h"
#include <set>


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
    //flags consist of command and it's possible values
    CReceiveDlg(wxWindow *parent, CUnit * pUnit, CLand* pLand);
    ~CReceiveDlg();

private:
    void init_item_types_combobox();

    std::set<std::string> get_item_types_list(CUnit* unit, CLand* land) const;
    std::vector<std::string> get_units_with_item(const std::string& item_type, CUnit* unit, CLand* land);

    //Events
    void OnItemChosen   (wxCommandEvent& event);
    void OnOk           (wxCommandEvent& event);
    void OnAdd          (wxCommandEvent& event);
    void OnCancel       (wxCommandEvent& event);
    //void onGiveAllButton(wxCommandEvent& event);

    //DECLARE_EVENT_TABLE()
};

#endif 