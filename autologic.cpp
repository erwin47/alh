
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
        
        runner = strchr(beg, '=');
        if (runner != nullptr)
        {
            beg = runner+1;
            if (beg < end && *beg == '=')
            {
                operation = Operation::EQUAL;
                beg = beg + 1;
            }                        
            else
            {
                operation = Operation::EQUAL;
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
        return true;
    }

    bool evaluate_statement(CLand* land, CUnit* unit, const std::string& statement)
    {
        Command command;
        std::string arg;
        Operation operation;
        std::string val;

        if (!parse_statement(statement, command, arg, operation, val))
            return false;

        switch(command) 
        {
            case Command::NONE: return false;
            case Command::UNIT_ITEM: {
                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                    code = arg;

                return evaluate_operation(unit_control::get_item_amount(unit, code), operation, atol(val.c_str()));
            }

            case Command::UNIT_SKILL: {
                std::string code, name, plural;
                arg = gpApp->ResolveAlias(arg.c_str());

                long skill_lvl = skills_control::get_skill_lvl_from_days(unit_control::get_current_skill_days(unit, arg));
                return evaluate_operation(skill_lvl, operation, atol(val.c_str()));
            }

            case Command::REGION_LOCATION: {
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
                if (numbers.size() == 2)
                    return x == numbers[0] && y == numbers[1] && z == 1;
                else if (numbers.size() == 3)
                    return x == numbers[0] && y == numbers[1] && z == numbers[2];
                else
                    return false;
            }
            case Command::REGION_SELL: {
                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                    code = arg;

                CProductMarket prod = land_control::get_for_sale(land->current_state_, code);
                return evaluate_operation(prod.item_.amount_, operation, atol(val.c_str()));
            }
            case Command::REGION_WANTED: {
                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                    code = arg;

                CProductMarket prod = land_control::get_wanted(land->current_state_, code);
                return evaluate_operation(prod.item_.amount_, operation, atol(val.c_str()));
            }
            case Command::REGION_RESOURCE: {
                std::string code, name, plural;
                if (!gpApp->ResolveAliasItems(arg, code, name, plural))
                    code = arg;

                return evaluate_operation(land_control::get_resource(land->current_state_, code), operation, atol(val.c_str()));
            }
        }
        return false;
    }
}