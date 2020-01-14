#include "receivedlg.h"

#include <wx/statline.h>
#include <sstream>
#include <algorithm>
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

    CItem silv;
    silv.code_name_ = "SILV";
    if (items.find(silv) != items.end())
    {
        std::string codename, name, plural;
        gpApp->ResolveAliasItems(silv.code_name_, codename, name, plural);

        long_to_short_item_names_[plural] = silv.code_name_;        
        combobox_item_types_->Append(plural);
        items.erase(silv);
    }
    for (const CItem& item : items)
    {
        std::string codename, name, plural;
        gpApp->ResolveAliasItems(item.code_name_, codename, name, plural);

        long_to_short_item_names_[plural] = item.code_name_;        
        combobox_item_types_->Append(plural);
    }        
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
        std::set<CItem> cur_products = unit_control::get_all_items(cur_unit);
        for (const auto& prod : cur_products)
            if (prod.amount_ > 0)
                item_types_list.insert(prod);
    }
    return item_types_list;
}

std::vector<std::string> CReceiveDlg::get_units_with_item(const std::string& item_type, CUnit* unit, CLand* land)
{
    unit_name_to_unit_.clear();
    std::vector<std::string> unit_names;
    std::vector<CUnit*> other_units;
    //get all our units except chosen one
    land_control::get_units_if(land, other_units, [&unit](CUnit* cur_unit) {
        return cur_unit->IsOurs && cur_unit->Id != unit->Id && !IS_NEW_UNIT(cur_unit);
    });

    //sort them out by amount of items they have
    std::vector<std::pair<long, CUnit*>> products_per_unit;
    for (CUnit* cur_unit : other_units)
        products_per_unit.emplace_back(std::pair<long, CUnit*>{unit_control::get_item_amount(cur_unit, item_type), cur_unit});

    std::sort(products_per_unit.begin(), products_per_unit.end(), 
        [](const std::pair<long, CUnit*>& u1, const std::pair<long, CUnit*>& u2) {
        return u1.first > u2.first;
    });

    for (const auto& ppu : products_per_unit)
    {
        if (ppu.first > 0)
        {
            std::stringstream unit_name;
            unit_name << "(" << std::to_string(ppu.second->Id) << ") ";
            unit_name << std::string(ppu.second->Name.GetData(), ppu.second->Name.GetLength());
            unit_name << " (max: " + std::to_string(ppu.first) + ")";
            unit_names.push_back(unit_name.str());
            unit_name_to_unit_[unit_name.str()] = ppu.second;
        }
    }
    return unit_names;
}

void CReceiveDlg::OnItemChosen   (wxCommandEvent& event)
{
    std::string long_name_item = combobox_item_types_->GetValue().ToStdString();
    std::vector<std::string> unit_names = get_units_with_item(long_to_short_item_names_[long_name_item], unit_, land_);

    combobox_units_->Clear();
    combobox_units_->Append(FROM_ALL_);
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

void CReceiveDlg::perform_give(CUnit* giving_one, CUnit* receiving_one, long amount, const std::string& item_code_name)
{
    auto order = orders::control::compose_give_order(receiving_one, amount, 
                                item_code_name, "", order_repeating_->GetValue());
    
    orders::control::add_order_to_unit(order, giving_one);
}

void CReceiveDlg::perform_take(CUnit* giving_one, CUnit* receiving_one, long amount, const std::string& item_code_name)
{
    std::string order = compose_take_order(giving_one, amount, item_code_name);
    orders::control::add_order_to_unit(order, receiving_one);
}

void CReceiveDlg::OnOk           (wxCommandEvent& event)
{
    std::string long_name = combobox_item_types_->GetValue().ToStdString();
    if (long_name.empty())
        return;

    std::string giver_name = combobox_units_->GetValue().ToStdString();
    if (giver_name == FROM_ALL_)
    {
        for (const auto& pair : unit_name_to_unit_)
        {
            long amount = unit_control::get_item_amount(pair.second, long_to_short_item_names_[long_name]);
            if (use_order_take_->GetValue())
                perform_take(pair.second, unit_, amount, long_to_short_item_names_[long_name]);
            else
                perform_give(pair.second, unit_, amount, long_to_short_item_names_[long_name]);            
        }
    }
    else 
    {
        if (unit_name_to_unit_.find(giver_name) == unit_name_to_unit_.end())
            return;

        long amount = spin_items_amount_->GetValue();
        if (amount <= 0)
            return;

        CUnit* giving_unit = unit_name_to_unit_[giver_name];
        if (use_order_take_->GetValue())
            perform_take(giving_unit, unit_, amount, long_to_short_item_names_[long_name]);
        else
            perform_give(giving_unit, unit_, amount, long_to_short_item_names_[long_name]);
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

