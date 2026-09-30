cmake_minimum_required(VERSION 3.28)
find_package(benchmark REQUIRED)

add_executable(orderbook_benchmark tests/benchmark.cpp)
target_link_libraries(orderbook_benchmark PRIVATE orderbook_lib benchmark::benchmark)
set(This orderbookTests)	
