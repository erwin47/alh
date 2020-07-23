
#include "autologic.h"
#include "ah_control.h"
#include <vector>


namespace autologic
{
    template<typename T>
    bool evaluate_operation(const T& arg, Operation op, const T& val)
    {
        switch(op)
        {
            case Operation::NONE: return arg > 0;
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
        else if (strnicmp(beg, "LOC", sizeof("LOC")-1) == 0)
            return Command::REGION_LOCATION;
        else if (strnicmp(beg, "SELL", sizeof("SELL")-1) == 0)
            return Command::REGION_SELL;
        else if (strnicmp(beg, "WANTED", sizeof("WANTED")-1) == 0)
            return Command::REGION_WANTED;
        else if (strnicmp(beg, "RESOURCE", sizeof("RESOURCE")-1) == 0)
            return Command::REGION_RESOURCE;
        else if (strnicmp(beg, "SPEED", sizeof("SPEED")-1) == 0)
            return Command::UNIT_SPEED;

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

    bool evaluate_statement(CLand* land, CUnit* unit, const std::string& statement, std::vector<unit_control::UnitError>& errors)
    {
        Command command;
        std::string arg;
        Operation operation;
        std::string val;

        if (!parse_statement(statement, command, arg, operation, val))
        {
            errors.push_back({"Error", unit, "Couldn't parse statement: "+statement});
            return false;
        }

        switch(command) 
        {
            case Command::NONE: {  
                errors.push_back({"Debug", unit, "Command was evaluated as Command::NONE, returns false"});
                return false;
            }
            case Command::UNIT_ITEM: {
                errors.push_back({"Debug", unit, "Command was evaluated as Command::UNIT_ITEM"});
                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    errors.push_back({"Debug", unit, "Couldn't resolve :"+arg});
                    code = arg;
                }
                long item_amount = unit_control::get_item_amount(unit, code);
                bool result = evaluate_operation(item_amount, operation, atol(val.c_str()));

                errors.push_back({"Debug", unit, "Item ("+code+") amount: "+std::to_string(item_amount) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }

            case Command::UNIT_SKILL: {
                errors.push_back({"Debug", unit, "Command was evaluated as Command::UNIT_SKILL"});                
                arg = gpApp->ResolveAlias(arg.c_str());

                long skill_lvl = skills_control::get_skill_lvl_from_days(unit_control::get_current_skill_days(unit, arg));
                //errors.push_back({"Debug", unit, "skill level: "+std::to_string(skill_lvl)});
                bool result = evaluate_operation(skill_lvl, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, "Skill ("+arg+") level: "+std::to_string(skill_lvl) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});

                return result;
            }

            case Command::UNIT_SPEED: {
                errors.push_back({"Debug", unit, "Command was evaluated as Command::UNIT_SPEED"});
                unit_control::MoveMode movemode = unit_control::get_move_state(unit);

                bool result = evaluate_operation((long)movemode.speed_, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, "Speed: "+std::to_string(movemode.speed_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }

            case Command::REGION_LOCATION: {
                errors.push_back({"Debug", unit, "Command was evaluated as Command::REGION_LOCATION"});

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
                    errors.push_back({"Debug", unit, "Actual region: "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+
                        "; Parsed coordinates: " + std::to_string(numbers[0]) + ", " + std::to_string(numbers[1])});
                    return x == numbers[0] && y == numbers[1] && z == 1;
                } else if (numbers.size() == 3) {
                    return x == numbers[0] && y == numbers[1] && z == numbers[2];
                } else {
                    errors.push_back({"Debug", unit, "Invalid amount of parsed arguments: "+std::to_string(numbers.size())});
                    return false;
                }
            }
            case Command::REGION_SELL: {
                errors.push_back({"Debug", unit, "Command was evaluated as Command::REGION_SELL"});

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, "Couldn't resolve :"+arg});
                }                    

                CProductMarket prod = land_control::get_for_sale(land->current_state_, code);
                bool result = evaluate_operation(prod.item_.amount_, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, "Sell ("+code+") amount: "+std::to_string(prod.item_.amount_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }
            case Command::REGION_WANTED: {
                    errors.push_back({"Debug", unit, "Command was evaluated as Command::REGION_WANTED"});        

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, "Couldn't resolve :"+arg});
                }                    

                CProductMarket prod = land_control::get_wanted(land->current_state_, code);
                bool result = evaluate_operation(prod.item_.amount_, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, "Wanted ("+code+") amount: "+std::to_string(prod.item_.amount_) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;                
            }
            case Command::REGION_RESOURCE: {
                errors.push_back({"Debug", unit, "Command was evaluated as Command::REGION_RESOURCE"});

                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                {
                    code = arg;
                    errors.push_back({"Debug", unit, "Couldn't resolve :"+arg});
                }                    
                long resources = land_control::get_resource(land->current_state_, code);
                bool result = evaluate_operation(resources, operation, atol(val.c_str()));
                errors.push_back({"Debug", unit, "Resources ("+code+") amount: "+std::to_string(resources) + 
                    "; op: " + to_string(operation) + "; val: "+val + " == " + (result ? "TRUE" : "FALSE")});
                return result;
            }
        }
        return false;
    }
}