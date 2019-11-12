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
    use_order_take_ = new wxCheckBox(this, -1, "take");

    combobox_item_types_->Bind(wxEVT_COMBOBOX, &CReceiveDlg::OnItemChosen, this);
    itemssizer->Add(new wxStaticText(this, -1, wxT("Item: ")), 0, wxALL);
    itemssizer->Add(combobox_item_types_, 1, wxALL);
    itemssizer->Add(spin_items_amount_, 0, wxALL);
    itemssizer->Add(order_repeating_, 0, wxALL);
    itemssizer->Add(use_order_take_, 0, wxALL);
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
    long_to_short_item_names_.clear();
    combobox_item_types_->Clear();
    std::set<CItem> items = get_item_types_list(unit_, land_);
    for (const CItem& item : items)
    {
        std::string name, plural;
        gpApp->ResolveAliasItems(item.code_name_, name, plural);

        long_to_short_item_names_[plural] = item.code_name_;        
        combobox_item_types_->Append(plural);
    }
        
}

std::string CReceiveDlg::compose_give_order(CUnit* to_whom, long amount, const std::string& item)
{
    std::stringstream order;
    order << "give ";
    if (IS_NEW_UNIT(to_whom))
        order << "NEW " << (long)REVERSE_NEW_UNIT_ID(to_whom->Id);
    else
        order << to_whom->Id;
    order << " " << amount << " " << item;
    return order.str();  
}
std::string CReceiveDlg::compose_give_comment(CUnit* from_whom, long amount, const std::string& item)
{
    std::stringstream comment;
    comment << ";receives "<< amount << " " << item;
    if (IS_NEW_UNIT(from_whom))
        comment << " from NEW " << (long)REVERSE_NEW_UNIT_ID(from_whom->Id);
    else
        comment << " from " << from_whom->Id;
    return comment.str();  
}

std::string CReceiveDlg::compose_take_order(CUnit* from_whom, long amount, const std::string& item)
{
    std::stringstream order;
    order << "take from ";
    if (IS_NEW_UNIT(from_whom))
        order << "NEW " << (long)REVERSE_NEW_UNIT_ID(from_whom->Id);
    else
        order << from_whom->Id;
    order << " " << amount << " " << item;
    return order.str();
}

std::string CReceiveDlg::compose_take_comment(CUnit* to_whom, long amount, const std::string& item)
{
    std::stringstream comment;
    comment << "gives " << amount << " " << item;
    if (IS_NEW_UNIT(to_whom))
        comment << " to NEW " << (long)REVERSE_NEW_UNIT_ID(to_whom->Id);
    else
        comment << " to " << to_whom->Id;
    return comment.str();
}

std::set<CItem> CReceiveDlg::get_item_types_list(CUnit* unit, CLand* land) const
{
    //getting units belonging to current faction
    std::vector<CUnit*> other_units;
    land_control::get_units_if(land, other_units, [&unit](CUnit* cur_unit) {
        return cur_unit->IsOurs && cur_unit->Id != unit->Id && !IS_NEW_UNIT(cur_unit);
    });

    //getting set of unique by ShortName products
    std::set<CItem> item_types_list;
    for (CUnit* cur_unit : other_units)
    {
        std::set<CItem>& cur_products = unit_control::get_items(cur_unit);
        for (const auto& prod : cur_products)
            item_types_list.insert(prod);
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
        std::set<CItem>& cur_products = unit_control::get_items(cur_unit);
        for (const auto& prod : cur_products)
        {
            if (prod.code_name_ == item_type && prod.amount_ > 0)
            {
                std::stringstream unit_name;
                unit_name << "(" << std::to_string(cur_unit->Id) << ") ";
                unit_name << std::string(cur_unit->Name.GetData(), cur_unit->Name.GetLength());
                unit_name << " (max: " + std::to_string(prod.amount_) + ")";
                unit_names.push_back(unit_name.str());
                unit_name_to_unit_[unit_name.str()] = cur_unit;
            }
        }
    }
    return unit_names;
}

void CReceiveDlg::OnItemChosen   (wxCommandEvent& event)
{
    std::string long_name_item = combobox_item_types_->GetValue().ToStdString();
    std::vector<std::string> unit_names = get_units_with_item(long_to_short_item_names_[long_name_item], unit_, land_);

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

    std::string long_name_item = combobox_item_types_->GetValue().ToStdString();
    int amount = unit_control::get_item_amount(giver, long_to_short_item_names_[long_name_item]);
    spin_items_amount_->SetValue(amount);
}

void CReceiveDlg::set_order(CUnit* unit, const std::string& order)
{
    if (!unit->Orders.IsEmpty())
        unit->Orders << EOL_SCR;
    if (order_repeating_->GetValue())
        unit->Orders << "@";
    unit->Orders << order.c_str();
}

void CReceiveDlg::set_comment(CUnit* unit, const std::string& comment)
{
    if (!unit->Comments.IsEmpty())
        unit->Comments << EOL_SCR;
    unit->Comments << comment.c_str();
}

void CReceiveDlg::OnOk           (wxCommandEvent& event)
{
    long amount = spin_items_amount_->GetValue();
    if (amount <= 0)
        return;
    
    std::string long_name = combobox_item_types_->GetValue().ToStdString();
    if (long_name.empty())
        return;

    std::string giver_name = combobox_units_->GetValue().ToStdString();
    if (unit_name_to_unit_.find(giver_name) == unit_name_to_unit_.end())
        return;

    CUnit* giving_unit = unit_name_to_unit_[giver_name];
    if (use_order_take_->GetValue())
    {
        std::string order = compose_take_order(giving_unit, amount, long_to_short_item_names_[long_name]);
        std::string comment = compose_take_comment(unit_, amount, long_to_short_item_names_[long_name]);
        set_order(unit_, order);
        set_comment(giving_unit, comment);
    }        
    else
    {
        std::string order = compose_give_order(unit_, amount, long_to_short_item_names_[long_name]);
        std::string comment = compose_give_comment(giving_unit, amount, long_to_short_item_names_[long_name]);
        set_order(giving_unit, order);
        set_comment(unit_, comment);
    }
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

