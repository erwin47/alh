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
        O_SPOILS,
        O_JOIN,
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
        O_COMMENT_AUTONAME,
        O_ERROR,//comment would contain description
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
        UnitOrders() : turn_endturn_started_(false) {}

        std::vector<std::shared_ptr<orders::Order>> orders_;
        std::unordered_map<orders::Type, std::vector<size_t>, OrderTypeHash> hash_;

        bool turn_endturn_started_;
        std::vector<std::shared_ptr<orders::Order>> turn_endturn_collection_;
        std::string errors_;
    };

    namespace parser
    {
        std::shared_ptr<Order> parse_line_to_order(const std::string& line);
        UnitOrders parse_lines_to_orders(const std::string& orders);
        std::string compose_string(const UnitOrders& orders);

        namespace specific
        {   //except = 0 give target_id all item
            //except > 0 give target_id all item except except
            //except = -1 -- give target_id amount item
            //target_id < 0 -- NEW ID
            //target_faction_id == 0 -- not specified
            bool parse_teaching(const std::shared_ptr<orders::Order>& order, long faction_id, std::vector<long>& students);
            bool parse_give(const std::shared_ptr<orders::Order>& order, long& target_id, 
                            long& target_faction_id, long& amount, std::string& item, long& except);
            bool parse_produce(const std::shared_ptr<orders::Order>& order, std::string& item, long& amount);
            bool parse_build(const std::shared_ptr<orders::Order>& order, std::string& building, bool& helps, long& unit_id);
            bool parse_claim(const std::shared_ptr<orders::Order>& order, long& amount);
            bool parse_study(const std::shared_ptr<orders::Order>& order, std::string& skill, long& level);
            bool parse_assassinate(const std::shared_ptr<orders::Order>& order, long& target_id);
            bool parse_attack(const std::shared_ptr<orders::Order>& order, std::vector<long>& targets);
            bool parse_steal(const std::shared_ptr<orders::Order>& order, long& target_id, std::string& item);
            bool parse_sellbuy(const std::shared_ptr<orders::Order>& order, std::string& item, long& amount, bool& all);
            bool parse_flags(const std::shared_ptr<orders::Order>& order, bool& flag);
            bool parse_flags_with_param(const std::shared_ptr<orders::Order>& order, std::string& param);

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
        CaravanInfo(CaravanSpeed speed, std::vector<RegionInfo>&& regions, CLand* goal_land) 
            : speed_(speed), regions_(regions), goal_land_(goal_land)  { }
        CaravanSpeed speed_;
        std::vector<RegionInfo> regions_;
        CLand* goal_land_;
    };

    namespace control
    {
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
            AO_OWNER
        };

        enum class LogicAction {
            NONE,
            SWITCH_COMMENT,
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
    }         
    
};
#endif
