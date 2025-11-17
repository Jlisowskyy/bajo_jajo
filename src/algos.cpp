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

static int calculate_incremental_cost(const Graph &g1, const Graph &g2, const Mapping &mapping, std::int32_t g1_vertex)
{
    int cost = 0;
    for (std::int32_t u = 0; u < g1.GetVertices(); ++u) {
        if (!mapping.is_g1_mapped(u))
            continue;
        if (const std::uint32_t edges_g1_uv = g1.GetEdges(u, g1_vertex); edges_g1_uv > 0) {
            const std::int32_t mapped_u = mapping.get_mapping_g1_to_g2(u);
            const std::int32_t mapped_v = mapping.get_mapping_g1_to_g2(g1_vertex);
            assert(mapped_u != -1 && mapped_v != -1);
            const std::uint32_t edges_g2 = g2.GetEdges(mapped_u, mapped_v);
            if (edges_g1_uv > edges_g2) {
                cost += edges_g1_uv - edges_g2;
            }
        }
        if (const std::uint32_t edges_g1_vu = g1.GetEdges(g1_vertex, u); edges_g1_vu > 0) {
            const std::int32_t mapped_v = mapping.get_mapping_g1_to_g2(g1_vertex);
            const std::int32_t mapped_u = mapping.get_mapping_g1_to_g2(u);
            assert(mapped_v != -1 && mapped_u != -1);
            const std::uint32_t edges_g2 = g2.GetEdges(mapped_v, mapped_u);
            if (edges_g1_vu > edges_g2) {
                cost += edges_g1_vu - edges_g2;
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

    if (depth == g1.GetVertices()) {
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

    for (std::int32_t candidate = 0; candidate < g2.GetVertices(); ++candidate) {
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

namespace
{
struct AStarState {
    Mapping mapping;
    std::vector<bool> used_g2_vertices;
    int g;
    int h;

    AStarState(const Graph &g1, const Graph &g2)
        : mapping(g1.GetVertices(), g2.GetVertices()), used_g2_vertices(g2.GetVertices(), false), g(0), h(0)
    {
    }

    AStarState(const AStarState &)            = default;
    AStarState &operator=(const AStarState &) = default;
    AStarState(AStarState &&)                 = default;
    AStarState &operator=(AStarState &&)      = default;

    int f() const { return g + h; }

    bool operator>(const AStarState &other) const { return f() > other.f(); }
};

int calculate_assignment_cost(
    const Graph &g1, const Graph &g2, const Mapping &mapping, std::int32_t g1_newly_mapped_vertex
)
{
    int cost                                  = 0;
    const std::int32_t g2_newly_mapped_vertex = mapping.get_mapping_g1_to_g2(g1_newly_mapped_vertex);
    assert(g2_newly_mapped_vertex != -1);

    for (std::int32_t u1 = 0; u1 < g1.GetVertices(); ++u1) {
        if (u1 == g1_newly_mapped_vertex || !mapping.is_g1_mapped(u1)) {
            continue;
        }

        const std::int32_t u2 = mapping.get_mapping_g1_to_g2(u1);
        assert(u2 != -1);

        if (const auto edges_g1 = g1.GetEdges(g1_newly_mapped_vertex, u1); edges_g1 > 0) {
            const auto edges_g2 = g2.GetEdges(g2_newly_mapped_vertex, u2);
            if (edges_g1 > edges_g2) {
                cost += edges_g1 - edges_g2;
            }
        }
        if (const auto edges_g1 = g1.GetEdges(u1, g1_newly_mapped_vertex); edges_g1 > 0) {
            const auto edges_g2 = g2.GetEdges(u2, g2_newly_mapped_vertex);
            if (edges_g1 > edges_g2) {
                cost += edges_g1 - edges_g2;
            }
        }
    }
    return cost;
}

int calculate_heuristic(const Graph &g1, const Graph &g2, const AStarState &state)
{
    int h = 0;

    for (std::int32_t u1 = 0; u1 < g1.GetVertices(); ++u1) {
        if (state.mapping.is_g1_mapped(u1)) {
            continue;
        }

        int min_cost_for_u1 = INT_MAX;

        for (std::int32_t u2 = 0; u2 < g2.GetVertices(); ++u2) {
            if (state.used_g2_vertices[u2]) {
                continue;
            }

            int cost_candidate = 0;
            for (std::int32_t v1 = 0; v1 < g1.GetVertices(); ++v1) {
                if (!state.mapping.is_g1_mapped(v1)) {
                    continue;
                }
                const std::int32_t v2 = state.mapping.get_mapping_g1_to_g2(v1);

                if (const auto edges_g1 = g1.GetEdges(u1, v1); edges_g1 > 0) {
                    const auto edges_g2 = g2.GetEdges(u2, v2);
                    if (edges_g1 > edges_g2) {
                        cost_candidate += edges_g1 - edges_g2;
                    }
                }
                if (const auto edges_g1 = g1.GetEdges(v1, u1); edges_g1 > 0) {
                    const auto edges_g2 = g2.GetEdges(v2, u2);
                    if (edges_g1 > edges_g2) {
                        cost_candidate += edges_g1 - edges_g2;
                    }
                }
            }
            min_cost_for_u1 = std::min(min_cost_for_u1, cost_candidate);
        }
        if (min_cost_for_u1 != INT_MAX) {
            h += min_cost_for_u1;
        }
    }
    return h;
}

std::int32_t select_next_vertex(const Graph &g1, const Mapping &mapping)
{
    std::int32_t best_v1     = -1;
    int max_mapped_neighbors = -1;
    int min_total_degree     = INT_MAX;

    if (mapping.get_mapped_count() == 0) {
        int max_degree = -1;
        for (std::int32_t v = 0; v < g1.GetVertices(); ++v) {
            std::unordered_set<std::uint32_t> neighbors;
            g1.IterateOutEdges(
                [&](std::uint32_t, std::uint32_t u) {
                    neighbors.insert(u);
                },
                v
            );
            g1.IterateInEdges(
                [&](std::uint32_t, std::uint32_t u) {
                    neighbors.insert(u);
                },
                v
            );
            const int current_degree = static_cast<int>(neighbors.size());

            if (current_degree > max_degree) {
                max_degree = current_degree;
                best_v1    = v;
            }
        }
        return best_v1;
    }

    for (std::int32_t v1 = 0; v1 < g1.GetVertices(); ++v1) {
        if (mapping.is_g1_mapped(v1)) {
            continue;
        }

        int mapped_neighbors = 0;
        std::unordered_set<std::uint32_t> neighbors;
        g1.IterateOutEdges(
            [&](std::uint32_t, std::uint32_t u) {
                neighbors.insert(u);
            },
            v1
        );
        g1.IterateInEdges(
            [&](std::uint32_t, std::uint32_t u) {
                neighbors.insert(u);
            },
            v1
        );

        for (const auto neighbor : neighbors) {
            if (mapping.is_g1_mapped(neighbor)) {
                mapped_neighbors++;
            }
        }
        const int total_degree = static_cast<int>(neighbors.size());

        if (mapped_neighbors > max_mapped_neighbors) {
            max_mapped_neighbors = mapped_neighbors;
            min_total_degree     = total_degree;
            best_v1              = v1;
        } else if (mapped_neighbors == max_mapped_neighbors) {
            if (total_degree < min_total_degree) {
                min_total_degree = total_degree;
                best_v1          = v1;
            }
        }
    }
    return best_v1;
}

}  // namespace

// ------------------------------
// A star
// ------------------------------

std::vector<Mapping> AccurateAStar(const Graph &g1, const Graph &g2, const int k)
{
    if (g1.GetVertices() > g2.GetVertices()) {
        return {};
    }

    std::priority_queue<AStarState, std::vector<AStarState>, std::greater<AStarState>> pq;
    std::multimap<int, Mapping> best_mappings;

    AStarState initial_state(g1, g2);
    initial_state.h = calculate_heuristic(g1, g2, initial_state);
    pq.push(initial_state);

    while (!pq.empty()) {
        AStarState current_state = pq.top();
        pq.pop();

        if (!best_mappings.empty() && best_mappings.size() == static_cast<size_t>(k)) {
            if (current_state.f() >= best_mappings.rbegin()->first) {
                continue;
            }
        }

        if (current_state.mapping.get_mapped_count() == g1.GetVertices()) {
            if (!is_mapping_present(current_state.mapping, best_mappings)) {
                if (best_mappings.size() < static_cast<size_t>(k)) {
                    best_mappings.insert({current_state.g, current_state.mapping});
                } else if (current_state.g < best_mappings.rbegin()->first) {
                    best_mappings.erase(std::prev(best_mappings.end()));
                    best_mappings.insert({current_state.g, current_state.mapping});
                }
            }
            continue;
        }

        const std::int32_t v1_to_map = select_next_vertex(g1, current_state.mapping);
        if (v1_to_map == -1) {
            continue;
        }

        for (std::int32_t v2_candidate = 0; v2_candidate < g2.GetVertices(); ++v2_candidate) {
            if (current_state.used_g2_vertices[v2_candidate]) {
                continue;
            }

            AStarState next_state = current_state;
            next_state.mapping.set_mapping(v1_to_map, v2_candidate);
            next_state.used_g2_vertices[v2_candidate] = true;

            const int incremental_cost = calculate_assignment_cost(g1, g2, next_state.mapping, v1_to_map);
            next_state.g               = current_state.g + incremental_cost;

            if (!best_mappings.empty() && best_mappings.size() == static_cast<size_t>(k)) {
                if (next_state.g >= best_mappings.rbegin()->first) {
                    continue;
                }
            }

            next_state.h = calculate_heuristic(g1, g2, next_state);

            if (!best_mappings.empty() && best_mappings.size() == static_cast<size_t>(k)) {
                if (next_state.f() >= best_mappings.rbegin()->first) {
                    continue;
                }
            }

            pq.push(next_state);
        }
    }

    std::vector<Mapping> result;
    result.reserve(best_mappings.size());
    for (const auto &pair : best_mappings) {
        result.push_back(pair.second);
    }

    return result;
}

// ------------------------------
// Approx A star
// ------------------------------

namespace
{
using ApproxState = AStarState;

constexpr int BEAM_WIDTH = 5;

void insert_sorted_and_capped(std::vector<ApproxState> &level_q, ApproxState &&new_state)
{
    auto it =
        std::upper_bound(level_q.begin(), level_q.end(), new_state, [](const ApproxState &a, const ApproxState &b) {
            return a.f() < b.f();
        });

    if (level_q.size() < BEAM_WIDTH || it != level_q.end()) {
        level_q.insert(it, std::move(new_state));
        if (level_q.size() > BEAM_WIDTH) {
            level_q.pop_back();
        }
    }
}
}  // namespace

std::vector<Mapping> ApproxAStar(const Graph &g1, const Graph &g2, const int k)
{
    if (g1.GetVertices() > g2.GetVertices() || g1.GetVertices() == 0) {
        return {};
    }

    const int n1 = g1.GetVertices();
    const int n2 = g2.GetVertices();

    std::vector<std::vector<ApproxState>> q(n1);

    const std::int32_t v_start = select_next_vertex(g1, Mapping(n1, n2));
    if (v_start == -1)
        return {};

    for (std::int32_t u2 = 0; u2 < n2; ++u2) {
        ApproxState initial_state(g1, g2);
        initial_state.mapping.set_mapping(v_start, u2);
        initial_state.used_g2_vertices[u2] = true;
        initial_state.g                    = 0;
        initial_state.h                    = calculate_heuristic(g1, g2, initial_state);
        insert_sorted_and_capped(q[0], std::move(initial_state));
    }

    using MasterQueueElement = std::pair<int, int>;
    std::priority_queue<MasterQueueElement, std::vector<MasterQueueElement>, std::greater<MasterQueueElement>>
        master_pq;

    std::vector<int> q_pointers(n1, 0);

    for (int i = 0; i < n1; ++i) {
        if (!q[i].empty()) {
            master_pq.push({q[i][0].f(), i});
        }
    }

    std::multimap<int, Mapping> best_mappings;

    while (!master_pq.empty()) {
        auto [current_f, level_idx] = master_pq.top();
        master_pq.pop();

        if (best_mappings.size() == static_cast<size_t>(k) && current_f >= best_mappings.rbegin()->first) {
            break;
        }

        if (q_pointers[level_idx] >= q[level_idx].size() || current_f > q[level_idx][q_pointers[level_idx]].f()) {
            continue;
        }

        ApproxState current_state = q[level_idx][q_pointers[level_idx]];
        q_pointers[level_idx]++;

        if (q_pointers[level_idx] < q[level_idx].size()) {
            master_pq.push({q[level_idx][q_pointers[level_idx]].f(), level_idx});
        }

        if (current_state.mapping.get_mapped_count() == n1) {
            if (!is_mapping_present(current_state.mapping, best_mappings)) {
                if (best_mappings.size() < static_cast<size_t>(k)) {
                    best_mappings.insert({current_state.g, current_state.mapping});
                } else if (current_state.g < best_mappings.rbegin()->first) {
                    best_mappings.erase(std::prev(best_mappings.end()));
                    best_mappings.insert({current_state.g, current_state.mapping});
                }
            }
            continue;
        }

        const int next_level_idx = level_idx + 1;
        if (next_level_idx >= n1)
            continue;

        const std::int32_t v1_to_map = select_next_vertex(g1, current_state.mapping);
        if (v1_to_map == -1)
            continue;

        bool next_level_was_empty = q[next_level_idx].empty();
        int f_before_update       = next_level_was_empty ? -1 : q[next_level_idx][0].f();

        for (std::int32_t v2_candidate = 0; v2_candidate < n2; ++v2_candidate) {
            if (current_state.used_g2_vertices[v2_candidate]) {
                continue;
            }

            ApproxState next_state = current_state;
            next_state.mapping.set_mapping(v1_to_map, v2_candidate);
            next_state.used_g2_vertices[v2_candidate] = true;

            const int incremental_cost = calculate_assignment_cost(g1, g2, next_state.mapping, v1_to_map);
            next_state.g += incremental_cost;
            next_state.h = calculate_heuristic(g1, g2, next_state);

            if (best_mappings.size() == static_cast<size_t>(k) && next_state.f() >= best_mappings.rbegin()->first) {
                continue;
            }

            insert_sorted_and_capped(q[next_level_idx], std::move(next_state));
        }

        if (!q[next_level_idx].empty()) {
            int f_after_update = q[next_level_idx][0].f();
            if (next_level_was_empty || f_after_update < f_before_update) {
                master_pq.push({f_after_update, next_level_idx});
            }
        }
    }

    std::vector<Mapping> result;
    result.reserve(best_mappings.size());
    for (const auto &pair : best_mappings) {
        result.push_back(pair.second);
    }
    return result;
}
