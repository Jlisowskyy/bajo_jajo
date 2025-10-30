#include "random_gen.hpp"
#include <assert.h>
#include <random>
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

        // Pick size_g1 vertices and populate g1
        std::unordered_set<std::uint32_t> vertices{};

        while (vertices.size() != size_g1) {
            const std::uint32_t v = dist_g2(g_Gen);
            vertices.insert(v);
        }

        // Add Missing edges
        for (const std::uint32_t v : vertices) {
            g2.IterateOutEdges(
                [&](const std::uint32_t edges, const std::uint32_t u) {
                    g1.AddEdges(v, u, edges);
                },
                v
            );
        }

        // Remove edges
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

    return std::make_tuple(g1, g2);
}
