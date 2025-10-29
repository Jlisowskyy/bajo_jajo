#ifndef RANDOM_GEN_HPP
#define RANDOM_GEN_HPP

#include <cstdint>
#include <tuple>
#include "graph.hpp"

std::tuple<Graph, Graph> GenerateExample(
    std::uint32_t size_g1, std::uint32_t size_g2, float density_g1, float density_g2, bool create_g1_based_on_g2
);

#endif  // RANDOM_GEN_HPP
