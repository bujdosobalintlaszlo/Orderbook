// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <chrono>                   // for operator""s, chrono_literals
//#include <ftxui/screen/screen.hpp>  // for Full, Screen
#include <iostream>                 // for cout, ostream
#include <memory>                   // for allocator, shared_ptr
#include <string>                   // for string, operator<<
#include <thread>                   // for sleep_for
 
//#include "ftxui/dom/elements.hpp"  // for hflow, paragraph, separator, hbox, vbox, filler, operator|, border, Element
//#include "ftxui/dom/node.hpp"      // for Render
//#include "ftxui/screen/box.hpp"    // for ftxui
#include "dataParser/dataParser.h"
//using namespace std::chrono_literals;
#include "orderbook/orderbook.h"
#include "orderbook/order.h"
#include "orderbook/market.h"
#include <chrono>

int main(){
    std::string path = std::string(PROJECT_ROOT) + "/" + "src/dataParser/orders_100.csv";
    OrderBook book;
	 DataParser::handleStream(book,path);


    return 0;
}
