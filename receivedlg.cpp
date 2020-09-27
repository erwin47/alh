#include "receivedlg.h"

#include <wx/statline.h>
#include <wx/listbox.h>
#include <sstream>
#include <algorithm>
#include "data_control.h"
//#include "ah_control.h"

std::set<CItem> get_item_types_list(CUnit* unit, CLand* land)
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
        for (const CItem& prod : cur_products) {
            CItem item = prod;
            std::set<CItem>::iterator it = item_types_list.find(prod);
            if (it != item_types_list.end() && item.amount_ > 0) {
                item.amount_ += it->amount_;
                item_types_list.erase(it);
            }
            if (item.amount_ > 0)
                item_types_list.insert(item);
        }
    }
    return item_types_list;
}


CReceiveDlg::CReceiveDlg(wxWindow *parent, CUnit * pUnit, CLand* pLand) : 
    CResizableDlg( parent, wxT("Receive order"), RECEIVE_DLG_SETTINGS ), 
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

    //!TODO: verify that vector consist of codenames, not longnames
    std::vector<std::string> group_records = game_control::get_game_config<std::string>(RECEIVE_DLG_SETTINGS, SZ_RECDLG_GROUPS);
    for (auto& group : group_records)
    {
        std::vector<std::string> group_values = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, group.c_str());
        if (group_values.size() > 0)
        {
            //fix out if name wasn't a codename, but short or long name
            for (std::string& group_value : group_values)
            {
                std::string name, plural;
                gpApp->ResolveAliasItems(group_value, group_value, name, plural);        
            }
            groups_[group] = group_values;
        }
    }

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
    bool beg(true);
    std::stringstream ss;
    for (const auto& group : groups_)
    {
        if (beg)
        {
            ss << group.first;
            beg = false;
        }
        else
        {
            ss << "," << group.first;
        }        
    }
    gpApp->SetConfig(RECEIVE_DLG_SETTINGS, SZ_RECDLG_GROUPS, ss.str().c_str());
}

void CReceiveDlg::init_item_types_combobox()
{
    long_to_short_item_names_.clear();
    combobox_item_types_->Clear();
    //gpApp->m_pAtlantis->RunLandOrders(land_, TurnSequence::SQ_GIVE_PRE);
    std::set<CItem> items = get_item_types_list(unit_, land_);

    gpApp->m_pAtlantis->RunLandOrders(land_, TurnSequence::SQ_FIRST, TurnSequence::SQ_GIVE);

    //Groups
    for (const auto& group : groups_)
    {
        //verify if we have a unit with an item belonging to a group
        std::vector<std::string> unit_names;
        std::vector<std::string> current_unit_names;
        for (const auto& item_name : group.second)
        {
            current_unit_names = get_units_with_item(item_name, unit_, land_);
            unit_names.insert(unit_names.end(), current_unit_names.begin(), current_unit_names.end());
        }
        if (unit_names.size() == 0)
            continue;//no units with an item from a group -> skip the group

        std::string long_name = "=[" + group.first + "]=";
        long_to_short_item_names_[long_name] = group.first;
        combobox_item_types_->Append(long_name);        
    }

    //Silver to be on top, as the most common
    CItem silv;
    silv.code_name_ = "SILV";
    if (items.find(silv) != items.end())
    {
        std::string codename, name, plural;
        gpApp->ResolveAliasItems(silv.code_name_, codename, name, plural);

        plural += " (" + std::to_string(items.find(silv)->amount_) + ")";
        long_to_short_item_names_[plural] = silv.code_name_;
        combobox_item_types_->Append(plural);
        items.erase(silv);
    }

    for (const CItem& item : items)
    {
        std::string codename, name, plural;
        gpApp->ResolveAliasItems(item.code_name_, codename, name, plural);
        plural += " (" + std::to_string(item.amount_) + ")";
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



std::string weight_to_represent(long weight, long capacity)
{
    if (weight > capacity)
        return "-";
    return std::to_string(capacity-weight);
}

std::vector<std::string> CReceiveDlg::get_units_with_item(const std::string& item_type, CUnit* unit, CLand* land)
{
    std::vector<std::string> unit_names;
    std::vector<CUnit*> other_units;
    //get all our units except chosen one
    land_control::get_units_if(land, other_units, [&unit](CUnit* cur_unit) {
        return cur_unit->IsOurs && cur_unit->Id != unit->Id && !IS_NEW_UNIT(cur_unit);
    });

    //sort them out by amount of items they have
    //gpApp->m_pAtlantis->RunLandOrders(land_, TurnSequence::SQ_FIRST, TurnSequence::SQ_GIVE_PRE);
    std::vector<std::pair<long, CUnit*>> products_per_unit;
    for (CUnit* cur_unit : other_units)
        products_per_unit.emplace_back(std::pair<long, CUnit*>{unit_control::get_item_amount(cur_unit, item_type), cur_unit});

    std::sort(products_per_unit.begin(), products_per_unit.end(), 
        [](const std::pair<long, CUnit*>& u1, const std::pair<long, CUnit*>& u2) {
        return u1.first > u2.first;
    });
    //gpApp->m_pAtlantis->RunLandOrders(land_, TurnSequence::SQ_GIVE_PRE, TurnSequence::SQ_GIVE);
    for (const auto& ppu : products_per_unit)
    {
        if (ppu.first > 0)
        {
            std::stringstream unit_name;
            unit_name << "(" << std::to_string(ppu.second->Id) << ") ";//number of unit
            unit_name << unit_control::get_item_amount_by_mask(ppu.second, PRP_MEN) << " ";//amount of men there
            unit_name << std::string(ppu.second->Name.GetData(), ppu.second->Name.GetLength());//name of unit
            unit_name << " (:" << unit_control::get_item_amount(ppu.second, item_type, true) << "(";//initial amount of items
            unit_name << ppu.first << ") " << item_type << ")";//current amount of items
            unit_name << " " << unit_control::flags::compose_flag_info(ppu.second);//current fullmoth action sign (to help determine what is the unit)

            long weights[5];
            unit_control::get_weights(ppu.second, weights);
            unit_name << "[";
            unit_name << weights[0] << "/"; //unit weight
            unit_name << weight_to_represent(weights[0], weights[1]) << "/"; //move carry weight
            unit_name << weight_to_represent(weights[0], weights[2]) << "/"; //ride carry weight
            unit_name << weight_to_represent(weights[0], weights[3]) << "/"; //fly carry weight
            unit_name << weight_to_represent(weights[0], weights[4]) << "/"; //swim carry weight
            unit_name << "]";

            unit_names.push_back(unit_name.str());
            unit_name_to_unit_[unit_name.str()] = {ppu.first, item_type, ppu.second};
        }
    }
    return unit_names;
}

void CReceiveDlg::OnItemChosen(wxCommandEvent& event)
{
    std::string long_name_item = combobox_item_types_->GetValue().ToStdString();
    std::vector<std::string> unit_names;
    unit_name_to_unit_.clear();
    if (groups_.find(long_to_short_item_names_[long_name_item]) != groups_.end())
    {
        std::vector<std::string> current_unit_names;
        for (const auto& item_name : groups_[long_to_short_item_names_[long_name_item]])
        {
            current_unit_names = get_units_with_item(item_name, unit_, land_);
            unit_names.insert(unit_names.end(), current_unit_names.begin(), current_unit_names.end());
        }
    }
    else
        unit_names = get_units_with_item(long_to_short_item_names_[long_name_item], unit_, land_);

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
    const ItemUnitPair& itemunit = unit_name_to_unit_[giver_name];
    spin_items_amount_->SetValue(itemunit.amount_);
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
    std::string giver_name = combobox_units_->GetValue().ToStdString();
    if (giver_name.empty())
        return;

    if (giver_name == FROM_ALL_)
    {
        for (const auto& pair : unit_name_to_unit_)
        {
            if (use_order_take_->GetValue())
                perform_take(pair.second.unit_, unit_, pair.second.amount_, pair.second.item_code_);
            else
                perform_give(pair.second.unit_, unit_, pair.second.amount_, pair.second.item_code_);
        }
    }
    else 
    {
        if (unit_name_to_unit_.find(giver_name) == unit_name_to_unit_.end())
            return;

        long amount = spin_items_amount_->GetValue();
        if (amount <= 0)
            return;

        const ItemUnitPair& itemunit = unit_name_to_unit_[giver_name];
        if (use_order_take_->GetValue())
            perform_take(itemunit.unit_, unit_, amount, itemunit.item_code_);
        else
            perform_give(itemunit.unit_, unit_, amount, itemunit.item_code_);
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
    gpApp->m_pAtlantis->RunOrders(land_);  
    StoreSize();
    EndModal(wxID_CANCEL);
}

//=============================

CItemChooseDlg::CItemChooseDlg(wxWindow *parent, CUnit * unit, CLand* land) : 
    CResizableDlg( parent, wxT("Item filter"), "UNIT_PANE_ITEM_FILTER_DLG" ), 
    unit_(unit),
    land_(land)
{
    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer* itemssizer = new wxBoxSizer( wxHORIZONTAL );

    listbox_items_ = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_NEEDED_SB | wxLB_SORT);
    listbox_items_->Bind(wxEVT_COMBOBOX, &CItemChooseDlg::OnItemChosen, this);

    //items_->Bind(wxEVT_LISTBOX, &CReceiveDlg::OnItemChosen, this);
    listbox_items_->Bind(wxEVT_LISTBOX_DCLICK, &CItemChooseDlg::OnOk, this);
    itemssizer->Add(listbox_items_, 1, wxALL | wxEXPAND);

    std::set<CItem> items = get_item_types_list(unit, land);
    for (const CItem& item : items)
    {
        std::string codename, name, plural;
        gpApp->ResolveAliasItems(item.code_name_, codename, name, plural);
        plural += " (" + std::to_string(item.amount_) + ")";
        plural_to_code_[plural] = item.code_name_;        
        listbox_items_->Append(plural);
    }

    wxBoxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );
    wxButton* ok_button = new wxButton(this, -1, wxT("Ok"));
    wxButton* cancel_button = new wxButton(this, -1, wxT("Cancel"));
    ok_button->Bind(wxEVT_BUTTON, &CItemChooseDlg::OnOk, this);
    cancel_button->Bind(wxEVT_BUTTON, &CItemChooseDlg::OnCancel, this);
    buttonsizer->Add(ok_button, 1, wxALIGN_CENTER | wxALL);
    buttonsizer->Add(cancel_button, 1, wxALIGN_CENTER | wxALL);

    topsizer->Add(itemssizer, 5, wxALL | wxEXPAND);
    //topsizer->Add(new wxStaticLine(this), 0, 0);
    topsizer->Add(buttonsizer, 0, wxALIGN_BOTTOM);

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

CItemChooseDlg::~CItemChooseDlg()
{

}

void CItemChooseDlg::OnItemChosen   (wxCommandEvent& event)
{
    return;
}

void CItemChooseDlg::OnOk           (wxCommandEvent& event)
{
    int selected_item = listbox_items_->GetSelection();
    if (selected_item == wxNOT_FOUND) {
        EndModal(wxID_CANCEL);
        return;
    }

    std::string long_name_item = listbox_items_->GetString(selected_item).ToStdString();
    chosen_code_ = plural_to_code_[long_name_item];
    StoreSize();  
    EndModal(wxID_OK);
}

void CItemChooseDlg::OnCancel       (wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}


  