#include "order_parser.h"
#include "ahapp.h"

#include <algorithm>
#include <sstream>


namespace orders_parser
{
    std::unordered_map<std::string, OrderType> types_mapping = {
        {"ADDRESS", OrderType::O_ADDRESS},
        {"ADVANCE", OrderType::O_ADVANCE},
        {"ARMOR", OrderType::O_ARMOR},
        {"ASSASSINATE", OrderType::O_ASSASSINATE},
        {"ATTACK", OrderType::O_ATTACK},
        {"AUTOTAX", OrderType::O_AUTOTAX},
        {"AVOID", OrderType::O_AVOID},
        {"BEHIND", OrderType::O_BEHIND},
        {"BUILD", OrderType::O_BUILD},
        {"BUY", OrderType::O_BUY},
        {"CAST", OrderType::O_CAST},
        {"CLAIM", OrderType::O_CLAIM},
        {"COMBAT", OrderType::O_COMBAT},
        {"CONSUME", OrderType::O_CONSUME},
        {"DECLARE", OrderType::O_DECLARE},
        {"DESCRIBE", OrderType::O_DESCRIBE},
        {"DESTROY", OrderType::O_DESTROY},
        {"ENDFORM", OrderType::O_ENDFORM},
        {"ENTER", OrderType::O_ENTER},
        {"ENTERTAIN", OrderType::O_ENTERTAIN},
        {"EVICT", OrderType::O_EVICT},
        {"EXCHANGE", OrderType::O_EXCHANGE},
        {"FACTION", OrderType::O_FACTION},
        {"FIND", OrderType::O_FIND},
        {"FORGET", OrderType::O_FORGET},
        {"FORM", OrderType::O_FORM},
        {"GIVE", OrderType::O_GIVE},
        {"GIVEIF", OrderType::O_GIVEIF},
        {"TAKE", OrderType::O_TAKE},
        {"SEND", OrderType::O_SEND},
        {"GUARD", OrderType::O_GUARD},
        {"HOLD", OrderType::O_HOLD},
        {"LEAVE", OrderType::O_LEAVE},
        {"MOVE", OrderType::O_MOVE},
        {"NAME", OrderType::O_NAME},
        {"NOAID", OrderType::O_NOAID},
        {"NOCROSS", OrderType::O_NOCROSS},
        {"NOSPOILS", OrderType::O_NOSPOILS},
        {"OPTION", OrderType::O_OPTION},
        {"PASSWORD", OrderType::O_PASSWORD},
        {"PILLAGE", OrderType::O_PILLAGE},
        {"PREPARE", OrderType::O_PREPARE},
        {"PRODUCE", OrderType::O_PRODUCE},
        {"PROMOTE", OrderType::O_PROMOTE},
        {"QUIT", OrderType::O_QUIT},
        {"RESTART", OrderType::O_RESTART},
        {"REVEAL", OrderType::O_REVEAL},
        {"SAIL", OrderType::O_SAIL},
        {"SELL", OrderType::O_SELL},
        {"SHARE", OrderType::O_SHARE},
        {"SHOW", OrderType::O_SHOW},
        {"SPOILS", OrderType::O_SPOILS},
        {"STEAL", OrderType::O_STEAL},
        {"STUDY", OrderType::O_STUDY},
        {"TAX", OrderType::O_TAX},
        {"TEACH", OrderType::O_TEACH},
        {"WEAPON", OrderType::O_WEAPON},
        {"WITHDRAW", OrderType::O_WITHDRAW},
        {"WORK", OrderType::O_WORK},
        {"RECRUIT", OrderType::O_RECRUIT},
        {"TYPE", OrderType::O_TYPE},
        {"LABEL", OrderType::O_LABEL},
// must be in this sequence! {"", OrderType::O_ENDXXX == {"", OrderType::O_XXX+1
        {"TURN", OrderType::O_TURN},
        {"ENDTURN", OrderType::O_ENDTURN},
        {"TEMPLATE", OrderType::O_TEMPLATE},
        {"ENDTEMPLATE", OrderType::O_ENDTEMPLATE},
        {"ALL", OrderType::O_ALL},
        {"ENDALL", OrderType::O_ENDALL},
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

    Order get_order_from_line(const std::string& line)
    {
        Order res;
        res.original_string_ = line;

        std::vector<std::string> words;
        utils::parse_order_line(line, words);

        for (const auto& word : words)
        {
            if (word[0] == ';')
            {
                res.comment_ = word;
                break;
            }
            std::string code_name, name, plural_name;
            if (!gpApp->ResolveAliasItems(word, code_name, name, plural_name))
                code_name = word;
            std::for_each(code_name.begin(), code_name.end(), [](char & c){
             c = ::toupper(c);
            });
            res.words_order_.push_back(code_name);
        }

        res.type_ = OrderType::NORDERS;
        if (res.words_order_.size() == 0 && res.comment_.size() > 0)
            res.type_ = OrderType::O_COMMENT;

        if (res.words_order_.size() > 0 && types_mapping.find(res.words_order_[0]) != types_mapping.end())
            res.type_ = types_mapping[res.words_order_[0]];
        return res;
    }

    void add_order_to_unit_orders(const Order& order, UnitOrders& unit_orders)
    {
        unit_orders.orders_.emplace_back(order);
        size_t pos = unit_orders.orders_.size() - 1;
        unit_orders.hash_[unit_orders.orders_[pos].type_].push_back(pos);        
    }

    UnitOrders get_unit_orders_from_string(const std::string& orders)
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
                    res.orders_.emplace_back(get_order_from_line(std::string(begin, runner)));
                    size_t pos = res.orders_.size() - 1;
                    res.hash_[res.orders_[pos].type_].push_back(pos);
                }
                ++runner;
                begin = runner;
            }
            else
                ++runner;
        }
        return res;
    }

    std::string compose_original_orders(const UnitOrders& orders)
    {
        std::stringstream res;
        for (const auto& order : orders.orders_)
            res << order.original_string_ << std::endl;
        return res.str();
    }

    std::vector<Order> get_unit_orders_by_type(OrderType type, const UnitOrders& unit_orders)
    {
        std::vector<Order> res;
        if (unit_orders.hash_.find(type) == unit_orders.hash_.end())
            return res;

        std::vector<size_t> ids = unit_orders.hash_.at(type);
        std::transform(ids.begin(), ids.end(), res.begin(), 
            [&unit_orders](size_t i) -> Order { return unit_orders.orders_[i]; });
        return res;
    }


};
