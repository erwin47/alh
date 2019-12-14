#ifndef ALH_ORDER_PARSER_H
#define ALH_ORDER_PARSER_H


#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

//#include "data.h" - circled inclusions

class CUnit;
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
        bool ignore_errors_;
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
        std::string compose_original_lines(const UnitOrders& orders);
    }

    namespace control
    {
        std::vector<std::shared_ptr<Order>> retrieve_orders_by_type(orders::Type type, const UnitOrders& unit_orders);
        void add_order_to_orders(std::shared_ptr<Order>& order, UnitOrders& unit_orders);

        //order specific functions
        std::vector<long> get_students(CUnit* unit);
        std::shared_ptr<Order> get_studying_order(const UnitOrders& unit_orders);
    }

      

    
    
};
#endif
