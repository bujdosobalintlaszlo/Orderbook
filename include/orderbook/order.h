#pragma once
#include "side.h"
#include "ordertype.h"
#include "types.h"
class Order{
private:
	 OrderId id_;
	 OrderType orderType_;
	 Side side_;
	 Price price_;
	 Quantity initial_quantity_;
	 Quantity remaining_quantity_;
	 Date date_;
	 Symbol symbol_;
	 double convertToDecimal(Price price) const;
public:
	 Order(OrderId id,OrderType orderType,Side side,Price price,Quantity quantity, Date date, Symbol symbol);
	 //getters
	 OrderId getId() const;
	 OrderType getOrderType() const;
	 Side getSide()const;
	 Price getPrice()const;
	 Quantity getInitialQuantity()const;
	 Quantity getRemainingQuantity()const;
	 Date getDate() const;
	 Symbol getSymbol() const;
	 //setters
	 void fill(Quantity quantity);
	 Quantity filledQuantity() const;
	 bool setQuantity(Quantity qnt);
	 bool setPrice(Price price);

	 //only for display
	 double getFufillmentOfOrder() const;
	 void printOrder() const;
	 bool isFilled() const; 

	 ~Order();
};
