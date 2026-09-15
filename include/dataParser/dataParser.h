#pragma once
#include <string>
#include <vector>
#include "orderbook/orderbook.h"
class DataParser{
public:
	 static void handleStream(OrderBook& book, std::string& path);
private:
	 static OrderPtr createOrder(const std::vector<std::string>& words);
    static std::vector<std::string> splitLine(const std::string& line, char delim);
	 static MarketOrderPtr createMarketOrder(const std::vector<std::string>& line);
	 static void modifyOrderPrice(const std::vector<std::string> &line,OrderBook& book);
	 static void modifyOrderQuantity(const std::vector<std::string> &line,OrderBook& book);
};
