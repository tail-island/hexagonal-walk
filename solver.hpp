#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>

#include "game.hpp"

namespace hexagonal_walk {
  inline const auto point(const std::vector<std::uint16_t>& indice) {
    return boost::accumulate(indice | boost::adaptors::transformed([](const auto& index) { return _points[index]; }), 0);
  }
  
  class fattening {
    std::atomic<bool> _stop;

  public:
    fattening()
      : _stop(false)
    {
      ;
    }

    const auto operator()(const std::vector<std::uint16_t>& indice) {
      std::vector<std::uint16_t> result; result.reserve(_tiles.size());
      result = indice;

      return result;
    }

    const auto operator()() {
      std::vector<std::uint16_t> result; result.reserve(_tiles.size());
      
      result.emplace_back(_start_index);
      for (const auto& adjacency_index : _adjacencies[_start_index]) {
        if (_points[adjacency_index] == 1) {
          result.emplace_back(adjacency_index);
          break;
        }
      }
      result.emplace_back(_start_index);

      return (*this)(result);
    }
    
    const auto stop() {
      _stop = true;
    }
  };
  
  class depth_first_search {
    std::atomic<bool> _stop;
    std::vector<std::uint16_t> _result;
    int _result_point;

    const auto compute(const std::vector<std::uint16_t>& indice, const std::uint64_t& indice_bitset, const std::uint8_t& point_capacity) {
      if (indice_bitset & static_cast<std::uint64_t>(1) << _start_index) {
        const auto indice_point = point(indice);
        if (indice_point > _result_point) {
          _result = indice;
          _result_point = indice_point;
        }
        
        return;
      }

      if (_stop) {
        return;
      }

      for (const auto& next_index : _adjacencies[indice.back()]) {
        const auto next_index_bit = static_cast<std::uint64_t>(1) << next_index;

        if (indice_bitset & next_index_bit) {
          continue;
        }

        const auto next_index_point = _points[next_index];

        if (next_index_point > point_capacity) {
          continue;
        }

        std::vector<std::uint16_t> next_indice(indice.size() + 1);
        next_indice = indice;
        next_indice.emplace_back(next_index);

        compute(next_indice, indice_bitset | next_index_bit, std::max<std::uint8_t>(point_capacity, next_index_point + 1));
      }
    }

  public:
    depth_first_search()
      : _stop(false), _result()
    {
      ;
    }

    const auto operator()() {
      if (_tiles.size() <= 64) {
        compute(std::vector<std::uint16_t>{_start_index}, static_cast<std::uint64_t>(0), 1);
      }

      if (_stop) {
        _result.clear();
      }

      return _result;
    }

    const auto stop() {
      _stop = true;
    }
  };
}
