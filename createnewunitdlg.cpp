#include "createnewunitdlg.h"
#include "autonaming.h"

#include <wx/statline.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

#define STUDY_REPEAT_CHKBOX 1
#define STUDY_ENABLE_CHKBOX 2

BEGIN_EVENT_TABLE(CCreateNewUnit, wxDialog)
    EVT_BUTTON  (wxID_CANCEL  ,  CCreateNewUnit::OnCancel)
    EVT_BUTTON  (wxID_OK      ,  CCreateNewUnit::OnOk)
    EVT_SPINCTRL(-1           ,  CCreateNewUnit::onAnySpinUpdate)
    EVT_COMBOBOX(-1           ,  CCreateNewUnit::onAnyComboBoxUpdate)
    EVT_CHECKBOX(STUDY_ENABLE_CHKBOX,  CCreateNewUnit::onStudyCheckBoxUpdate)
    EVT_CHECKBOX(STUDY_REPEAT_CHKBOX,  CCreateNewUnit::onStudyCheckBoxUpdate)
    EVT_CHECKBOX(-1           ,  CCreateNewUnit::onAnyComboBoxUpdate)
END_EVENT_TABLE()

CCreateNewUnit::CCreateNewUnit(wxWindow *parent, CUnit * pUnit, CLand* pLand) : 
    CResizableDlg( parent, wxT("Create new unit"), "WINDOW_SPLIT_UNIT_DLG" ), 
    unit_(pUnit),
    land_(pLand)
{
    InitializeNaming(land_);
    wxBoxSizer* unit_names_sizer = NamingToForm();
    InitializeBuyItems(land_);
    wxBoxSizer* unit_buy_sizer = BuyItemsToForm();
    InitializeStudy();
    wxBoxSizer* unit_study_sizer = StudyToForm();
    InitializeRecvSilver(unit_->FactionId, land_);
    wxBoxSizer* unit_silver_sizer = RecvSilverToForm();
    InitializeMaintenance();
    wxBoxSizer* maintenance_sizer = MaintenanceToForm();
    InitializeExpences();
    wxBoxSizer* unit_expenses_sizer = ExpensesToForm();
    InitializeAdditionalOrders();
    wxBoxSizer* additional_order_sizer = AdditionalOrdersToForm();


    wxBoxSizer* unitsizer = new wxBoxSizer( wxVERTICAL );
    unitsizer->Add(unit_names_sizer, 0, 0);
    //unitsizer->Add(new wxStaticLine(this), 0, 0);
    unitsizer->Add(unit_buy_sizer, 0, wxALL);
    //unitsizer->Add(new wxStaticLine(this), 1, 0);
    unitsizer->Add(unit_study_sizer, 0, wxALL);
    //unitsizer->Add(new wxStaticLine(this), 1, 0);
    unitsizer->Add(unit_silver_sizer, 0, wxALL);
    //unitsizer->Add(new wxStaticLine(this), 1, 0);
    unitsizer->Add(maintenance_sizer, 0, wxALL);
    //unitsizer->Add(new wxStaticLine(this), 1, 0);
    unitsizer->Add(unit_expenses_sizer, 0, wxALL);
    //unitsizer->Add(new wxStaticLine(this), 1, 0);
    unitsizer->Add(additional_order_sizer, 1, wxALL | wxEXPAND);
    //unitsizer->Add(new wxStaticLine(this), 1, 0);

    InitializeFlags(unit_);
    wxBoxSizer* flagsizer = FlagsToForm();

    wxBoxSizer* mainsizer = new wxBoxSizer( wxHORIZONTAL );
    mainsizer->Add(unitsizer, 1, wxALL | wxEXPAND);
    mainsizer->Add(flagsizer, 0, wxALL | wxEXPAND);

    InitializeButtons();
    wxBoxSizer* buttonsizer = ButtonsToForm();

    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );
    topsizer->Add(mainsizer, 1, wxALL);
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

CCreateNewUnit::~CCreateNewUnit() 
{

}

void CCreateNewUnit::InitializeButtons()
{
    spin_copies_amount_ = new wxSpinCtrl(this, -1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
}

wxBoxSizer* CCreateNewUnit::ButtonsToForm()
{
    wxBoxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );    
    buttonsizer->Add(new wxButton(this, wxID_OK, wxT("Ok")), 1, wxALIGN_CENTER | wxALL);
    buttonsizer->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")), 1, wxALIGN_CENTER | wxALL);
    buttonsizer->Add(new wxStaticText(this, -1, wxT("  Copies: ")), 0, wxALL);
    buttonsizer->Add(spin_copies_amount_, 0, wxALL);
    return buttonsizer;
}

void CCreateNewUnit::InitializeNaming(CLand* land)
{
    spin_new_num_alias_ = new wxSpinCtrl(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    spin_new_num_alias_->SetValue(land->GetNextNewUnitNo());
    text_name_ = new wxTextCtrl(this, -1, wxT(""));
    text_loc_description_ = new wxTextCtrl(this, -1, wxT(""));
    text_description_ = new wxTextCtrl(this, -1, wxT(""));
}

wxBoxSizer* CCreateNewUnit::NamingToForm()
{
    wxBoxSizer* temp_sizer;
    wxBoxSizer* unit_names_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Alias(num):")), 0, wxALL);
    temp_sizer->Add(spin_new_num_alias_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Name: ")), 0, wxALL);
    temp_sizer->Add(text_name_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Local descr: ")), 0, wxALL);
    temp_sizer->Add(text_loc_description_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Description: ")), 0, wxALL);
    temp_sizer->Add(text_description_, 1, wxALL);
    unit_names_sizer->Add(temp_sizer, 0, wxALL);
    return unit_names_sizer;
}

void CCreateNewUnit::InitializeBuyItems(CLand* land)
{
    spin_buy_units_amount_ = new wxSpinCtrl(this, -1, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    combobox_buy_units_type_ = new wxComboBox(this, -1, wxT("buying"), wxDefaultPosition, wxDefaultSize, 0, NULL);
    for (auto& item : land->current_state_.for_sale_)
    {
        std::string codename, name, plural;
        std::string temp;
        if (gpApp->ResolveAliasItems(item.first, codename, name, plural))
            temp = plural + std::string(": ") + std::to_string(item.second.item_.amount_) + 
                std::string(" at $") + std::to_string(item.second.price_);
        else
            temp = item.first + std::string(": ") + std::to_string(item.second.item_.amount_) + 
                std::string(" at $") + std::to_string(item.second.price_);

        combobox_buy_units_type_->Append( temp );
        sale_products_.insert({temp, item.second});
    }
    if (combobox_buy_units_type_->GetCount() > 0)
        combobox_buy_units_type_->SetSelection(0);

    flag_buy_repeating_ = new wxCheckBox(this, -1, "repeating");
    flag_buy_all_ = new wxCheckBox(this, -1, "buy all");
}

wxBoxSizer* CCreateNewUnit::BuyItemsToForm()
{
    wxBoxSizer* temp_sizer;
    wxBoxSizer* unit_buy_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(spin_buy_units_amount_, 0, wxALL);
    temp_sizer->Add(combobox_buy_units_type_, 1, wxALL);
    unit_buy_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(flag_buy_repeating_, 0, wxALL);
    temp_sizer->Add(flag_buy_all_, 0, wxALL);
    unit_buy_sizer->Add(temp_sizer, 0, wxALL);
    return unit_buy_sizer;
}

void CCreateNewUnit::InitializeStudy()
{
    flag_check_study_ = new wxCheckBox(this, STUDY_ENABLE_CHKBOX, "");

    combobox_skills_ = new wxComboBox(this, -1, wxT("studying"), wxDefaultPosition, wxDefaultSize, 0, NULL);

    std::vector<Skill> skills;
    skills_control::get_skills_if(skills, [](const Skill& skill) {
        return !gpApp->IsMagicSkill(skill.short_name_.c_str());
    });
    for (const Skill& skill : skills)
        combobox_skills_->Append(skill.long_name_);
    if (combobox_skills_->GetCount() > 0)
        combobox_skills_->SetSelection(0);

    combobox_skill_lvl_ = new wxComboBox(this, -1, wxT("lvl"), wxDefaultPosition, wxDefaultSize, 0, NULL);
    combobox_skill_lvl_->Append("---");
    for (unsigned i = 0; i < 5; ++i)
        combobox_skill_lvl_->Append(std::to_string(i+1));
    combobox_skill_lvl_->SetSelection(0);
    combobox_skill_lvl_->Disable();

    flag_study_repeating_ = new wxCheckBox(this, STUDY_REPEAT_CHKBOX, "repeating");;
}

wxBoxSizer* CCreateNewUnit::StudyToForm()
{
    wxBoxSizer* temp_sizer;
    wxBoxSizer* unit_study_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(flag_check_study_, 0, wxALL);
    temp_sizer->Add(combobox_skills_, 1, wxALL);
    temp_sizer->Add(combobox_skill_lvl_, 0, wxALL);
    unit_study_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(flag_study_repeating_, 0, wxALL);
    unit_study_sizer->Add(temp_sizer, 0, wxALL);
    return unit_study_sizer;
}

void CCreateNewUnit::InitializeRecvSilver(int faction_id, CLand* land)
{
    spin_silver_amount_ = new wxSpinCtrl(this, -1, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
    combobox_units_ = new wxComboBox(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL);

    std::vector<CUnit*> local_units;
    land_control::get_units_if(land, local_units, [&faction_id](CUnit* unit) {
        return unit->FactionId == faction_id;
    });
    std::sort(local_units.begin(), local_units.end(), [](CUnit* u1, CUnit* u2) {
        return unit_control::get_item_amount(u1, PRP_SILVER) > unit_control::get_item_amount(u2, PRP_SILVER);
    });

    for (CUnit* unit : local_units)
    {
        if (unit_control::get_item_amount(unit, PRP_SILVER) > 0)
        {
            std::string unit_and_silver = std::string(unit->Name.GetData()) + "(" + 
                std::to_string(unit->Id) + ") " + std::string(unit->pFaction->Name.GetData()) + 
                " (max: " + std::to_string( unit_control::get_item_amount(unit, PRP_SILVER)) + ")";
            
            silver_holders_.insert({unit_and_silver, unit});
            combobox_units_->Append(unit_and_silver);
        }
    }

    button_give_all_ = new wxButton(this, -1, wxT("all needs"));
    button_give_all_->Bind(wxEVT_BUTTON, &CCreateNewUnit::onGiveAllButton, this);
    flag_receive_silver_repeating_ = new wxCheckBox(this, -1, "repeating");
}

wxBoxSizer* CCreateNewUnit::RecvSilverToForm()
{
    wxBoxSizer* temp_sizer;
    wxBoxSizer* unit_silver_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Receive: ")), 0, wxALL);
    temp_sizer->Add(spin_silver_amount_, 0, wxALL);
    temp_sizer->Add(button_give_all_, 0, wxALL);
    temp_sizer->Add(flag_receive_silver_repeating_, 0, wxALL);
    unit_silver_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("From: ")), 0, wxALL);
    temp_sizer->Add(combobox_units_, 0, wxALL);
    unit_silver_sizer->Add(temp_sizer, 1, wxALL);
    return unit_silver_sizer;
}

void CCreateNewUnit::InitializeMaintenance()
{
    spin_maintenance_turns_ = new wxSpinCtrl(this, -1, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
}

wxBoxSizer* CCreateNewUnit::MaintenanceToForm()
{
    wxBoxSizer* temp_sizer;
    wxBoxSizer* maintenance_sizer = new wxBoxSizer( wxVERTICAL );    
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("Maintenance for ")), 0, wxALL);
    temp_sizer->Add(spin_maintenance_turns_, 0, wxALL);
    maintenance_sizer->Add(temp_sizer, 0, wxALL);
    return maintenance_sizer;
}

void CCreateNewUnit::InitializeExpences()
{
    expenses_buying_ = new wxStaticText(this, -1, wxT("0"));
    expenses_studying_ = new wxStaticText(this, -1, wxT("0"));
    expenses_maintenance_ = new wxStaticText(this, -1, wxT("0"));
    expenses_all_ = new wxStaticText(this, -1, wxT("0")); 
}

wxBoxSizer* CCreateNewUnit::ExpensesToForm()
{
    wxBoxSizer* temp_sizer;
    wxBoxSizer* unit_expenses_sizer = new wxBoxSizer( wxVERTICAL );
    unit_expenses_sizer->Add(new wxStaticText(this, -1, wxT("Expenses: ")), 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("    For buying: ")), 0, wxALL);
    temp_sizer->Add(expenses_buying_, 0, wxALL);
    unit_expenses_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("    For studying: ")), 0, wxALL);
    temp_sizer->Add(expenses_studying_, 0, wxALL);    
    unit_expenses_sizer->Add(temp_sizer, 0, wxALL);
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("    For maintenance: ")), 0, wxALL);
    temp_sizer->Add(expenses_maintenance_, 0, wxALL);    
    unit_expenses_sizer->Add(temp_sizer, 0, wxALL);    
    temp_sizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer->Add(new wxStaticText(this, -1, wxT("    Sum: ")), 0, wxALL);
    temp_sizer->Add(expenses_all_, 0, wxALL);
    unit_expenses_sizer->Add(temp_sizer, 0, wxALL);
    return unit_expenses_sizer;
}

void CCreateNewUnit::InitializeAdditionalOrders()
{
    additional_orders_ = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE); 
}//wxTE_MULTILINE

wxBoxSizer* CCreateNewUnit::AdditionalOrdersToForm()
{
    wxBoxSizer* additional_order_sizer = new wxBoxSizer( wxVERTICAL );
    additional_order_sizer->Add(new wxStaticText(this, -1, wxT("Additional orders (not affect expences):")), 0, wxALL);
    additional_order_sizer->Add(additional_orders_, 1, wxALL | wxEXPAND);
    return additional_order_sizer;
}

void CCreateNewUnit::InitializeFlags(CUnit* unit)
{
    flag_behind_ = new wxCheckBox(this, -1, "behind");
    flag_behind_->SetValue(unit_control::flags::is_behind(unit));
    flag_avoid_ = new wxCheckBox(this, -1, "avoid");
    flag_avoid_->SetValue(unit_control::flags::is_avoid(unit));
    flag_hold_ = new wxCheckBox(this, -1, "hold");
    flag_hold_->SetValue(unit_control::flags::is_hold(unit));
    flag_noaid_ = new wxCheckBox(this, -1, "noaid");
    flag_noaid_->SetValue(unit_control::flags::is_noaid(unit));
    flag_guard_ = new wxCheckBox(this, -1, "guard");
    flag_guard_->SetValue(unit_control::flags::is_guard(unit));
    flag_nocross_ = new wxCheckBox(this, -1, "nocross");
    flag_nocross_->SetValue(unit_control::flags::is_nocross(unit));
    flag_share_ = new wxCheckBox(this, -1, "share");
    flag_share_->SetValue(unit_control::flags::is_sharing(unit));

    wxArrayString buf;
    buf.Add(wxT("none"));
    buf.Add(wxT("unit"));
    buf.Add(wxT("faction"));
    radiobox_flag_reveal_ = new wxRadioBox(this, -1, wxT("reveal"), wxDefaultPosition,
                        wxDefaultSize, buf, 1);
    if (unit_control::flags::is_reveal(unit, "unit"))
        radiobox_flag_reveal_->SetSelection(1);
    else if (unit_control::flags::is_reveal(unit, "faction"))
        radiobox_flag_reveal_->SetSelection(2);
    
    buf.clear();
    buf.Add(wxT("none"));
    buf.Add(wxT("walk"));
    buf.Add(wxT("ride"));    
    buf.Add(wxT("fly"));
    buf.Add(wxT("swim"));
    buf.Add(wxT("sail"));
    buf.Add(wxT("all"));
    radiobox_flag_spoils_ = new wxRadioBox(this, -1, wxT("spoils"), wxDefaultPosition,
                        wxDefaultSize, buf, 1);
    if (unit_control::flags::is_spoils(unit, "none"))
        radiobox_flag_spoils_->SetSelection(0);
    else if (unit_control::flags::is_spoils(unit, "walk"))
        radiobox_flag_spoils_->SetSelection(1);
    else if (unit_control::flags::is_spoils(unit, "ride"))
        radiobox_flag_spoils_->SetSelection(2);
    else if (unit_control::flags::is_spoils(unit, "fly"))
        radiobox_flag_spoils_->SetSelection(3);
    else if (unit_control::flags::is_spoils(unit, "swim"))
        radiobox_flag_spoils_->SetSelection(4);
    else if (unit_control::flags::is_spoils(unit, "sail"))
        radiobox_flag_spoils_->SetSelection(5);
    else if (unit_control::flags::is_spoils(unit, "all"))
        radiobox_flag_spoils_->SetSelection(6);

    buf.clear();
    buf.Add(wxT("none"));
    buf.Add(wxT("unit"));
    buf.Add(wxT("faction"));
    radiobox_flag_consume_ = new wxRadioBox(this, -1, wxT("consume"), wxDefaultPosition,
                        wxDefaultSize, buf, 1);
    if (unit_control::flags::is_consume(unit, "unit"))
        radiobox_flag_consume_->SetSelection(1);
    else if (unit_control::flags::is_consume(unit, "faction"))
        radiobox_flag_consume_->SetSelection(2);   
}

wxBoxSizer* CCreateNewUnit::FlagsToForm()
{
    wxBoxSizer* temp_sizer;
    wxBoxSizer* flagsizer = new wxBoxSizer( wxHORIZONTAL );
    temp_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer->Add(flag_behind_, 0, wxALL);
    temp_sizer->Add(flag_avoid_, 0, wxALL);
    temp_sizer->Add(flag_hold_, 0, wxALL);
    temp_sizer->Add(flag_noaid_, 0, wxALL);
    temp_sizer->Add(flag_guard_, 0, wxALL);
    temp_sizer->Add(flag_nocross_, 0, wxALL);
    temp_sizer->Add(flag_share_, 0, wxALL);
    temp_sizer->Add(radiobox_flag_consume_, 0, wxALL);
    flagsizer->Add(temp_sizer, 0, wxALL);
    flagsizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, 0);
    
    temp_sizer = new wxBoxSizer( wxVERTICAL );
    temp_sizer->Add(radiobox_flag_reveal_, 0, wxALL);
    temp_sizer->Add(radiobox_flag_spoils_, 0, wxALL);    
    flagsizer->Add(temp_sizer, 0, wxALL);
    return flagsizer;
}

void CCreateNewUnit::UpdateExpences()
{
    try 
    {
        //buying calculation
        std::string buy_unit_type(combobox_buy_units_type_->GetValue().mb_str());
        CProductMarket product = sale_products_.at(buy_unit_type);
        CItem& item = product.item_;
        long buying_expences = spin_buy_units_amount_->GetValue() * product.price_;

        //studying calculation
        long studying_expences = 0;
        if (flag_check_study_->IsChecked())
        {
            std::string study_skill(combobox_skills_->GetValue().mb_str());
            std::vector<Skill> skills;
            skills_control::get_skills_if(skills, [&study_skill](const Skill& skill) {
                return !gpApp->IsMagicSkill(skill.short_name_.c_str()) && 
                       !skill.long_name_.compare(study_skill.c_str());
            });
            if (skills.size() > 0)
                studying_expences = skills[0].study_price_ * spin_buy_units_amount_->GetValue();
        }

        //upkeep calculation        
        long upkeep_amount = 0;
        if (gpDataHelper->IsMan(item.code_name_.c_str()))
        {
            std::string name, plural;
            gpApp->ResolveAliasItems(item.code_name_, item.code_name_, name, plural);
            //actually I'd prefer to not know which section is it, needs to be refactored

            static std::vector<std::string> leaders = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, PRP_MEN_LEADER);
            if (std::find(leaders.begin(), leaders.end(), name) == leaders.end())
                upkeep_amount = spin_buy_units_amount_->GetValue() * game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_PEASANT);
            else
                upkeep_amount = spin_buy_units_amount_->GetValue() * game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_UPKEEP_LEADER);
        }
        long maintenance_expences = spin_maintenance_turns_->GetValue() * upkeep_amount;
        
        //summarize
        expenses_buying_->SetLabel(std::to_string(buying_expences));
        expenses_studying_->SetLabel(std::to_string(studying_expences));
        expenses_maintenance_->SetLabel(std::to_string(maintenance_expences));
        expenses_all_->SetLabel(std::to_string(buying_expences + studying_expences + maintenance_expences));
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

void CCreateNewUnit::UpdateAutoname()
{
    if (!game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_AUTONAMING))
        return;

    if (text_loc_description_->IsEmpty() || 
        text_loc_description_->GetValue().Find(" $c") ||
        text_loc_description_->GetValue().Find(" !c"))
    {
        //std::string descr = "[]" + std::string(combobox_skills_->GetValue().mb_str()) + " $c";
        //text_loc_description_->SetValue(descr.c_str());
        std::string study_skill;
        if (flag_check_study_->IsChecked())
        {
            study_skill = combobox_skills_->GetValue().mb_str();
            Skill skill;
            skills_control::get_first_skill_if(skill, [&](const Skill& cur_skill) {
                return cur_skill.long_name_ == study_skill;
            });
            study_skill = skill.short_name_;
        }

        std::string buy_unit_type(combobox_buy_units_type_->GetValue().mb_str());
        CProductMarket product = sale_products_.at(buy_unit_type);
     
        std::string result = autonaming::generate_initial_unit_autoname(product.item_.code_name_, study_skill);

        text_loc_description_->SetValue(result.c_str());
    }
}

void CCreateNewUnit::OnCancel       (wxCommandEvent& event)
{
    StoreSize();
    EndModal(wxID_CANCEL);
}
void CCreateNewUnit::OnOk           (wxCommandEvent& event)
{
    std::stringstream unit_order;
    unit_order << "behind " << (flag_behind_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "avoid " << (flag_avoid_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "hold " << (flag_hold_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "noaid " << (flag_noaid_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "guard " << (flag_guard_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "nocross " << (flag_nocross_->IsChecked()? 1 : 0) << std::endl;
    unit_order << "share " << (flag_share_->IsChecked()? 1 : 0) << std::endl;
    std::string temp_flag = radiobox_flag_reveal_->GetString(radiobox_flag_reveal_->GetSelection()).ToStdString();
    unit_order << "reveal " << (temp_flag.compare("none") ? temp_flag : "") << std::endl;
    temp_flag = radiobox_flag_spoils_->GetString(radiobox_flag_spoils_->GetSelection()).ToStdString();
    unit_order << "spoils " << temp_flag << std::endl;
    temp_flag = radiobox_flag_consume_->GetString(radiobox_flag_consume_->GetSelection()).ToStdString();
    unit_order << "consume " << (temp_flag.compare("none") ? temp_flag : "") << std::endl;

    std::string unit_name(text_name_->GetValue().ToStdString());
    if (!unit_name.empty())
        unit_order << "name unit \"" << unit_name << "\"" << std::endl;
    else if (game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_AUTONAMING)) //to autogenerated name
        unit_order << "@name unit \"\"" << std::endl;

    std::string unit_local_description(text_loc_description_->GetValue().mb_str());
    if (!unit_local_description.empty())
        unit_order << "@;;" << unit_local_description << std::endl;

    std::string unit_description(text_description_->GetValue().mb_str());
    if (!unit_description.empty())
        unit_order << "describe unit \"" << unit_description << "\"" << std::endl;

    int buy_units = spin_buy_units_amount_->GetValue();
    if (buy_units > 0)
    {
        std::string combo_buy_unit(combobox_buy_units_type_->GetValue().mb_str());
        CProductMarket product = sale_products_[combo_buy_unit];
        if (flag_buy_repeating_->IsChecked())
            unit_order << "@";
        if (flag_buy_all_->IsChecked())
            unit_order << "buy all " << product.item_.code_name_ << std::endl;
        else
            unit_order << "buy " << buy_units << " " << product.item_.code_name_ << std::endl;
    }

    if (flag_check_study_->IsChecked())
    {
        std::string lvl = std::string(combobox_skill_lvl_->GetValue().mb_str());

        if (flag_study_repeating_->IsChecked())
            unit_order << "@study \""<< combobox_skills_->GetValue() << "\"" << std::endl;
        else if (lvl != "---")
            unit_order << "study \"" << combobox_skills_->GetValue() << "\" " << lvl << std::endl;
        else 
            unit_order << "study \"" << combobox_skills_->GetValue() << "\"" << std::endl;
    }

    std::string add_orders = std::string(additional_orders_->GetValue().mb_str());
    if (add_orders.size() > 0)
        unit_order << add_orders;

    unit_->Orders.TrimRight(TRIM_ALL);
    if (!unit_->Orders.IsEmpty() && (unit_->Orders.GetData()[unit_->Orders.GetLength()-1] != '\n'))
        unit_->Orders << EOL_SCR;

    int new_unit_id = spin_new_num_alias_->GetValue();
    int recv_silver = spin_silver_amount_->GetValue();
    for (size_t i=0; i<spin_copies_amount_->GetValue(); i++)
    {
        CUnit * pUnitNew = gpApp->m_pAtlantis->SplitUnit(unit_, new_unit_id+i);
        if (pUnitNew)
            pUnitNew->Orders << unit_order.str().c_str();

        if (recv_silver > 0) 
        {
            std::string unit_name = std::string(combobox_units_->GetValue().mb_str());
            CUnit* giving_unit = silver_holders_[unit_name];            
            std::stringstream giver_orders;
            giver_orders << "give NEW " << new_unit_id+i << " " << recv_silver << " SILV";
            if (!giving_unit->Orders.IsEmpty() && (giving_unit->Orders.GetData()[giving_unit->Orders.GetLength()-1] != '\n'))
                giving_unit->Orders << EOL_SCR;
            giving_unit->Orders << giver_orders.str().c_str();
        }            
    }

    gpApp->m_pAtlantis->RunOrders(land_);

    StoreSize();
    EndModal(wxID_OK);
}
void CCreateNewUnit::onAnySpinUpdate(wxSpinEvent& event)
{
    UpdateExpences();
    UpdateAutoname();
}
void CCreateNewUnit::onAnyComboBoxUpdate(wxCommandEvent& event)
{
    UpdateExpences();
    UpdateAutoname();
}

void CCreateNewUnit::onGiveAllButton(wxCommandEvent & event)
{
    long expences;
    expenses_all_->GetLabel().ToLong(&expences);

    std::string unit_name = std::string(combobox_units_->GetValue().mb_str());
    CUnit* giving_unit = silver_holders_[unit_name];
    if (giving_unit != nullptr)
    {
        long unit_silver_amount = unit_control::get_item_amount(giving_unit, PRP_SILVER);
        spin_silver_amount_->SetValue(std::min(unit_silver_amount, expences));
    }
}

void CCreateNewUnit::onStudyCheckBoxUpdate(wxCommandEvent& event)
{
    if (!flag_check_study_->IsChecked())
        combobox_skill_lvl_->Disable();
    else if (flag_study_repeating_->IsChecked())
        combobox_skill_lvl_->Disable();
    else
        combobox_skill_lvl_->Enable();
}


