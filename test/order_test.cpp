#include<gtest/gtest.h>
#include"orderbook/order.h"
TEST(OrderTestSuite,CreateValidObject){
	 Order order("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 67,1655716096498,"APPL");
	 EXPECT_EQ(order.getId(),"e2a85d9f-07a5-4f94-8d5f-789dc3deb097");
	 EXPECT_EQ(order.getOrderType(),OrderType::PostOnly);
	 EXPECT_EQ(order.getSide(),Side::BUY);
	 EXPECT_EQ(order.getPrice(),16767);
	 EXPECT_EQ(order.getInitialQuantity(),67);
	 EXPECT_EQ(order.filledQuantity(),0);
	 EXPECT_FALSE(order.isFilled());
	 EXPECT_EQ(order.getRemainingQuantity(),order.getInitialQuantity());
}

TEST(OrderTestSuite, CreateOrderWithZeroQuantity) {
    try {
        Order order("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 0,1655716096498,"APPL");
        FAIL() << "Expected std::invalid_argument for zero quantity!";
    } catch (const std::invalid_argument &e) {
        EXPECT_STREQ(e.what(), "Order quantity must be positive! Order ID: e2a85d9f-07a5-4f94-8d5f-789dc3deb097 with QUANTITY: 0");
    } catch (...) {
        FAIL() << "Expected std::invalid_argument, but caught a different exception!";
    }
}

TEST(OrderTestSuite,CreateOrderWithInvalidPrice){
	 try{
        Order order("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 0, 16732,1655716096498,"APPL");
		  FAIL() << "Expected a std::invalid_argument but got nothing!";
	 }catch(const std::invalid_argument &e){
		  EXPECT_STREQ(e.what(),"Order price must be positive! Order ID: e2a85d9f-07a5-4f94-8d5f-789dc3deb097 with PRICE: 0");
	 }catch(...){
		  FAIL() << "Didn't result in std::invalid_argument!";
	 }
}

TEST(OrderTestSuite,ValidPartialOrderFill){
	 Order order("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 10,1655716096498,"APPL");
	 order.fill(8);
	 EXPECT_EQ(order.getId(),"e2a85d9f-07a5-4f94-8d5f-789dc3deb097");
	 EXPECT_EQ(order.getOrderType(),OrderType::PostOnly);
	 EXPECT_EQ(order.getSide(),Side::BUY);
	 EXPECT_EQ(order.getPrice(),16767);
	 EXPECT_EQ(order.getInitialQuantity(),10);
	 EXPECT_EQ(order.getRemainingQuantity(),2);
	 EXPECT_EQ(order.filledQuantity(),8);
	 EXPECT_FALSE(order.isFilled());
	 EXPECT_EQ(order.getFufillmentOfOrder(),80.0);
}

TEST(OrderTestSuite,ValidFullOrderFill){
	 Order order("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 382910,1655716096498,"APPL");
	 order.fill(382910);
	 EXPECT_EQ(order.getId(),"e2a85d9f-07a5-4f94-8d5f-789dc3deb097");
	 EXPECT_EQ(order.getOrderType(),OrderType::PostOnly);
	 EXPECT_EQ(order.getSide(),Side::BUY);
	 EXPECT_EQ(order.getPrice(),16767);
	 EXPECT_EQ(order.getInitialQuantity(),382910);
	 EXPECT_EQ(order.getRemainingQuantity(),0);
	 EXPECT_EQ(order.filledQuantity(),382910);
	 EXPECT_TRUE(order.isFilled());
	 EXPECT_EQ(order.getFufillmentOfOrder(),100.0);
}

TEST(OrderTestSuite,InvalidOrderFill){
	 try{
		  Order order("e2a85d9f-07a5-4f94-8d5f-789dc3deb097", OrderType::PostOnly, Side::BUY, 16767, 1,1655716096498,"APPL");
		  order.fill(67);
		  FAIL() << "Expected std::invalid_argument exception but got no exception!";
	 }catch(const std::invalid_argument &e){
		  EXPECT_STREQ(e.what(),"Tried to fill more than remaining quantity!");
	 }catch(...){
		  FAIL() << "Didn't result in std::invalid_argument!";
	 }
}

int main(int argc,char** argv){
	 testing::InitGoogleTest(&argc,argv);
	 return RUN_ALL_TESTS();
}
