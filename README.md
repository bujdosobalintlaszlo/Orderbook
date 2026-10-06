# Orderbook / Matching Engine

This project implements an order book and matching engine supporting the following order types:

- Fill and Kill
- Fill or Kill
- Post Only
- Limit
- Market
- Good Till Cancel
- Good for a Day *(not implemented yet)*

## Complexity

Let **P** be the number of price levels in the order book.

- **Add resting order:** O(log P) to locate or create its price level, followed by O(1) insertion into the price level's order list.
- **Order lookup:** O(1) average using an `unordered_map`.
- **Cancel:** O(1) average lookup and O(1) list removal. Removing an empty price level may additionally require O(log P).
- **Modify quantity:** O(1) average when the order remains at the same price level.
- **Modify price:** O(log P), as the order must be moved to another price level.
- **Matching:** Depends on the number of resting orders and price levels consumed by the incoming order. An incoming order may match against multiple resting orders, so the complete matching operation cannot generally be described as O(1).

# Functionality

> **Note:** The input `.csv` files are currently not included in the repository. I'm working on a solution so anyone can easily provide input and demo the project themselves.

### Running the program

<img width="1715" height="471" alt="Orderbook program output" src="https://github.com/user-attachments/assets/4aa1d099-4374-40de-8bc3-af4b51f523fe" />

The program processes and matches orders from the provided input. After processing, it displays the execution time and the resulting order book, separated into bid and ask sides.

### Benchmark

<img width="1631" height="268" alt="Orderbook benchmark results" src="https://github.com/user-attachments/assets/c2007c18-3126-455b-9a87-35ec88c6dfd7" />

Benchmarks are available for insertions, modifications, and mixed workloads. Parsing is excluded from these benchmarks to measure the performance of the matching engine itself.

# Structure

The project separates the parsing and order book components, with declarations and implementations split between header and `.cpp` files. Tests are kept separately, and the project is built using CMake.

# Technical Approach

## Order Book

The order book uses ordered maps for both the ask and bid sides. Each map key represents a price, with its corresponding value representing a price level.

Orders within each price level are stored in a linked list. Orders are owned using `unique_ptr`, providing explicit single ownership and automatic lifetime management without requiring manual memory deallocation.

Because the price levels are stored in ordered maps, locating or creating a price level takes O(log P), where P is the number of price levels. Once the price level has been located, inserting an order into its linked list takes O(1).

Insertion, cancellation, and modification are supported by an additional `unordered_map`. Its keys are order IDs, while its values contain information about where each order is stored, including an iterator pointing directly to the order within its price-level list.

This allows an order to be located in O(1) average time without iterating through the book. The stored iterator also allows the order to be removed directly from its list or moved when necessary.

### Future improvements

- Add a map where symbols are keys and order books are values. This would separate books by symbol and eliminate the need to check symbols during matching.
- Implement the Good for a Day order type.

## Orders

Orders are primarily represented by a single class. I chose this approach because the different order types share most of their data and behavior, and using a class hierarchy felt unnecessary for the current design.

Instead, an enum is used to identify the order type, avoiding the runtime and design overhead associated with virtual dispatch.

Market orders are currently an exception and use a separate struct because they do not have a price. This was the simplest solution for the current implementation without significantly complicating the general order representation.

### Future improvements

- Benchmark and evaluate the overhead and design trade-offs of representing different order types using derived classes.
- Explore representing market orders through a class invariant or a union-based representation.

## Parsing

The parser currently supports `.csv` input files. It parses comma-separated order data and reports when a row is skipped because it contains an invalid or malformed value.

## Tests

Test coverage is still a work in progress. The current tests were primarily written during development and focus on the order and order book components.

Both common scenarios and edge cases are tested using Google Test.
