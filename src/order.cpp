#include "orderbook/order.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include "orderbook/types.h"
#include "orderbook/constants.h"
//IMPLEMENT LIMIT
Order::Order(OrderId id, OrderType orderType, Side side, Price price, Quantity quantity, Date date, Symbol symbol) 
    : id_(id), orderType_(orderType), side_(side), price_(price), 
      initial_quantity_(quantity), remaining_quantity_(quantity), date_(date), symbol_(symbol) {
      
    if (quantity <= 0) {
        throw std::invalid_argument("Order quantity must be positive! Order ID: " + id+ " with QUANTITY: " + std::to_string(quantity));
    }
	 if(price <= 0){
        throw std::invalid_argument("Order price must be positive! Order ID: " + id+ " with PRICE: " + std::to_string(price));
	 }
	 if(orderType == OrderType::Market){
		  throw std::invalid_argument("Market orders needs to be constructed with a other class!");
	 }
}

// Getters
OrderId Order::getId() const { return id_; }
OrderType Order::getOrderType() const { return orderType_; }
Side Order::getSide() const { return side_; }
Price Order::getPrice() const { return price_; }
Quantity Order::getInitialQuantity() const { return initial_quantity_; }
Quantity Order::getRemainingQuantity() const { return remaining_quantity_; }
Date Order::getDate() const {return date_;}
Symbol Order::getSymbol() const {return symbol_;}
//setters
bool Order::setPrice(Price price){
	 if(price < 0 || price == price_) throw std::invalid_argument("An invalid price modification at ID: " + id_);
	 price_=price;
	 return true;
}

/*
 * theo:
if qnt <0 || qnt == init || qnt == rema throw
if qnt < init 
	 rema = qnt
else
	 init = rema =qnt
*/
bool Order::setQuantity(Quantity qnt){
	if(qnt < 0 || qnt == remaining_quantity_ || (remaining_quantity_ == initial_quantity_ && remaining_quantity_ == qnt)){
		  throw std::invalid_argument("An invalid quantity was given at ID: "+id_);
	}
	if(qnt < initial_quantity_){
		  remaining_quantity_=qnt;
	}else{
		  initial_quantity_=qnt;
		  remaining_quantity_=qnt;
	}
	 return true;	
}
// Modifiers & Calculations
void Order::fill(Quantity quantity) {
    if (quantity > remaining_quantity_) {
        throw std::invalid_argument("Tried to fill more than remaining quantity!");
    }
    remaining_quantity_ -= quantity;
}

Quantity Order::filledQuantity() const {
    return initial_quantity_ - remaining_quantity_;
}

double Order::getFufillmentOfOrder() const {
    if (initial_quantity_ == 0) return 0.0;
    return (static_cast<double>(filledQuantity()) / initial_quantity_) * 100.0;
}

bool Order::isFilled() const { 
    return remaining_quantity_ == 0; 
}
double Order::convertToDecimal(Price price) const{
	 return price / 10000.0;
}

//PS.: SUPPORING 4 decimals
void Order::printOrder() const {
    std::cout << "Order ID: " << getId() 
              << ", Type: " << getOrderType() 
              << ", Side: " << getSide()
              << ", Price: " << convertToDecimal(getPrice()) 
              << ", Initial Qty: " << getInitialQuantity() 
              << ", Remaining Qty: " << getRemainingQuantity() 
              << ", Fulfillment: " << getFufillmentOfOrder()  
				  << ", Date: " << getDate() << "\n";
}

