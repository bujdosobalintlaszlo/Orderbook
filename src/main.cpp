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
//int main() {
/*
  using namespace ftxui;
  std::string p =
      R"(In probability theory and statistics, Bayes' theorem (alternatively Bayes' law or Bayes' rule) describes the probability of an event, based on prior knowledge of conditions that might be related to the event. For example, if cancer is related to age, then, using Bayes' theorem, a person's age can be used to more accurately assess the probability that they have cancer, compared to the assessment of the probability of cancer made without knowledge of the person's age. One of the many applications of Bayes' theorem is Bayesian inference, a particular approach to statistical inference. When applied, the probabilities involved in Bayes' theorem may have different probability interpretations. With the Bayesian probability interpretation the theorem expresses how a subjective degree of belief should rationally change to account for availability of related evidence. Bayesian inference is fundamental to Bayesian statistics.)";
 
  std::string reset_position;
  while (true) {
    auto document = vbox({
                        hflow(paragraph(p)),
                        separator(),
                        hflow(paragraph(p)),
                        separator(),
                        hbox({
                            hflow(paragraph(p)),
                            separator(),
                            hflow(paragraph(p)),
                        }),
                    }) |
                    border;
 
    document = vbox(filler(), document);
 
    // auto screen = Screen::Create(Dimension::Fit(document));
    // Render(screen, document);
    // screen.Print();
    // getchar();
 
    auto screen = Screen::Create(Dimension::Full());
    Render(screen, document);
    std::cout << reset_position;
    screen.Print();
    reset_position = screen.ResetPosition();
 
    std::this_thread::sleep_for(0.01s);
  }
 */

int main(){
    std::string path = std::string(PROJECT_ROOT) + "/" + "src/dataParser/orders.csv";
    OrderBook book;
//    std::chrono::nanoseconds totalMatchTime{0};
//	 std::cout << "Give the input file's path: " << '\n';
	 DataParser::handleStream(book,path);
	 /*
    while(std::getline(f, line)){
        if(line.empty()) continue;

        std::vector<std::string> data = DataParser::splitLine(line, ',');
		  if(data.size() == 9){
				ModId mod = std::stoi(data.at(0));
				switch(mod){
					 case 0:
						  book.modifyOrderPrice(stoi());
				}
		  }else{

		  }
        OrderType ot = static_cast<OrderType>(std::stoi(data.at(1)));

        Trades trades;
        if(ot == OrderType::Market){
            MarketOrderPtr m = DataParser::createMarketOrder(line);
            if(!m) continue;

            auto start = std::chrono::high_resolution_clock::now();
            trades = book.placeOrder(std::move(m));
            auto end = std::chrono::high_resolution_clock::now();
            totalMatchTime += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        } else {
            OrderPtr order = DataParser::createOrder(line);
            if(!order) continue;

            auto start = std::chrono::high_resolution_clock::now();
            trades = book.placeOrder(std::move(order));
            auto end = std::chrono::high_resolution_clock::now();
            totalMatchTime += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        }
        orderCount++;
    }
	 */

//    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(totalMatchTime).count();
    //std::cout << "Orders processed: " << orderCount << '\n';
  //  std::cout << "Total match time: " << ms << " ms\n";
    //std::cout << "Avg per order: " 
    //           << (totalMatchTime.count() / (double)orderCount) << " ns\n";

    return 0;
}
