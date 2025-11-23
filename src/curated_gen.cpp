#include "curated_gen.hpp"

#include <cmath>
#include <random>
#include <tuple>
#include <vector>

// -----------------------------------------------------------------------------
// Topology Builders (Base Structures)
// -----------------------------------------------------------------------------

static Graph BuildClique(int n)
{
    Graph g(static_cast<Vertices>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                g.AddEdges(static_cast<Vertex>(i), static_cast<Vertex>(j));
            }
        }
    }
    return g;
}

static Graph BuildGrid(int width, int height)
{
    int n = width * height;
    Graph g(static_cast<Vertices>(n));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Vertex u = static_cast<Vertex>(y * width + x);
            if (x + 1 < width) {
                Vertex v = static_cast<Vertex>(y * width + (x + 1));
                g.AddEdges(u, v);
                g.AddEdges(v, u);
            }
            if (y + 1 < height) {
                Vertex v = static_cast<Vertex>((y + 1) * width + x);
                g.AddEdges(u, v);
                g.AddEdges(v, u);
            }
        }
    }
    return g;
}

static Graph BuildLadder(int len)
{
    int n = 2 * len;
    Graph g(static_cast<Vertices>(n));
    for (int i = 0; i < len; ++i) {
        // Rung
        g.AddEdges(static_cast<Vertex>(i), static_cast<Vertex>(len + i));
        g.AddEdges(static_cast<Vertex>(len + i), static_cast<Vertex>(i));
        // Rails
        if (i + 1 < len) {
            g.AddEdges(static_cast<Vertex>(i), static_cast<Vertex>(i + 1));
            g.AddEdges(static_cast<Vertex>(i + 1), static_cast<Vertex>(i));
            g.AddEdges(static_cast<Vertex>(len + i), static_cast<Vertex>(len + i + 1));
            g.AddEdges(static_cast<Vertex>(len + i + 1), static_cast<Vertex>(len + i));
        }
    }
    return g;
}

static Graph BuildPetersen()
{
    Graph g(10);
    for (int i = 0; i < 5; ++i) {
        g.AddEdges(i, (i + 1) % 5);
        g.AddEdges((i + 1) % 5, i);
    }
    for (int i = 0; i < 5; ++i) {
        Vertex u = 5 + i;
        Vertex v = 5 + ((i + 2) % 5);
        g.AddEdges(u, v);
        g.AddEdges(v, u);
    }
    for (int i = 0; i < 5; ++i) {
        g.AddEdges(i, i + 5);
        g.AddEdges(i + 5, i);
    }
    return g;
}

static Graph BuildBinaryTree(int depth)
{
    int nodes = (1 << depth) - 1;
    Graph g(static_cast<Vertices>(nodes));
    for (int i = 0; i < nodes; ++i) {
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < nodes) {
            g.AddEdges(i, left);
            g.AddEdges(left, i);
        }
        if (right < nodes) {
            g.AddEdges(i, right);
            g.AddEdges(right, i);
        }
    }
    return g;
}

static Graph BuildRandom(int n, double density, int seed)
{
    Graph g(static_cast<Vertices>(n));
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (dis(gen) < density) {
                g.AddEdges(i, j);
                g.AddEdges(j, i);
            }
        }
    }
    return g;
}

// -----------------------------------------------------------------------------
// Brutal Multigraph Helpers
// -----------------------------------------------------------------------------

// Builds a K_n where edge (i,j) weight = (i*j + i + j) % modulus + 1
// This creates a highly specific "fingerprint" for every edge.
// Topological symmetry is complete (K_n), but edge-weight symmetry is broken chaotically.
static Graph BuildArithmeticClique(int n, int modulus)
{
    Graph g(static_cast<Vertices>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                int w = ((i * j) + i + j) % modulus + 1;
                g.AddEdges(static_cast<Vertex>(i), static_cast<Vertex>(j), static_cast<Edges>(w));
            }
        }
    }
    return g;
}

// Bipartite graph K_{n,n} with extremely heavy edges (hundreds/thousands).
// Weights alternate: even rows get heavy weights, odd rows get light.
static Graph BuildHeavyBipartite(int n, int heavy_weight)
{
    int total_nodes = 2 * n;
    Graph g(static_cast<Vertices>(total_nodes));
    for (int i = 0; i < n; ++i) {      // Left Set
        for (int j = 0; j < n; ++j) {  // Right Set (indices n to 2n-1)
            int target = n + j;
            int w      = (i % 2 == 0) ? heavy_weight : 1;
            g.AddEdges(static_cast<Vertex>(i), static_cast<Vertex>(target), static_cast<Edges>(w));
            g.AddEdges(static_cast<Vertex>(target), static_cast<Vertex>(i), static_cast<Edges>(w));
        }
    }
    return g;
}

// -----------------------------------------------------------------------------
// Generator Implementations
// -----------------------------------------------------------------------------

std::vector<TestCase> GenerateAllCurated()
{
    std::vector<TestCase> cases;

    // =========================================================
    // 1. EXACT ALGORITHM TESTS (Simple Graphs, N ~ 10-12)
    // =========================================================

    cases.push_back(TestCase{"01_exact_petersen", []() {
                                 return std::make_pair(BuildPetersen(), BuildPetersen());
                             }});

    cases.push_back(TestCase{"02_exact_clique_11", []() {
                                 return std::make_pair(BuildClique(11), BuildClique(11));
                             }});

    cases.push_back(TestCase{"03_exact_grid_3x4", []() {
                                 return std::make_pair(BuildGrid(3, 4), BuildGrid(3, 4));
                             }});

    cases.push_back(TestCase{"04_exact_random_dense_12", []() {
                                 return std::make_pair(BuildRandom(12, 0.6, 42), BuildRandom(12, 0.6, 42));
                             }});

    // =========================================================
    // 2. APPROX ALGORITHM TESTS (Large Graphs, N ~ 50-100)
    // =========================================================

    cases.push_back(TestCase{"05_approx_grid_10x10", []() {
                                 return std::make_pair(BuildGrid(5, 5), BuildGrid(10, 10));
                             }});

    cases.push_back(TestCase{"06_approx_ladder_80", []() {
                                 return std::make_pair(BuildLadder(20), BuildLadder(40));
                             }});

    cases.push_back(TestCase{"07_approx_binary_tree", []() {
                                 return std::make_pair(BuildBinaryTree(5), BuildBinaryTree(6));
                             }});

    cases.push_back(TestCase{"08_approx_random_dense_60", []() {
                                 Graph g2 = BuildRandom(60, 0.5, 12345);
                                 Graph g1(30);
                                 for (int i = 0; i < 30; ++i) {
                                     for (int j = i + 1; j < 30; ++j) {
                                         if (g2.GetEdges(i, j) > 0) {
                                             g1.AddEdges(i, j);
                                             g1.AddEdges(j, i);
                                         }
                                     }
                                 }
                                 return std::make_pair(std::move(g1), std::move(g2));
                             }});

    cases.push_back(TestCase{"09_approx_sparse_100", []() {
                                 Graph g2 = BuildRandom(100, 0.08, 999);
                                 Graph g1(40);
                                 for (int i = 0; i < 40; ++i) {
                                     for (int j = i + 1; j < 40; ++j) {
                                         if (g2.GetEdges(i, j) > 0) {
                                             g1.AddEdges(i, j);
                                             g1.AddEdges(j, i);
                                         }
                                     }
                                 }
                                 return std::make_pair(std::move(g1), std::move(g2));
                             }});

    cases.push_back(TestCase{"10_approx_massive_star", []() {
                                 auto BuildMassiveStar = [](int n) {
                                     Graph g(static_cast<Vertices>(n));
                                     for (int i = 1; i < n; ++i) {
                                         g.AddEdges(0, i);
                                         g.AddEdges(i, 0);
                                     }
                                     return g;
                                 };
                                 return std::make_pair(BuildMassiveStar(50), BuildMassiveStar(100));
                             }});

    // =========================================================
    // 3. BRUTAL MULTIGRAPH TESTS (High Multiplicity, Constraints)
    // =========================================================

    // 11. The "Arithmetic" Clique (Exact)
    // N=10. K10 is highly symmetric (10! perms), but edge weights are unique/chaotic.
    // Forces the algo to find the ONLY valid mapping based on edge weights.
    // If it relies only on topology, it will explore too many states.
    cases.push_back(TestCase{"11_multi_arithmetic_clique_10", []() {
                                 // Modulus 20 creates weights 1..20.
                                 return std::make_pair(BuildArithmeticClique(10, 20), BuildArithmeticClique(10, 20));
                             }});

    // 12. The "Heavy" Bipartite (Approx/Exact)
    // N=20 (10+10). K10,10.
    // Half the rows have edge weights of 1000, half have 1.
    // This tests if the algo correctly handles massive edge counts and doesn't overflow or ignore them.
    cases.push_back(TestCase{"12_multi_heavy_bipartite_20", []() {
                                 // Weight 1000 for "heavy" edges
                                 return std::make_pair(BuildHeavyBipartite(10, 1000), BuildHeavyBipartite(10, 1000));
                             }});

    // 13. The "Deep Fail" (Exact)
    // G1: K8 where ONE edge has weight 50. All others 10.
    // G2: K8 where ALL edges have weight 10.
    // Topologically identical.
    // The algo must dive deep to realize the heavy edge in G1 cannot be satisfied by G2.
    // This punishes lookahead that only checks "connectivity" and not "capacity".
    cases.push_back(TestCase{"13_multi_deep_fail_K8", []() {
                                 Graph g1 = BuildClique(8);
                                 // BuildClique makes edges=1. Let's pump them.
                                 // Set all to 10.
                                 g1.IterateEdges([&](Edges, Vertex u, Vertex v) {
                                     g1.AddEdges(u, v, 9);  // 1 + 9 = 10
                                 });
                                 // Make (0,1) heavy
                                 g1.AddEdges(0, 1, 40);  // 10 + 40 = 50
                                 g1.AddEdges(1, 0, 40);

                                 Graph g2 = BuildClique(8);
                                 g2.IterateEdges([&](Edges, Vertex u, Vertex v) {
                                     g2.AddEdges(u, v, 9);  // All 10
                                 });

                                 return std::make_pair(std::move(g1), std::move(g2));
                             }});

    // 14. The "Sparse vs Dense" Multigraph (Approx)
    // G1 has few edges but HIGH weights.
    // G2 has many edges but LOW weights.
    // Total "edge volume" might be similar, but structure is different.
    // This tests if the heuristic gets confused by "strength" vs "connectivity".
    cases.push_back(TestCase{"14_multi_strength_mismatch", []() {
                                 // G1: Ladder 20 (40 nodes), Rails weight 100.
                                 Graph g1 = BuildLadder(20);
                                 g1.IterateEdges([&](Edges, Vertex u, Vertex v) {
                                     g1.AddEdges(u, v, 99);
                                 });

                                 // G2: Grid 10x10 (100 nodes), Standard weights (1).
                                 Graph g2 = BuildGrid(10, 10);

                                 // This should fail quickly if heuristics work, or take forever if they chase node
                                 // degree.
                                 return std::make_pair(std::move(g1), std::move(g2));
                             }});

    // 15. The "Modulo" Ring (Exact)
    // Ring of 12 nodes.
    // Edge (i, i+1) has weight 3^i % 100.
    // Strictly requires matching the sequence.
    cases.push_back(TestCase{"15_multi_modulo_ring_12", []() {
                                 auto BuildModRing = [](int n) {
                                     Graph g(static_cast<Vertices>(n));
                                     for (int i = 0; i < n; ++i) {
                                         int next   = (i + 1) % n;
                                         int weight = static_cast<int>(pow(3, i)) % 100 + 1;
                                         g.AddEdges(i, next, weight);
                                         g.AddEdges(next, i, weight);
                                     }
                                     return g;
                                 };
                                 return std::make_pair(BuildModRing(12), BuildModRing(12));
                             }});

    return cases;
}
