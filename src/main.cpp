#include <iostream>
#include "orderbook/order.h"
#include <string> 
#include "dataParser/dataParser.h"
#include "orderbook/orderbook.h"
#include <chrono>


int main() {
    std::string path = std::string(PROJECT_ROOT) + "/" + "src/dataParser/orders.csv";
    OrderBook book;

    auto start = std::chrono::high_resolution_clock::now();

    DataParser::handleStream(book, path);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << "Engine ran in: " << duration.count() << " ms" << std::endl;
    return 0;
}
