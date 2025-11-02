#ifndef RANDOM_GEN_HPP
#define RANDOM_GEN_HPP

#include <cstdint>
#include <tuple>
#include "graph.hpp"

struct GraphSpec {
    std::uint32_t size_g1;
    std::uint32_t size_g2;
    double density_g1;
    double density_g2;
    bool create_g1_based_on_g2;
};

std::tuple<Graph, Graph> GenerateExample(GraphSpec spec);

#endif  // RANDOM_GEN_HPP
