#include "algos.hpp"
#include "mcts.hpp"

#include <cassert>
#include <vector>

struct BestMap {
    BestMap(const size_t size_g1, const size_t size_g2) : mapping(size_g1, size_g2) {}
    std::int32_t cost{};
    std::int32_t best_cost{INT32_MAX};
    Mapping mapping;
};

// ------------------------------
// Hungarian
// ------------------------------

NODISCARD Mapping HungarianLike(const Graph &g1, const Graph &g2) {}

// ------------------------------
// Brute force
// ------------------------------

static void BruteForce_(
    const Graph &g1, const Graph &g2, Mapping &mapping, BestMap &best_map, std::vector<bool> &used,
    const std::int32_t depth
)
{
    if (depth == g1.GetVertices()) {
        if (best_map.cost < best_map.best_cost) {
            best_map.best_cost = best_map.cost;
            best_map.mapping   = mapping;
        }
        return;
    }

    if (best_map.cost >= best_map.best_cost) {
        /* cutoff */
        return;
    }

    for (std::int32_t candidate = 0; candidate < g2.GetVertices(); ++candidate) {
        if (used[candidate])
            continue;

        mapping.set_mapping(depth, candidate);
        used[candidate] = true;

        // Calculate cost of this mapped vertex
        std::int32_t local_cost{};
        if (depth != 0) {
            g1.IterateEdges(
                [&](const std::uint32_t edges, const std::uint32_t u, const std::uint32_t v) {
                    if (v > depth || u > depth)
                        return;
                    const std::int32_t mapped_u = mapping.get_mapping_g1_to_g2(u);
                    const std::int32_t mapped_v = mapping.get_mapping_g1_to_g2(v);
                    assert(mapped_v != -1);
                    assert(mapped_u != -1);

                    if (const std::uint32_t edges_count = g2.GetEdges(mapped_u, mapped_v); edges_count < edges) {
                        local_cost += edges - edges_count;
                    }
                },
                depth
            );
        }

        best_map.cost += local_cost;
        BruteForce_(g1, g2, mapping, best_map, used, depth + 1);
        best_map.cost -= local_cost;

        used[candidate] = false;
        mapping.remove_mapping_g1(depth);
    }
}

NODISCARD Mapping AccurateBruteForce(const Graph &g1, const Graph &g2)
{
    Mapping map(g1.GetVertices(), g2.GetVertices());
    BestMap best_map(g1.GetVertices(), g2.GetVertices());

    std::vector used(g2.GetVertices(), false);
    BruteForce_(g1, g2, map, best_map, used, 0);
    return best_map.mapping;
}

// ------------------------------
// A star
// ------------------------------

NODISCARD Mapping AccurateAStar(const Graph &g1, const Graph &g2) {}

// ------------------------------
// Mcts
// ------------------------------

NODISCARD Mapping Mcts(const Graph &g1, const Graph &g2)
{
    static constexpr int kNumIters = 1'000'000;

    MCTS mcts_solver(g1, g2);
    State best_state = mcts_solver.Search(kNumIters);
    return best_state.mapping;
}
