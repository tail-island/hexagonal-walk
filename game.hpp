#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <vector>

#include <boost/container/static_vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/numeric.hpp>

namespace hexagonal_walk {
  class tile {
    union {
      const std::uint8_t _values[2];
      const std::uint16_t _value;
    };

  public:
    tile(const std::uint8_t& x, const std::uint8_t& y) noexcept
      : _values{y, x}
    {
      ;
    }

    const auto& x() const noexcept {
      return _values[1];
    }

    const auto& y() const noexcept {
      return _values[0];
    }

    const auto around_tiles() const noexcept {
      return std::array<tile, 6>{tile(x() + 1, y()), tile(x() + 1, y() - 1), tile(x(), y() - 1), tile(x() - 1, y()), tile(x() - 1, y() + 1), tile(x(), y() + 1)};
    }

    const auto operator==(const tile& other) const noexcept {
      return _value == other._value;
    }

    friend std::hash<hexagonal_walk::tile>;
  };

  inline std::ostream& operator<<(std::ostream& stream, const tile& tile) {
    return stream << static_cast<int>(tile.x()) << "," << static_cast<int>(tile.y());
  }

  class cubed_tile {
    const int _cube_x;
    const int _cube_z;
    const int _cube_y;

  public:
    cubed_tile(const tile& tile) noexcept
      : _cube_x(tile.x()), _cube_z(tile.y()), _cube_y(0 - _cube_x - _cube_z)
    {
      ;
    }

    int distance(const cubed_tile& other) const noexcept {
      return (std::abs(_cube_x - other._cube_x) + std::abs(_cube_y - other._cube_y) + std::abs(_cube_z - other._cube_z)) / 2;
    }
  };
}

namespace std {
  template <>
  struct hash<hexagonal_walk::tile> {
    std::size_t operator()(const hexagonal_walk::tile& tile) const {
      return tile._value;
    }
  };
}

namespace hexagonal_walk {
  extern std::vector<tile> _tiles;
  extern std::vector<std::uint8_t> _points;

  extern std::vector<boost::container::static_vector<std::uint16_t, 6>> _adjacencies;
  extern std::uint16_t _start_index;
  extern std::vector<std::uint16_t> _distances;

  inline const auto read_question() noexcept {
    auto set_adjacencies = []() {
      const auto indice_map = boost::copy_range<std::unordered_map<tile, std::uint16_t>>(
        _tiles |
        boost::adaptors::indexed() |
        boost::adaptors::transformed(
          [](const auto& indexed_tile) {
            return std::make_pair(indexed_tile.value(), indexed_tile.index());
          }));

      // vector<vector>だとメモリが連続しないので、CPUのキャッシュにのる可能性が減る……ような気がします。vector<static_vector>に変更してみます。
      // _adjacencies = boost::copy_range<std::vector<std::vector<std::uint16_t>>>(
      //   _tiles |
      //   boost::adaptors::transformed(
      //     [&](const auto& tile) {
      //       auto result = boost::copy_range<std::vector<std::uint16_t>>(
      //         tile.around_tiles() |
      //         boost::adaptors::transformed(
      //           [&](const auto& around_tile) {
      //             return indice_map.find(around_tile);
      //           }) |
      //         boost::adaptors::filtered(
      //           [&](const auto& indice_map_it) {
      //             return indice_map_it != std::end(indice_map);
      //           }) |
      //         boost::adaptors::transformed(
      //           [&](const auto& indice_map_it) { return indice_map_it->second; }));
      //
      //       boost::sort(result);
      //
      //       return result;
      //     }));

      _adjacencies = std::vector<boost::container::static_vector<std::uint16_t, 6>>(_tiles.size());

      // static_vectorはムーブができません。だから、直接構築してみます。
      for (const auto& indexed_tile : _tiles | boost::adaptors::indexed()) {
        for (const auto& around_tile : indexed_tile.value().around_tiles()) {
          const auto& indice_map_it = indice_map.find(around_tile);
          if (indice_map_it == std::end(indice_map)) {
            continue;
          }
          _adjacencies[indexed_tile.index()].emplace_back(indice_map_it->second);
        }

        boost::sort(_adjacencies[indexed_tile.index()]);
      }
    };

    auto set_start_index = []() {
      _start_index = std::distance(std::begin(_points), boost::find(_points, 0));
    };

    auto set_distances = []() {
      const cubed_tile start_cubed_tile(_tiles[_start_index]);

      _distances = boost::copy_range<std::vector<std::uint16_t>>(
        _tiles |
        boost::adaptors::transformed(
          [&](const auto& tile) {
            return static_cast<std::uint16_t>(start_cubed_tile.distance(cubed_tile(tile)));
          }));
    };

    _tiles.reserve(20000);
    _points.reserve(20000);

    while (std::cin) {
      int x, y, point; char comma;
      std::cin >> x >> comma >> y >> comma >> point;

      if (std::cin.fail()) {
        continue;
      }

      _tiles.emplace_back(tile(x, y));
      _points.emplace_back(point);
    }

    // ポイント面で到達不可能（2がないのに3がある等）なタイルを除去します。
    {
      const auto max_point = []() {
        const auto point_set = boost::copy_range<std::unordered_set<std::uint8_t>>(_points);

        for (auto i = static_cast<std::uint8_t>(2); i < point_set.size(); ++i) {
          if (point_set.find(i) == std::end(point_set)) {
            return i;
          }
        }

        return static_cast<std::uint8_t>(point_set.size());
      }();

      std::vector<tile> tiles; tiles.reserve(_tiles.size());
      std::vector<std::uint8_t> points; points.reserve(_points.size());
      for (auto i = 0; i < static_cast<int>(_tiles.size()); ++i) {
        if (_points[i] > max_point) {
          continue;
        }

        tiles.emplace_back(_tiles[i]);
        points.emplace_back(_points[i]);
      }

      _tiles = std::move(tiles);
      _points = std::move(points);
    }

    set_adjacencies();
    set_start_index();

    // 明らかに到達不可能なタイルを除去します。
    {
      boost::dynamic_bitset<> connected_indice_bitset(_tiles.size());
      connected_indice_bitset[_start_index] = true;

      std::vector<std::uint16_t> oneway_entrance_indice; oneway_entrance_indice.reserve(_tiles.size());

      // とりあえず、一方通行が「含まれない」エリアを探します。
      {
        std::queue<std::uint16_t> queue;
        for (const auto& adjacency_index : _adjacencies[_start_index]) {
          queue.emplace(adjacency_index);
          connected_indice_bitset[adjacency_index] = true;
        }

        while (!queue.empty()) {
          const auto index = queue.front(); queue.pop();

          for (const auto& adjacency_index : _adjacencies[index]) {
            if (connected_indice_bitset[adjacency_index]) {
              continue;
            }

            bool can_return = [&]() {
              auto connected_count = 0;

              for (const auto& adjacency_adjacency_index : _adjacencies[adjacency_index]) {
                if (connected_indice_bitset[adjacency_adjacency_index]) {
                  if (++connected_count >= 2) {
                    return true;
                  }
                }
              }

              return false;
            }();

            if (!can_return) {
              oneway_entrance_indice.emplace_back(adjacency_index);
              continue;
            }

            connected_indice_bitset[adjacency_index] = true;

            queue.emplace(adjacency_index);
          }
        }
      }

      // 2箇所以上で繋がっていれば、一方通行の先を救済します。その先に一方通行があっても、計算コストが高いので無視して取り込みます。
      {
        for (const auto& oneway_entrance_index : oneway_entrance_indice) {
          if (connected_indice_bitset[oneway_entrance_index]) {
            continue;
          }

          std::stack<std::uint16_t> stack;
          stack.emplace(oneway_entrance_index);

          boost::dynamic_bitset<> maybe_connected_indice_bitset(_tiles.size());
          maybe_connected_indice_bitset[oneway_entrance_index] = true;

          auto connected_count = 0;
          while (!stack.empty()) {
            const auto index = stack.top(); stack.pop();

            for (const auto& adjacency_index : _adjacencies[index]) {
              if (maybe_connected_indice_bitset[adjacency_index]) {
                continue;
              }

              if (connected_indice_bitset[adjacency_index]) {
                connected_count++;
                continue;
              }

              maybe_connected_indice_bitset[adjacency_index] = true;

              stack.emplace(adjacency_index);
            }
          }

          if (connected_count >= 2) {
            connected_indice_bitset |= maybe_connected_indice_bitset;
          }
        }
      }

      std::vector<tile> tiles; tiles.reserve(_tiles.size());
      std::vector<std::uint8_t> points; points.reserve(_points.size());
      for (auto i = 0; i < static_cast<int>(_tiles.size()); ++i) {
        if (!connected_indice_bitset[i]) {
          continue;
        }

        tiles.emplace_back(_tiles[i]);
        points.emplace_back(_points[i]);
      }

      _tiles = std::move(tiles);
      _points = std::move(points);
    }

    set_adjacencies();
    set_start_index();
    set_distances();
  }

  template <typename T>
  inline const auto point(const T& indice) noexcept {
    return boost::accumulate(
      indice |
      boost::adaptors::transformed(
        [](const auto& index) {
          return _points[index];
        }),
      0);
  }

  template <typename T>
  inline const auto write_answer(const T& indice) noexcept {
    for (const auto& index : indice) {
      std::cout << _tiles[index] << std::endl;
    }
  }
}
