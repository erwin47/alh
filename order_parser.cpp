#include "order_parser.h"
#include "ahapp.h"

#include <algorithm>
#include <sstream>


namespace orders
{
    std::unordered_map<std::string, orders::Type> types_mapping = {
        {"ADDRESS", orders::Type::O_ADDRESS},
        {"ADVANCE", orders::Type::O_ADVANCE},
        {"ARMOR", orders::Type::O_ARMOR},
        {"ASSASSINATE", orders::Type::O_ASSASSINATE},
        {"ATTACK", orders::Type::O_ATTACK},
        {"AUTOTAX", orders::Type::O_AUTOTAX},
        {"AVOID", orders::Type::O_AVOID},
        {"BEHIND", orders::Type::O_BEHIND},
        {"BUILD", orders::Type::O_BUILD},
        {"BUY", orders::Type::O_BUY},
        {"CAST", orders::Type::O_CAST},
        {"CLAIM", orders::Type::O_CLAIM},
        {"COMBAT", orders::Type::O_COMBAT},
        {"CONSUME", orders::Type::O_CONSUME},
        {"DECLARE", orders::Type::O_DECLARE},
        {"DESCRIBE", orders::Type::O_DESCRIBE},
        {"DESTROY", orders::Type::O_DESTROY},
        {"ENDFORM", orders::Type::O_ENDFORM},
        {"ENTER", orders::Type::O_ENTER},
        {"ENTERTAIN", orders::Type::O_ENTERTAIN},
        {"EVICT", orders::Type::O_EVICT},
        {"EXCHANGE", orders::Type::O_EXCHANGE},
        {"FACTION", orders::Type::O_FACTION},
        {"FIND", orders::Type::O_FIND},
        {"FORGET", orders::Type::O_FORGET},
        {"FORM", orders::Type::O_FORM},
        {"GIVE", orders::Type::O_GIVE},
        {"GIVEIF", orders::Type::O_GIVEIF},
        {"TAKE", orders::Type::O_TAKE},
        {"SEND", orders::Type::O_SEND},
        {"GUARD", orders::Type::O_GUARD},
        {"HOLD", orders::Type::O_HOLD},
        {"LEAVE", orders::Type::O_LEAVE},
        {"MOVE", orders::Type::O_MOVE},
        {"NAME", orders::Type::O_NAME},
        {"NOAID", orders::Type::O_NOAID},
        {"NOCROSS", orders::Type::O_NOCROSS},
        {"NOSPOILS", orders::Type::O_NOSPOILS},
        {"OPTION", orders::Type::O_OPTION},
        {"PASSWORD", orders::Type::O_PASSWORD},
        {"PILLAGE", orders::Type::O_PILLAGE},
        {"PREPARE", orders::Type::O_PREPARE},
        {"PRODUCE", orders::Type::O_PRODUCE},
        {"PROMOTE", orders::Type::O_PROMOTE},
        {"QUIT", orders::Type::O_QUIT},
        {"RESTART", orders::Type::O_RESTART},
        {"REVEAL", orders::Type::O_REVEAL},
        {"SAIL", orders::Type::O_SAIL},
        {"SELL", orders::Type::O_SELL},
        {"SHARE", orders::Type::O_SHARE},
        {"SHOW", orders::Type::O_SHOW},
        {"SPOILS", orders::Type::O_SPOILS},
        {"STEAL", orders::Type::O_STEAL},
        {"STUDY", orders::Type::O_STUDY},
        {"TAX", orders::Type::O_TAX},
        {"TEACH", orders::Type::O_TEACH},
        {"WEAPON", orders::Type::O_WEAPON},
        {"WITHDRAW", orders::Type::O_WITHDRAW},
        {"WORK", orders::Type::O_WORK},
        {"RECRUIT", orders::Type::O_RECRUIT},
        {"TYPE", orders::Type::O_TYPE},
        {"LABEL", orders::Type::O_LABEL},
// must be in this sequence! {"", orders::Type::O_ENDXXX == {"", orders::Type::O_XXX+1
        {"TURN", orders::Type::O_TURN},
        {"ENDTURN", orders::Type::O_ENDTURN},
        {"TEMPLATE", orders::Type::O_TEMPLATE},
        {"ENDTEMPLATE", orders::Type::O_ENDTEMPLATE},
        {"ALL", orders::Type::O_ALL},
        {"ENDALL", orders::Type::O_ENDALL},
    };

    namespace utils 
    {
        void parse_order_line(const std::string& line, std::vector<std::string>& res)
        {
            const char* begin = line.c_str();
            const char* end = begin + line.size();        
            while (!isalpha(*begin) && *begin != ';' && begin < end)
                ++begin;
            const char* runner = begin;

            while (runner < end)
            {
                switch(*runner) 
                {
                    case ';':
                        if (begin != runner)
                            res.emplace_back(std::string(begin, runner));
                        res.emplace_back(std::string(runner, end));
                        runner = end;
                        begin = end;
                        break;
                    case '"':
                        ++runner;
                        begin = runner;
                        while (*runner != '"' && runner < end)
                            ++runner;
                        res.emplace_back(std::string(begin, runner));
                        ++runner;
                        begin = runner;
                        break;
                    case ' ':
                        if (begin != runner)
                            res.emplace_back(std::string(begin, runner));
                        while (*runner == ' ' && runner < end)
                            ++runner;
                        begin = runner;
                        break;
                    default:
                        ++runner; 
                        break;
                }
            }
            if (begin != runner)
                res.emplace_back(std::string(begin, runner));
        }
    }

    namespace parser 
    {
        std::shared_ptr<Order> parse_line_to_order(const std::string& line)
        {
            std::shared_ptr<Order> res = std::make_shared<Order>();
            res->original_string_ = line;

            std::vector<std::string> words;
            utils::parse_order_line(line, words);

            for (auto& word : words)
            {
                if (word[0] == ';')
                {
                    res->comment_ = word;
                    break;
                }

                std::string code_name, name, plural_name;
                if (gpApp->ResolveAliasItems(word, code_name, name, plural_name))
                {
                    res->words_order_.push_back(code_name);
                    continue;
                }

                std::string alias_word = word;
                std::replace(alias_word.begin(), alias_word.end(), ' ', '_'); //magic rule of aliases
                word = gpApp->ResolveAlias(alias_word.c_str());
                //need additional message or word for case when there is no alias for the word.
                //at least check that word fits any of skills/items/number
                std::for_each(word.begin(), word.end(), [](char & c){
                c = ::toupper(c);
                });
                res->words_order_.push_back(word);
            }

            res->type_ = orders::Type::NORDERS;
            if (res->words_order_.size() == 0 && res->comment_.size() > 0)
                res->type_ = orders::Type::O_COMMENT;

            if (res->words_order_.size() > 0 && types_mapping.find(res->words_order_[0]) != types_mapping.end())
                res->type_ = types_mapping[res->words_order_[0]];
            return res;
        }

        UnitOrders parse_lines_to_orders(const std::string& orders)
        {
            UnitOrders res;
            const char* begin = orders.c_str();
            const char* end = begin + orders.size();
            const char* runner = begin;
            while(runner < end)
            {
                if (*runner == '\n')
                {
                    if (begin != runner)
                    {
                        res.orders_.emplace_back(parse_line_to_order(std::string(begin, runner)));
                        size_t pos = res.orders_.size() - 1;
                        res.hash_[res.orders_[pos]->type_].push_back(pos);
                    }
                    ++runner;
                    begin = runner;
                }
                else
                    ++runner;
            }
            if (begin != runner)
            {
                res.orders_.emplace_back(parse_line_to_order(std::string(begin, runner)));
                size_t pos = res.orders_.size() - 1;
                res.hash_[res.orders_[pos]->type_].push_back(pos);
            }
            return res;
        }

        std::string compose_original_lines(const UnitOrders& orders)
        {
            std::stringstream res;
            for (const auto& order : orders.orders_)
                res << order->original_string_ << std::endl;
            return res.str();
        }    
    }


    namespace control
    {
        void add_order_to_orders(std::shared_ptr<Order>& order, UnitOrders& unit_orders)
        {
            unit_orders.orders_.emplace_back(order);
            size_t pos = unit_orders.orders_.size() - 1;
            unit_orders.hash_[unit_orders.orders_[pos]->type_].push_back(pos);        
        }

        std::vector<std::shared_ptr<Order>> retrieve_orders_by_type(orders::Type type, const UnitOrders& unit_orders)
        {
            std::vector<std::shared_ptr<Order>> res;
            if (unit_orders.hash_.find(type) == unit_orders.hash_.end())
                return res;

            std::vector<size_t> ids = unit_orders.hash_.at(type);
            for (const auto& id : ids)
                res.push_back(unit_orders.orders_[id]);
            return res;
        }

        std::vector<long> get_students(CUnit* unit)
        {
            std::vector<long> ret;
            std::vector<std::shared_ptr<Order>> teaching_orders = retrieve_orders_by_type(orders::Type::O_TEACH, unit->orders_);
            for (const std::shared_ptr<Order>& ord : teaching_orders)
            {
                size_t i(1);
                while (i < ord->words_order_.size())
                {
                    if ((ord->words_order_[i] == "NEW" || ord->words_order_[i] == "new") &&
                        (i+1 < ord->words_order_.size()))
                    {
                        ret.push_back(NEW_UNIT_ID(atol(ord->words_order_[i+1].c_str()), unit->FactionId));
                        i += 2;
                    }
                    else if ((ord->words_order_[i] == "FACTION" || ord->words_order_[i] == "faction") &&
                             (i+3 < ord->words_order_.size()))
                    {
                        long faction_id = atol(ord->words_order_[i+1].c_str());
                        long unit_new_id = atol(ord->words_order_[i+3].c_str());
                        ret.push_back(NEW_UNIT_ID(unit_new_id, faction_id));
                        i += 4;
                    }
                    else
                    {
                        ret.push_back(atol(ord->words_order_[i].c_str()));
                        i += 1;
                    }                    
                }
            }
            return ret;
        }

        std::shared_ptr<Order> get_studying_order(const UnitOrders& unit_orders)
        {
            std::vector<std::shared_ptr<Order>> studying_orders = retrieve_orders_by_type(orders::Type::O_STUDY, unit_orders);
            if (studying_orders.size() > 0)
                return studying_orders[0];
            return nullptr;
        }
    }
};
