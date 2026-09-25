#include "orderbook/order.h"
#include "orderbook/orderbook.h"
#include "orderbook/side.h"
#include<memory>
#include "orderbook/ordertype.h"
#include "orderbook/trade_info.h"
#include "orderbook/market.h"
#include<algorithm>
//#include "orderbook/market.h"

OrderBook::OrderBook() = default;

const std::map<uint64_t,Orders,std::greater<uint64_t>>& OrderBook::getBids() const noexcept{return bids_;}
const std::map<uint64_t,Orders>& OrderBook::getAsks() const noexcept{return asks_;}
void OrderBook::displayBids() const {
	 std::cout << "___ BIDS DEBUG ___" << '\n';
    for (const auto& [price, order_list] : bids_) {
        for (const auto& order_ptr : order_list) {
				order_ptr->printOrder();
        }
    }
}

void OrderBook::displayAsks() const {
	 std::cout << "___ ASKS DEBUG ___" << '\n';
    for (const auto& [price, order_list] : asks_) {
        for (const auto& order_ptr : order_list) {
				order_ptr->printOrder();
        }
    }
}
template<typename Compare>
Trades OrderBook::matchMarketOrder(MarketOrderPtr &order,std::map<Price,Orders, Compare> &book){
	 Trades trades{};
	 auto it = book.begin();
	 while(it!=book.end() && order->getRemainingQuantity() >0){
		  auto &orders = it->second;
		  auto orders_it = orders.begin();
		  while(orders_it != orders.end() && order->getRemainingQuantity() >0){
				auto &curr_order = *orders_it;
				if(curr_order->getSymbol() == order->getSymbol())
				{
					 uint64_t fill_qty = std::min(order->getRemainingQuantity(), curr_order->getRemainingQuantity());
					 if(fill_qty > 0 ){
						  Price fill_price = curr_order->getPrice();
						  order->fill(fill_qty);
						  curr_order->fill(fill_qty);
						  trades.push_back(createTradeData(order, curr_order, fill_price));
					 }
				}
				if (curr_order->getRemainingQuantity() == 0) {
					 orders_it = orders.erase(orders_it);
				} else {
					 ++orders_it;
				}
				
		  }
		  if (orders.empty()) {
				it = book.erase(it);
		  } else {
				++it;
		  }
	 }

	 return trades;
}
///<summary>
///params: the orders pointer, and bids_ or asks_ as a refference depending on BUY or SELL side order.
///return: A vector of trades which went trough. Could be empty, which means it couldn't macth.
///</summary>
template <typename Compare>
Trades OrderBook::matchLimitOrder(OrderPtr &order, std::map<Price,Orders, Compare> &book) {
	 std::cout << "ENTERED LIMIT MATCH" << '\n';
	 Trades trades{};
	 auto it = book.begin();
	 while (it != book.end() && order->getRemainingQuantity() > 0) {
		  auto &orders = it->second;
		  auto orders_it = orders.begin();
		  while (orders_it != orders.end() && order->getRemainingQuantity() > 0) {
				auto &current_order = *orders_it;
				if(order->getSymbol() == current_order->getSymbol()){
					 uint64_t fill_qty{0};
					 if(order->getSide() == Side::BUY && order->getPrice() >= current_order->getPrice()){
						  fill_qty = std::min(order->getRemainingQuantity(), current_order->getRemainingQuantity());
					 }
					 else if(order->getSide() == Side::SELL && order->getPrice() <= current_order->getPrice()){
						  fill_qty = std::min(order->getRemainingQuantity(), current_order->getRemainingQuantity());
					 }
					 if(fill_qty > 0){
						  Price fill_price = current_order->getPrice();
						  order->fill(fill_qty);
						  current_order->fill(fill_qty);
						  trades.push_back(createTradeData(order, current_order,fill_price));
					 }
				}
				//if the current order from the book got fully filled, then we remove it from the list, storing trades at the given price level
				if (current_order->getRemainingQuantity() == 0) {
					 orders_it = orders.erase(orders_it);
				} else {
					 ++orders_it;
				}
		  }
		  //checking if the price level got emptyed. If so we delete it from the book.
		  if (orders.empty()) {
				it = book.erase(it);
		  } else {
				++it;
		  }
	 }
	 return trades;
}
///<summary> 
///Creates a Trade obj which consists of 2 TradeInfo objects. Those can be created by the orders traits;
///<summary>
///
template<ProperOrder BidOrderPtr,ProperOrder AskOrderPtr>
Trade OrderBook::createTradeData(const BidOrderPtr &bidOrder,const AskOrderPtr &askOrder,Price fillPrice){
	  TradeInfo bidInf(bidOrder->getId(),bidOrder->getOrderType(),fillPrice,bidOrder->getSide(),bidOrder->getRemainingQuantity(),bidOrder->getInitialQuantity());
	  TradeInfo askInf(askOrder->getId(),askOrder->getOrderType(),fillPrice,askOrder->getSide(),askOrder->getRemainingQuantity(),askOrder->getInitialQuantity());
	  Trade trade(bidInf,askInf);
	  return trade;
}

template<typename Comparator>
bool OrderBook::insertIntoBook(OrderPtr &order,std::map<Price,Orders,Comparator> &book){
	 if(!order) return false;
	 const Price price = order->getPrice();
	 const Side side = order->getSide();
	 const OrderId id = order->getId();
	 if(orders_.find(order->getId()) != orders_.end()){
		  return false;
	 }
	 auto [it, inserted] = book.try_emplace(price);
	 it->second.push_back(std::move(order));
	 auto order_it = std::prev(it->second.end());
	 OrderBook::InsertInfo i(price,side,order_it);
	 orders_.insert({id,i}); 
	 return true;
}

bool OrderBook::cancelOrder(OrderId id){
	 auto order_it = orders_.find(id);
	 if(order_it != orders_.end()){
		  if(order_it->second.side_ == Side::BUY){
				return cancel(bids_,order_it);
	 }else{
		  return cancel(asks_,order_it);
		  }
	 }
		  return false;
} 
template<typename BookType>
bool OrderBook::cancel(BookType& book,std::unordered_map<OrderId,InsertInfo>::iterator order_it){
	 if(order_it != orders_.end()){
		  auto level_it = book.find(order_it->second.price_);
		  if(level_it != book.end()){
				level_it->second.erase(order_it->second.it_);
				if(level_it->second.empty()){
					 book.erase(level_it);
				}
				orders_.erase(order_it);
				return true;
		  }
	 }
	 return false;
}

///FONTOS FOK tobb orderbol is fillelheto csak a vegere nem maradhta
template<typename Comparator>
Trades OrderBook::FOK(OrderPtr &order,std::map<Price,Orders,Comparator> &book){
	if(!canMatch(order,book)){
		  return {};
	}
	Trades trades{};
	auto it = book.begin();
	while(it != book.end()){
		  auto &orders = it->second;
		  auto orders_it = orders.begin();
		  while(orders_it != orders.end()){
				auto &current_order = *orders_it;
				if(order->getRemainingQuantity() > 0){
					 if(current_order->getSymbol() == order->getSymbol()){
						  uint64_t fill_qty{0};
						  if(order->getSide() == Side::BUY && order->getPrice() >= current_order->getPrice()&& order->getSymbol() == current_order->getSymbol()){
								fill_qty = std::min(order->getRemainingQuantity(), current_order->getRemainingQuantity());
						  }
						  else if(order->getSide() == Side::SELL && order->getPrice() <= current_order->getPrice()&& order->getSymbol() == current_order->getSymbol()){
								fill_qty = std::min(order->getRemainingQuantity(), current_order->getRemainingQuantity());
						  }
						  if(fill_qty > 0){
								uint64_t fill_price = current_order->getPrice();
								order->fill(fill_qty);
								current_order->fill(fill_qty);
								trades.push_back(createTradeData(order, current_order,fill_price));
						  }
					 }
				}
				//itt a hiba utolagos torles kene mert kitorli ha partial fillel
				//if the current order from the book got fully filled, then we remove it from the list, storing trades at the given price level
				if (current_order->getRemainingQuantity() == 0) {
					 //std::cout << "ORDER DELETED FROM BOOK" << '\n';
					 orders_it = orders.erase(orders_it);
				} else {
					 ++orders_it;
				}
		  }
		  if (orders.empty()) {
				//std::cout << "LEVEL DELETED FROM BOOK" << '\n';
				it = book.erase(it);
		  } else {
				++it;
		  }
	}
	 return trades;
}
bool OrderBook::isInBook(const OrderPtr &order) const {
    if (!order) return false;
	 auto it = orders_.find(order->getId());
	 if(it != orders_.end()){
		  return true;
	 }
	 return false;
}

template<typename Comparator>
bool OrderBook::canMatch(OrderPtr &order, std::map<Price,Orders,Comparator> &book){
    Quantity copy = order->getRemainingQuantity();
    auto it = book.begin();
    while(it != book.end()){
        Price levelPrice = it->first;
        bool priceOk = (order->getSide() == Side::BUY  && order->getPrice() >= levelPrice) || (order->getSide() == Side::SELL && order->getPrice() <= levelPrice);
        if(!priceOk){
            break;
		  }

        auto &orders = it->second;
        for(auto &current_order : orders){
            copy -= std::min(copy, current_order->getRemainingQuantity());
            if(copy == 0){
                return true;
            }
        }
        ++it;
    }
    return false;
}

template<typename Comparator>
void OrderBook::executePriceMod(std::map<Price,Orders,Comparator> &book,Price newPrice,Orders::iterator item_it,HelperMapIt &order_it){
    std::cout << "exe price" << '\n';

    Price oldPrice = order_it->second.price_;
    auto oldLevel_it = book.find(oldPrice);
    auto [it, inserted] = book.try_emplace(newPrice);
    item_it->get()->setPrice(newPrice);
    it->second.splice(it->second.end(), oldLevel_it->second, item_it);
    if (oldLevel_it->second.empty()) {
        book.erase(oldLevel_it);
    }
    orders_.erase(order_it);
    std::cout << "succesfull pricemod" << '\n';
}
bool OrderBook::modifyOrderPrice(OrderId id,Price newPrice){
	 std::cout << "____ MODIFY PRICE ____" << '\n';
	 auto order_it = orders_.find(id);
	 std::cout << order_it->first << " = " << id << '\n'; 
	 if(order_it != orders_.end()){
		  std::cout << "IN MOD PRICE IF" << '\n';
		  if(order_it->second.side_ == Side::BUY){
				std::cout << "Mod price on buy" << '\n';
				executePriceMod(bids_,newPrice,order_it->second.it_,order_it);
				
		  }else{
				std::cout << "Mod price on sell" << '\n';
				executePriceMod(asks_,newPrice,order_it->second.it_,order_it);
		  }
		  //orders_.erase(order_it);
		  return true;
	 }
	 return false;
}
template<typename Comparator>
void OrderBook::executeModifyOrder(std::map<Price,Orders,Comparator> &book,HelperMapIt order_it,Quantity newQuantity){
	 auto level_it = book.find(order_it->second.price_);
	 auto mod_order = std::find(level_it->second.begin(),level_it->second.end(),*(order_it->second.it_));
	 if(mod_order != level_it->second.end()){
		 (mod_order)->get()->setQuantity(newQuantity);
		 std::cout << "succesfull quant mod" << '\n';
	 }
} 
bool OrderBook::modifyOrderQuantity(OrderId id,Quantity newQuantity){
	 auto order_it = orders_.find(id);
	 if(order_it != orders_.end()){
		 if(order_it->second.side_ == Side::BUY){
				executeModifyOrder(bids_,order_it,newQuantity);
		 }else{
				executeModifyOrder(asks_,order_it,newQuantity);
		 } 
		 return true;
	 }
	 return false;
}


Trades OrderBook::placeOrder(OrderPtr order){
	 if(!order) throw std::invalid_argument("Invalid order was submited!");
	 switch(order->getOrderType()){
		  case OrderType::GoodForDay:
				//NOT IMPLEMENTED YET
				if(order->getSide() == Side::BUY){
					 Trades t = matchLimitOrder(order,asks_);
					 if(order->getRemainingQuantity() >0){
						  insertIntoBook(order,bids_);
					 }
					 return t;
				}else{
					 Trades t = matchLimitOrder(order,bids_);
					 if(order->getRemainingQuantity() > 0){
						  insertIntoBook(order,asks_);
					 }
					 return t;
				}
				break;		
		  case OrderType::GoodTillCancel:
				if(order->getSide() == Side::BUY){
					 Trades t = matchLimitOrder(order,asks_);
					 if(order->getRemainingQuantity() >0){
						  insertIntoBook(order,bids_);
					 }
					 return t;
				}else{
					 Trades t = matchLimitOrder(order,bids_);
					 if(order->getRemainingQuantity() > 0){
						  insertIntoBook(order,asks_);
					 }
					 return t;
				}
				break;
		  //done 
		  case OrderType::PostOnly:
				if(order->getSide() == Side::BUY){
					 insertIntoBook(order,bids_);
					 return Trades{};
				}else{
					 insertIntoBook(order,asks_);
					 return Trades{};
				}
				break;
		  case OrderType::FillAndKill:
				if(order->getSide() == Side::BUY){
					 return matchLimitOrder(order,asks_);
				}
				else{
					 return matchLimitOrder(order,bids_);
				}
				break;

		  case OrderType::FillOrKill:
				if(order->getSide() == Side::BUY){
					 return FOK(order,asks_);
				}
				else{
					 return FOK(order,bids_);
				}
				break;
		  case OrderType::Limit:
				if(order->getSide() == Side::BUY){
					 return matchLimitOrder(order,asks_);
				}else{
					return matchLimitOrder(order,bids_);
				}
				break;
	 }
	 return Trades{};
}

Trades OrderBook::placeOrder(MarketOrderPtr order){
	 if(order->getSide() == Side::BUY){
				return matchMarketOrder(order,asks_);
	 }
	 return matchMarketOrder(order,bids_);
}
/*
void OrderBook::runBook(){
	 while(true){
		  
	 }
}
*/
