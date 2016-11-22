#pragma once

#include <atomic>
#include <cstdint>
#include <queue>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/numeric.hpp>

#include "game.hpp"

namespace hexagonal_walk {
  class beam_search {
    std::atomic<bool> _stop;
    std::unordered_set<std::size_t> _searched_hashes;  // ハッシュ値そのものをキーにしているので、たまたまハッシュが一致して正解を逃す危険性があります。……が、気にしません。

    class game_state {
      std::vector<std::uint16_t> _indice;
      boost::dynamic_bitset<> _indice_bitset;
      std::uint16_t _point_capacity;
      float _score;

    public:
      game_state(std::vector<std::uint16_t>&& indice, boost::dynamic_bitset<>&& indice_bitset, const std::uint16_t& point_capacity, const float& score)
        : _indice(indice), _indice_bitset(indice_bitset), _point_capacity(point_capacity), _score(score)
      {
        ;
      }

      const auto& indice() const {
        return _indice;
      }

      const auto& indice_bitset() const {
        return _indice_bitset;
      }

      const auto& point_capacity() const {
        return _point_capacity;
      }

      const auto is_goaled() const {
        return _indice_bitset[_start_index];
      }

      const auto operator<(const game_state& other) const {
        return _score < other._score;
      }
    };

    const auto maybe_returnable(const game_state& game_state, const std::uint16_t& next_index) const {
      std::priority_queue<std::tuple<std::uint16_t, std::uint16_t>> queue;
      queue.emplace(UINT16_MAX - _distances[next_index], next_index);  // priority_queueは、大きい順です。だから、UINT16_MAXから距離を引いて、ゴールに近い順に処理します。

      boost::dynamic_bitset<> indice_bitset(game_state.indice_bitset());
      indice_bitset[next_index] = true;

      int size = 0;
      while (!queue.empty() && ++size <= 200) {  // どうせ長い手は解けないので、一定の数で探索を諦めます。
        const auto index = std::get<1>(queue.top()); queue.pop();

        if (index == _start_index) {
          return true;
        }

        for (const auto& adjacency_index : _adjacencies[index]) {
          if (indice_bitset[adjacency_index]) {
            continue;
          }
          indice_bitset[adjacency_index] = true;

          queue.emplace(UINT16_MAX - _distances[adjacency_index], adjacency_index);
        }
      }

      return false;
    }

    const auto score(const std::vector<std::uint16_t>& indice, const boost::dynamic_bitset<>& indice_bitset) const {
      return
        boost::accumulate(
          indice |
          boost::adaptors::transformed(
            [&](const auto& index) {
              return static_cast<float>(6 - _adjacencies[index].size()) / 6;  // 周囲のタイルの数が少ないところ（＝端）を攻めます。
            }),
          0.0f) +
        boost::accumulate(
          indice |
          boost::adaptors::transformed(
            [&](const auto& index) {
              return
                static_cast<float>(boost::count_if(  // 周囲の探索済みタイルの数。
                                     _adjacencies[index],
                                     [&](const auto& adjacency_index) {
                                       return indice_bitset[adjacency_index];
                                     })) /
                _adjacencies[index].size();          // 周囲のタイルの数。
            }),
          0.0f);
    }

    const auto next_game_states(const game_state& game_state) {
      std::vector<beam_search::game_state> result;
      result.reserve(6);

      std::size_t indice_bitset_hash = 0;
      boost::hash_combine(indice_bitset_hash, boost::hash_value(game_state.indice_bitset().m_bits));

      for (const auto& next_index : _adjacencies[game_state.indice().back()]) {
        if (game_state.indice_bitset()[next_index]) {
          continue;
        }

        const auto next_index_point = _points[next_index];

        if (next_index_point > game_state.point_capacity()) {
          continue;
        }

        std::size_t hash = indice_bitset_hash;
        boost::hash_combine(hash, std::hash<std::uint16_t>()(next_index));

        if (!_searched_hashes.emplace(hash).second) {
          continue;
        }

        if (!maybe_returnable(game_state, next_index)) {
          continue;
        }

        std::vector<std::uint16_t> next_indice(game_state.indice().size() + 1);
        next_indice = game_state.indice();
        next_indice.emplace_back(next_index);

        boost::dynamic_bitset<> next_indice_bitset(game_state.indice_bitset());
        next_indice_bitset[next_index] = true;

        result.emplace_back(
          std::move(next_indice),
          std::move(next_indice_bitset),
          std::max<std::uint16_t>(game_state.point_capacity(), next_index_point + 1),
          score(next_indice, next_indice_bitset));
      }

      return result;
    }

  public:
    beam_search()
      : _stop(false), _searched_hashes(400000)
    {
      ;
    }

    const auto operator()() {
      std::vector<std::uint16_t> result;

      std::priority_queue<game_state> queue;
      queue.emplace(std::vector<std::uint16_t>{_start_index}, boost::dynamic_bitset<>(_tiles.size()), 1, 0.0f);

      while (!queue.empty() && !_stop) {
        std::priority_queue<game_state> next_queue;

        for (auto i = 0; i < 300 && !queue.empty(); ++i) {
          const auto game_state = queue.top(); queue.pop();

          if (game_state.is_goaled()) {
            result = std::move(game_state.indice());
            continue;
          }

          for (const auto& next_game_state : next_game_states(game_state)) {
            next_queue.emplace(std::move(next_game_state));
          }
        }

        queue = std::move(next_queue);
      }

      return result;
    }

    const auto stop() {
      _stop = true;
    }
  };

  class local_search {
    std::atomic<bool> _stop;

    const auto node(const std::vector<std::uint16_t>& indice) const {
      std::vector<std::uint16_t> node(_tiles.size());

      std::random_device random_device;
      std::default_random_engine rand(random_device());

      std::unordered_map<std::uint16_t, std::uint16_t> indice_map(indice.size());
      for (auto i = 0; i < static_cast<int>(indice.size()) - 1; ++i) {
        indice_map.emplace(indice[i], indice[i + 1]);
      }

      for (auto i = 0; i < static_cast<int>(_tiles.size()); ++i) {
        const auto& it = indice_map.find(i);
        if (it != std::end(indice_map)) {
          node[i] = it->second;
        } else {
          node[i] = _adjacencies[i][rand() % _adjacencies[i].size()];
        }
      }

      return node;
    }

    const auto path(const std::vector<std::uint16_t>& node) const {
      std::vector<std::uint16_t> indice;
      indice.reserve(_tiles.size() + 1);
      indice.emplace_back(_start_index);

      boost::dynamic_bitset<> indice_bitset(_tiles.size());

      std::uint16_t point_capacity = 1;

      for (auto index = node[_start_index]; ; index = node[index]) {
        if (indice_bitset[index]) {
          break;
        }

        auto next_index_point = _points[index];

        if (next_index_point > point_capacity) {
          break;
        }

        indice.emplace_back(index);
        indice_bitset[index] = true;
        point_capacity = std::max<std::uint16_t>(point_capacity, next_index_point + 1);
      }

      return indice;
    }

    const auto cycle(const std::vector<std::uint16_t>& path) const {
      if (path.front() == path.back()) {
        return path;
      }

      return std::vector<std::uint16_t>{_start_index};
    }

    const auto score(const std::vector<std::uint16_t>& node) const {
      const auto path = local_search::path(node);
      const auto cycle = local_search::cycle(path);

      return point(cycle) + static_cast<int>(cycle.size()) + static_cast<int>(path.size()) * 3;
    }

    const auto compute(const std::vector<std::uint16_t>& initial_node, const std::vector<std::uint16_t>& changeable_indice) const {
      std::random_device random_device;
      std::default_random_engine rand(random_device());

      auto answer_node = initial_node;
      auto answer_node_point = point(cycle(path(initial_node)));
      auto best_score = score(initial_node);
      auto node = initial_node;
      auto staying_count = 0;

      while (staying_count++ < 30000 && !_stop) {
        // コピー作成のコストを回避した結果、一つのデータに対して修正と復帰を繰り返すわかりづらいコードになってしまいました……。

        std::vector<std::uint16_t> next_node;
        auto next_node_score = 0;

        for (auto i = 0; i < std::min<int>(changeable_indice.size() * 3, 120); ++i) {
          std::unordered_map<std::uint16_t, std::uint16_t> original_values(120);  // 同じ箇所が複数回変更された場合にも元の値を保持するために、mapを使用します。

          for (auto j = 0; j < 3; ++j) {
            auto index = changeable_indice[rand() % changeable_indice.size()];

            original_values.emplace(index, node[index]);
            node[index] = _adjacencies[index][rand() % _adjacencies[index].size()];
          }

          const auto node_score = score(node);
          if (node_score > next_node_score) {
            next_node_score = node_score;
            next_node = node;
          }

          for (const auto original_value : original_values) {
            node[original_value.first] = original_value.second;
          }
        }

        if (next_node_score > best_score) {
          best_score = next_node_score;
          staying_count = 0;
        }

        const auto next_node_point = point(cycle(path(next_node)));
        if (next_node_point > answer_node_point) {
          answer_node = next_node;
          answer_node_point = next_node_point;
        }

        node = std::move(next_node);
      }

      return answer_node;
    }

  public:
    local_search()
      : _stop(false)
    {
      ;
    }

    const auto operator()(const std::vector<std::uint16_t>& indice, const std::vector<std::uint16_t>& changeable_indice) {
      if (changeable_indice.empty()) {
        return indice;
      }

      return cycle(path(compute(node(indice), changeable_indice)));
    }

    const auto operator()(const std::vector<std::uint16_t>& indice) {
      std::vector<std::uint16_t> changeable_indice;
      changeable_indice.reserve(_tiles.size());

      for (auto i = 0; i < static_cast<int>(_tiles.size()); ++i) {
        changeable_indice.emplace_back(i);
      }

      return (*this)(indice, changeable_indice);
    }

    const auto stop() {
      _stop = true;
    }
  };

  inline auto indice_bitset(const std::vector<std::uint16_t>& indice) {
    boost::dynamic_bitset<> result(_tiles.size());
    boost::for_each(
      indice,
      [&](const auto& index) {
        result[index] = true;
      });

    return result;
  }

  inline auto maybe_visitable_indice(const std::vector<std::uint16_t>& indice) {
    std::vector<std::uint16_t> result;
    result.reserve(_tiles.size());

    const auto& indice_bitset = hexagonal_walk::indice_bitset(indice);

    for (const auto& index : indice) {
      if ([&]() {
          for (const auto& adjacency_index : _adjacencies[index]) {
            if (!indice_bitset[adjacency_index]) {
              return true;
            }
          }

          return false;
        }())
      {
        boost::copy(_adjacencies[index], std::back_inserter(result));
        result.emplace_back(index);
      }
    }

    boost::erase(result, boost::unique<boost::return_found_end>(boost::sort(result)));

    return result;
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
      std::vector<std::uint16_t> result;
      result.reserve(_tiles.size() + 1);
      result = indice;

      auto result_bitset = hexagonal_walk::indice_bitset(indice);

      auto inserted_index = 0;
      auto inserted = true;
      while (inserted && !_stop) {
        inserted = false;

        auto point_capacity = 1;
        for (auto i = 0; i < static_cast<int>(result.size()) - 1; ++i) {
          if (_points[result[i]] == point_capacity) {
            ++point_capacity;
          }

          if (i < inserted_index) {
            continue;
          }

          const auto& adjacency_1 = _adjacencies[result[i]];
          const auto& adjacency_2 = _adjacencies[result[i + 1]];

          auto it_1 = std::begin(adjacency_1);
          auto it_2 = std::begin(adjacency_2);

          while (it_1 != std::end(adjacency_1) && it_2 != std::end(adjacency_2)) {
            if (*it_1 < *it_2) {
              ++it_1;
              continue;
            }

            if (*it_2 < *it_1) {
              ++it_2;
              continue;
            }

            if (!result_bitset[*it_1] && _points[*it_1] <= point_capacity) {
              result.insert(std::begin(result) + i + 1, *it_1);
              result_bitset[*it_1] = true;

              if (!inserted) {
                inserted_index = i;
              }
              inserted = true;

              ++i;
              break;
            }

            ++it_1;
            ++it_2;
          }
        }
      }

      return result;
    }

    const auto operator()() {
      std::vector<std::uint16_t> indice;
      indice.reserve(3);

      indice.emplace_back(_start_index);
      for (const auto& adjacency_index : _adjacencies[_start_index]) {
        if (_points[adjacency_index] == 1) {
          indice.emplace_back(adjacency_index);
          break;
        }
      }
      indice.emplace_back(_start_index);

      return (*this)(indice);
    }

    const auto stop() {
      _stop = true;
    }
  };

  class depth_first_search {  // 単純なフィールドでの速度勝負に対応するために、素の深さ有線探索を追加しました。。。
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
      : _stop(false), _result(), _result_point(0)
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
