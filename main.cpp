#include <future>
#include <iostream>

#include "game.hpp"
#include "solver.hpp"

int main(int argc, char** argv) {
  const auto starting_time = std::chrono::steady_clock::now();

  std::cin.tie(0);
  std::ios::sync_with_stdio(false);
  
  hexagonal_walk::read_question();

  const auto result_1 = [&]() {
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

    hexagonal_walk::beam_search beam_search;
    auto beam_search_future = std::async(
      std::launch::async,
      [&]() {
        return beam_search();
      });
    
    depth_first_search_future.wait_until(starting_time + std::chrono::milliseconds(200));
    depth_first_search.stop();
    auto depth_first_search_result = depth_first_search_future.get();
    
    if (!depth_first_search_result.empty()) {
      fattening.stop();
      fattening_future.get();

      beam_search.stop();
      beam_search_future.get();

      hexagonal_walk::write_answer(depth_first_search_result);
      std::exit(0);
    }

    fattening_future.wait_until(starting_time + std::chrono::milliseconds(500));
    fattening.stop();
    auto fattening_result = fattening_future.get();
    
    if (fattening_result.size() > 500) {
      beam_search.stop();
      beam_search_future.get();

      return fattening_result;
    }

    beam_search_future.wait_until(starting_time + std::chrono::milliseconds(3000));
    beam_search.stop();
    auto beam_search_result = beam_search_future.get();
    
    return std::max(fattening_result, beam_search_result, [](const auto& result_1, const auto& result_2) { return hexagonal_walk::point(result_1) < hexagonal_walk::point(result_2); });
  }();

  if (result_1.size() == hexagonal_walk::_tiles.size() + 1) {
    hexagonal_walk::write_answer(result_1);
    std::exit(0);
  }

  const auto result_2 = [&]() {
    hexagonal_walk::fattening fattening;
    auto fattening_future = std::async(
      std::launch::async,
      [&]() {
        return fattening(result_1);
      });

    hexagonal_walk::local_search local_search_1;
    auto local_search_1_future = std::async(
      std::launch::async,
      [&]() {
        return local_search_1(result_1);
      });

    hexagonal_walk::local_search local_search_2;
    auto local_search_2_future = std::async(
      std::launch::async,
      [&]() {
        return local_search_2(result_1);
      });

    hexagonal_walk::local_search local_search_3;
    auto local_search_3_future = std::async(
      std::launch::async,
      [&]() {
        return local_search_3(result_1);
      });

    fattening_future.wait_until(starting_time + std::chrono::milliseconds(4200));
    fattening.stop();
    auto fattening_result = fattening_future.get();

    local_search_1_future.wait_until(starting_time + std::chrono::milliseconds(4200));
    local_search_1.stop();
    auto local_search_1_result = local_search_1_future.get();

    local_search_2_future.wait_until(starting_time + std::chrono::milliseconds(4200));
    local_search_2.stop();
    auto local_search_2_result = local_search_2_future.get();

    local_search_3_future.wait_until(starting_time + std::chrono::milliseconds(4200));
    local_search_3.stop();
    auto local_search_3_result = local_search_3_future.get();

    return std::max({fattening_result, local_search_1_result, local_search_2_result, local_search_3_result}, [](const auto& result_1, const auto& result_2) { return hexagonal_walk::point(result_1) < hexagonal_walk::point(result_2); });
  }();

  if (result_2.size() == hexagonal_walk::_tiles.size() + 1) {
    hexagonal_walk::write_answer(result_2);
    std::exit(0);
  }

  const auto result_3 = [&]() {
    const auto changeable_indice = hexagonal_walk::maybe_visitable_indice(result_2);

    hexagonal_walk::local_search local_search_1;
    auto local_search_1_future = std::async(
      std::launch::async,
      [&]() {
        return local_search_1(result_2, changeable_indice);
      });

    hexagonal_walk::local_search local_search_2;
    auto local_search_2_future = std::async(
      std::launch::async,
      [&]() {
        return local_search_2(result_2, changeable_indice);
      });

    hexagonal_walk::local_search local_search_3;
    auto local_search_3_future = std::async(
      std::launch::async,
      [&]() {
        return local_search_3(result_2, changeable_indice);
      });

    hexagonal_walk::local_search local_search_4;
    auto local_search_4_future = std::async(
      std::launch::async,
      [&]() {
        return local_search_4(result_2, changeable_indice);
      });
    
    local_search_1_future.wait_until(starting_time + std::chrono::milliseconds(4600));
    local_search_1.stop();
    auto local_search_1_result = local_search_1_future.get();

    local_search_2_future.wait_until(starting_time + std::chrono::milliseconds(4600));
    local_search_2.stop();
    auto local_search_2_result = local_search_2_future.get();

    local_search_3_future.wait_until(starting_time + std::chrono::milliseconds(4600));
    local_search_3.stop();
    auto local_search_3_result = local_search_3_future.get();

    local_search_4_future.wait_until(starting_time + std::chrono::milliseconds(4600));
    local_search_4.stop();
    auto local_search_4_result = local_search_4_future.get();

    return std::max({local_search_1_result, local_search_2_result, local_search_3_result, local_search_4_result}, [](const auto& result_1, const auto& result_2) { return hexagonal_walk::point(result_1) < hexagonal_walk::point(result_2); });
   }();
  
  hexagonal_walk::write_answer(result_3);
  std::exit(0);
  
  return 0;
}
