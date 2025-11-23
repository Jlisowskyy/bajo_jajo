#include "algos.hpp"
#include "graph.hpp"
#include "gtest/gtest.h"

// Test fixture for Algos tests
class AlgosTest : public ::testing::Test
{
    protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Validates that GetMinimalEdgeExtension correctly identifies missing edges
TEST_F(AlgosTest, GetMinimalEdgeExtension_Verification)
{
    // Setup G1: A simple edge (0, 1) with weight 10
    Graph g1(2);
    g1.AddEdges(0, 1, 10);

    // Setup G2: Edge (0, 1) with weight 2 (Deficit of 8)
    Graph g2(2);
    g2.AddEdges(0, 1, 2);

    // Create an identity mapping: 0->0, 1->1
    Mapping mapping(2, 2);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    // Call function
    std::vector<EdgeExtension> extensions = GetMinimalEdgeExtension(g1, g2, mapping);

    // Assertions
    ASSERT_EQ(extensions.size(), 1);

    const EdgeExtension &ext = extensions[0];
    EXPECT_EQ(ext.u, 0);
    EXPECT_EQ(ext.v, 1);
    EXPECT_EQ(ext.mapped_u, 0);
    EXPECT_EQ(ext.mapped_v, 1);
    EXPECT_EQ(ext.weight_needed, 10);
    EXPECT_EQ(ext.weight_found, 2);

    // Check calculated deficit
    EXPECT_EQ(ext.weight_needed - ext.weight_found, 8);
}

// Validates no extension reported if G2 has sufficient weight
TEST_F(AlgosTest, GetMinimalEdgeExtension_NoDeficit)
{
    Graph g1(2);
    g1.AddEdges(0, 1, 5);

    Graph g2(2);
    g2.AddEdges(0, 1, 10);  // More than needed

    Mapping mapping(2, 2);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    std::vector<EdgeExtension> extensions = GetMinimalEdgeExtension(g1, g2, mapping);

    EXPECT_TRUE(extensions.empty());
}

// Validates extension when edge is completely missing in G2
TEST_F(AlgosTest, GetMinimalEdgeExtension_MissingEdge)
{
    Graph g1(2);
    g1.AddEdges(0, 1, 5);

    Graph g2(2);
    // No edges in G2

    Mapping mapping(2, 2);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    std::vector<EdgeExtension> extensions = GetMinimalEdgeExtension(g1, g2, mapping);

    ASSERT_EQ(extensions.size(), 1);
    EXPECT_EQ(extensions[0].weight_needed, 5);
    EXPECT_EQ(extensions[0].weight_found, 0);
}
