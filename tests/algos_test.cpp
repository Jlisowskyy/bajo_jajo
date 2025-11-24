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

// ========================================
// GetMinimalExtension Tests
// ========================================

TEST_F(AlgosTest, GetMinimalExtension_BasicExtension)
{
    Graph g1(2);
    g1.AddEdges(0, 1, 10);

    Graph g2(2);
    g2.AddEdges(0, 1, 2);

    Mapping mapping(2, 2);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    Graph extendedG2 = GetMinimalExtension(g1, g2, mapping);

    EXPECT_EQ(extendedG2.GetEdges(0, 1), 10);
    EXPECT_EQ(extendedG2.GetVertices(), 2);
}

// Validates no extension needed when G2 already has sufficient edges
TEST_F(AlgosTest, GetMinimalExtension_NoExtensionNeeded)
{
    Graph g1(2);
    g1.AddEdges(0, 1, 5);

    Graph g2(2);
    g2.AddEdges(0, 1, 10);

    Mapping mapping(2, 2);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    Graph extendedG2 = GetMinimalExtension(g1, g2, mapping);

    EXPECT_EQ(extendedG2.GetEdges(0, 1), 10);
}

// Validates extension when edge is completely missing in G2
TEST_F(AlgosTest, GetMinimalExtension_MissingEdgeInG2)
{
    Graph g1(2);
    g1.AddEdges(0, 1, 8);

    Graph g2(2);

    Mapping mapping(2, 2);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    Graph extendedG2 = GetMinimalExtension(g1, g2, mapping);

    EXPECT_EQ(extendedG2.GetEdges(0, 1), 8);
}

// Validates extension with multiple edges needing extension
TEST_F(AlgosTest, GetMinimalExtension_MultipleEdges)
{
    Graph g1(3);
    g1.AddEdges(0, 1, 5);
    g1.AddEdges(1, 2, 7);
    g1.AddEdges(0, 2, 3);

    Graph g2(3);
    g2.AddEdges(0, 1, 2);  // Deficit of 3
    g2.AddEdges(1, 2, 7);  // No deficit

    Mapping mapping(3, 3);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);
    mapping.set_mapping(2, 2);

    Graph extendedG2 = GetMinimalExtension(g1, g2, mapping);

    EXPECT_EQ(extendedG2.GetEdges(0, 1), 5);
    EXPECT_EQ(extendedG2.GetEdges(1, 2), 7);
    EXPECT_EQ(extendedG2.GetEdges(0, 2), 3);
}

// Validates extension with unmapped vertices (partial mapping)
TEST_F(AlgosTest, GetMinimalExtension_PartialMapping)
{
    Graph g1(3);
    g1.AddEdges(0, 1, 4);
    g1.AddEdges(1, 2, 6);
    g1.AddEdges(0, 2, 2);

    Graph g2(4);
    g2.AddEdges(0, 1, 1);  // Deficit of 3
    // Vertices 2 is unmapped in G1

    Mapping mapping(3, 4);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    Graph extendedG2 = GetMinimalExtension(g1, g2, mapping);

    EXPECT_EQ(extendedG2.GetEdges(0, 1), 4);
    EXPECT_EQ(extendedG2.GetEdges(1, 2), 0);
    EXPECT_EQ(extendedG2.GetEdges(0, 2), 0);
}

// Validates extension with self-loops
TEST_F(AlgosTest, GetMinimalExtension_SelfLoops)
{
    Graph g1(2);
    g1.AddEdges(0, 0, 5);
    g1.AddEdges(0, 1, 3);

    Graph g2(2);
    g2.AddEdges(0, 0, 2);  // Deficit of 3
    g2.AddEdges(0, 1, 3);  // No deficit

    Mapping mapping(2, 2);
    mapping.set_mapping(0, 0);
    mapping.set_mapping(1, 1);

    Graph extendedG2 = GetMinimalExtension(g1, g2, mapping);

    EXPECT_EQ(extendedG2.GetEdges(0, 0), 5);
    EXPECT_EQ(extendedG2.GetEdges(0, 1), 3);
}

// Validates extension with non-identity mapping
TEST_F(AlgosTest, GetMinimalExtension_NonIdentityMapping)
{
    Graph g1(3);
    g1.AddEdges(0, 1, 4);
    g1.AddEdges(1, 2, 6);

    Graph g2(3);
    g2.AddEdges(2, 1, 1);  // Maps to (0, 1) in G1, deficit of 3

    Mapping mapping(3, 3);
    mapping.set_mapping(0, 2);
    mapping.set_mapping(1, 1);
    mapping.set_mapping(2, 0);

    Graph extendedG2 = GetMinimalExtension(g1, g2, mapping);

    EXPECT_EQ(extendedG2.GetEdges(2, 1), 4);  // (0, 1) in G1 maps to (2, 1) in G2
    EXPECT_EQ(extendedG2.GetEdges(1, 0), 6);  // (1, 2) in G1 maps to (1, 0) in G2
}
