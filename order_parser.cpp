#include "order_parser.h"
#include "ahapp.h"
#include "data_control.h"
#include "consts_ah.h"

#include <algorithm>
#include <sstream>
#include <functional>

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
        {"SPOILS", orders::Type::O_SPOILS},
        {"JOIN", orders::Type::O_JOIN},
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
        {"TRANSPORT", orders::Type::O_TRANSPORT},
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
//std::map< unsigned int, std::function<int(int,int)> > callbackMap;
    std::map<orders::Type, std::function<bool(const std::vector<std::string>&)> > sanity_checks = {
        {orders::Type::O_ADDRESS, [](const std::vector<std::string>&) {
            return true;
        } },
        {orders::Type::O_ADVANCE, [](const std::vector<std::string>&) {  
            return true;  
        } },
        {orders::Type::O_ARMOR, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_ASSASSINATE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_ATTACK, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_AUTOTAX, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_AVOID, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_BEHIND, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_BUILD, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_BUY, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_CAST, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_CLAIM, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_COMBAT, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_CONSUME, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_DECLARE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_DESCRIBE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_DESTROY, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_ENDFORM, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_ENTER, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_ENTERTAIN, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_EVICT, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_EXCHANGE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_FACTION, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_FIND, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_FORGET, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_FORM, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_GIVE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_GIVEIF, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_TAKE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_SEND, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_GUARD, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_HOLD, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_LEAVE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_MOVE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_NAME, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_NOAID, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_NOCROSS, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_SPOILS, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_JOIN, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_OPTION, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_PASSWORD, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_PILLAGE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_PREPARE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_PRODUCE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_PROMOTE, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_QUIT, [](const std::vector<std::string>&) {  return true;  } },
        {orders::Type::O_RESTART, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_REVEAL, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_SAIL, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_SELL, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_SHARE, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_SHOW, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_SPOILS, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_STEAL, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_STUDY, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_TAX, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_TEACH, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_WEAPON, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_WITHDRAW, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_WORK, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_RECRUIT, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_TYPE, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_LABEL, [](const std::vector<std::string>& words) {  return true;  } },
// must be in this sequence! {"", orders::Type::O_ENDXXX == {"", orders::Type::O_XXX+1
        {orders::Type::O_TURN, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_ENDTURN, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_TEMPLATE, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_ENDTEMPLATE, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_ALL, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_ENDALL, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_COMMENT, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_COMMENT_AUTONAME, [](const std::vector<std::string>& words) {  return true;  } },
        {orders::Type::O_ERROR, [](const std::vector<std::string>& words) {  return true;  } }
    };


    std::string& resolve_item_alias(std::string& word) 
    {
        std::string code_name, name, plural_name;
        if (gpApp->ResolveAliasItems(word, code_name, name, plural_name))
            word = code_name;
        return word;
    }

    std::string& resolve_skill_alias(std::string& word)
    {
        std::string alias_word = word;
        std::replace(alias_word.begin(), alias_word.end(), ' ', '_'); //magic rule of aliases
        word = gpApp->ResolveAlias(alias_word.c_str());
        return word;
    }

    std::map<orders::Type, std::function<void(std::vector<std::string>&)> > OrderAliases = {
        {orders::Type::O_ARMOR, [](std::vector<std::string>& words) {
            for (size_t i = 1; i < words.size(); ++i)//ARMOR [item1] [item2] ...
                words[i] = resolve_item_alias(words[i]);
        }},
        {orders::Type::O_BUILD, [](std::vector<std::string>& words) {  
            if (words.size() >= 2 && words[1] != "HELP") {//BUILD [object type] vs BUILD HELP [unit] 
                resolve_item_alias(words[1]);
            }
        }},
        {orders::Type::O_BUY, [](std::vector<std::string>& words) {  
            if (words.size() >= 3)//BUY [quantity] [item]  vs BUY ALL [item] 
                resolve_item_alias(words[2]);
        }},
        {orders::Type::O_CAST, [](std::vector<std::string>&) {  return;  } },//should we resolve a spell?
        {orders::Type::O_COMBAT, [](std::vector<std::string>&) {  return;  } },//should we resolve a spell?
        {orders::Type::O_EXCHANGE, [](std::vector<std::string>& words) {
            if (words.size() == 6)//EXCHANGE [unit] [quantity given] [item given] [quantity expected] [item expected] 
            {
                resolve_item_alias(words[3]);
                resolve_item_alias(words[5]);
            }            
        }},
        {orders::Type::O_FORGET, [](std::vector<std::string>& words) { 
            if (words.size() == 2)//FORGET [skill]
                resolve_skill_alias(words[1]);
        }},
        {orders::Type::O_GIVE, [](std::vector<std::string>& words) {
            if (words.size() > 3)//GIVE [unit] ALL [item] EXCEPT [quantity]
                resolve_item_alias(words[3]);
        }},
        {orders::Type::O_PREPARE, [](std::vector<std::string>& words) {
            if (words.size() > 1)//PREPARE [item]
                resolve_item_alias(words[1]);
        }},
        {orders::Type::O_PRODUCE, [](std::vector<std::string>& words) {  
            if (words.size() == 2)//PRODUCE [item]
                resolve_item_alias(words[1]);
            if (words.size() == 3)//PRODUCE [number] [item] 
                resolve_item_alias(words[2]);
        }},
        {orders::Type::O_SELL, [](std::vector<std::string>& words) {  
            if (words.size() == 3)//SELL [number] [item] 
                resolve_item_alias(words[2]);
        }},
        {orders::Type::O_SHOW, [](std::vector<std::string>& words) {  
            if (words.size() == 3)
            {
                if (words[1] == "SKILL")
                    resolve_skill_alias(words[2]);
                if (words[1] == "ITEM")
                    resolve_item_alias(words[2]);
                if (words[1] == "OBJECT")
                    resolve_skill_alias(words[2]);//skill?
            }
        }},
        {orders::Type::O_STEAL, [](std::vector<std::string>& words) {  
            if (words.size() == 3)//STEAL [unit] [item]
                resolve_item_alias(words[2]);
        }},
        {orders::Type::O_STUDY, [](std::vector<std::string>& words) {  
            if (words.size() >= 2)//STUDY [skill] [level] 
                resolve_skill_alias(words[1]);
        }},
        {orders::Type::O_TAKE, [](std::vector<std::string>& words) {  
            if (words.size() >= 5)//TAKE FROM [unit] ALL [item] EXCEPT [quantity]
                resolve_item_alias(words[4]);
        }},
        {orders::Type::O_TRANSPORT, [](std::vector<std::string>& words) {
            if (words.size() >= 4)//TRANSPORT [unit] ALL [item] EXCEPT [amount] 
                resolve_item_alias(words[3]);
        }},
        {orders::Type::O_WEAPON, [](std::vector<std::string>& words) {
            for (size_t i = 1; i < words.size(); ++i)//WEAPON [item] ... 
                resolve_item_alias(words[i]);
        }},
        {orders::Type::O_WITHDRAW, [](std::vector<std::string>& words) {  
            if (words.size() == 2)//WITHDRAW [item] 
                resolve_item_alias(words[1]);
            if (words.size() == 3)//WITHDRAW [quantity] [item]
                resolve_item_alias(words[2]);
        }},
    };

    namespace control 
    {
        bool is_order_type(const std::shared_ptr<Order>& order, const orders::Type& type)
        {
            return (order->type_ == type) || (order->type_ == (type | orders::Type::O_SUPRESS_ERRORS));
        }

        bool should_supress_error(const std::shared_ptr<Order>& order)
        {
            return ((order->type_ & orders::Type::O_SUPRESS_ERRORS) == orders::Type::O_SUPRESS_ERRORS);
        }

        /*bool ignore_order(const std::shared_ptr<Order>& order)
        {
            return order->comment_.size() == 4 && //to avoid other comments which fit ";$ne*"/";!ne*"
                    (strnicmp(order->comment_.c_str(), ";$ne", 4) == 0 ||
                    strnicmp(order->comment_.c_str(), ";!ne", 4) == 0);
        }*/


    }


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

        void add_order_to_orders(std::shared_ptr<Order>& order, UnitOrders& unit_orders)
        {
            if (order->type_ == orders::Type::O_TURN)
            {
                if (unit_orders.turn_endturn_started_)
                    unit_orders.errors_.append("Got TURN inside another TURN order\n");
                unit_orders.turn_endturn_started_ = true;
            }
                
            if (unit_orders.turn_endturn_started_)
            {
                unit_orders.turn_endturn_collection_.emplace_back(order);
            }
            else
            {
                unit_orders.orders_.emplace_back(order);
                size_t pos = unit_orders.orders_.size() - 1;
                
                //if ((unit_orders.orders_[pos]->type_ & orders::Type::O_COMMENT) != orders::Type::O_NORDERS &&
                //    unit_orders.orders_[pos]->type_ != orders::Type::O_COMMENT)
                //    unit_orders.hash_[orders::Type::O_COMMENT].push_back(pos);//additional record in hash
                unit_orders.hash_[unit_orders.orders_[pos]->type_].push_back(pos);        
            }

            if (order->type_ == orders::Type::O_ENDTURN)
            {
                if (!unit_orders.turn_endturn_started_)
                    unit_orders.errors_.append("Got ENDTURN without TURN order\n");
                unit_orders.turn_endturn_started_ = false;
            }               
        }        
    }

    namespace parser 
    {
        std::shared_ptr<Order> parse_line_to_order(const std::string& line)
        {
            std::shared_ptr<Order> res = std::make_shared<Order>();
            res->type_ = orders::Type::O_NORDERS;
            res->original_string_ = line;
            std::vector<std::string> words;

            //split to words
            utils::parse_order_line(line, words);
            if (words.empty())
                return nullptr;

            //sort out commends and words of order
            for (size_t i = 0; i < words.size(); ++i)
            {
                //force uppercase
                std::for_each(words[i].begin(), words[i].end(), [](char & c){
                c = ::toupper(c);
                });

                if (words[i].size() > 0 && words[i][0] == ';')
                {
                    res->comment_ = words[i];
                    break;
                }
                res->words_order_.push_back(words[i]);
            }

            if (res->words_order_.empty() && !res->comment_.empty())
            {//if comment
                if (res->comment_.find(";;") != std::string::npos && 
                    (res->comment_.find(" $C") != std::string::npos ||
                    res->comment_.find(" !C") != std::string::npos))
                    res->type_ = orders::Type::O_COMMENT_AUTONAME;
                else {
                    //find a subtype if exists (example: `;sell all ITEM`)
                    res->type_ = orders::Type::O_COMMENT;
                    if (res->comment_.size() > 1)
                    {
                        std::vector<std::string> commented_words;
                        utils::parse_order_line(res->comment_.substr(1), commented_words);
                        if (!commented_words.empty() && 
                             types_mapping.find(commented_words[0]) != types_mapping.end())
                        {
                            res->type_ = res->type_ | types_mapping[commented_words[0]];
                        }
                    }
                }                    
            }
            else if (!res->words_order_.empty() && 
                     types_mapping.find(res->words_order_[0]) != types_mapping.end())
            {//if known order
                res->type_ = types_mapping[res->words_order_[0]];
                
                //resolve aliases
                if (OrderAliases.find(res->type_) != OrderAliases.end())
                    OrderAliases[res->type_](res->words_order_);
            }            

            if (res->comment_.size() == 4 && //to avoid other comments which fit ";$ne*"/";!ne*"
                (strncmp(res->comment_.c_str(), ";$NE", 4) == 0 ||
                 strncmp(res->comment_.c_str(), ";!NE", 4) == 0)) 
            {
                res->type_ = res->type_ | orders::Type::O_SUPRESS_ERRORS;
                return res;
            }                

            if ((res->type_ & orders::Type::O_COMMENT) == orders::Type::O_NORDERS &&
                 sanity_checks.find(res->type_) == sanity_checks.end())
            {
                res->type_ = orders::Type::O_ERROR;
                res->comment_ = "Didn't find this order among known orders";
                return res;
            }
            if ((res->type_ & orders::Type::O_COMMENT) == orders::Type::O_NORDERS && 
                 !sanity_checks[res->type_](res->words_order_))
            {
                res->type_ = orders::Type::O_ERROR;
                res->comment_ = "Wrong format";
            }
            return res;
        }

        UnitOrders parse_lines_to_orders(const std::string& orders)
        {
            UnitOrders res;
            const char* begin = orders.c_str();
            const char* end = begin + orders.size();
            const char* runner = begin;
            int inside_turn_endturn = 0;
            while(runner < end)
            {
                if (*runner == '\n')
                {
                    if (begin != runner)
                    {
                        auto order = parse_line_to_order(std::string(begin, runner));
                        if (order != nullptr)
                            utils::add_order_to_orders(order, res);
                    }
                    ++runner;
                    begin = runner;
                }
                else
                    ++runner;
            }
            if (begin != runner)
            {
                auto order = parse_line_to_order(std::string(begin, runner));
                if (order != nullptr)
                    utils::add_order_to_orders(order, res);
            }
            return res;
        }

        std::string compose_string(const UnitOrders& orders)
        {
            std::stringstream res;
            for (const auto& order : orders.orders_)
                compose_string(res, order);
            for (const auto& order : orders.turn_endturn_collection_)
                compose_string(res, order);
            return res.str();
        }

        void recalculate_hash(UnitOrders& uorders)
        {
            uorders.hash_.clear();
            for (size_t i = 0; i < uorders.orders_.size(); ++i) {
                //if ((uorders.orders_[i]->type_ & orders::Type::O_COMMENT) != orders::Type::O_NORDERS &&
                //    uorders.orders_[i]->type_ != orders::Type::O_COMMENT)
                //    uorders.hash_[orders::Type::O_COMMENT].push_back(i);//additional record in hash
                uorders.hash_[uorders.orders_[i]->type_].push_back(i);
            }
                
        }

        namespace specific
        {
            //target_id < 0 -- new unit number
            //target_id == 0 -- giving out
            //target_faction_id = 0 -- current faction
            template<typename ITER>
            bool parse_unit_id(ITER& it_beg, ITER it_end, long& target_id, long& target_faction_id)
            {
                target_faction_id = 0;
                target_id = 0;
                if (it_beg >= it_end)
                    return false;

                if ((*it_beg).size() == 1 && (*it_beg)[0] == '0')
                {//target is giving out.
                    it_beg += 1;
                    return true;
                }

                if (stricmp((*it_beg).c_str(), "FACTION") == 0)
                {//other faction new unit
                    it_beg += 1;
                    if (it_beg >= it_end)
                        return false;
                    target_faction_id = atol((*it_beg).c_str());
                    if (target_faction_id <= 0)
                        return false;

                    it_beg += 1;
                    if (it_beg >= it_end || stricmp((*it_beg).c_str(), "NEW") != 0)
                        return false;

                    it_beg += 1;
                    if (it_beg >= it_end)
                        return false;
                    target_id = atol((*it_beg).c_str());
                    if (target_id <= 0)
                        return false;
                    target_id = -target_id;

                    it_beg += 1;
                    return true;
                }

                if (stricmp((*it_beg).c_str(), "NEW") == 0)
                {//local new unit
                    it_beg += 1;
                    if (it_beg >= it_end)
                        return false;
                    target_id = atol((*it_beg).c_str());
                    if (target_id <= 0)
                        return false;
                    target_id = -target_id;

                    it_beg += 1;
                    return true;
                }
                target_id = atol((*it_beg).c_str());
                if (target_id <= 0)
                    return false;                    
                it_beg += 1;
                return true;
            }

            bool parse_teaching(const std::shared_ptr<orders::Order>& order, long x, long y, long z, long faction_id, std::vector<long>& students)
            {
                std::vector<std::string>::iterator it_beg = order->words_order_.begin();
                if (it_beg == order->words_order_.end() || 
                        stricmp((*it_beg).c_str(), "TEACH") != 0)
                    return false;

                long target_id;
                long target_faction_id;
                it_beg +=1;
                while(it_beg != order->words_order_.end())
                {
                    if (!parse_unit_id(it_beg, order->words_order_.end(), target_id, target_faction_id))
                        return false;

                    if (target_id == 0)//no student with such number
                        return false;

                    if (target_id < 0)//new unit
                    {
                        if (target_faction_id == 0)
                            target_faction_id = faction_id;

                        students.push_back(NEW_UNIT_ID(x, y, z, target_faction_id, abs(target_id)));
                    } 
                    else
                    {
                        students.push_back(target_id);
                    }
                }
                return true;
            }

            bool parse_give(const std::shared_ptr<orders::Order>& order, long& target_id,
                long& target_faction_id, long& amount, std::string& item, long& except)
            {
                std::vector<std::string>::iterator it_beg = order->words_order_.begin();
                if (it_beg == order->words_order_.end() || 
                        stricmp((*it_beg).c_str(), "GIVE") != 0)
                    return false;

                it_beg +=1;
                if (!parse_unit_id(it_beg, order->words_order_.end(), target_id, target_faction_id))
                    return false;

                if (it_beg >= order->words_order_.end())
                    return false;

                if (stricmp("UNIT", (*it_beg).c_str()) == 0)
                {
                    amount = 0;
                    item = "UNIT";
                    return true;
                }

                if (it_beg >= order->words_order_.end())
                    return false;

                if (it_beg != order->words_order_.end() && stricmp("ALL", (*it_beg).c_str()) == 0)
                {
                    amount = 0;
                    except = 0;
                    it_beg += 1;
                } 
                else
                {
                    amount = atol((*it_beg).c_str());
                    except = -1;
                    it_beg += 1;
                }

                if (it_beg >= order->words_order_.end())
                    return false;

                std::string name, plural;
                if (!gpApp->ResolveAliasItems((*it_beg), item, name, plural))
                    item = (*it_beg);
                it_beg += 1;

                if (it_beg < order->words_order_.end())
                {
                    if (except == 0 && //ALL EXCEPT variant
                        stricmp("EXCEPT", (*it_beg).c_str()) == 0)
                    {
                        it_beg += 1;
                        if (it_beg >= order->words_order_.end())
                            return false;

                        except = atol((*it_beg).c_str());
                        if (except <= 0)
                            return false;
                        return true;
                    }
                    return false;//additional words without EXCEPT
                }
                return true;
            }

            bool parse_produce(const std::shared_ptr<orders::Order>& order, std::string& item, long& amount)
            {
                if (order->words_order_.size() == 0 || 
                        stricmp(order->words_order_[0].c_str(), "produce") != 0)
                    return false;

                if (order->words_order_.size() == 3)
                {
                    item = gpApp->ResolveAlias(order->words_order_[2].c_str());
                    amount = atol(order->words_order_[1].c_str());
                    return true;
                }
                else if (order->words_order_.size() == 2)
                {
                    item = gpApp->ResolveAlias(order->words_order_[1].c_str());
                    amount = -1;
                    return true;
                }
                return false;
            }

            bool parse_build(const std::shared_ptr<orders::Order>& order, std::string& building, bool& helps, long& unit_id)
            {
                if (order->words_order_.size() == 0 || 
                        stricmp(order->words_order_[0].c_str(), "build") != 0)
                    return false;

                if (order->words_order_.size() == 1) 
                {//build
                    building.clear();
                    helps = false;
                    unit_id = -1;
                    return true;
                }
                if (order->words_order_.size() == 2) 
                {//build TYPE
                    building = order->words_order_[1];
                    helps = false;
                    unit_id = -1;
                    return true;
                } 
                else if (order->words_order_.size() == 3 && 
                        strnicmp(order->words_order_[1].c_str(), "help", 4) == 0)
                {//build help unit_id
                    building.clear();
                    helps = true;
                    unit_id = atol(order->words_order_[2].c_str());
                    return true;
                }
                return false;              
            }
            bool parse_claim(const std::shared_ptr<orders::Order>& order, long& amount)
            {
                if (order->words_order_.size() == 0 || 
                        stricmp(order->words_order_[0].c_str(), "claim") != 0)
                    return false;

                if (order->words_order_.size() == 2)
                {
                    amount = atol(order->words_order_[1].c_str());
                    return true;
                }               
                return false; 
            }
            bool parse_study(const std::shared_ptr<orders::Order>& order, std::string& skill, long& level)
            {
                if (order->words_order_.size() == 0 || 
                        stricmp(order->words_order_[0].c_str(), "study") != 0)
                    return false;

                if (order->words_order_.size() == 2)
                {
                    
                    skill = gpApp->ResolveAlias(order->words_order_[1].c_str());
                    level = -1;
                    return true;
                }
                else if (order->words_order_.size() == 3)
                {
                    skill = gpApp->ResolveAlias(order->words_order_[1].c_str());
                    level = atol(order->words_order_[2].c_str());
                    return true;
                }             
                return false; 
            }
            bool parse_assassinate(const std::shared_ptr<orders::Order>& order, long& target_id)
            {
                if (order->words_order_.size() == 0 || 
                        stricmp(order->words_order_[0].c_str(), "assassinate") != 0)
                    return false;

                if (order->words_order_.size() == 2)
                {
                    target_id = atol(order->words_order_[1].c_str());
                    return true;
                }
                return false;
            }
            bool parse_attack(const std::shared_ptr<orders::Order>& order, std::vector<long>& targets)
            {
                if (order->words_order_.size() == 0 || 
                        stricmp(order->words_order_[0].c_str(), "attack") != 0)
                    return false;

                if (order->words_order_.size() < 2)
                    return false;

                for (size_t i = 1; i < order->words_order_.size(); ++i)
                {
                    long target_id = atol(order->words_order_[i].c_str());
                    if (target_id == 0)
                        return false;
                    else
                        targets.push_back(target_id);
                }                    
                return true;
            }
            bool parse_steal(const std::shared_ptr<orders::Order>& order, long& target_id, std::string& item)
            {
                if (order->words_order_.size() == 0 || 
                        stricmp(order->words_order_[0].c_str(), "steal") != 0)
                    return false;

                if (order->words_order_.size() == 3)
                {
                    target_id = atol(order->words_order_[1].c_str());
                    item = item_control::codename(order->words_order_[2]);
                    return true;
                }
                return false;
            }
            bool parse_sellbuy(const std::shared_ptr<orders::Order>& order, std::string& item, long& amount, bool& all)
            {
                if (order->words_order_.size() == 0 || 
                        (stricmp(order->words_order_[0].c_str(), "sell") != 0 &&
                         stricmp(order->words_order_[0].c_str(), "buy") != 0))
                    return false;                
                if (order->words_order_.size() == 3)
                {
                    if (strnicmp(order->words_order_[1].c_str(), "all", 3) == 0)
                    {
                        all = true;
                        amount = -1;
                    }
                    else
                    {
                        all = false;
                        amount = atol(order->words_order_[1].c_str());
                    }
                    item = item_control::codename(order->words_order_[2]);
                    return true;
                }
                return false;            
            }

            bool parse_flags(const std::shared_ptr<orders::Order>& order, bool& flag)
            {
                if (order->words_order_.size() == 2)
                {
                    flag = (atol(order->words_order_[1].c_str()) == 1);
                    return true;
                }
                return false;
            }

            bool parse_flags_with_param(const std::shared_ptr<orders::Order>& order, std::string& param)
            {
                if (order->words_order_.size() == 1)
                {
                    param.clear();
                    return true;
                }
                else if (order->words_order_.size() == 2)
                {
                    param = order->words_order_[1];
                    return true;
                }
                return false;
            }
        }        
    }


    namespace control
    {
        template<> 
        long flag_by_order_type<orders::Type::O_AUTOTAX>() {  return UNIT_FLAG_TAXING; }
        template<>
        long flag_by_order_type<orders::Type::O_AVOID>() {  return UNIT_FLAG_AVOIDING; }
        template<>
        long flag_by_order_type<orders::Type::O_BEHIND>() {  return UNIT_FLAG_BEHIND; }
        template<>
        long flag_by_order_type<orders::Type::O_GUARD>() {  return UNIT_FLAG_GUARDING; }
        template<>
        long flag_by_order_type<orders::Type::O_HOLD>() {  return UNIT_FLAG_HOLDING; }
        template<>
        long flag_by_order_type<orders::Type::O_NOAID>() {  return UNIT_FLAG_RECEIVING_NO_AID; }
        template<>
        long flag_by_order_type<orders::Type::O_NOCROSS>() {  return UNIT_FLAG_NO_CROSS_WATER; }
        template<>
        long flag_by_order_type<orders::Type::O_SHARE>() {  return UNIT_FLAG_SHARING; }


        bool modify_order(CUnit* unit, std::shared_ptr<Order>& order, const std::string& new_order_line)
        {
            auto new_order = orders::parser::parse_line_to_order(new_order_line);
            if (new_order != nullptr) {
                order->comment_ = new_order->comment_;
                order->type_ = new_order->type_;
                order->words_order_ = new_order->words_order_;
                order->original_string_ = new_order_line;

                unit->Orders.Empty();
                unit->Orders << orders::parser::compose_string(unit->orders_).c_str();
                parser::recalculate_hash(unit->orders_);
                gpApp->orders_changed(true);
                return true;
            }
            return false;
        }

        void remove_empty_lines(CUnit* unit)
        {
            std::string orders(unit->Orders.GetData(), unit->Orders.GetLength());
            orders.erase(std::unique(orders.begin(), orders.end(),
                      [] (char a, char b) {return a == '\n' && b == '\n';}),
            orders.end());
            unit->Orders = orders.c_str();
        }

        void add_order_to_unit(std::shared_ptr<Order>& order, CUnit* unit)
        {
            utils::add_order_to_orders(order, unit->orders_);
            if (unit->Orders.GetLength() > 0)
                unit->Orders << EOL_SCR;                
            parser::compose_string(unit->Orders, order);
            gpApp->orders_changed(true);
            remove_empty_lines(unit);
        }
        
        void add_autoorder_to_unit(std::shared_ptr<Order>& order, CUnit* unit)
        {
            /*std::vector<std::shared_ptr<Order>> orders = retrieve_orders_by_type(order->type_, unit->orders_);
            for (const auto& ord: orders)
            {
                if (order->words_order_.size() != ord->words_order_.size())
                    continue;

                size_t i = 0;
                for (; i < order->words_order_.size(); ++i)
                    if (order->words_order_[i] != ord->words_order_[i])
                        break;
                if (i == order->words_order_.size())
                {//unit already have exactly the same order
                    return;
                }
            }*/
            add_order_to_unit(order, unit);
        }

        void add_order_to_unit(std::string order_line, CUnit* unit)
        {
            std::shared_ptr<orders::Order> order = orders::parser::parse_line_to_order(order_line);
            if (order != nullptr) {
                add_order_to_unit(order, unit);
            }                
        }

        std::vector<std::shared_ptr<Order>> retrieve_orders_by_type(orders::Type type, const UnitOrders& unit_orders)
        {
            if (!has_orders_with_type(type, unit_orders))
                return {};

            std::vector<std::shared_ptr<Order>> res;
            if (unit_orders.hash_.find(type) != unit_orders.hash_.end()) {
                std::vector<size_t> ids = unit_orders.hash_.at(type);
                for (const auto& id : ids) {
                    res.push_back(unit_orders.orders_[id]);
                }
            }

            if (unit_orders.hash_.find(type | orders::Type::O_SUPRESS_ERRORS) != unit_orders.hash_.end()) {
                std::vector<size_t> ids_supressed = unit_orders.hash_.at(type | orders::Type::O_SUPRESS_ERRORS);    
                for (const auto& id : ids_supressed) {
                    res.push_back(unit_orders.orders_[id]);
                }            
            }
            return res;
        }

        bool has_orders_with_type(orders::Type type, const UnitOrders& unit_orders)
        {
            //check for the type or the type with supress_flag
            if (unit_orders.hash_.find(type) != unit_orders.hash_.end() || 
                unit_orders.hash_.find(type | orders::Type::O_SUPRESS_ERRORS) != unit_orders.hash_.end())
                return true;
            return false;
        }

        void remove_orders_by_comment(CUnit* unit, const std::string& pattern)
        {
            unit->orders_.orders_.erase(std::remove_if(unit->orders_.orders_.begin(), 
                                                     unit->orders_.orders_.end(), 
                                                       [&pattern](std::shared_ptr<Order> order) {
                return order->comment_.find(pattern) != std::string::npos;
            }), unit->orders_.orders_.end());            
            parser::recalculate_hash(unit->orders_);

            unit->Orders.Empty();
            unit->Orders << orders::parser::compose_string(unit->orders_).c_str();            
        }

        void comment_order_out(std::shared_ptr<Order>& order, CUnit* unit)
        {
            if (strnicmp(order->original_string_.c_str(), "@;", 2) == 0 ||
                strnicmp(order->original_string_.c_str(), ";", 1) == 0)
                return;

            std::string new_order_line = order->original_string_;
            if (new_order_line.size() > 0 && new_order_line[0] == '@')
                new_order_line.insert(1, ";");
            else 
                new_order_line.insert(0, ";");

            orders::control::modify_order(unit, order, new_order_line);
        }

        void uncomment_order(std::shared_ptr<Order>& order, CUnit* unit)
        {
            if (strnicmp(order->original_string_.c_str(), "@;", 2) != 0 &&
                strnicmp(order->original_string_.c_str(), ";", 1) != 0)
                return;

            std::string new_order_line = order->original_string_;
            if (new_order_line.size() > 0 && new_order_line[0] == '@')
                new_order_line.erase(1, 1);
            else 
                new_order_line.erase(0, 1);

            orders::control::modify_order(unit, order, new_order_line);
        }

        std::shared_ptr<Order> compose_give_order(CUnit* target, long amount, const std::string& item, const std::string& comment, bool repeating)
        {
            std::shared_ptr<Order> res = std::make_shared<Order>();
            res->type_ = orders::Type::O_GIVE;
            if (repeating)
                res->words_order_.emplace_back("@GIVE");
            else
                res->words_order_.emplace_back("GIVE");
            if (IS_NEW_UNIT(target))
            {
                res->words_order_.emplace_back("NEW");
                res->words_order_.emplace_back(std::to_string((long)REVERSE_NEW_UNIT_ID(target->Id)));
            }
            else
                res->words_order_.emplace_back(std::to_string(target->Id));
            res->words_order_.emplace_back(std::to_string(amount));
            res->words_order_.emplace_back(item);
            if (comment.size() > 0)
                res->comment_ = comment;
            else
                res->comment_ = ";auto";
            return res;
        }

        std::shared_ptr<Order> get_studying_order(const UnitOrders& unit_orders)
        {
            if (!has_orders_with_type(orders::Type::O_STUDY, unit_orders))
                return nullptr;

            std::vector<std::shared_ptr<Order>> studying_orders = retrieve_orders_by_type(orders::Type::O_STUDY, unit_orders);
            return studying_orders[0];
        }
    }

    namespace autoorders 
    {
        AO_TYPES has_autoorders(const std::shared_ptr<Order>& order)
        {
            if (order->comment_.find(";$GET") != std::string::npos ||
                order->comment_.find(";!GET") != std::string::npos)
                return AO_TYPES::AO_GET;

            if (order->comment_.find(";$COND") != std::string::npos ||
                order->comment_.find(";!COND") != std::string::npos ||
                order->comment_.find(";$ERR") != std::string::npos ||
                order->comment_.find(";!ERR") != std::string::npos ||
                order->comment_.find(";$WARN") != std::string::npos ||
                order->comment_.find(";!WARN") != std::string::npos)
                return AO_TYPES::AO_CONDITION;

            if (order->comment_.find(";$OWNER") != std::string::npos ||
                order->comment_.find(";!OWNER") != std::string::npos)
                return AO_TYPES::AO_OWNER;

            if (order->comment_.find(";$HELP") != std::string::npos ||
                order->comment_.find(";!HELP") != std::string::npos)
                return AO_TYPES::AO_HELP;

            return AO_TYPES::AO_NONE;
        }

        bool parse_get(const std::shared_ptr<Order>& order, long& amount, std::string& item)
        {
            std::vector<std::string> words;
            orders::utils::parse_order_line(order->comment_.substr(2), words);
            if (words.size() != 3)
                return false;

            if (stricmp(words[0].c_str(), "get") != 0)
                return false;

            amount = atol(words[1].c_str());
            if (amount == 0 && words[1].size() != 1 && words[1][0] != '0')
                return false;

            std::string codename, name, plural;
            if (gpApp->ResolveAliasItems(words[2], codename, name, plural))
                item = codename;
            else
                item = words[2];
            return true;
        }

        bool parse_logic(const std::shared_ptr<Order>& order, LogicAction& action, std::string& statement, bool& debug)
        {
            debug = false;
            size_t pos = std::min(order->comment_.find("$COND"), order->comment_.find("!COND"));
            if (pos != std::string::npos)
            {
                if (std::min(order->comment_.find("$COND_D"), order->comment_.find("!COND_D")) != std::string::npos)
                {
                    debug = true;
                    statement = order->comment_.substr(pos+sizeof("$COND_D"));
                }
                else
                {
                    statement = order->comment_.substr(pos+sizeof("$COND"));
                }                   
                action = LogicAction::SWITCH_COMMENT;
                return true;
            }

            pos = std::min(order->comment_.find("$WARN"), order->comment_.find("!WARN"));
            if (pos != std::string::npos)
            {
                if (std::min(order->comment_.find("$WARN_D"), order->comment_.find("!WARN_D")) != std::string::npos)
                {
                    debug = true;
                    statement = order->comment_.substr(pos+sizeof("$COND_D"));
                }
                else
                {
                    statement = order->comment_.substr(pos+sizeof("$COND"));
                }   
                action = LogicAction::LOGIC_ERROR;
                return true;
            }
            return false;
        }
    }

    namespace autoorders_caravan
    {
        namespace utils 
        {
            bool is_region_in_caravan_list(std::shared_ptr<orders::CaravanInfo>& caravan_info, CLand* land)
            {
                int x, y, z;
                LandIdToCoord(land->Id, x, y, z);
                return std::find_if(caravan_info->regions_.begin(), 
                                    caravan_info->regions_.end(), 
                                    [&](RegionInfo& reginfo) {
                                        return x == reginfo.x_ && y == reginfo.y_ &&
                                            z == reginfo.z_;
                                    }) != caravan_info->regions_.end();
            }

            bool is_route_same(const std::shared_ptr<CaravanInfo>& caravan_info1, 
                            const std::shared_ptr<CaravanInfo>& caravan_info2)
            {
                if (caravan_info1->regions_.size() != caravan_info2->regions_.size())
                    return false;

                bool ret = true;
                for (const orders::RegionInfo& rinfo1 : caravan_info1->regions_)
                {
                    bool match = false;
                    for (const orders::RegionInfo& rinfo2 : caravan_info2->regions_)
                    {
                        if (rinfo1.x_ == rinfo2.x_ && rinfo1.y_ == rinfo2.y_ && rinfo1.z_ == rinfo2.z_)
                        {
                            match = true;
                            break;
                        }
                    }
                    ret = ret && match;
                    if (!ret)
                        break;
                }
                return ret;
            }

            RegionInfo region_info_from_coordinates(const std::vector<long>& numbers)
            {
                RegionInfo ret;
                ret.x_ = numbers[0];
                ret.y_ = numbers[1];
                if (numbers.size() == 3)
                    ret.z_ = numbers[2];
                else
                    ret.z_ = land_control::get_plane_id(DEFAULT_PLANE);
                return ret;
            }            
        }

        namespace parser 
        {

            namespace utils 
            {
                const char* get_amount(const char* begin, const char* end, long& amount)
                {
                    //get to next word            
                    while(begin < end && *begin == ' ')
                        ++begin;
                    if (begin >= end)
                        return begin;

                    //extract amount
                    const char* runner = strchr(begin, ' ');
                    if (!memcmp(begin, "all", runner - begin) || !memcmp(begin, "ALL", runner - begin))
                        amount = -1;
                    else
                        amount = atol(begin);
                    return runner;
                }
                //type: P Y
                void get_priority(const char* begin, const char* end, long& priority)
                {
                    //get to next word            
                    while(begin < end && *begin == ' ')
                        ++begin;
                    if (begin >= end)
                        return;

                    //priority
                    if ((begin[0] == 'P') || (begin[0] == 'p'))
                    {
                        while(begin < end && !isdigit(*begin) && *begin != '-')
                            ++begin;

                        if (begin < end)
                            priority = atol(begin);
                    }            
                }

                //type: X ITEM P Y
                //! amount = -1 -- means request for as many as possible
                void get_demand(const char* begin, const char* end, std::string& type, long& amount, long& priority)
                {
                    const char* runner;
                    begin = get_amount(begin, end, amount);
                    //get to next word
                    while(begin < end && *begin == ' ')
                        ++begin;
                    if (begin >= end)
                        return;

                    //get item name
                    if (*begin == '"')
                    {
                        ++begin;
                        runner = strchr(begin, '"');
                        if (runner == nullptr)
                            runner = end;
                        type = std::string(begin, runner);
                        begin = runner+1;
                    }
                    else
                    {
                        runner = strchr(begin, ' ');
                        if (runner == nullptr)
                            runner = end;
                        type = std::string(begin, runner);
                        begin = runner;
                    }

                    get_priority(begin, end, priority);
                }

                //type: ITEM ITEM... P Y
                void get_item_array(const char* begin, const char* end, std::vector<std::string>& items, long& priority)
                {
                    std::string item;
                    const char* runner;
                    do {
                        //get to next word
                        while(begin < end && *begin == ' ')
                            ++begin;
                        if (begin >= end)
                            return;

                        //get item name
                        if (*begin == '"')
                        {
                            ++begin;
                            runner = strchr(begin, '"');
                            if (runner == nullptr)
                                runner = end;
                            item = std::string(begin, runner);
                            begin = runner+1;
                        }
                        else
                        {
                            runner = strchr(begin, ' ');
                            if (runner == nullptr)
                                runner = end;
                            item = std::string(begin, runner);
                            begin = runner;
                        }

                        if (item != "P" && item != "p")
                            items.push_back(item);
                        else
                        {
                            while(begin < end && !isdigit(*begin) && *begin != '-')
                                ++begin;

                            if (begin < end)
                                priority = atol(begin);
                            break;
                        }                                           
                    } while(true);
                }
    
                void add_source(CUnit* unit, const std::string& codename, long sharing_border, long priority, std::vector<AutoSource>& sources)
                {
                    if (sharing_border == -1)//no need to calculate amount, because -1 means "ALL"
                        sharing_border = 0;
                    if (sharing_border >= 0)
                    {
                        long to_share = unit_control::get_item_amount(unit, codename) - sharing_border;
                        if (to_share > 0)
                            sources.emplace_back(AutoSource{codename, to_share, priority, unit});
                    }            
                }

                void add_need(CUnit* unit, const std::string& codename, long unit_req, long priority, std::vector<AutoRequirement>& needs)
                {
                    if (unit_req == -1)//no need to calculate amount, because -1 means "ALL"
                        needs.emplace_back(AutoRequirement{codename, -1, priority, unit});
                    else if (unit_req >= 0)
                    {
                        long to_request = unit_req - unit_control::get_item_amount(unit, codename);
                        if (to_request > 0)
                            needs.emplace_back(AutoRequirement{codename, to_request, priority, unit});
                    }            
                }

                void add_need_mount(CUnit* unit, const std::string& codename, long unit_req, long priority, std::vector<AutoRequirement>& needs)
                {
                    if (unit_req == -1)//no need to calculate amount, because -1 means "ALL"
                        needs.emplace_back(AutoRequirement{codename, -1, priority, unit});
                    else if (unit_req >= 0)
                    {
                        long to_request = unit_req - unit_control::get_item_amount(unit, codename);
                        if (to_request > 0)
                            needs.emplace_back(AutoRequirement{codename, to_request, priority, unit});
                    }            
                }

                void add_needreg(CUnit* unit, const std::string& codename, long unit_req, long priority, std::vector<AutoRequirement>& needs)
                {
                    if (unit_req == -1)//no need to calculate amount, because -1 means "ALL"
                        needs.emplace_back(AutoRequirement{codename, -1, priority, unit});
                    else if (unit_req >= 0)
                    {
                        CLand* land = land_control::get_land(unit->LandId);
                        if (land != nullptr)
                        {
                            long existing_amount(0);
                            land_control::perform_on_each_unit(land, [&](CUnit* cur_unit) {
                                if (cur_unit->IsOurs && cur_unit->caravan_info_ == nullptr)
                                    existing_amount += unit_control::get_item_amount(cur_unit, codename);
                            }); 
                            long to_request = unit_req - existing_amount;
                            if (to_request > 0)
                                needs.emplace_back(AutoRequirement{codename, to_request, priority, unit});
                        }
                    }            
                }

                void extract_items(const std::string& item_name, std::vector<std::string>& items)
                {
                    items = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, item_name.c_str());
                    if (items.size() > 0)
                        return;

                    std::string codename, name, plural;
                    if (gpApp->ResolveAliasItems(item_name, codename, name, plural))
                        items.push_back(codename);
                    else //just in case of unknown & unresolved item, we still want to process it
                        items.push_back(item_name);
                }
            }



            void get_unit_sources_and_needs(CUnit* unit, 
                                            std::vector<AutoSource>& sources,
                                            std::vector<AutoRequirement>& needs)
            {
                std::set<std::string> specified_items;
                long store_all_priority = -1;

                std::vector<std::shared_ptr<Order>> orders = control::retrieve_orders_by_type(orders::Type::O_COMMENT, unit->orders_);
                for (const auto& order : orders)
                {
                    long auto_amount;
                    std::string item_type;
                    std::vector<std::string> items;

                    if (order->comment_.find(";!SOURCE") != std::string::npos || order->comment_.find(";$SOURCE") != std::string::npos)
                    {
                        long priority(-1);
                        const char* runner = order->comment_.c_str() + sizeof(";!SOURCE") - 1;
                        const char* end = order->comment_.c_str() + order->comment_.size();

                        std::vector<std::string> listed_items;
                        runner = utils::get_amount(runner, end, auto_amount);
                        utils::get_item_array(runner, end, listed_items, priority);
                        for (const auto& listed_item : listed_items) 
                        {
                            std::vector<std::string> temp_items;
                            utils::extract_items(listed_item, temp_items);
                            items.insert(items.end(), temp_items.begin(), temp_items.end());
                        }
                        for (const auto& item : items)
                        {
                            specified_items.insert(item);
                            utils::add_source(unit, item, auto_amount, priority, sources);
                        }
                    }
                    else if (order->comment_.find(";!NEEDREG") != std::string::npos || order->comment_.find(";$NEEDREG") != std::string::npos)
                    {
                        long priority(20);
                        const char* runner = order->comment_.c_str() + sizeof(";!NEEDREG") - 1;
                        const char* end = order->comment_.c_str() + order->comment_.size();
                        utils::get_demand(runner, end, item_type, auto_amount, priority);
                        if (auto_amount <= 0)
                        {
                            unit_control::order_message(unit, "Incorrect use of NEEDREG:", "it has amount <= 0");
                        }
                        else 
                        {
                            utils::extract_items(item_type, items);
                            for (const auto& item : items)
                            {
                                specified_items.insert(item);
                                utils::add_needreg(unit, item, auto_amount, priority, needs);
                            }
                        }
                    }
                    else if (order->comment_.find(";!NEED") != std::string::npos || order->comment_.find(";$NEED") != std::string::npos)
                    {
                        long priority(10);
                        const char* runner = order->comment_.c_str() + sizeof(";!NEED") - 1;
                        const char* end = order->comment_.c_str() + order->comment_.size();

                        std::vector<std::string> listed_items;
                        runner = utils::get_amount(runner, end, auto_amount);
                        utils::get_item_array(runner, end, listed_items, priority);
                        for (const auto& listed_item : listed_items) 
                        {
                            std::vector<std::string> temp_items;
                            utils::extract_items(listed_item, temp_items);
                            items.insert(items.end(), temp_items.begin(), temp_items.end());
                        }

                        for (const auto& item : items)
                        {
                            specified_items.insert(item);
                            utils::add_need(unit, item, auto_amount, priority, needs);
                        }
                    }                
                    else if (order->comment_.find(";!STORE_ALL") != std::string::npos || order->comment_.find(";$STORE_ALL") != std::string::npos)
                    {
                        if (store_all_priority > 0)
                            unit_control::order_message(unit, "Incorrect use of STORE_ALL:", "used twice in one unit");

                        const char* runner = order->comment_.c_str() + sizeof(";!STORE_ALL") - 1;
                        const char* end = order->comment_.c_str() + order->comment_.size();
                        utils::get_priority(runner, end, store_all_priority);
                        if (store_all_priority <= 0)
                            unit_control::order_message(unit, "Incorrect use of STORE_ALL:", "priority have to be set and positive");
                    }
                    else if (order->comment_.find(";!STORE") != std::string::npos || order->comment_.find(";$STORE") != std::string::npos)
                    {
                        long priority(-1);
                        const char* runner = order->comment_.c_str() + sizeof(";!STORE") - 1;
                        const char* end = order->comment_.c_str() + order->comment_.size();

                        std::vector<std::string> listed_items;
                        runner = utils::get_amount(runner, end, auto_amount);
                        utils::get_item_array(runner, end, listed_items, priority);
                        for (const auto& listed_item : listed_items) 
                        {
                            std::vector<std::string> temp_items;
                            utils::extract_items(listed_item, temp_items);
                            items.insert(items.end(), temp_items.begin(), temp_items.end());
                        }

                        for (const auto& item : items)
                        {
                            specified_items.insert(item);
                            if (priority == -1)
                            {
                                utils::add_source(unit, item, auto_amount, -1, sources);                        
                                utils::add_need(unit, item, -1, 10, needs);
                            }
                            else 
                            {
                                utils::add_source(unit, item, auto_amount, priority, sources);                        
                                utils::add_need(unit, item, -1, priority, needs);
                            }
                        }  
                    }
                    else if (order->comment_.find(";!EQUIP") != std::string::npos || order->comment_.find(";$EQUIP") != std::string::npos)
                    {
                        long priority(-1);
                        const char* runner = order->comment_.c_str() + sizeof(";!EQUIP") - 1;
                        const char* end = order->comment_.c_str() + order->comment_.size();
                        std::vector<std::string> listed_items;
                        std::vector<std::string> temp_items;
                        utils::get_item_array(runner, end, listed_items, priority);
                        for (const auto& listed_item : listed_items) 
                        {
                            utils::extract_items(listed_item, temp_items);
                            items.insert(items.end(), temp_items.begin(), temp_items.end());
                        }

                        auto_amount = unit_control::get_item_amount_by_mask(unit, PRP_MEN);
                        for (auto& item : items)
                        {
                            specified_items.insert(item);
                            if (priority == -1)
                            {
                                utils::add_source(unit, item, auto_amount, -1, sources);
                                utils::add_need(unit, item, auto_amount, 10, needs);
                            }
                            else
                            {
                                utils::add_source(unit, item, auto_amount, -1, sources);
                                utils::add_need(unit, item, auto_amount, priority, needs);
                            }
                        }
                    }
                }

                //STORE_ALL COMMANDS
                if (store_all_priority > 0)
                {
                    std::set<CItem> items_to_giveout = unit->items_initial_;

                    //clean items which already have specific autoorder
                    for (const auto& specified_item : specified_items)
                        items_to_giveout.erase({0, specified_item});

                    //for each item to give: source order
                    for (const CItem& item : items_to_giveout)
                    {
                        if (item.amount_ > 0)
                            utils::add_source(unit, item.code_name_, 0, store_all_priority, sources);
                    }
                    //add specific NEED ALL_ITEMS, will be handled later
                    utils::add_need(unit, "ALL_ITEMS", -1, store_all_priority, needs);
                }

                //CARAVAN RELATED SOURCES
                if (unit->caravan_info_ != nullptr)
                {
                    std::set<CItem> items_to_giveout = unit_control::get_all_items(unit);

                    //clean items which already have specific autoorder
                    for (const auto& specified_item : specified_items)
                        items_to_giveout.erase({0, specified_item});

                    //for each item to give: source order
                    for (const CItem& item : items_to_giveout)
                    {
                        if (item.amount_ > 0)
                            utils::add_source(unit, item.code_name_, 0, -1, sources);
                    }   
                }
            }  

        }

        bool is_caravan(const UnitOrders& unit_orders)
        {
            auto orders = control::retrieve_orders_by_type(orders::Type::O_COMMENT, unit_orders);
            for (const auto& order : orders)
            {
                if (order->comment_.find(";!CARAVAN") != std::string::npos || order->comment_.find(";$CARAVAN") != std::string::npos)
                    return true;
            }
            return false;
        }

        std::shared_ptr<CaravanInfo> get_caravan_info(UnitOrders& unit_orders)
        {
            //CaravanInfo retCI = std::make_shared<CaravanInfo>();
            CaravanSpeed speed;
            std::vector<RegionInfo> regions;
            CLand* goal_region = nullptr;
            auto order_comments = orders::control::retrieve_orders_by_type(orders::Type::O_COMMENT, unit_orders);
            for (const auto& order : order_comments)
            {
                if (order->comment_.find(";!CARAVAN") != std::string::npos || order->comment_.find(";$CARAVAN") != std::string::npos)
                {
                    const char* runner = order->comment_.c_str() + sizeof(";!CARAVAN") - 1;
                    const char* end = order->comment_.c_str() + order->comment_.size();
                    while (runner < end && *runner == ' ')
                        ++runner;

                    if (runner < end)
                    {
                        switch(*runner)
                        {
                            case 'W':
                            case 'M': speed = CaravanSpeed::MOVE; break;
                            case 'R': speed = CaravanSpeed::RIDE; break;
                            case 'F': speed = CaravanSpeed::FLY; break;
                            case 'S': {
                                    const char* next = runner + 1;
                                    if (next < end && *next == 'W')
                                        speed = CaravanSpeed::SWIM;
                                    else
                                        speed = CaravanSpeed::SAIL; 
                                }
                                break;
                            default: speed = CaravanSpeed::UNDEFINED; break;
                        }
                    }
                }

                if (order->comment_.find(";!REGION") != std::string::npos || order->comment_.find(";$REGION") != std::string::npos)
                {
                    const char* runner = order->comment_.c_str() + sizeof(";!REGION") - 1;
                    const char* end = order->comment_.c_str() + order->comment_.size();
                    std::vector<long> numbers;
                    while (runner < end)
                    {
                        if (isdigit(*runner))
                        {
                            numbers.push_back(atol(runner));
                            while (runner < end && isdigit(*runner))
                                ++runner;
                        }
                        if (numbers.size() > 1 && *runner != ',')
                        {
                            regions.emplace_back(utils::region_info_from_coordinates(numbers));
                            numbers.clear();
                        }
                        ++runner;
                    }
                    if (numbers.size() > 1)
                    {
                        regions.emplace_back(utils::region_info_from_coordinates(numbers));
                    }
                }
                
                if (order->comment_.find(";!CUR_REG") != std::string::npos || order->comment_.find(";$CUR_REG") != std::string::npos)
                {
                    const char* runner = order->comment_.c_str() + sizeof(";!CUR_REG") - 1;
                    const char* end = order->comment_.c_str() + order->comment_.size();
                    std::vector<long> numbers;
                    while (runner < end)
                    {
                        if (isdigit(*runner))
                        {
                            numbers.push_back(atol(runner));
                            while (runner < end && isdigit(*runner))
                                ++runner;
                        }
                        if (numbers.size() > 1 && *runner != ',')
                            break;
                        ++runner;
                    }
                    if (numbers.size() > 1)
                    {
                        RegionInfo goal_reg_info = utils::region_info_from_coordinates(numbers);
                        goal_region = land_control::get_land(goal_reg_info.x_,
                                                             goal_reg_info.y_,
                                                             goal_reg_info.z_);
                    }
                }
            }
            return std::make_shared<CaravanInfo>(speed, std::move(regions), goal_region);
        }        

        std::unordered_map<std::string, std::vector<long>> create_source_table(const std::vector<AutoSource>& sources)
        {
            std::unordered_map<std::string, std::vector<long>> ret;
            for (size_t i = 0; i < sources.size(); ++i)
                ret[sources[i].name_].push_back(i);

            return ret;
        }   

        void get_land_autosources_and_autoneeds(CLand* land, 
                                                std::vector<orders::AutoSource>& sources,
                                                std::vector<orders::AutoRequirement>& needs)
        {
            std::vector<orders::AutoRequirement> cur_needs;
            land_control::perform_on_each_unit(land, [&](CUnit* unit) {
                if (unit->IsOurs && unit->caravan_info_ == nullptr)
                {
                    cur_needs.clear();
                    parser::get_unit_sources_and_needs(unit, sources, cur_needs);
                    for (auto& need : cur_needs)
                    {
                        int x, y, z;
                        LandIdToCoord(land->Id, x, y, z);
                        std::string unit_name = unit_control::compose_unit_name(unit);
                        std::stringstream ss;
                        ss << "Reg[" << x << "," << y << "," << z << "]" 
                            << "[" << need.amount_ << "p" << need.priority_ << "]";
                        
                        need.description_ = std::make_shared<std::string>(ss.str());
                    }
                    if (cur_needs.size() > 0)
                        needs.insert(needs.end(), cur_needs.begin(), cur_needs.end());                                       
                }
            });            
        }

        void get_land_caravan_autosources_and_autoneeds(CLand* land, 
                                                        std::vector<orders::AutoSource>& sources,
                                                        std::vector<orders::AutoRequirement>& needs)
        {
            land_control::perform_on_each_unit(land, [&](CUnit* unit) {

                if (unit->IsOurs && unit->caravan_info_ != nullptr && 
                    utils::is_region_in_caravan_list(unit->caravan_info_, land))
                {
                    for (const auto& region : unit->caravan_info_->regions_)
                    {
                        CLand* farland = land_control::get_land(region.x_, region.y_, region.z_);
                        if (farland == NULL)
                        {
                            std::stringstream ss;
                            ss << "("<< region.x_ << ", " << region.y_ << ", " << region.z_ << ")";
                            unit_control::order_message(unit, "Didn't find region", ss.str().c_str());
                            continue;//throw? return error
                        }  

                        if (farland == land)
                        {//we should collect source/request items if caravan in the region listed in the REGION list
                            parser::get_unit_sources_and_needs(unit, sources, needs);
                        }
                        else 
                        {//we should collect requests from farlands listed in the REGION list
                            std::vector<orders::AutoSource> farland_sources;
                            std::vector<orders::AutoRequirement> farland_needs;
                            orders::autoorders_caravan::get_land_autosources_and_autoneeds(farland, 
                                                                                           farland_sources,
                                                                                           farland_needs);
                            for (auto& need : farland_needs)
                            {
                                need.unit_ = unit;
                            }
                            if (farland_needs.size() > 0)
                                needs.insert(needs.end(), farland_needs.begin(), farland_needs.end());                               
                        }
                    }

                    
                }
            });            
        }

        long weight_max_amount_of_items(CUnit* unit, const std::string& item_name)
        {
            //we don't care about weight if it's not a caravan
            if (unit->caravan_info_ != nullptr)
            {
                //if its a caravan, we want to avoid overweight
                long allowed_weight(0);
                long weight_step(0);//if we don't know, we assume it weights nothing.

                long unit_weights[5];
                unit_control::get_weights(unit, unit_weights);

                int *item_weights;
                int movecount;
                const char** movenames;                    
                gpDataHelper->GetItemWeights(item_name.c_str(), item_weights, movenames, movecount);

                switch(unit->caravan_info_->speed_) 
                {
                    case orders::CaravanSpeed::MOVE:
                        allowed_weight = unit_weights[1] - unit_weights[0];
                        weight_step = item_weights[1] - item_weights[0];
                        break;
                    case orders::CaravanSpeed::RIDE:
                        allowed_weight = unit_weights[2] - unit_weights[0];
                        weight_step = item_weights[2] - item_weights[0];
                        break;
                    case orders::CaravanSpeed::FLY:
                        allowed_weight = unit_weights[3] - unit_weights[0];
                        weight_step = item_weights[3] - item_weights[0];
                        break;
                    case orders::CaravanSpeed::SWIM:
                        allowed_weight = unit_weights[4] - unit_weights[0];
                        weight_step = item_weights[4] - item_weights[0];
                        break;
                    case orders::CaravanSpeed::SAIL: {
                            long struct_id = unit_control::structure_id(unit);
                            allowed_weight = 0;
                            weight_step = -item_weights[0];//just weight of item
                            if (struct_id > 0) {
                                CLand* cur_land = land_control::get_land(unit->LandId);
                                if (cur_land == nullptr)
                                    break;

                                land_control::structures::update_struct_weights(cur_land);
                                CStruct* ship = land_control::get_struct(cur_land, struct_id);
                                if (ship == nullptr)
                                    break;

                                allowed_weight = ship->capacity_ - ship->occupied_capacity_;
                            }                        
                        }
                        break;                        
                    default:
                        unit_control::order_message(unit, "Caravan speed is undefined", "(unlimited load)");              
                        break;
                }
                if (weight_step >= 0)
                    return -1;//any amount, since it actually increases requested capacity of the unit
                if (allowed_weight <= 0)
                    return 0;//nothing

                return allowed_weight / abs(weight_step);
            }
            return -1;
        }

        bool distribute_autoorder(orders::AutoSource& source, orders::AutoRequirement& need)
        {
            if (source.unit_ == need.unit_ || source.amount_ <= 0)
                return false;//no need to give to itself or parse sources/need without amount

            if (source.unit_->caravan_info_ != nullptr && 
                need.unit_->caravan_info_ != nullptr && 
                utils::is_route_same(source.unit_->caravan_info_, need.unit_->caravan_info_))
                return false;

            if (source.priority_ != -1 && source.priority_ <= need.priority_)
                return false;//don't give to lower priority

            long give_amount = source.amount_;
            if (need.amount_ >= 0)//need to short if expected amount is finite & below
                give_amount = std::min(give_amount, need.amount_);

            long max_amount_according_to_weight = weight_max_amount_of_items(need.unit_, source.name_);
            if (max_amount_according_to_weight == 0)
                return false;
            if (max_amount_according_to_weight != -1)//need to short by weight limit
                give_amount = std::min(give_amount, max_amount_according_to_weight);

            if (give_amount <= 0)
                return false;//do nothing if resulting amount is 0 or somehow became below

            std::string comment = ";!ao";
            if (need.unit_->caravan_info_ != nullptr && need.description_ != nullptr)
                comment = ";!ao " + *(need.description_);
                
            source.amount_ -= give_amount;
            if (need.amount_ != -1) //in case we do not need eternal amount, we can decrease need.amount
                need.amount_ -= give_amount;

            unit_control::modify_item_from_unit(source.unit_, need.unit_, source.name_, -give_amount);
            unit_control::modify_item_from_unit(need.unit_, source.unit_, source.name_, +give_amount);
            
            auto give_order = orders::control::compose_give_order(need.unit_, give_amount, source.name_, comment.c_str());
            orders::control::add_autoorder_to_unit(give_order, source.unit_);
            return true;
        }


        bool distribute_autoorders(std::vector<orders::AutoSource>& sources, std::vector<orders::AutoRequirement>& needs)
        {//needs expected to be already sorted out by priority
            bool ret(false);
            std::unordered_map<std::string, std::vector<long>> sources_table = orders::autoorders_caravan::create_source_table(sources);
            for (auto& need : needs)
            {
                if (strcmp(need.name_.c_str(), "ALL_ITEMS") == 0)
                {
                    for (auto& source : sources) 
                    {
                        std::vector<std::string> items = game_control::get_game_config<std::string>(SZ_SECT_UNITPROP_GROUPS, PRP_MEN);
                        if (std::find(items.begin(), items.end(), source.name_) != items.end()) //no men
                            continue;
                        if (source.name_ == "SILV") //no silver
                            continue;
                        ret = distribute_autoorder(source, need) || ret;
                    }
                        
                }
                else 
                {
                    if (sources_table.find(need.name_) == sources_table.end())
                        continue;

                    for (long i : sources_table[need.name_])
                    {
                        orders::AutoSource& source = sources[i];
                        ret = distribute_autoorder(source, need) || ret;
                    }
                }
            }
            return ret;
        }
    }     
};
