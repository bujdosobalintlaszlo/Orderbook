#include<gtest/gtest.h>
#include "orderbook/orderbook.h"
#include "orderbook/ordertype.h"
#include "orderbook/order.h"
#include "orderbook/trade.h"
#include "orderbook/market.h"
#include <gmock/gmock.h> 
using ::testing::AllOf;
using ::testing::Field;
using ::testing::ElementsAre;

class OrderBookTest : public ::testing::Test{
protected:
	 OrderBook ob;
	 //___ HELPERS FOR TESTING ___
	 bool checkTradeVals(Trades expected, Trades output) const{
		  if(expected.size() != output.size()) return false;
		  for(size_t i{0};i<expected.size();++i){
				if(expected[i].getBidTrade() != output[i].getBidTrade() || expected[i].getAskTrade() != output[i].getAskTrade()){
					 return false;
				}
		  }
		  return true;
	 }
};
TEST_F(OrderBookTest,CreatingBookWithOrders){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 670000,1655716096498,"APPL");
	 Trades trades = ob.placeOrder(std::make_unique<Order>(o));
	 ASSERT_EQ(trades.size(),0);
	 ASSERT_EQ(ob.getBids().size(),1);
}

TEST_F(OrderBookTest,AdddingRedundantIdsToTheBook){
	 Order o1("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 670000,1655716096498,"APPL");
	 Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098",OrderType::PostOnly,Side::BUY,6700000,670000,1655716096502,"APPL");
	 Trades trades1 = ob.placeOrder(std::make_unique<Order>(o1));
	 Trades trades2 = ob.placeOrder(std::make_unique<Order>(o2));
	 ASSERT_EQ(trades1.size(),0);
	 ASSERT_EQ(trades2.size(),0);
	 EXPECT_EQ(ob.getBids().size(),1);
}

TEST_F(OrderBookTest,CancelValidOrder){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 670000,1655716096498,"APPL");
	 //no needs to store trades
	 ob.placeOrder(std::make_unique<Order>(o));
	 bool succesfullDel = ob.cancelOrder("e2a85d9f-07a5-4f94-8d5f-789dc3deb097");
	 ASSERT_TRUE(succesfullDel);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest, CancelOnEmptyBook){
    bool succesfulDel = ob.cancelOrder("e2a85d9f-07a5-4f94-8d5f-789dc3deb097");
    ASSERT_FALSE(succesfulDel);
    EXPECT_EQ(ob.getBids().size(), 0);
    EXPECT_EQ(ob.getAsks().size(), 0);
}

TEST_F(OrderBookTest,CancelInvalidOrder){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 670000,1655716096498,"APPL");
	 //no need to store trades
	 ob.placeOrder(std::make_unique<Order>(o));
	 bool succesfulDel = ob.cancelOrder("e2a85d9f-07a5-4f94-8d5f-789dc3deb099");
	 ASSERT_FALSE(succesfulDel);
	 EXPECT_EQ(ob.getBids().size(),1);
}

TEST_F(OrderBookTest,LimitOrderTest){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097",OrderType::PostOnly,Side::SELL,66000,670000,1655716096498,"APPL");
	 Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098",OrderType::Limit,Side::BUY,670000,670000,1655716096700,"APPL");
	 ob.placeOrder(std::make_unique<Order>(o));
	 Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 std::cout << trade.size() << '\n';
	 ASSERT_EQ(trade.size(),1);
}

TEST_F(OrderBookTest,LimitOrderFail){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097",OrderType::PostOnly,Side::SELL,6700800,67,1655716096498,"APPL");
	 Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098",OrderType::Limit,Side::BUY,670000,67,1655716097498,"APPL");
	 ob.placeOrder(std::make_unique<Order>(o));
	 Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 ASSERT_EQ(trade.size(),0);
}

TEST_F(OrderBookTest, LimitOrderPartialFill){
    Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 6700800, 67,1655716097498,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::Limit, Side::BUY, 6700800, 40,1655716097598,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
    EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,27)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_, 0)
            ))
        )
    ));

}

TEST_F(OrderBookTest,MarketOrderMatch){
    Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 67,1655716097498,"APPL");
    Market o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098",OrderType::Market,Side::BUY, 40,1655716097698,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Market>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,27)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_, 0)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),1);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest,MarketOrderPartialFill){
    Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 7,1655716097498,"APPL");
    Market o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098",OrderType::Market,Side::BUY, 40,1655716097598,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Market>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,33)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
	 
}

TEST_F(OrderBookTest,MarketOrderCantFill){
    Market o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098",OrderType::Market,Side::BUY, 40,1655716097498,"APPL");
    Trades trade = ob.placeOrder(std::make_unique<Market>(o2));
	 ASSERT_EQ(trade.size(),0);
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
	 
}

TEST_F(OrderBookTest,FillAndKillOrderMatch){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 7,1655716097498,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::FillAndKill, Side::BUY, 67008000000, 7,1655716097898,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,0)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest,FillAndKillCantMatch){
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::FillAndKill, Side::BUY, 67008000000, 7,16557160697498,"APPL");
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 ASSERT_EQ(trade.size(),0); 
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest,FillAndKillPartialFill){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 6,1655716097898,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::FillAndKill, Side::BUY, 67008000000, 7,1655716097899,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,1)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest,FillOrKillCanMatch){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 6,1655716097898,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::FillOrKill, Side::BUY, 67008000000, 6,1655726097898,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,0)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest, FillOrKillCanMatchMultipleOrder) {
    Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 3,1655716097498,"APPL");
    Order o3("e2a85d9f-07a5-4f94-8d5f-789dc3deb099", OrderType::PostOnly, Side::SELL, 67008000000, 3,1655716097499,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::FillOrKill, Side::BUY, 67008000000, 6,1655716097410,"APPL");

    ob.placeOrder(std::make_unique<Order>(o));
    ob.placeOrder(std::make_unique<Order>(o3));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 std::cout << "trade size: " << trade.size() << '\n'; 
    EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_, 0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_, 3)
            ))
        ),
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb099"),
                Field(&TradeInfo::quantity_, 0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_, 0)
            ))
        )
    ));

    EXPECT_EQ(ob.getAsks().size(), 0);
    EXPECT_EQ(ob.getBids().size(), 0);
}
TEST_F(OrderBookTest,FillOrKillCantMatch){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 3,1655716097498,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::FillOrKill, Side::BUY, 67008000000, 6,1658716097498,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 ASSERT_EQ(trade.size(),0); 
	 EXPECT_EQ(ob.getAsks().size(),1);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest,GoodTillCancelInsertFilledBeforeInBook){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 6,1655716097498,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::GoodTillCancel, Side::BUY, 67008000000, 6,1655716099498,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,0)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest,GoodTillCancelPartialFill){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 6,1655716097498,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::GoodTillCancel, Side::BUY, 67008000000, 12,1665716097498,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,6)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),1);
}

TEST_F(OrderBookTest,GoodTillCancelFillWithMultipleOrder){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 6,1655716097898,"APPL");
    Order o2("e2a85d9f-07a5-4f94-8d5f-789dc3deb098", OrderType::GoodTillCancel, Side::BUY, 67008000000, 12,1655816097898,"APPL");
    Order o3("e2a85d9f-07a5-4f94-8d5f-789dc3deb099", OrderType::PostOnly, Side::SELL, 67008000000, 6,1655716098898,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
    ob.placeOrder(std::make_unique<Order>(o3));
    Trades trade = ob.placeOrder(std::make_unique<Order>(o2));
	 EXPECT_THAT(trade, ElementsAre(
        AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb097"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,6)
            ))
        ),
		  AllOf(
            Property(&Trade::getAskTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb099"),
                Field(&TradeInfo::quantity_,0)
            )),
            Property(&Trade::getBidTrade, AllOf(
                Field(&TradeInfo::orderId_, "e2a85d9f-07a5-4f94-8d5f-789dc3deb098"),
                Field(&TradeInfo::quantity_,0)
            ))
        )
    ));
	 EXPECT_EQ(ob.getAsks().size(),0);
	 EXPECT_EQ(ob.getBids().size(),0);
}

TEST_F(OrderBookTest,GoodTillCancelCantFill){
	 Order o("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::SELL, 67008000000, 6,1655716097898,"APPL");
    ob.placeOrder(std::make_unique<Order>(o));
	 EXPECT_EQ(ob.getAsks().size(),1);
	 EXPECT_EQ(ob.getBids().size(),0);
}


TEST_F(OrderBookTest,WorkFlow1){

}

TEST_F(OrderBookTest,WorkFlow2){

}

TEST_F(OrderBookTest,WorkFlow3){

}

TEST_F(OrderBookTest,WorkFlow4){

}
