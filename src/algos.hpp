#ifndef ALGOS_HPP
#define ALGOS_HPP

#include <graph.hpp>

NODISCARD Graph HungarianLike(Graph g1, Graph g2);
NODISCARD Graph AccurateBruteForce(Graph g1, Graph g2);
NODISCARD Graph AccurateAStar(Graph g1, Graph g2);
NODISCARD Graph Mcts(Graph g1, Graph g2);

#endif  // ALGOS_HPP
