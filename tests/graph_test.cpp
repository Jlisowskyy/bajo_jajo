#include "graph.hpp"
#include "gtest/gtest.h"

TEST(GraphTest, Constructor)
{
    const std::uint32_t num_vertices = 10;
    Graph g(num_vertices);

    EXPECT_EQ(g.GetVertices(), num_vertices);
    EXPECT_EQ(g.GetEdges(), 0);
}

TEST(GraphTest, AddAndGetEdges)
{
    Graph g(5);
    g.AddEdges(0, 1, 2);
    g.AddEdges(1, 2, 3);

    EXPECT_EQ(g.GetEdges(0, 1), 2);
    EXPECT_EQ(g.GetEdges(1, 2), 3);
    EXPECT_EQ(g.GetEdges(0, 2), 0);
    EXPECT_EQ(g.GetEdges(), 5);
}

TEST(GraphTest, RemoveEdges)
{
    Graph g(5);
    g.AddEdges(0, 1, 5);
    g.AddEdges(1, 0, 5);

    g.RemoveEdges(0, 1, 2);
    EXPECT_EQ(g.GetEdges(0, 1), 3);
    EXPECT_EQ(g.GetEdges(), 8);

    g.RemoveEdges(0, 1, 3);
    EXPECT_EQ(g.GetEdges(0, 1), 0);
    EXPECT_EQ(g.GetEdges(), 5);
}

TEST(GraphTest, MoveConstructor)
{
    const std::uint32_t num_vertices = 5;
    Graph g1(num_vertices);
    g1.AddEdges(0, 1, 2);
    g1.AddEdges(2, 3, 4);

    Graph g2(std::move(g1));

    EXPECT_EQ(g2.GetVertices(), num_vertices);
    EXPECT_EQ(g2.GetEdges(), 6);
    EXPECT_EQ(g2.GetEdges(0, 1), 2);
    EXPECT_EQ(g2.GetEdges(2, 3), 4);
}

TEST(GraphTest, MoveAssignmentOperator)
{
    const std::uint32_t num_vertices1 = 5;
    Graph g1(num_vertices1);
    g1.AddEdges(0, 1, 2);

    const std::uint32_t num_vertices2 = 10;
    Graph g2(num_vertices2);
    g2.AddEdges(3, 4, 5);

    g1 = std::move(g2);

    EXPECT_EQ(g1.GetVertices(), num_vertices2);
    EXPECT_EQ(g1.GetEdges(), 5);
    EXPECT_EQ(g1.GetEdges(3, 4), 5);
}

TEST(GraphTest, IterateOutEdges)
{
    Graph g(3);
    g.AddEdges(0, 1, 1);
    g.AddEdges(0, 2, 2);

    std::vector<std::pair<std::uint32_t, std::uint32_t>> edges;
    g.IterateOutEdges(
        [&](std::uint32_t num_edges, std::uint32_t v) {
            edges.emplace_back(num_edges, v);
        },
        0
    );

    ASSERT_EQ(edges.size(), 2);
    EXPECT_EQ(edges[0].first, 1);
    EXPECT_EQ(edges[0].second, 1);
    EXPECT_EQ(edges[1].first, 2);
    EXPECT_EQ(edges[1].second, 2);
}

TEST(GraphTest, IterateInEdges)
{
    Graph g(3);
    g.AddEdges(1, 0, 1);
    g.AddEdges(2, 0, 2);

    std::vector<std::pair<std::uint32_t, std::uint32_t>> edges;
    g.IterateInEdges(
        [&](std::uint32_t num_edges, std::uint32_t u) {
            edges.emplace_back(num_edges, u);
        },
        0
    );

    ASSERT_EQ(edges.size(), 2);
    EXPECT_EQ(edges[0].first, 1);
    EXPECT_EQ(edges[0].second, 1);
    EXPECT_EQ(edges[1].first, 2);
    EXPECT_EQ(edges[1].second, 2);
}

TEST(GraphTest, IterateEdgesForVertex)
{
    Graph g(3);
    g.AddEdges(0, 1, 1);
    g.AddEdges(2, 0, 2);

    std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>> edges;
    g.IterateEdges(
        [&](std::uint32_t num_edges, std::uint32_t u, std::uint32_t v) {
            edges.emplace_back(num_edges, u, v);
        },
        0
    );

    ASSERT_EQ(edges.size(), 2);

    bool found_out_edge = false;
    bool found_in_edge  = false;
    for (const auto &edge : edges) {
        if (std::get<0>(edge) == 1 && std::get<1>(edge) == 0 && std::get<2>(edge) == 1) {
            found_out_edge = true;
        }
        if (std::get<0>(edge) == 2 && std::get<1>(edge) == 2 && std::get<2>(edge) == 0) {
            found_in_edge = true;
        }
    }
    EXPECT_TRUE(found_out_edge);
    EXPECT_TRUE(found_in_edge);
}

TEST(GraphTest, IterateAllEdges)
{
    Graph g(3);
    g.AddEdges(0, 1, 1);
    g.AddEdges(1, 2, 2);
    g.AddEdges(2, 0, 3);

    std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>> all_edges;
    g.IterateEdges([&](std::uint32_t num_edges, std::uint32_t u, std::uint32_t v) {
        all_edges.emplace_back(num_edges, u, v);
    });

    ASSERT_EQ(all_edges.size(), 3);
    EXPECT_TRUE(std::find(all_edges.begin(), all_edges.end(), std::make_tuple(1, 0, 1)) != all_edges.end());
    EXPECT_TRUE(std::find(all_edges.begin(), all_edges.end(), std::make_tuple(2, 1, 2)) != all_edges.end());
    EXPECT_TRUE(std::find(all_edges.begin(), all_edges.end(), std::make_tuple(3, 2, 0)) != all_edges.end());
}
