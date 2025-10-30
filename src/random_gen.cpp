#include "random_gen.hpp"
#include <assert.h>
#include <random>
#include <unordered_map>
#include <unordered_set>

static std::mt19937 g_Gen(kSeed);

// in case of create_g1_based_on_g2 density_g1 should be in range [0.0, 1.0]
std::tuple<Graph, Graph> GenerateExample(
    const std::uint32_t size_g1, const std::uint32_t size_g2, const double density_g1, const double density_g2,
    const bool create_g1_based_on_g2
)
{
    assert(size_g1 != 0);
    assert(size_g2 != 0);

    Graph g1(size_g1);
    Graph g2(size_g2);

    // Populate big graph
    const auto edges_g2 = static_cast<std::uint32_t>(density_g2 * size_g2);
    std::uniform_int_distribution<std::uint32_t> dist_g2(0, size_g2 - 1);
    for (std::uint32_t i = 0; i < edges_g2; ++i) {
        const std::uint32_t v1 = dist_g2(g_Gen);
        const std::uint32_t v2 = dist_g2(g_Gen);

        g2.AddEdges(v1, v2);
    }

    if (!create_g1_based_on_g2) {
        const auto edges_g1 = static_cast<std::uint32_t>(density_g1 * size_g1);
        std::uniform_int_distribution<std::uint32_t> dist_g1(0, size_g1 - 1);

        for (std::uint32_t i = 0; i < edges_g1; ++i) {
            const std::uint32_t v1 = dist_g1(g_Gen);
            const std::uint32_t v2 = dist_g1(g_Gen);

            g1.AddEdges(v1, v2);
        }
    } else {
        assert(size_g1 <= size_g2);

        // Pick size_g1 vertices from G2
        std::unordered_set<std::uint32_t> selected_vertices_g2{};

        while (selected_vertices_g2.size() != size_g1) {
            const std::uint32_t v = dist_g2(g_Gen);
            selected_vertices_g2.insert(v);
        }

        // Create mapping from G2 vertices to G1 vertices
        std::unordered_map<std::uint32_t, std::uint32_t> g2_to_g1_mapping;
        std::uint32_t g1_vertex_idx = 0;
        for (const std::uint32_t g2_vertex : selected_vertices_g2) {
            g2_to_g1_mapping[g2_vertex] = g1_vertex_idx++;
        }

        // Add edges from G2 subgraph to G1
        for (const std::uint32_t v_g2 : selected_vertices_g2) {
            const std::uint32_t v_g1 = g2_to_g1_mapping[v_g2];

            g2.IterateOutEdges(
                [&](const std::uint32_t edges, const std::uint32_t u_g2) {
                    // Only add edge if the target vertex is also in our selected set
                    if (selected_vertices_g2.find(u_g2) != selected_vertices_g2.end()) {
                        const std::uint32_t u_g1 = g2_to_g1_mapping[u_g2];
                        g1.AddEdges(v_g1, u_g1, edges);
                    }
                },
                v_g2
            );
        }

        // Remove edges based on density_g1 (acts as a keep probability)
        std::uniform_real_distribution dist(0.0, 1.0);
        const double removal_prob = 1.0 - density_g1;
        g1.IterateEdges([&](const std::uint32_t edges, const std::uint32_t u, const std::uint32_t v) {
            for (std::uint32_t i = 0; i < edges; ++i) {
                if (dist(g_Gen) < removal_prob) {
                    g1.RemoveEdges(u, v);
                }
            }
        });
    }

    return std::make_tuple(std::move(g1), std::move(g2));
}
