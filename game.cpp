#include "game.hpp"

std::vector<hexagonal_walk::tile> hexagonal_walk::_tiles;
std::vector<std::uint8_t> hexagonal_walk::_points;

std::vector<std::vector<std::uint16_t>> hexagonal_walk::_adjacencies;
std::uint16_t hexagonal_walk::_start_index;
std::vector<std::uint16_t> hexagonal_walk::_distances;
