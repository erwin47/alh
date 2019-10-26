#include "receivedlg.h"

#include <wx/statline.h>
#include <sstream>
#include "data_control.h"
//#include "ah_control.h"

CReceiveDlg::CReceiveDlg(wxWindow *parent, CUnit * pUnit, CLand* pLand) : 
    CResizableDlg( parent, wxT("Receive order"), "Window to help unit receive items it needs" ), 
    unit_(pUnit),
    land_(pLand)
{
    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* itemssizer = new wxBoxSizer( wxHORIZONTAL );
    combobox_item_types_ = new wxComboBox(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL);
    spin_items_amount_ = new wxSpinCtrl(this, -1, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    order_repeating_ = new wxCheckBox(this, -1, "repeating");

    combobox_item_types_->Bind(wxEVT_COMBOBOX, &CReceiveDlg::OnItemChosen, this);
    itemssizer->Add(new wxStaticText(this, -1, wxT("Item: ")), 0, wxALL);
    itemssizer->Add(combobox_item_types_, 1, wxALL);
    itemssizer->Add(spin_items_amount_, 0, wxALL);
    itemssizer->Add(order_repeating_, 0, wxALL);
    init_item_types_combobox();

    wxBoxSizer* unitchoosesizer = new wxBoxSizer( wxHORIZONTAL );
    combobox_units_ = new wxComboBox(this, -1, wxT(""), wxDefaultPosition, wxSize(270,28), 0, NULL);
    wxButton* max_possible_button = new wxButton(this, -1, wxT("max"));
    max_possible_button->Bind(wxEVT_BUTTON, &CReceiveDlg::OnMax, this);
    unitchoosesizer->Add(new wxStaticText(this, -1, wxT("From: ")), 0, wxALL);
    unitchoosesizer->Add(combobox_units_, 1, wxALL);
    unitchoosesizer->Add(max_possible_button, 0, wxALL);

    wxBoxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );
    wxButton* ok_button = new wxButton(this, -1, wxT("Ok"));
    wxButton* add_button = new wxButton(this, -1, wxT("Add"));
    wxButton* cancel_button = new wxButton(this, -1, wxT("Cancel"));
    ok_button->Bind(wxEVT_BUTTON, &CReceiveDlg::OnOk, this);
    add_button->Bind(wxEVT_BUTTON, &CReceiveDlg::OnAdd, this);
    cancel_button->Bind(wxEVT_BUTTON, &CReceiveDlg::OnCancel, this);
    buttonsizer->Add(ok_button, 1, wxALIGN_CENTER | wxALL);
    buttonsizer->Add(add_button, 1, wxALIGN_CENTER | wxALL);
    buttonsizer->Add(cancel_button, 1, wxALIGN_CENTER | wxALL);

    topsizer->Add(itemssizer, 0, wxALL);
    topsizer->Add(new wxStaticLine(this), 0, 0);
    topsizer->Add(unitchoosesizer, 1, wxALL);
    topsizer->Add(new wxStaticLine(this), 0, 0);
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

CReceiveDlg::~CReceiveDlg() 
{

}

void CReceiveDlg::init_item_types_combobox()
{
    combobox_item_types_->Clear();
    std::set<std::string> item_types = get_item_types_list(unit_, land_);
    for (const std::string& item_type : item_types)
        combobox_item_types_->Append(item_type);
}

std::set<std::string> CReceiveDlg::get_item_types_list(CUnit* unit, CLand* land) const
{
    std::vector<CUnit*> other_units;
    land_control::get_units_if(land, other_units, [&unit](CUnit* cur_unit) {
        return cur_unit->IsOurs && cur_unit->Id != unit->Id && !IS_NEW_UNIT(cur_unit);
    });

    std::set<std::string> item_types_list;
    for (CUnit* cur_unit : other_units)
    {
        std::set<CProduct>& cur_products = unit_control::get_items(cur_unit);
        for (const auto& prod : cur_products)
            item_types_list.insert(std::string(prod.ShortName.GetData(), prod.ShortName.GetLength()));
    }
    return item_types_list;
}

std::vector<std::string> CReceiveDlg::get_units_with_item(const std::string& item_type, CUnit* unit, CLand* land)
{
    unit_name_to_unit_.clear();
    std::vector<std::string> unit_names;
    std::vector<CUnit*> other_units;
    land_control::get_units_if(land, other_units, [&unit](CUnit* cur_unit) {
        return cur_unit->IsOurs && cur_unit->Id != unit->Id && !IS_NEW_UNIT(cur_unit);
    });

    for (CUnit* cur_unit : other_units)
    {
        std::set<CProduct>& cur_products = unit_control::get_items(cur_unit);
        for (const auto& prod : cur_products)
        {
            if (SafeCmp(prod.ShortName.GetData(), item_type.c_str()) == 0 && prod.Amount > 0)
            {
                std::stringstream unit_name;
                unit_name << "(" << std::to_string(cur_unit->Id) << ") ";
                unit_name << std::string(cur_unit->Name.GetData(), cur_unit->Name.GetLength());
                unit_name << " (max: " + std::to_string(prod.Amount) + ")";
                unit_names.push_back(unit_name.str());
                unit_name_to_unit_[unit_name.str()] = cur_unit;
            }
        }
    }
    return unit_names;
}

void CReceiveDlg::OnItemChosen   (wxCommandEvent& event)
{
    std::string item = combobox_item_types_->GetValue().ToStdString();
    std::vector<std::string> unit_names = get_units_with_item(item, unit_, land_);

    combobox_units_->Clear();
    for (const std::string& unit_name : unit_names)
        combobox_units_->Append(unit_name);
}

void CReceiveDlg::OnMax          (wxCommandEvent& event)
{
    std::string giver_name = combobox_units_->GetValue().ToStdString();
    if (unit_name_to_unit_.find(giver_name) == unit_name_to_unit_.end())
        return;
    CUnit* giver = unit_name_to_unit_[giver_name];

    int amount = unit_control::get_item_amount(giver, combobox_item_types_->GetValue().ToStdString());
    spin_items_amount_->SetValue(amount);
}

void CReceiveDlg::OnOk           (wxCommandEvent& event)
{
    long amount = spin_items_amount_->GetValue();
    if (amount <= 0)
        return;

    std::stringstream order;
    if (order_repeating_->GetValue())
        order << "@";
    order << "give ";
    if (IS_NEW_UNIT(unit_))
        order << "NEW " << (long)REVERSE_NEW_UNIT_ID(unit_->Id);
    else
        order << unit_->Id;
    order << " " << amount << " " << combobox_item_types_->GetValue().ToStdString() << std::endl;

    std::string giver_name = combobox_units_->GetValue().ToStdString();
    if (unit_name_to_unit_.find(giver_name) == unit_name_to_unit_.end())
        return;
    CUnit* giver = unit_name_to_unit_[giver_name];

    if (!giver->Orders.IsEmpty())
        giver->Orders << EOL_SCR;
    giver->Orders << order.str().c_str();

    gpApp->m_pAtlantis->RunOrders(land_);

    StoreSize();
    EndModal(wxID_OK);
}

void CReceiveDlg::OnAdd          (wxCommandEvent& event)
{

}

void CReceiveDlg::OnCancel       (wxCommandEvent& event)
{
    StoreSize();
    EndModal(wxID_CANCEL);
}

