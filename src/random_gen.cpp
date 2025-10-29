#include "random_gen.hpp"
#include <assert.h>
#include <random>
#include <unordered_set>

static std::mt19937 g_Gen(kSeed);

std::tuple<Graph, Graph> GenerateExample(
    const std::uint32_t size_g1, const std::uint32_t size_g2, const float density_g1, const float density_g2,
    const bool create_g1_based_on_g2
)
{
    assert(size_g1 != 0);
    assert(size_g2 != 0);

    std::random_device rd;

    Graph g1(size_g1);
    Graph g2(size_g2);

    // Populate big graph
    const std::uint32_t edges_g2 = static_cast<std::uint32_t>(density_g2 * size_g2);
    std::uniform_int_distribution<std::uint32_t> dist_g2(0, size_g2 - 1);
    for (std::uint32_t i = 0; i < edges_g2; ++i) {
        const std::uint32_t v1 = dist_g2(g_Gen);
        const std::uint32_t v2 = dist_g2(g_Gen);

        g2.AddEdge(v1, v2);
    }

    if (!create_g1_based_on_g2) {
        const std::uint32_t edges_g1 = static_cast<std::uint32_t>(density_g1 * size_g1);
        std::uniform_int_distribution<std::uint32_t> dist_g1(0, size_g1 - 1);

        for (std::uint32_t i = 0; i < edges_g1; ++i) {
            const std::uint32_t v1 = dist_g1(g_Gen);
            const std::uint32_t v2 = dist_g1(g_Gen);

            g1.AddEdge(v1, v2);
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
                [&](std::uint32_t &edges, const std::uint32_t u) {
                    g1.AddEdge(v, u);
                },
                v
            );
        }

        // Remove edges
    }

    return std::make_tuple(g1, g2);
}
