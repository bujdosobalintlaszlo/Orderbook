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
#include <stdexcept>
#include <fstream>

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
		  throw;
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
	 OrderId id = line.at(0);
	 Quantity newQuantity = stoull(line.at(4));
	 Date date = stoull(line.at(5));
	 book.modifyOrderQuantity(id,newQuantity);
}
int DataParser::parseInt(const std::string& value) {
    std::size_t consumed = 0;
    const int result = std::stoi(value, &consumed);

    if (consumed != value.size()) {
        throw std::invalid_argument("Invalid integer: '" + value + "'");
    }

    return result;
}

void DataParser::handleStream(OrderBook& book, const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        try {
            const std::vector<std::string> data = splitLine(line, ',');

            if (data.empty() || data.back().empty()) {
                throw std::invalid_argument("Missing action ID");
            }

            const int actionId = parseInt(data.back());
            switch (actionId) {
                case 0: {
                    if (data.size() < 3) {
                        throw std::invalid_argument(
                            "Too few fields for placing an order");
                    }

                    const int typeId = parseInt(data.at(1));

                    if (typeId == static_cast<int>(OrderType::Market)) {
                        book.placeOrder(createMarketOrder(data));
                    } else {
                        book.placeOrder(createOrder(data));
                    }
                    break;
                }

                case 1:
                    modifyOrderPrice(data, book);
                    break;

                case 2:
                    modifyOrderQuantity(data, book);
                    break;

                default:
                    throw std::invalid_argument(
                        "Unknown action ID: " + std::to_string(actionId));
            }
        } catch (const std::exception& error) {
					 std::cerr << path << ":" << lineNumber
              << ": skipping row: " << error.what() << '\n';
		  continue;
	 }
    }

    if (file.bad()) {
        throw std::runtime_error("Failed while reading file: " + path);
    }
}/*
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
