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

// ------------------------------
// A star
// ------------------------------

// ------------------------------
// Approx A star
// ------------------------------
