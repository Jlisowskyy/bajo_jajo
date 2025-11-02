#ifndef ALGOS_HPP
#define ALGOS_HPP

#include <State.hpp>
#include <graph.hpp>

NODISCARD Mapping HungarianLike(const Graph &g1, const Graph &g2);
NODISCARD Mapping AccurateBruteForce(const Graph &g1, const Graph &g2);
NODISCARD Mapping AccurateAStar(const Graph &g1, const Graph &g2);
NODISCARD Mapping Mcts(const Graph &g1, const Graph &g2);

#endif  // ALGOS_HPP
