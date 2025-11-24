#ifndef ALGOS_HPP
#define ALGOS_HPP

#include "State.hpp"
#include "graph.hpp"

#include <vector>

// Structure to hold detailed information about edge mismatches
struct EdgeExtension {
    Vertex u;             // G1 source
    Vertex v;             // G1 target
    Vertex mapped_u;      // G2 source
    Vertex mapped_v;      // G2 target
    Edges weight_needed;  // Weight in G1
    Edges weight_found;   // Weight in G2
};

// Computes the specific edges required to make the mapping valid
NODISCARD std::vector<EdgeExtension> GetMinimalEdgeExtension(const Graph &g1, const Graph &g2, const Mapping &mapping);
NODISCARD Graph GetMinimalExtension(const Graph &g1, const Graph &g2, const Mapping &mapping);

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
    return ApproxAStar5(g1, g2, k);
}

#endif  // ALGOS_HPP
