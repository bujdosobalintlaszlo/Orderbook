#pragma once
#include <string>
#include <vector>
#include <istream>
#include "orderbook/orderbook.h"

class DataParser {
private:
	 static int parseInt(const std::string& value);
    static OrderPtr createOrder(const std::vector<std::string>& words);
    static std::vector<std::string> splitLine(const std::string& line, char delim);
    static MarketOrderPtr createMarketOrder(const std::vector<std::string>& line);
    static void modifyOrderPrice(const std::vector<std::string>& line, OrderBook& book);
    static void modifyOrderQuantity(const std::vector<std::string>& line, OrderBook& book);

public:
	 static void handleStream(OrderBook& book, const std::string& path);
};
