#include <future>
#include <iostream>

#include "game.hpp"
#include "solver.hpp"

int main(int argc, char** argv) {
  std::cin.tie(0);
  std::ios::sync_with_stdio(false);
  
  const auto starting_time = std::chrono::steady_clock::now();
  // const auto core_size = static_cast<int>(std::thread::hardware_concurrency());

  hexagonal_walk::read_question();

  auto result = [&]() {
    hexagonal_walk::depth_first_search depth_first_search;
    auto depth_first_search_future = std::async(
      std::launch::async,
      [&]() {
        return depth_first_search();
      });

    hexagonal_walk::fattening fattening;
    auto fattening_future = std::async(
      std::launch::async,
      [&]() {
        return fattening();
      });

    depth_first_search_future.wait_until(starting_time + std::chrono::milliseconds(500));
    depth_first_search.stop();

    auto depth_first_search_result = depth_first_search_future.get();
    if (!depth_first_search_result.empty()) {
      fattening.stop();
      fattening_future.get();

      hexagonal_walk::_answer = std::move(depth_first_search_result);
      hexagonal_walk::write_answer();
        
      std::exit(0);
    }

    fattening_future.wait_until(starting_time + std::chrono::milliseconds(500));
    fattening.stop();

    return fattening_future.get();
  }();

  hexagonal_walk::_answer = std::move(result);
  hexagonal_walk::write_answer();
  
  return 0;
}
