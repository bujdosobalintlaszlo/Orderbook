#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "orderbook/order.h"
#include "orderbook/market.h"
#include "orderbook/orderbook.h"
#include "orderbook/ordertype.h"
#include "orderbook/side.h"

namespace {
void counters(benchmark::State& state, std::int64_t batch,
              std::int64_t completed, std::int64_t failed = 0) {
    const double ops = static_cast<double>(batch) * completed;
    state.counters["operations/batch"] = static_cast<double>(batch);
    state.counters["total operations"] = ops;
    state.counters["Mops/s"] = benchmark::Counter(
        ops / 1e6, benchmark::Counter::kIsRate);
    state.counters["timed_ms"] = benchmark::Counter(
        0.001, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    state.counters["mod_false"] = static_cast<double>(failed);
}

struct Event {
    int action = 0;
    OrderId id;
    OrderType type = OrderType::PostOnly;
    Side side = Side::SELL;
    Price price = 0;
    Quantity quantity = 0;
    Date date = 0;
    Symbol symbol;
};

struct Input {
    std::vector<Event> events;
    std::size_t skipped = 0;
    std::size_t adds = 0, priceMods = 0, quantityMods = 0;
    std::string error;
};

// Benchmark-local parsing only. No changes to DataParser's private methods.
std::uint64_t number(const std::string& value) {
    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument("Expected unsigned integer");
    return std::stoull(value);
}

bool supportedType(OrderType type) {
    switch (type) {
        case OrderType::Market:
        case OrderType::GoodForDay:
        case OrderType::GoodTillCancel:
        case OrderType::PostOnly:
        case OrderType::FillAndKill:
        case OrderType::FillOrKill:
        case OrderType::Limit:
            return true;
        default: return false;
    }
}

const Input& csvInput() {
    static const Input input = [] {
        Input result;
        const char* overridePath = std::getenv("ORDERBOOK_CSV");
        const std::string path = overridePath ? overridePath :
            std::string(PROJECT_ROOT) + "/src/dataParser/orders.csv";
        std::ifstream file(path);
        if (!file) {
            result.error = "Cannot open CSV: " + path;
            return result;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            try {
                std::vector<std::string> fields;
                std::istringstream stream(line);
                std::string field;
                while (std::getline(stream, field, ',')) fields.push_back(field);
                if (!line.empty() && line.back() == ',') fields.emplace_back();
                if (fields.size() != 8 || fields[0].empty())
                    throw std::invalid_argument("Expected 8 fields and an ID");
                const auto action = number(fields[7]);
                if (action > 2) throw std::invalid_argument("Unknown action");
                Event e;
                e.action = static_cast<int>(action);
                e.id = fields[0];
                if (e.action == 0) {
                    const auto type = number(fields[1]);
                    const auto side = number(fields[2]);
                    bool typeFound = false;
                    for (const auto t : {OrderType::Market, OrderType::GoodForDay,
                         OrderType::GoodTillCancel, OrderType::PostOnly,
                         OrderType::FillAndKill, OrderType::FillOrKill,
                         OrderType::Limit}) {
                        if (type == static_cast<std::uint64_t>(t)) {
                            e.type = t;
                            typeFound = true;
                            break;
                        }
                    }
                    if (!typeFound || !supportedType(e.type))
                        throw std::invalid_argument("Unknown order type");
                    if (side == static_cast<std::uint64_t>(Side::BUY)) e.side = Side::BUY;
                    else if (side == static_cast<std::uint64_t>(Side::SELL)) e.side = Side::SELL;
                    else throw std::invalid_argument("Unknown side");
                    if (e.type != OrderType::Market) e.price = number(fields[3]);
                    e.quantity = number(fields[4]);
                    e.date = number(fields[5]);
                    e.symbol = fields[6];
                } else {
                    if (e.action == 1) e.price = number(fields[3]);
                    else e.quantity = number(fields[4]);
                    e.date = number(fields[5]);
                }
                result.events.push_back(std::move(e));
                if (action == 0) ++result.adds;
                else if (action == 1) ++result.priceMods;
                else ++result.quantityMods;
            } catch (const std::invalid_argument&) {
                ++result.skipped;
            } catch (const std::out_of_range&) {
                ++result.skipped;
            }
        }
        if (file.bad()) result.error = "Read failed: " + path;
        if (result.events.empty() && result.error.empty()) result.error = "CSV has no valid events";
        std::cerr << "CSV: " << path << "\nValid events: " << result.events.size()
                  << " | adds: " << result.adds << " | price modifications: "
                  << result.priceMods << " | quantity modifications: "
                  << result.quantityMods << " | skipped rows: " << result.skipped << '\n';
        return result;
    }();
    return input;
}
} // namespace

static void BM_OrderAdd(benchmark::State& state) {
    const auto count = state.range(0);
    std::int64_t completed = 0;
    for (auto _ : state) {
        state.PauseTiming();
        bool valid = false;
        {
            OrderBook book;
            std::vector<OrderPtr> orders;
            orders.reserve(static_cast<std::size_t>(count));
            for (std::int64_t i = 0; i < count; ++i)
                orders.push_back(std::make_unique<Order>(
                    std::to_string(i), OrderType::PostOnly, Side::SELL,
                    static_cast<Price>(100 + i % 100), Quantity{1}, Date{0}, Symbol{"TEST"}));
            state.ResumeTiming();
            for (auto& order : orders) {
                auto trades = book.placeOrder(std::move(order));
                benchmark::DoNotOptimize(trades.data());
            }
            benchmark::ClobberMemory();
            state.PauseTiming();
            std::size_t stored = 0;
            for (const auto& level : book.getAsks()) stored += level.second.size();
            valid = stored == static_cast<std::size_t>(count);
        }
        state.ResumeTiming();
        if (!valid) { state.SkipWithError("Unexpected stored order count"); break; }
        ++completed;
    }
    counters(state, count, completed);
}

static void BM_OrderModifyQuantity(benchmark::State& state) {
    const auto count = state.range(0);
    std::int64_t completed = 0;
    for (auto _ : state) {
        state.PauseTiming();
        std::int64_t successful = 0;
        {
            OrderBook book;
            std::vector<OrderId> ids;
            ids.reserve(static_cast<std::size_t>(count));
            for (std::int64_t i = 0; i < count; ++i) {
                ids.push_back(std::to_string(i));
                book.placeOrder(std::make_unique<Order>(
                    ids.back(), OrderType::PostOnly, Side::SELL,
                    static_cast<Price>(100 + i % 100), Quantity{1}, Date{0}, Symbol{"TEST"}));
            }
            state.ResumeTiming();
            for (const auto& id : ids) {
                const bool ok = book.modifyOrderQuantity(id, Quantity{2});

                successful += ok;
            }
            benchmark::ClobberMemory();
            state.PauseTiming();
        }
        state.ResumeTiming();
        if (successful != count) { state.SkipWithError("Quantity modification failed"); break; }
        ++completed;
    }
    counters(state, count, completed);
}

static void BM_MixedCsvReplay(benchmark::State& state) {
    const auto& input = csvInput(); // Parsed once, outside timed iterations.
    if (!input.error.empty()) { state.SkipWithError(input.error.c_str()); return; }
    std::int64_t completed = 0, failedMods = 0;
    for (auto _ : state) {
        state.PauseTiming();
        {
            OrderBook book;
            std::vector<OrderPtr> limits;
            std::vector<MarketOrderPtr> markets;
            limits.reserve(input.adds);
            markets.reserve(input.adds);
            // Fresh instances each replay: placement consumes unique_ptrs.
            for (const auto& e : input.events) {
                if (e.action != 0) continue;
                if (e.type == OrderType::Market)
                    markets.push_back(std::make_unique<Market>(
                        e.id, e.type, e.side, e.quantity, e.date, e.symbol));
                else
                    limits.push_back(std::make_unique<Order>(
                        e.id, e.type, e.side, e.price, e.quantity, e.date, e.symbol));
            }
            std::size_t limitIndex = 0, marketIndex = 0;
            state.ResumeTiming();
            for (const auto& e : input.events) {
                if (e.action == 0) {
                    auto trades = e.type == OrderType::Market
                        ? book.placeOrder(std::move(markets[marketIndex++]))
                        : book.placeOrder(std::move(limits[limitIndex++]));
                    benchmark::DoNotOptimize(trades.data());
                } else {
                    const bool ok = e.action == 1
                        ? book.modifyOrderPrice(e.id, e.price)
                        : book.modifyOrderQuantity(e.id, e.quantity);
                    failedMods += !ok;
                }
            }
            benchmark::ClobberMemory();
            state.PauseTiming();
        }
        state.ResumeTiming();
        ++completed;
    }
    counters(state, static_cast<std::int64_t>(input.events.size()), completed, failedMods);
    state.SetLabel("actual CSV order; parsing/construction excluded");
}

BENCHMARK(BM_OrderAdd)->Arg(1000)->Arg(10000)
    ->Unit(benchmark::kSecond)->UseRealTime();
BENCHMARK(BM_OrderModifyQuantity)->Arg(1000)->Arg(10000)
    ->Unit(benchmark::kSecond)->UseRealTime();
BENCHMARK(BM_MixedCsvReplay)->Unit(benchmark::kSecond)->UseRealTime();

BENCHMARK_MAIN();
