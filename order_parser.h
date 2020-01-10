#ifndef ALH_ORDER_PARSER_H
#define ALH_ORDER_PARSER_H


#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
//#include "data_control.h"

//#include "data.h" - circled inclusions

class CUnit;
class CLand;
namespace orders
{
    enum class Type {
        O_ADDRESS = 1,
        O_ADVANCE,
        O_ARMOR,
        O_ASSASSINATE,
        O_ATTACK,
        O_AUTOTAX,
        O_AVOID,
        O_BEHIND,
        O_BUILD,
        O_BUY,
        O_CAST,
        O_CLAIM,
        O_COMBAT,
        O_CONSUME,
        O_DECLARE,
        O_DESCRIBE,
        O_DESTROY,
        O_ENDFORM,
        O_ENTER,
        O_ENTERTAIN,
        O_EVICT,
        O_EXCHANGE,
        O_FACTION,
        O_FIND,
        O_FORGET,
        O_FORM,
        O_GIVE,
        O_GIVEIF,
        O_TAKE,
        O_SEND,
        O_GUARD,
        O_HOLD,
        O_LEAVE,
        O_MOVE,
        O_NAME,
        O_NOAID,
        O_NOCROSS,
        O_NOSPOILS,
        O_OPTION,
        O_PASSWORD,
        O_PILLAGE,
        O_PREPARE,
        O_PRODUCE,
        O_PROMOTE,
        O_QUIT,
        O_RESTART,
        O_REVEAL,
        O_SAIL,
        O_SELL,
        O_SHARE,
        O_SHOW,
        O_SPOILS,
        O_STEAL,
        O_STUDY,
        O_TAX,
        O_TEACH,
        O_WEAPON,
        O_WITHDRAW,
        O_WORK,
        O_RECRUIT,
        O_TYPE,
        O_LABEL,
        // must be in this sequence! O_ENDXXX == O_XXX+1
        O_TURN,
        O_ENDTURN,
        O_TEMPLATE,
        O_ENDTEMPLATE,
        O_ALL,
        O_ENDALL,
        O_COMMENT,
        NORDERS
    };

    extern std::unordered_map<std::string, orders::Type> types_mapping;

    struct Order 
    {
        orders::Type type_;
        std::vector<std::string> words_order_;
        std::string comment_;
        std::string original_string_;
    };

    struct OrderTypeHash {
        std::size_t operator()(orders::Type t) const
        {
            return static_cast<std::size_t>(t);
        } 
    };

    struct UnitOrders
    {
        std::vector<std::shared_ptr<orders::Order>> orders_;
        std::unordered_map<orders::Type, std::vector<size_t>, OrderTypeHash> hash_;
    };

    namespace parser
    {
        std::shared_ptr<Order> parse_line_to_order(const std::string& line);
        UnitOrders parse_lines_to_orders(const std::string& orders);
        std::string compose_string(const UnitOrders& orders);

        void recalculate_hash(UnitOrders& uorders);
    }

    enum class CaravanSpeed {
        MOVE, RIDE, FLY, SWIM, UNDEFINED
    };

    struct RegionInfo
    {
        long x_;
        long y_;
        long z_;
    };

    struct CaravanInfo
    {
        CaravanSpeed speed_;
        std::vector<RegionInfo> regions_;
    };

    namespace control
    {
        //! returns collection of orders of specified type
        std::vector<std::shared_ptr<Order>> retrieve_orders_by_type(orders::Type type, const UnitOrders& unit_orders);

        //! adds order to unit
        void add_order_to_unit(std::shared_ptr<Order>& order, CUnit* unit);
        void add_order_to_unit(std::string order_line, CUnit* unit);

        //! will be added just if not exists similar. And with specific comment.
        void add_autoorder_to_unit(std::shared_ptr<Order>& order, CUnit* unit);
        
        //! removes empty lines from unit's orders.
        void remove_empty_lines(CUnit* unit);

        //! removes orders with specified pattern in comments
        void remove_orders_by_comment(CUnit* unit, const std::string& pattern);

        std::shared_ptr<Order> compose_give_order(CUnit* target, long amount, const std::string& item, const std::string& comment);
        //order specific functions
        
        std::vector<long> get_students(CUnit* unit);
        std::shared_ptr<Order> get_studying_order(const UnitOrders& unit_orders);
    }

    struct AutoSource
    {
        std::string name_;
        long amount_;
        CUnit* unit_;
    };
    
    struct AutoRequirement
    {
        std::string name_;
        long amount_;//-1 all
        long priority_;//the lower the better. 10 default. 20 for -1
        bool regional_;//determines if it is NEED or NEEDREG
        CUnit* unit_;
    };

    namespace autoorders 
    {
        //! checks if warnings related to current order have to be suspended 
        bool should_suspend_warnings(const std::shared_ptr<Order>& order);

        //! checks if orders of unit contain caravan info
        bool is_caravan(const UnitOrders& unit_orders);

        //! extract CaravanInfo from orders of unit
        CaravanInfo get_caravan_info(UnitOrders& unit_orders);

        //! parse orders to find out all SOURCE marks of current unit
        bool get_unit_autosources(const UnitOrders& unit_orders, std::vector<AutoSource>& sources);

        //! checks actual amount of items in unit to define how many of them it actually can share
        void adjust_unit_sources(CUnit* unit, std::vector<AutoSource>& sources);

        //! parse orders to find out all NEED requests for current unit and for current region
        bool get_unit_autoneeds(const UnitOrders& unit_orders, std::vector<AutoRequirement>& unit_needs);
        
        //! checks actual amount of items in unit to define real request
        void adjust_unit_needs(CLand* land, CUnit* unit, std::vector<AutoRequirement>& unit_needs);

        std::unordered_map<std::string, std::vector<long>> create_source_table(const std::vector<AutoSource>& sources);


    }    
    
};
#endif
