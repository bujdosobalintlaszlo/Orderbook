#include <sstream>
#include <iostream>
#include <memory>
#include "dataParser/dataParser.h"
#include "orderbook/types.h"
#include "orderbook/ordertype.h"
#include "orderbook/order.h" 
#include "orderbook/market.h" 
#include "orderbook/ordertype.h"
#include "orderbook/orderbook.h"
#include<string>
#include <fstream>
/*rework idea:
methodId(placeorder,modifyPrice,modifyAmount),
*/
std::vector<std::string> DataParser::splitLine(const std::string& line, char delim){
    std::vector<std::string> words;
    std::stringstream s(line);
    std::string word;
    while(std::getline(s, word, delim)){
        words.push_back(word);
    }
    return words;
}

OrderPtr DataParser::createOrder(const std::vector<std::string>& words){
    try{
		  OrderId id = words.at(0);
		  OrderType orderType = static_cast<OrderType>(std::stoi(words.at(1)));
		  Side side = static_cast<Side>(std::stoi(words.at(2)));
		  Price price = std::stoull(words.at(3));
		  Quantity quantity = std::stoull(words.at(4));
		  Date date = std::stoull(words.at(5));
		  Symbol symbol = words.at(6);
		  return std::make_unique<Order>(id, orderType, side, price, quantity,date,symbol);
    }catch(const std::exception& e){
        std::cerr << "Failed to parse line: " << e.what() << '\n';
        return nullptr;
    }
	 return nullptr;
}

MarketOrderPtr DataParser::createMarketOrder(const std::vector<std::string>& words){
	 try{
		  OrderId id = words.at(0);
		  OrderType orderType = static_cast<OrderType>(std::stoi(words.at(1)));
		  Side side = static_cast<Side>(std::stoi(words.at(2)));
		  Quantity quantity = std::stoull(words.at(4));
		  Date date = std::stoull(words.at(5));
		  Symbol symbol = words.at(6);
		  return std::make_unique<Market>(id, orderType, side, quantity,date,symbol);
    }catch(const std::exception& e){
        std::cerr << "Failed to parse line: " << e.what() << '\n';
        return nullptr;
    }
	 return nullptr;
}

void DataParser::modifyOrderPrice(const std::vector<std::string> &line,OrderBook& book){
	 OrderId id = line.at(0);
	 Price newPrice = stoull(line.at(3));
	 Date date = stoull(line.at(5));
	 book.modifyOrderPrice(id,newPrice);
}

void DataParser::modifyOrderQuantity(const std::vector<std::string> &line,OrderBook& book){
	 std::cout << "mod quant" << " " << line.size() << '\n';
	 OrderId id = line.at(0);
	 Quantity newQuantity = stoull(line.at(4));
	 Date date = stoull(line.at(5));
	 book.modifyOrderQuantity(id,newQuantity);
}
void DataParser::handleStream(OrderBook& book, std::string& path){
	 std::ifstream f(path);
    if(!f.is_open()){
        std::cerr << "Failed to open file: " << path << '\n';
		  throw;
    }
	 std::string line;
	 //mod line: modId,orderId,amount,date
	 while(std::getline(f,line)){
		  std::vector<std::string> data = splitLine(line,',');
		  try{	
				std::cout << "------START OF A ORDER-------" << '\n';
				//std::cout << "action_id bef parse" << "" << data.size() << " " << data.back() << '\n';
				ModId action_id = std::stoi(data.back());
				std::cout << "action_id af parse" << '\n';
				std::cout << line << '\n';
				std::cout << '\n';
				switch(action_id){
					 //must add {} so the compiler knows that orderTypes lifetime ends in case 0
					 case 0:{
						  OrderType orderType = static_cast<OrderType>(std::stoi(data.at(1)));
						  if(orderType == OrderType::Market){
								std::cout << "Sub market" << '\n';
								book.placeOrder(createMarketOrder(data));
						  }else{
								std::cout << "Sub other" << '\n';
								book.placeOrder(createOrder(data));
						  }
						  break;
					 }
					 case 1:{
						  modifyOrderPrice(data,book);
						  break;
					 }
					 case 2:{
						  modifyOrderQuantity(data,book);
						  break;
					 }
				}
		  book.displayAsks(); 
		  book.displayBids(); 
		  std::cout << "-------------" << '\n';
		  }catch(...){
				throw;
		  } 
	 }
	 
}
/*
{
  "id": "e2a85d9f-07a5-4f94-8d5f-789dc3deb097",
  "method": "order.place",
  "params": {
    "symbol": "BTCUSDT",
    "side": "BUY",
    "type": "LIMIT",
    "price": "0.1",
    "quantity": "10",
    "timeInForce": "GTC",
    "timestamp": 1655716096498,
    "apiKey": "T59MTDLWlpRW16JVeZ2Nju5A5C98WkMm8CSzWC4oqynUlTm1zXOxyauT8LmwXEv9",
    "signature": "5942ad337e6779f2f4c62cd1c26dba71c91514400a24990a3e7f5edec9323f90"
  }
}
*/
