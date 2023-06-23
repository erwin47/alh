#ifndef ALH_ORDER_PARSER_H
#define ALH_ORDER_PARSER_H

//#include "atlaparser.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
//#include "data_control.h"

//#include "data.h" - circled inclusions

class CUnit;
class CLand;
namespace orders
{
    enum class Type : uint64_t {
        O_NORDERS = 0,
        O_ADDRESS = 1,
        O_ADVANCE = 2,
        O_ARMOR = 3,
        O_ASSASSINATE = 4,
        O_ATTACK = 5,
        O_AUTOTAX = 6,
        O_AVOID = 7,
        O_BEHIND = 8,
        O_BUILD = 9,
        O_BUY = 10,
        O_CAST = 11,
        O_CLAIM = 12,
        O_COMBAT = 13,
        O_CONSUME = 14,
        O_DECLARE = 15,
        O_DESCRIBE = 16,
        O_DESTROY = 17,
        O_ENDFORM = 18,
        O_ENTER = 19,
        O_ENTERTAIN = 20,
        O_EVICT = 21,
        O_EXCHANGE = 22,
        O_FACTION = 23,
        O_FIND = 24,
        O_FORGET = 25,
        O_FORM = 26,
        O_GIVE = 27,
        O_GIVEIF = 28,
        O_TAKE = 29,
        O_SEND = 30,
        O_GUARD = 31,
        O_HOLD = 32,
        O_LEAVE = 33,
        O_MOVE = 34,
        O_NAME = 35,
        O_NOAID = 36,
        O_NOCROSS = 37,
        O_SPOILS = 38,
        O_JOIN = 39,
        O_OPTION = 40,
        O_PASSWORD = 41,
        O_PILLAGE = 42,
        O_PREPARE = 43,
        O_PRODUCE = 44,
        O_PROMOTE = 45,
        O_QUIT = 46,
        O_RESTART = 47,
        O_REVEAL = 48,
        O_SAIL = 49,
        O_SELL = 50,
        O_SHARE = 51,
        O_SHOW = 52,
        O_STEAL = 53,
        O_STUDY = 54,
        O_TAX = 55,
        O_TEACH = 56,
        O_TRANSPORT = 57,
        O_DISTRIBUTE = 58,
        O_WEAPON = 59,
        O_WITHDRAW = 60,
        O_WORK = 61,
        O_RECRUIT = 62,
        O_TYPE = 63,
        O_LABEL = 64,
        // must be in this sequence! O_ENDXXX == O_XXX+1
        O_TURN = 65,
        O_ENDTURN = 66,
        O_TEMPLATE = 67,
        O_ENDTEMPLATE = 68,
        O_ALL = 69,
        O_ENDALL = 70,
        O_ERROR = 71,
        O_COMMENT = 256,//any comment not belonging to any other comments groups
        O_COMMENT_AUTONAME = 512,//specific comment which belongs to autoname generation system
        O_SUPRESS_ERRORS = 1024,//specific comment which belongs to autoname generation system
    };

    extern std::unordered_map<std::string, orders::Type> types_mapping;

    struct Order 
    {
        orders::Type type_;
        std::vector<std::string> words_order_;
        std::string comment_;
        std::string original_string_;
    };

    static Order SUPRESS_ERROR = {orders::Type::O_SUPRESS_ERRORS, {}, {}, {}};

    struct UnitOrders
    {
        struct OrderTypeHash {
            std::size_t operator()(orders::Type t) const
            {
                return static_cast<std::size_t>(t);
            } 
        };

        UnitOrders() : turn_endturn_started_(false), is_modified_(false) {}

        std::vector<std::shared_ptr<orders::Order>> orders_;
        std::unordered_map<orders::Type, std::vector<size_t>, OrderTypeHash> hash_;

        bool turn_endturn_started_;
        bool is_modified_;
        std::vector<std::shared_ptr<orders::Order>> turn_endturn_collection_;
        std::string errors_;
    };

    namespace parser
    {
        std::shared_ptr<Order> parse_line_to_order(const std::string& line);
        UnitOrders parse_lines_to_orders(const std::string& orders);

        template<typename T>
        void compose_string(T& res, const std::shared_ptr<Order>& order)
        {
            if (order->original_string_.size() > 0)
                res << order->original_string_.c_str() << "\n";
            else 
            {
                for (const auto& word : order->words_order_)
                    res << word.c_str() << " ";
                
                if (order->comment_.size() > 0)
                    res << order->comment_.c_str();
                res << "\n";
            }
        }

        std::string compose_string(const UnitOrders& orders);

        namespace specific
        {   //except = 0 give target_id all item
            //except > 0 give target_id all item except except
            //except = -1 -- give target_id amount item
            //target_id < 0 -- NEW ID
            //target_faction_id == 0 -- not specified
            bool parse_teaching(const std::shared_ptr<orders::Order>& order, long x, long y, long z, long faction_id, std::vector<long>& students);
            bool parse_give(const std::shared_ptr<orders::Order>& order, long& target_id, 
                            long& target_faction_id, long& amount, std::string& item, long& except);
            bool parse_produce(const std::shared_ptr<orders::Order>& order, std::string& item, long& amount);
            bool parse_build(const std::shared_ptr<orders::Order>& order, std::string& building, bool& helps, long& unit_id);
            bool parse_claim(const std::shared_ptr<orders::Order>& order, long& amount);
            bool parse_withdraw(const std::shared_ptr<orders::Order>& order, std::string& item, long& amount);
            bool parse_study(const std::shared_ptr<orders::Order>& order, std::string& skill, long& level);
            bool parse_assassinate(const std::shared_ptr<orders::Order>& order, long& target_id);
            bool parse_attack(const std::shared_ptr<orders::Order>& order, std::vector<long>& targets);
            bool parse_steal(const std::shared_ptr<orders::Order>& order, long& target_id, std::string& item);
            bool parse_sellbuy(const std::shared_ptr<orders::Order>& order, std::string& item, long& amount, bool& all);
            bool parse_flags(const std::shared_ptr<orders::Order>& order, bool& flag);
            bool parse_flags_with_param(const std::shared_ptr<orders::Order>& order, std::string& param);
            bool parse_namedescribe(const std::shared_ptr<orders::Order>& order, std::string& name, bool& object);
            bool parse_enter(const std::shared_ptr<orders::Order>& order, long& struct_id);
            bool parse_promote(const std::shared_ptr<orders::Order>& order, long& target_id);
            bool parse_transport(const std::shared_ptr<orders::Order>& order, long& target_id, long& amount, std::string& item, long& except);

        }

        void recalculate_hash(UnitOrders& uorders);
    }

    enum class CaravanSpeed {
        MOVE, RIDE, FLY, SWIM, SAIL, UNDEFINED
    };

    struct RegionInfo
    {
        long x_;
        long y_;
        long z_;
    };

    struct CaravanInfo
    {
        CaravanInfo() : speed_(CaravanSpeed::UNDEFINED), goal_land_(nullptr) {}
        CaravanInfo(CaravanSpeed speed, std::vector<RegionInfo>&& regions, CLand* goal_land) 
            : speed_(speed), regions_(regions), goal_land_(goal_land)  { }
        CaravanSpeed speed_;
        std::vector<RegionInfo> regions_;
        CLand* goal_land_;
    };

    namespace control
    {
        //! checks if the order belongs to a specified type
        bool is_order_type(const std::shared_ptr<Order>& order, const orders::Type& type);

        //! checks if the order have to be supressed
        bool should_supress_error(const std::shared_ptr<Order>& order);

        //! modifies existing order to adjust it to provided string, true on success
        bool modify_order(CUnit* unit, std::shared_ptr<Order>& order, const std::string& new_order_line);
             
        //! returns collection of orders of specified type
        std::vector<std::shared_ptr<Order>> retrieve_orders_by_type(orders::Type type, const UnitOrders& unit_orders);
        
        //! returns true if unit_orders has order of specified type
        bool has_orders_with_type(orders::Type type, const UnitOrders& unit_orders);

        //! adds order to unit
        void add_order_to_unit(std::shared_ptr<Order>& order, CUnit* unit);
        void add_order_to_unit(std::string order_line, CUnit* unit);

        //! will be added just if not exists similar. And with specific comment.
        void add_autoorder_to_unit(std::shared_ptr<Order>& order, CUnit* unit);
        
        //! removes empty lines from unit's orders.
        void remove_empty_lines(CUnit* unit);

        //! removes order
        void remove_order(CUnit* unit, std::shared_ptr<Order>& order);
        //! removes orders with specified pattern in comments
        void remove_orders_by_comment(CUnit* unit, const std::string& pattern);

        void comment_order_out(std::shared_ptr<Order>& order, CUnit* unit);
        void uncomment_order(std::shared_ptr<Order>& order, CUnit* unit);

        std::shared_ptr<Order> compose_give_order(CUnit* target, long amount, const std::string& item, const std::string& comment, bool repeating = false);
        //order specific functions
        
        std::shared_ptr<Order> get_studying_order(const UnitOrders& unit_orders);

        template<orders::Type TYPE>
        long flag_by_order_type();
    }

    struct AutoSource
    {
        std::string name_;
        long amount_;
        long priority_;//-1 means no priority (default). In other cases gives just to needs which are lower
        CUnit* unit_;
    };
    
    struct AutoRequirement
    {
        std::string name_;
        long amount_;//-1 means all, -2 means equal to unit man's amount
        long priority_;//the lower the better. 10 default. 20 for -1
        CUnit* unit_;
        std::shared_ptr<std::string> description_;
    };

    namespace autoorders 
    {
        enum class AO_TYPES {
            AO_NONE,
            AO_GET,
            AO_CONDITION,
            AO_OWNER,
            AO_HELP
        };

        enum class LogicAction {
            NONE,
            SWITCH_COMMENT,
            DEL_COMMENT,
            LOGIC_ERROR
        };

        AO_TYPES has_autoorders(const std::shared_ptr<Order>& order);
        bool parse_get(const std::shared_ptr<Order>& order, long& amount, std::string& item);
        bool parse_logic(const std::shared_ptr<Order>& order, LogicAction& action, std::string& statement, bool& debug);

    }    

    namespace autoorders_caravan
    {
        //! checks if orders of unit contain caravan info
        bool is_caravan(const UnitOrders& unit_orders);

        //! extract CaravanInfo from orders of unit
        std::shared_ptr<CaravanInfo> get_caravan_info(UnitOrders& unit_orders);

        std::unordered_map<std::string, std::vector<long>> create_source_table(const std::vector<AutoSource>& sources);
        
        void get_land_autosources_and_autoneeds(CLand* land, 
                                                std::vector<orders::AutoSource>& sources,
                                                std::vector<orders::AutoRequirement>& needs);

        void get_land_caravan_autosources_and_autoneeds(CLand* land, 
                                                        std::vector<orders::AutoSource>& sources,
                                                        std::vector<orders::AutoRequirement>& needs);                                                

        //!gets all land's caravan sources
        //void get_land_caravan_sources(CLand* land, std::vector<orders::AutoSource>& sources);

        //!gets all needs of the land except foreign needs of caravans
        //void get_land_autoneeds(CLand* land, std::vector<orders::AutoRequirement>& needs);

        //!gets all foreign needs of caravans
       //void get_land_caravan_needs(CLand* land, std::vector<orders::AutoRequirement>& needs);    

        //!returns amount of items this unit can take according to his weight policy
        long weight_max_amount_of_items(CUnit* unit, const std::string& item_name);

        //!distributes sources among needs according to priorities, returns true if added any change
        bool  distribute_autoorders(std::vector<orders::AutoSource>& sources, std::vector<orders::AutoRequirement>& needs);

        namespace parser 
        {
            void get_unit_sources_and_needs(CUnit* unit, 
                                  std::vector<AutoSource>& sources,
                                  std::vector<AutoRequirement>& needs);
        }
    }         
    
};


inline orders::Type operator|(orders::Type a, orders::Type b)
{
    return static_cast<orders::Type>(static_cast<long>(a) | static_cast<long>(b));
}

inline orders::Type operator&(orders::Type a, orders::Type b)
{
    return static_cast<orders::Type>(static_cast<long>(a) & static_cast<long>(b));
}

#endif
