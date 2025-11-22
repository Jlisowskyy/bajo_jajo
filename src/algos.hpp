#ifndef ALGOS_HPP
#define ALGOS_HPP

#include "State.hpp"
#include "graph.hpp"

#include <vector>

NODISCARD std::vector<Mapping> AccurateBruteForce(const Graph &g1, const Graph &g2, int k);
NODISCARD std::vector<Mapping> AccurateAStar(const Graph &g1, const Graph &g2, int k);
NODISCARD std::vector<Mapping> ApproxAStar(const Graph &g1, const Graph &g2, int k);
NODISCARD std::vector<Mapping> ApproxAStar5(const Graph &g1, const Graph &g2, int k);

NODISCARD inline std::vector<Mapping> Accurate(const Graph &g1, const Graph &g2, const int k)
{
    return AccurateAStar(g1, g2, k);
}

NODISCARD inline std::vector<Mapping> Approximate(const Graph &g1, const Graph &g2, const int k)
{
    return ApproxAStar(g1, g2, k);
}

#endif  // ALGOS_HPP
