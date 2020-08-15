
#include "autologic.h"
#include "ah_control.h"
#include <vector>


namespace autologic
{

    void split_to_chunks(const std::string& str, std::string separator, std::vector<std::string>& chunks)
    {
        size_t pos = 0;
        size_t cur = str.find(separator, pos);
        while(cur != std::string::npos)
        {
            chunks.push_back(trim(str.substr(pos, cur - pos)));
            pos = cur+separator.size();
            cur = str.find(separator, pos);
        }
        size_t end_point = str.find('.', pos);
        chunks.push_back(trim(str.substr(pos)));
    }

    template<typename T>
    bool evaluate_operation(const T& arg, Operation op, const T& val)
    {
        switch(op)
        {
            case Operation::NONE: return false;
            case Operation::MORE: return arg > val;
            case Operation::LESS: return arg < val;
            case Operation::EQUAL: return arg == val;
            case Operation::MORE_OR_EQUAL: return arg >= val;
            case Operation::LESS_OR_EQUAL: return arg <= val;
            case Operation::NOT_EQUAL: return arg != val;
        }
        return false;
    }

    Command extract_command(const char* beg, const char* end)
    {
        const char* running_end = end;
        while (beg < end && !std::isalpha(*beg) && !std::isdigit(*beg))
            ++beg;
        while (beg < running_end && !std::isalpha(*running_end) && !std::isdigit(*running_end))
            --running_end;
        if (running_end != end)
            ++running_end;
        if (beg >= running_end)
            return Command::NONE;

        if (strnicmp(beg, "ITEM", sizeof("ITEM")-1) == 0)
            return Command::UNIT_ITEM;
        else if (strnicmp(beg, "SKILL", sizeof("SKILL")-1) == 0)
            return Command::UNIT_SKILL;
        else if (strnicmp(beg, "SPEED", sizeof("SPEED")-1) == 0)
            return Command::UNIT_SPEED;
        else if (strnicmp(beg, "FACTION", sizeof("FACTION")-1) == 0)
            return Command::UNIT_FACTION;
        else if (strnicmp(beg, "REG_NAME", sizeof("REG_NAME")-1) == 0)
            return Command::REGION_NAME;
        else if (strnicmp(beg, "LOC", sizeof("LOC")-1) == 0)
            return Command::REGION_LOCATION;
        else if (strnicmp(beg, "SELL", sizeof("SELL")-1) == 0)
            return Command::REGION_SELL_AMOUNT;
        else if (strnicmp(beg, "SELL_PRICE", sizeof("SELL_PRICE")-1) == 0)
            return Command::REGION_SELL_PRICE;
        else if (strnicmp(beg, "WANTED", sizeof("WANTED")-1) == 0)
            return Command::REGION_WANTED_AMOUNT;
        else if (strnicmp(beg, "WANTED_PRICE", sizeof("WANTED_PRICE")-1) == 0)
            return Command::REGION_WANTED_PRICE;            
        else if (strnicmp(beg, "RESOURCE", sizeof("RESOURCE")-1) == 0)
            return Command::REGION_RESOURCE;
        return Command::NONE;
    }

    std::string extract_argument(const char* beg, const char* end)
    {
        const char* running_end = end;
        while (beg < end && !std::isalpha(*beg) && !std::isdigit(*beg))
            ++beg;
        while (beg < running_end && !std::isalpha(*running_end) && !std::isdigit(*running_end))
            --running_end;
        if (running_end != end)
            ++running_end;
        if (beg >= running_end)
            return "";
        return std::string(beg, running_end);
    }

    bool parse_statement(const std::string& statement, Command& command, std::string& arg, Operation& operation, std::string& val)
    {
        const char* beg = statement.c_str();
        const char* end = beg + statement.size();
        const char* runner = strchr(beg, '[');
        if (runner == nullptr)
            return false;
        
        command = extract_command(beg, runner);
        beg = runner + 1;
        if (beg >= end)
            return false;
        
        runner = strchr(beg, ']');
        if (runner == nullptr)
            return false;

        arg = extract_argument(beg, runner);
        beg = runner + 1;
        if (beg >= end)
            return true;

        runner = strchr(beg, '>');
        if (runner != nullptr)
        {
            beg = runner+1;
            if (beg < end && *beg == '=')
            {
                operation = Operation::MORE_OR_EQUAL;
                beg = beg + 1;
            }                        
            else
            {
                operation = Operation::MORE;
            }
            val = extract_argument(beg, end);
            return true;
        }

        runner = strchr(beg, '<');
        if (runner != nullptr)
        {
            beg = runner+1;
            if (beg < end && *beg == '=')
            {
                operation = Operation::LESS_OR_EQUAL;
                beg = beg + 1;
            }                        
            else
            {
                operation = Operation::LESS;
            }
            val = extract_argument(beg, end);
            return true;
        }        

        runner = strchr(beg, '!');
        if (runner != nullptr)
        {
            beg = runner+1;
            if (beg < end && *beg == '=')
            {
                operation = Operation::NOT_EQUAL;
                beg = beg + 1;
            }                        
            else
            {
                return false;
            }
            val = extract_argument(beg, end);
            return true;
        }        

        runner = strchr(beg, '=');
        if (runner != nullptr)
        {
            beg = runner+1;
            if (beg < end && *beg == '=')
                beg = beg + 1;//for `==` case

            operation = Operation::EQUAL;
            val = extract_argument(beg, end);
            return true;
        }
        return true;
    }

    std::string to_string(Operation op)
    {
        switch(op) {
            case Operation::NONE: return "NONE";
            case Operation::MORE: return "MORE";
            case Operation::LESS: return "LESS";
            case Operation::EQUAL: return "EQUAL";
            case Operation::MORE_OR_EQUAL: return "MORE_OR_EQUAL";
            case Operation::LESS_OR_EQUAL: return "LESS_OR_EQUAL";
            case Operation::NOT_EQUAL: return "NOT_EQUAL";
        }
        return "Unknown";
    }

    bool evaluate_condition(CLand* land, CUnit* unit, const std::string& statement, std::vector<unit_control::UnitError>& errors)
    {
        Command command;
        std::string arg;
        Operation operation;
        std::string val;
        bool result;

        if (!parse_statement(statement, command, arg, operation, val))
        {
            errors.push_back({"Error", unit, nullptr, "Couldn't parse statement: "+statement});
            return false;
        }

        switch(command)
        {
            case Command::NONE: {  
                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::NONE, returns false"});
                return false;
            }
            case Command::UNIT_ITEM: {
                if (unit == nullptr) {
                    errors.push_back({"Debug", nullptr, nullptr, "Command was evaluated as Command::UNIT_ITEM with no unit provided"});
                    return false;
                }

                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::UNIT_ITEM"});
                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    errors.push_back({"Debug", unit, nullptr, "Couldn't resolve :"+arg});
                    code = arg;
                }

                result = evaluate_operation(unit_control::get_item_amount(unit, code), operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, nullptr, "Item ("+code+") amount: "+std::to_string(unit_control::get_item_amount(unit, code)) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }

            case Command::UNIT_SKILL: {
                if (unit == nullptr) {
                    errors.push_back({"Debug", nullptr, nullptr, "Command was evaluated as Command::UNIT_SKILL with no unit provided"});
                    return false;
                }

                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::UNIT_SKILL"});
                arg = gpApp->ResolveAlias(arg.c_str());
                long skill_lvl;

                skill_lvl = skills_control::get_skill_lvl_from_days(unit_control::get_current_skill_days(unit, arg));
                result = evaluate_operation(skill_lvl, operation, atol(val.c_str()));
                
                errors.push_back({"Debug", unit, nullptr, "Skill ("+arg+") level: "+std::to_string(skill_lvl) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});

                return result;
            }

            case Command::UNIT_SPEED: {
                if (unit == nullptr) {
                    errors.push_back({"Debug", nullptr, nullptr, "Command was evaluated as Command::UNIT_SPEED with no unit provided"});
                    return false;
                }

                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::UNIT_SPEED"});
                unit_control::MoveMode movemode = unit_control::get_move_state(unit);

                result = evaluate_operation((long)movemode.speed_, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, nullptr, "Speed: "+std::to_string(movemode.speed_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});

                return result;
            }
            case Command::UNIT_FACTION: {
                if (unit == nullptr) {
                    errors.push_back({"Debug", nullptr, nullptr, "Command was evaluated as Command::UNIT_FACTION with no unit provided"});
                    return false;
                }

                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::UNIT_FACTION"});
                result = evaluate_operation(unit->FactionId, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, nullptr, "Faction: "+std::to_string(unit->FactionId) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }            
            case Command::REGION_NAME: {
                std::string reg_name = land->Name.GetData();
                result = evaluate_operation<std::string>(reg_name, operation, val);
                return result;
            }
            case Command::REGION_LOCATION: {
                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::REGION_LOCATION"});

                const char* runner = arg.c_str();
                const char* end = runner + arg.size();
                std::vector<long> numbers;
                while (runner < end)
                {
                    if (isdigit(*runner))
                    {
                        numbers.push_back(atol(runner));
                        while (runner < end && isdigit(*runner))
                            ++runner;
                    }
                    ++runner;
                }
                int x, y, z;
                LandIdToCoord(land->Id,x,y,z);

                if (numbers.size() == 2) {
                    errors.push_back({"Debug", unit, nullptr, "Actual region: "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+
                        "; Parsed coordinates: " + std::to_string(numbers[0]) + ", " + std::to_string(numbers[1])});
                    return x == numbers[0] && y == numbers[1] && z == 1;
                } else if (numbers.size() == 3) {
                    return x == numbers[0] && y == numbers[1] && z == numbers[2];
                } else {
                    errors.push_back({"Debug", unit, nullptr, "Invalid amount of parsed arguments: "+std::to_string(numbers.size())});
                    return false;
                }
            }
            case Command::REGION_SELL_AMOUNT: {
                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::REGION_SELL_AMOUNT"});

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, nullptr, "Couldn't resolve :"+arg});
                }                    

                CProductMarket prod = land_control::get_for_sale(land->current_state_, code);
                result = evaluate_operation(prod.item_.amount_, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, nullptr, "Sell ("+code+") amount: "+std::to_string(prod.item_.amount_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }
            case Command::REGION_SELL_PRICE: {
                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::REGION_SELL_PRICE"});

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, nullptr, "Couldn't resolve :"+arg});
                }                    

                CProductMarket prod = land_control::get_for_sale(land->current_state_, code);
                result = evaluate_operation(prod.price_, operation, atol(val.c_str()));

                errors.push_back({"Debug", unit, nullptr, "Sell ("+code+") price: "+std::to_string(prod.price_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }            
            case Command::REGION_WANTED_AMOUNT: {
                    errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::REGION_WANTED_AMOUNT"});        

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, nullptr, "Couldn't resolve :"+arg});
                }                    

                CProductMarket prod = land_control::get_wanted(land->current_state_, code);
                result = evaluate_operation(prod.item_.amount_, operation, atol(val.c_str()));

                errors.push_back({"Debug", unit, nullptr, "Wanted ("+code+") amount: "+std::to_string(prod.item_.amount_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;                
            }
            case Command::REGION_WANTED_PRICE: {
                    errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::REGION_WANTED_PRICE"});        

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, nullptr, "Couldn't resolve :"+arg});
                }                    

                CProductMarket prod = land_control::get_wanted(land->current_state_, code);
                result = evaluate_operation(prod.price_, operation, atol(val.c_str()));

                errors.push_back({"Debug", unit, nullptr, "Wanted ("+code+") price: "+std::to_string(prod.price_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;                
            }            
            case Command::REGION_RESOURCE: {
                errors.push_back({"Debug", unit, nullptr, "Command was evaluated as Command::REGION_RESOURCE"});

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, nullptr, "Couldn't resolve :"+arg});
                }                    
                long resources = land_control::get_resource(land->current_state_, code);
                bool result = evaluate_operation(resources, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, nullptr, "Resources ("+code+") amount: "+std::to_string(resources) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }
        }
        return false;
    }

    bool evaluate_amount_filter(long amount, const std::string& filter)
    {
        autologic::Command command;
        std::string arg;
        autologic::Operation operation;
        std::string val;              
        if (autologic::parse_statement("[]"+filter, command, arg, operation, val))
        {
            switch(operation)
            {
                case Operation::NONE: return false;
                case Operation::MORE: return amount > atol(val.c_str());
                case Operation::LESS: return amount < atol(val.c_str());
                case Operation::EQUAL: return amount == atol(val.c_str());
                case Operation::MORE_OR_EQUAL: return amount >= atol(val.c_str());
                case Operation::LESS_OR_EQUAL: return amount <= atol(val.c_str());
                case Operation::NOT_EQUAL: return amount != atol(val.c_str());
            }
        }
        return false;
    }

    bool evaluate_unit_statement(CLand* land, CUnit* unit, 
                                 const std::string& statement, 
                                 std::vector<unit_control::UnitError>& evaluation_errors)
    {
        bool result = true;
        size_t pos = 0;
        size_t or_op = statement.find("||");
        while(or_op != std::string::npos)
        {//parse each statement separated by "||" separately
            std::string substatement = statement.substr(pos, or_op - pos);
            size_t sub_beg = 0;
            size_t and_op = substatement.find("&&");
            while (and_op != std::string::npos)
            {//each statement separated by "&&" should be evaluated
                result = result && evaluate_condition(land, unit, substatement.substr(sub_beg, and_op-sub_beg), evaluation_errors);
                if (!result)//false, no need evaluate other AND
                    break;

                sub_beg = and_op+2;
                and_op = substatement.find("&&", sub_beg);
            }

            if (result)//evaluate last/only statement if up to now it's true
                result = result && evaluate_condition(land, unit, substatement.substr(sub_beg), evaluation_errors);

            //true, no need to evaluate other OR
            if (result)
                return result;

            pos = or_op+2;
            or_op = statement.find("||", pos);
            result = true;//need to reset it for other statements separated by `||`
        }

        size_t and_op = statement.find("&&", pos);
        while (and_op != std::string::npos)
        {
            result = result && evaluate_condition(land, unit, statement.substr(pos, and_op-pos), evaluation_errors);
            if (!result)//false, no need evaluate other AND
                return result;

            pos = and_op+2;
            and_op = statement.find("&&", pos);
        }
        return result && evaluate_condition(land, unit, statement.substr(pos), evaluation_errors);
    }

    bool evaluate_block_statement(CLand* land, long& amount, const std::string& statement, 
                      std::vector<unit_control::UnitError>& evaluation_errors)
    {
        Command first_command;
        std::string first_argument;
        Operation first_operation;
        std::string first_val;
        bool result = false;

        std::vector<std::string> conditions;
        split_to_chunks(statement, "&&", conditions);

        std::vector<CUnit*> units_fit_statement;
        land_control::perform_on_each_unit(land, [&](CUnit* unit) {

            bool result_per_unit = true;            
            for (size_t j = 0; j < conditions.size(); ++j)
            {
                if (j == 0)
                {//first condition                    
                    parse_statement(conditions[0], first_command, first_argument, first_operation, first_val);
                }
                result_per_unit = result_per_unit && evaluate_condition(land, unit, conditions[j], evaluation_errors);
                      
            }
            if (result_per_unit)
            {
                units_fit_statement.push_back(unit);
                result = true;
            }                
        });

        if (land->units_seq_.size() == 0) 
        {
            result = true;
            for (size_t j = 0; j < conditions.size(); ++j)
            {
                if (j == 0)
                {//first condition                    
                    parse_statement(conditions[0], first_command, first_argument, first_operation, first_val);
                }
                result = result && evaluate_condition(land, nullptr, conditions[j], evaluation_errors);
            }          
            
        }

        amount = 0;
        if (!result)
            return result;//no need to continue

        switch(first_command) {
            case Command::UNIT_ITEM:
            {
                for (CUnit* unit : units_fit_statement)
                    amount += unit_control::get_item_amount(unit, first_argument);
                break;
            }          
            case Command::UNIT_SKILL:
            case Command::UNIT_SPEED:
            case Command::UNIT_FACTION:
            {
                for (CUnit* unit : units_fit_statement)
                    amount += unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                break;
            }
            case Command::REGION_NAME:
                amount = 1; break;
            case Command::REGION_LOCATION:
                amount = 1; break;
            case Command::REGION_SELL_AMOUNT:
                amount = land_control::get_for_sale(land->current_state_, first_argument).item_.amount_; break;
            case Command::REGION_SELL_PRICE:
                amount = land_control::get_for_sale(land->current_state_, first_argument).price_; break;
            case Command::REGION_WANTED_AMOUNT:
                amount = land_control::get_wanted(land->current_state_, first_argument).item_.amount_; break;
            case Command::REGION_WANTED_PRICE: 
                amount = land_control::get_wanted(land->current_state_, first_argument).price_; break;
            case Command::REGION_RESOURCE:
                amount = land_control::get_resource(land->current_state_, first_argument); break;
        }
        return result;
    }

    bool evaluate_land_statement(CLand* land, long& amount, const std::string& statement, 
                      std::vector<unit_control::UnitError>& evaluation_errors)
    {
        bool result = false;
        amount = 0;

        std::vector<std::string> substatements;
        split_to_chunks(statement, "||", substatements);
        for (size_t i = 0; i < substatements.size(); ++i)
        {
            long substatement_amount(0);
            if (evaluate_block_statement(land, substatement_amount, substatements[i], evaluation_errors))
            {
                amount += substatement_amount;
                result = true;
            }
        }
        return result;
    }

    std::map<std::string, std::string> _gen_function_descriptions()
    {
        return {
          {"ITEM[MITH]>5", "True for all units, having item MITH more than 5 (and counts amount of MITH in the region)"},
          {"SKILL[ARMO]>3", "True for all units, having skill ARMO more than 3 (and counts amount of peasants among such units)"},
          {"SPEED[]>3", "True for all units, having speed above 3 (and counts amount of peasants among such units)"},
          {"FACTION[]==3", "True for all units, belonging to faction 3 (and counts amount of peasants among such units)"},
          {"REG_NAME[]==\"Onle\"", "True for all regions, having name \"Onle\""},
          {"LOC[15,27,2]", "True for region with specified coordinates"},
          {"SELL[MITH]>5", "True for region which sells more than 5 items of MITH"},
          {"SELL_PRICE[MITH]>120", "True for region which sells MITH for price higher than 120"},
          {"WANTED[MITH]>5", "True for region which buys more than 5 items of MITH"},
          {"WANTED_PRICE[MITH]>120", "True for region which buys MITH for price higher than 120"},
          {"RESOURCE[MITH]>7", "True for region where its possible to produce MITH more than 7 items"},
        };  
    }

    const std::map<std::string, std::string>& function_descriptions() 
    {
        static std::map<std::string, std::string> ret = _gen_function_descriptions();
        return ret;
    }
}