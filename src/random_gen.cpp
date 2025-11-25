#include "random_gen.hpp"
#include <assert.h>
#include <random>
#include <unordered_map>
#include <unordered_set>

// in case of create_g1_based_on_g2 density_g1 should be in range [0.0, 1.0]
std::pair<Graph, Graph> GenerateExample(const GraphSpec spec)
{
    std::mt19937 kGenerator(kSeed);

    assert(spec.size_g1 != 0);
    assert(spec.size_g2 != 0);

    Graph g1(spec.size_g1);
    Graph g2(spec.size_g2);

    // Populate big graph
    const auto edges_g2 = static_cast<std::uint32_t>(spec.density_g2 * spec.size_g2 * spec.size_g2);
    std::uniform_int_distribution<std::uint32_t> dist_g2(0, spec.size_g2 - 1);
    for (std::uint32_t i = 0; i < edges_g2; ++i) {
        const auto v1 = static_cast<Vertex>(dist_g2(kGenerator));
        const auto v2 = static_cast<Vertex>(dist_g2(kGenerator));

        g2.AddEdges(v1, v2);
    }

    if (!spec.create_g1_based_on_g2) {
        const auto edges_g1 = static_cast<std::uint32_t>(spec.density_g1 * spec.size_g1 * spec.size_g1);
        std::uniform_int_distribution<std::uint32_t> dist_g1(0, spec.size_g1 - 1);

        for (std::uint32_t i = 0; i < edges_g1; ++i) {
            const auto v1 = static_cast<Vertex>(dist_g1(kGenerator));
            const auto v2 = static_cast<Vertex>(dist_g1(kGenerator));

            g1.AddEdges(v1, v2);
        }
    } else {
        assert(spec.size_g1 <= spec.size_g2);
        assert(spec.density_g1 >= 0);
        assert(spec.density_g1 <= 1);

        // Pick size_g1 vertices from G2
        std::unordered_set<std::uint32_t> selected_vertices_g2{};

        while (selected_vertices_g2.size() != spec.size_g1) {
            const auto v = static_cast<Vertex>(dist_g2(kGenerator));
            selected_vertices_g2.insert(v);
        }

        // Create mapping from G2 vertices to G1 vertices
        std::unordered_map<Vertex, Vertex> g2_to_g1_mapping;
        Vertex g1_vertex_idx = 0;
        for (const Vertex g2_vertex : selected_vertices_g2) {
            g2_to_g1_mapping[g2_vertex] = g1_vertex_idx++;
        }

        // Add edges from G2 subgraph to G1
        for (const Vertex v_g2 : selected_vertices_g2) {
            const Vertex v_g1 = g2_to_g1_mapping[v_g2];

            g2.IterateOutEdges(
                [&](const Edges edges, const Vertex u_g2) {
                    // Only add edge if the target vertex is also in our selected set
                    if (selected_vertices_g2.contains(u_g2)) {
                        const Vertex u_g1 = g2_to_g1_mapping[u_g2];
                        g1.AddEdges(v_g1, u_g1, edges);
                    }
                },
                v_g2
            );
        }

        // Remove edges based on density_g1 (acts as a keep probability)
        std::uniform_real_distribution dist(0.0, 1.0);
        const double removal_prob = 1.0 - spec.density_g1;
        g1.IterateEdges([&](const Edges edges, const Vertex u, const Vertex v) {
            for (std::uint32_t i = 0; i < edges; ++i) {
                if (dist(kGenerator) < removal_prob) {
                    g1.RemoveEdges(u, v);
                }
            }
        });
    }

    return std::make_pair(std::move(g1), std::move(g2));
}
