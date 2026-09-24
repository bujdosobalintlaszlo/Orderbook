#pragma once
#include<map>
#include<list>
#include<unordered_map>
#include "trade.h"
#include "market.h"
#include <unordered_map>
#include "types.h"
#include <concepts>
#include "ordertype.h"

//Securing that a OrderType has these getters for future advencements used in createTradeData 
template<typename T>
concept ProperOrder = requires(T o) {
    { o->getId() } -> std::convertible_to<OrderId>;
    { o->getOrderType() } -> std::convertible_to<OrderType>;
    { o->getSide() } -> std::convertible_to<Side>;
    { o->getRemainingQuantity() } -> std::convertible_to<Quantity>;
    { o->getInitialQuantity() } -> std::convertible_to<Quantity>;
};
class OrderBook{
private:
	 struct InsertInfo{
		  Price price_;
		  Side side_;
		  Orders::iterator it_;
		  InsertInfo(Price price,Side side,Orders::iterator it) : price_(price),side_(side),it_(it){};

	 };
	 std::map<Price,Orders,std::greater<uint64_t>> bids_;
	 std::map<Price,Orders> asks_;
	 std::unordered_map<OrderId,InsertInfo> orders_;
	 using HelperMapIt = std::unordered_map<OrderId,InsertInfo>::iterator;
public:
	 const std::map<Price,Orders,std::greater<uint64_t>>& getBids() const noexcept;
	 const std::map<Price,Orders>& getAsks() const noexcept;
	 OrderBook();
	 Trades placeOrder(OrderPtr order);
	 Trades placeOrder(MarketOrderPtr order);
	 bool isInBook(const OrderPtr &order) const;
	 void displayBids() const;
	 void displayAsks() const;
	 template <typename Compare>
	 Trades matchMarketOrder(MarketOrderPtr &order,std::map<Price,Orders, Compare> &book);
	 template<typename Comparator>
	 bool insertIntoBook(OrderPtr &order,std::map<Price,Orders,Comparator> &book);
	 //Securing that a OrderType has these getters for future advencements
	 template<ProperOrder BidOrderPtr,ProperOrder AskOrderPtr>
	 Trade createTradeData(const BidOrderPtr &bidOrder,const AskOrderPtr &askOrder,Price fillPrice);
	 bool cancelOrder(OrderId id);
	 template<typename Comparator>
	 Trades FOK(OrderPtr &order,std::map<Price,Orders,Comparator> &book);
	 template<typename BookType>
	 bool cancel(BookType& book,std::unordered_map<OrderId,InsertInfo>::iterator order_it);
	 template<typename Compare>
	 Trades matchLimitOrder(OrderPtr &order, std::map<Price,Orders, Compare> &book);
	 template<typename Comparator>
	 bool canMatch(OrderPtr &order,std::map<Price,Orders,Comparator> &book);
	 //order modifications
	 bool modifyOrderPrice(OrderId id,Price newPrice);
	 template<typename Comparator>
	 void executePriceMod(std::map<Price,Orders,Comparator> &book,Price newPrice,Orders::iterator item_it,HelperMapIt &order_it);
	 bool modifyOrderQuantity(OrderId id,Quantity newQuantity);
	 template<typename Comparator>
	 void executeModifyOrder(std::map<Price,Orders,Comparator> &book,HelperMapIt order_it,Quantity newQuantity);
	 //---
	 template<typename Comparator>
	 bool modifyOrderQuantity(OrderId id,Quantity newQuantity);
};

