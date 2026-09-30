#include <benchmark/benchmark.h>
#include <filesystem>
#include <string>

#include "orderbook/orderbook.h"
#include "dataParser/dataParser.h"

static void BM_OrderBook_FullPipeline(benchmark::State& state) {
    std::string originalPath = std::string(PROJECT_ROOT) + "/" + "src/dataParser/orders.csv";
    std::string ramPath = "/dev/shm/orders_temp.csv";
    
    try {
        if (std::filesystem::exists(originalPath)) {
            std::filesystem::copy_file(originalPath, ramPath, std::filesystem::copy_options::overwrite_existing);
        } else {
            ramPath = originalPath;
        }
    } catch (...) {
        ramPath = originalPath;
    }

    for (auto _ : state) {
        OrderBook book;
        DataParser::handleStream(book, ramPath);
    }

    if (std::filesystem::exists("/dev/shm/orders_temp.csv")) {
        std::filesystem::remove("/dev/shm/orders_temp.csv");
    }
}
BENCHMARK(BM_OrderBook_FullPipeline);

BENCHMARK_MAIN();
