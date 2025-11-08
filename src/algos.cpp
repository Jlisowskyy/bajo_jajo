#include "algos.hpp"
#include "mcts.hpp"

#include <cassert>
#include <queue>
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

struct AStarState {
    Mapping mapping;
    std::int32_t mapped_count;
    std::uint64_t used_mask;  // bitmask for used vertices in G2
    std::int32_t g;           // actual cost (edges added)
    std::int32_t f;           // priority: f = g + h

    AStarState(std::int32_t size_g1, std::int32_t size_g2)
        : mapping(size_g1, size_g2), mapped_count(0), used_mask(0), g(0), f(0)
    {
    }

    // For priority queue - min heap by f value
    bool operator>(const AStarState &other) const
    {
        if (f != other.f)
            return f > other.f;
        return g > other.g;  // tie-breaker: prefer lower g
    }
};

// Calculate heuristic: minimum cost to complete the mapping
static std::int32_t CalculateHeuristic(const AStarState &state, const Graph &g1, const Graph &g2)
{
    std::int32_t h        = 0;
    const std::int32_t n1 = g1.GetVertices();
    const std::int32_t n2 = g2.GetVertices();

    // For each unmapped vertex in G1
    for (std::int32_t u1 = 0; u1 < n1; ++u1) {
        if (state.mapping.is_g1_mapped(u1))
            continue;

        std::int32_t min_cost = INT32_MAX;

        // Try each available vertex in G2
        for (std::int32_t u2 = 0; u2 < n2; ++u2) {
            if (state.used_mask & (1ULL << u2))
                continue;  // already used

            std::int32_t cost = 0;

            // Calculate cost for edges to already mapped neighbors
            g1.IterateOutEdges(
                [&](const std::uint32_t edges, const std::uint32_t v1) {
                    if (state.mapping.is_g1_mapped(v1)) {
                        const std::int32_t v2           = state.mapping.get_mapping_g1_to_g2(v1);
                        const std::uint32_t edges_in_g2 = g2.GetEdges(u2, v2);
                        if (edges > edges_in_g2) {
                            cost += (edges - edges_in_g2);
                        }
                    }
                },
                u1
            );

            g1.IterateInEdges(
                [&](const std::uint32_t edges, const std::uint32_t v1) {
                    if (state.mapping.is_g1_mapped(v1)) {
                        const std::int32_t v2           = state.mapping.get_mapping_g1_to_g2(v1);
                        const std::uint32_t edges_in_g2 = g2.GetEdges(v2, u2);
                        if (edges > edges_in_g2) {
                            cost += (edges - edges_in_g2);
                        }
                    }
                },
                u1
            );

            min_cost = std::min(min_cost, cost);
        }

        if (min_cost != INT32_MAX) {
            h += min_cost;
        }
    }

    return h;
}

// Choose next vertex to map using fail-first heuristic
static std::int32_t ChooseNextVertex(const AStarState &state, const Graph &g1)
{
    std::int32_t best_vertex          = -1;
    std::int32_t max_mapped_neighbors = -1;
    std::int32_t max_degree           = -1;

    const std::int32_t n1 = g1.GetVertices();

    for (std::int32_t v1 = 0; v1 < n1; ++v1) {
        if (state.mapping.is_g1_mapped(v1))
            continue;

        // Count already mapped neighbors
        std::int32_t mapped_neighbors = 0;
        g1.IterateEdges(
            [&](const std::uint32_t, const std::uint32_t, const std::uint32_t u1) {
                if (state.mapping.is_g1_mapped(u1)) {
                    mapped_neighbors++;
                }
            },
            v1
        );

        // Calculate degree
        std::int32_t degree = 0;
        g1.IterateEdges(
            [&](const std::uint32_t edges, const std::uint32_t, const std::uint32_t) {
                degree += edges;
            },
            v1
        );

        // Fail-first: prefer vertices with most mapped neighbors
        // Tie-breaker: prefer vertices with highest degree
        if (mapped_neighbors > max_mapped_neighbors ||
            (mapped_neighbors == max_mapped_neighbors && degree > max_degree)) {
            best_vertex          = v1;
            max_mapped_neighbors = mapped_neighbors;
            max_degree           = degree;
        }
    }

    return best_vertex;
}

// Calculate cost of adding a mapping v1 -> v2
static std::int32_t CalculateMappingCost(
    const AStarState &state, const Graph &g1, const Graph &g2, std::int32_t v1, std::int32_t v2
)
{
    std::int32_t cost = 0;

    // Check outgoing edges from v1
    g1.IterateOutEdges(
        [&](const std::uint32_t edges, const std::uint32_t u1) {
            if (state.mapping.is_g1_mapped(u1)) {
                const std::int32_t u2           = state.mapping.get_mapping_g1_to_g2(u1);
                const std::uint32_t edges_in_g2 = g2.GetEdges(v2, u2);
                if (edges > edges_in_g2) {
                    cost += (edges - edges_in_g2);
                }
            }
        },
        v1
    );

    // Check incoming edges to v1
    g1.IterateInEdges(
        [&](const std::uint32_t edges, const std::uint32_t u1) {
            if (state.mapping.is_g1_mapped(u1)) {
                const std::int32_t u2           = state.mapping.get_mapping_g1_to_g2(u1);
                const std::uint32_t edges_in_g2 = g2.GetEdges(u2, v2);
                if (edges > edges_in_g2) {
                    cost += (edges - edges_in_g2);
                }
            }
        },
        v1
    );

    return cost;
}

NODISCARD Mapping AccurateAStar(const Graph &g1, const Graph &g2)
{
    const std::int32_t n1 = g1.GetVertices();
    const std::int32_t n2 = g2.GetVertices();

    if (n1 > n2) {
        // No solution possible without adding vertices
        return Mapping(n1, n2);
    }

    // Priority queue: min-heap by f value
    std::priority_queue<AStarState, std::vector<AStarState>, std::greater<AStarState>> pq;

    // Initial state
    AStarState initial(n1, n2);
    initial.f = CalculateHeuristic(initial, g1, g2);
    pq.push(initial);

    AStarState best_solution(n1, n2);
    best_solution.g = INT32_MAX;

    while (!pq.empty()) {
        AStarState current = pq.top();
        pq.pop();

        // Pruning: if current f >= best solution found, skip
        if (current.f >= best_solution.g) {
            continue;
        }

        // Check if complete mapping
        if (current.mapped_count == n1) {
            if (current.g < best_solution.g) {
                best_solution = current;
            }
            continue;
        }

        // Choose next vertex to map
        std::int32_t v1 = ChooseNextVertex(current, g1);
        if (v1 == -1)
            continue;

        // Try mapping v1 to each available vertex in G2
        for (std::int32_t v2 = 0; v2 < n2; ++v2) {
            if (current.used_mask & (1ULL << v2))
                continue;  // already used

            // Create new state
            AStarState next(n1, n2);
            next.mapping = current.mapping;
            next.mapping.set_mapping(v1, v2);
            next.mapped_count = current.mapped_count + 1;
            next.used_mask    = current.used_mask | (1ULL << v2);

            // Calculate cost of this mapping
            std::int32_t added_cost = CalculateMappingCost(current, g1, g2, v1, v2);
            next.g                  = current.g + added_cost;

            // Calculate heuristic
            std::int32_t h = CalculateHeuristic(next, g1, g2);
            next.f         = next.g + h;

            // Pruning
            if (next.f < best_solution.g) {
                pq.push(next);
            }
        }
    }

    return best_solution.mapping;
}

// ------------------------------
// Mcts
// ------------------------------

NODISCARD Mapping Mcts(const Graph &g1, const Graph &g2)
{
    static constexpr int kNumIters = 10'000;

    MCTS mcts_solver(g1, g2);
    State best_state = mcts_solver.Search(kNumIters);
    return best_state.mapping;
}
