#ifndef ALH_ORDER_PARSER_H
#define ALH_ORDER_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>

namespace orders_parser
{
    enum class OrderType {
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

    extern std::unordered_map<std::string, OrderType> types_mapping;

    struct Order 
    {
        OrderType type_;
        std::vector<std::string> words_order_;
        std::string comment_;
        std::string original_string_;
    };

    Order get_order_from_line(const std::string& line);

    struct UnitOrders
    {
        std::vector<Order> orders_;
        std::unordered_map<OrderType, std::vector<size_t>> hash_;
    };

    UnitOrders get_unit_orders_from_string(const std::string& orders);
    std::string compose_original_orders(const UnitOrders& orders);
    std::vector<Order> get_unit_orders_by_type(OrderType type, const UnitOrders& unit_orders);
    void add_order_to_unit_orders(const Order& order, UnitOrders& unit_orders);


    
};
#endif
