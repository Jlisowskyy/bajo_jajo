#include "algos.hpp"

#include <cassert>
#include <climits>
#include <map>
#include <queue>
#include <unordered_set>
#include <vector>

static bool is_mapping_present(const Mapping &mapping, const std::multimap<int, Mapping> &mappings)
{
    for (const auto &pair : mappings) {
        if (pair.second == mapping) {
            return true;
        }
    }
    return false;
}

static int calculate_incremental_cost(const Graph &g1, const Graph &g2, const Mapping &mapping, const Vertex g1_vertex)
{
    int cost = 0;
    for (Vertex u = 0; u < g1.GetVertices(); ++u) {
        if (!mapping.is_g1_mapped(u)) {
            continue;
        }

        if (const Edges edges_g1_uv = g1.GetEdges(u, g1_vertex); edges_g1_uv > 0) {
            const MappedVertex mapped_u = mapping.get_mapping_g1_to_g2(u);
            const MappedVertex mapped_v = mapping.get_mapping_g1_to_g2(g1_vertex);
            assert(mapped_u != -1 && mapped_v != -1);

            const Edges edges_g2 = g2.GetEdges(mapped_u, mapped_v);
            if (edges_g1_uv > edges_g2) {
                cost += static_cast<int>(edges_g1_uv - edges_g2);
            }
        }

        if (const Edges edges_g1_vu = g1.GetEdges(g1_vertex, u); edges_g1_vu > 0) {
            const MappedVertex mapped_v = mapping.get_mapping_g1_to_g2(g1_vertex);
            const MappedVertex mapped_u = mapping.get_mapping_g1_to_g2(u);
            assert(mapped_v != -1 && mapped_u != -1);

            const Edges edges_g2 = g2.GetEdges(mapped_v, mapped_u);
            if (edges_g1_vu > edges_g2) {
                cost += static_cast<int>(edges_g1_vu - edges_g2);
            }
        }
    }
    return cost;
}

static void BruteForceRecursive(
    const Graph &g1, const Graph &g2, const int k, Mapping &current_mapping, int current_cost,
    std::vector<bool> &used_g2_vertices, const std::int32_t depth, std::multimap<int, Mapping> &best_mappings
)
{
    if (!best_mappings.empty() && best_mappings.size() == static_cast<size_t>(k)) {
        if (current_cost >= best_mappings.rbegin()->first) {
            return;
        }
    }

    if (depth == static_cast<std::int32_t>(g1.GetVertices())) {
        if (is_mapping_present(current_mapping, best_mappings)) {
            return;
        }

        if (best_mappings.size() < static_cast<size_t>(k)) {
            best_mappings.insert({current_cost, current_mapping});
        } else {
            if (current_cost < best_mappings.rbegin()->first) {
                best_mappings.erase(std::prev(best_mappings.end()));
                best_mappings.insert({current_cost, current_mapping});
            }
        }
        return;
    }

    for (Vertex candidate = 0; candidate < g2.GetVertices(); ++candidate) {
        if (used_g2_vertices[candidate]) {
            continue;
        }

        current_mapping.set_mapping(depth, candidate);
        used_g2_vertices[candidate] = true;

        const int incremental_cost = calculate_incremental_cost(g1, g2, current_mapping, depth);
        BruteForceRecursive(
            g1, g2, k, current_mapping, current_cost + incremental_cost, used_g2_vertices, depth + 1, best_mappings
        );

        used_g2_vertices[candidate] = false;
        current_mapping.remove_mapping_g1(depth);
    }
}

// ------------------------------
// Accurate Brute Force
// ------------------------------

std::vector<Mapping> AccurateBruteForce(const Graph &g1, const Graph &g2, int k)
{
    if (g1.GetVertices() > g2.GetVertices()) {
        return {};
    }

    std::multimap<int, Mapping> best_mappings;
    Mapping current_mapping(g1.GetVertices(), g2.GetVertices());
    std::vector<bool> used_g2_vertices(g2.GetVertices(), false);

    BruteForceRecursive(g1, g2, k, current_mapping, 0, used_g2_vertices, 0, best_mappings);

    std::vector<Mapping> result;
    result.reserve(best_mappings.size());
    for (const auto &pair : best_mappings) {
        result.push_back(pair.second);
    }

    return result;
}

// ------------------------------
// A star helpers
// ------------------------------

static Vertex PickNextVertex_(const Graph &g1, const State &state)
{
    // If no vertices are mapped yet, choose the one with the most neighbors
    if (state.mapping.get_mapped_count() == 0) {
        Vertex best_v1         = ~static_cast<Vertex>(0);
        Vertices max_neighbors = 0;

        for (Vertex v1 = 0; v1 < g1.GetVertices(); ++v1) {
            const Vertices neighbor_count = g1.GetNumOfNeighbours(v1);
            if (neighbor_count >= max_neighbors) {
                max_neighbors = neighbor_count;
                best_v1       = v1;
            }
        }

        assert(best_v1 != ~static_cast<Vertex>(0));
        return best_v1;
    }

    Vertex best_v1                = ~static_cast<Vertex>(0);
    Vertices max_mapped_neighbors = 0;
    Vertices min_total_neighbors  = ~static_cast<Vertices>(0);

    for (Vertex v1 = 0; v1 < g1.GetVertices(); ++v1) {
        if (state.mapping.is_g1_mapped(v1)) {
            continue;
        }

        Vertices mapped_neighbours = 0;
        Vertices total_neighbours  = 0;

        g1.IterateNeighbours(
            [&](const Vertex neighbour) {
                if (state.mapping.is_g1_mapped(neighbour)) {
                    mapped_neighbours++;
                }
                total_neighbours++;
            },
            v1
        );

        // Choose vertex with most mapped neighbors
        // Break ties by choosing vertex with fewer total neighbors
        if (mapped_neighbours > max_mapped_neighbors ||
            (mapped_neighbours == max_mapped_neighbors && total_neighbours < min_total_neighbors)) {
            best_v1              = v1;
            max_mapped_neighbors = mapped_neighbours;
            min_total_neighbors  = total_neighbours;
        }
    }

    return best_v1;
}

static int CalculateAssignmentCost_(const Graph &g1, const Graph &g2, const Mapping &mapping, Vertex v1, Vertex v2)
{
    int cost = 0;

    // Iterate over all neighbors of v1 in G1 that are already mapped
    g1.IterateNeighbours(
        [&](const Vertex neighbour) {
            if (!mapping.is_g1_mapped(neighbour)) {
                return;
            }

            const MappedVertex u2 = mapping.get_mapping_g1_to_g2(neighbour);
            assert(u2 != -1);

            // Cost for edges from v1 to u1
            const Edges edges_g1_v1u1 = g1.GetEdges(v1, neighbour);
            if (edges_g1_v1u1 > 0) {
                const Edges edges_g2_v2u2 = g2.GetEdges(v2, u2);
                if (edges_g1_v1u1 > edges_g2_v2u2) {
                    cost += static_cast<int>(edges_g1_v1u1 - edges_g2_v2u2);
                }
            }

            // Cost for edges from u1 to v1
            const Edges edges_g1_u1v1 = g1.GetEdges(neighbour, v1);
            if (edges_g1_u1v1 > 0) {
                const Edges edges_g2_u2v2 = g2.GetEdges(u2, v2);
                if (edges_g1_u1v1 > edges_g2_u2v2) {
                    cost += static_cast<int>(edges_g1_u1v1 - edges_g2_u2v2);
                }
            }
        },
        v1
    );

    return cost;
}

static int CalculateHeuristic_(const Graph &g1, const Graph &g2, const State &state)
{
    int h = 0;
    for (Vertex v1 = 0; v1 < g1.GetVertices(); ++v1) {
        if (state.mapping.is_g1_mapped(v1)) {
            continue;
        }
        int min_cost = INT_MAX;

        // Find the minimum cost assignment to any available vertex in G2
        for (Vertex v2 : state.availableVertices) {
            int cost_candidate = 0;
            g1.IterateNeighbours(
                [&](Vertex neighbour) {
                    if (!state.mapping.is_g1_mapped(neighbour)) {
                        return;
                    }

                    const MappedVertex u2 = state.mapping.get_mapping_g1_to_g2(neighbour);
                    assert(u2 != -1);

                    // Cost for edges from u1 to v1
                    const Edges edges_g1_u1v1 = g1.GetEdges(v1, neighbour);
                    if (edges_g1_u1v1 > 0) {
                        const Edges edges_g2_u2v2 = g2.GetEdges(v2, u2);
                        if (edges_g1_u1v1 > edges_g2_u2v2) {
                            cost_candidate += static_cast<int>(edges_g1_u1v1 - edges_g2_u2v2);
                        }
                    }

                    // Cost for edges from v1 to u1
                    const Edges edges_g1_v1u1 = g1.GetEdges(neighbour, v1);
                    if (edges_g1_v1u1 > 0) {
                        const Edges edges_g2_v2u2 = g2.GetEdges(u2, v2);
                        if (edges_g1_v1u1 > edges_g2_v2u2) {
                            cost_candidate += static_cast<int>(edges_g1_v1u1 - edges_g2_v2u2);
                        }
                    }
                },
                v1
            );
            min_cost = std::min(min_cost, cost_candidate);
        }
        assert(min_cost != INT_MAX);
        h += min_cost;
    }
    return h;
}

// ------------------------------
// A star
// ------------------------------

struct AStarState {
    State state;
    int g;  // Real cost so far
    int f;  // f = g + h (priority)

    AStarState() : state(0, 0), g(0), f(0) {}

    AStarState(std::int32_t size_g1, std::int32_t size_g2) : state(size_g1, size_g2), g(0), f(0) {}

    AStarState(const State &s, int cost_g, int cost_f) : state(s), g(cost_g), f(cost_f) {}

    bool operator>(const AStarState &other) const { return f > other.f; }
};

std::vector<Mapping> AccurateAStar(const Graph &g1, const Graph &g2, const int k)
{
    if (g1.GetVertices() > g2.GetVertices()) {
        return {};
    }
    std::priority_queue<AStarState, std::vector<AStarState>, std::greater<AStarState>> pq;

    // Initialize with empty mapping
    AStarState initial = AStarState(g1.GetVertices(), g2.GetVertices());
    pq.push(initial);

    while (!pq.empty()) {
        AStarState current = pq.top();
        pq.pop();

        if (current.state.mapping.get_mapped_count() == g1.GetVertices()) {
            return {current.state.mapping};
        }

        // Choose next vertex to map
        const Vertex v1 = PickNextVertex_(g1, current.state);

        // Try mapping v1 to each available vertex in G2
        for (Vertex v2 : current.state.availableVertices) {
            AStarState next_state;
            next_state.state = current.state;
            next_state.state.set_mapping(v1, v2);

            const int cost_increment = CalculateAssignmentCost_(g1, g2, current.state.mapping, v1, v2);
            next_state.g             = current.g + cost_increment;

            // Calculate heuristic
            const int h  = CalculateHeuristic_(g1, g2, next_state.state);
            next_state.f = next_state.g + h;

            pq.push(next_state);
        }
    }

    return {};
}

// ------------------------------
// Approx A star
// ------------------------------

NODISCARD std::vector<Mapping> ApproxAStar(const Graph &g1, const Graph &g2, int k) { return {}; }
