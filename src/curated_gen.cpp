#include "curated_gen.hpp"
#include "random_gen.hpp"

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
        g.AddEdges(static_cast<Vertex>(i), static_cast<Vertex>(len + i));
        g.AddEdges(static_cast<Vertex>(len + i), static_cast<Vertex>(i));
        if (i + 1 < len) {
            g.AddEdges(static_cast<Vertex>(i), static_cast<Vertex>(i + 1));
            g.AddEdges(static_cast<Vertex>(i + 1), static_cast<Vertex>(i));
            g.AddEdges(static_cast<Vertex>(len + i), static_cast<Vertex>(len + i + 1));
            g.AddEdges(static_cast<Vertex>(len + i + 1), static_cast<Vertex>(len + i));
        }
    }
    return g;
}

static Graph BuildPetersen(const int n)
{
    Graph g(n);
    const int h = n / 2;
    for (int i = 0; i < h; ++i) {
        g.AddEdges(i, (i + 1) % h);
        g.AddEdges((i + 1) % h, i);
    }
    for (int i = 0; i < h; ++i) {
        Vertex u = h + i;
        Vertex v = h + ((i + 2) % h);
        g.AddEdges(u, v);
        g.AddEdges(v, u);
    }
    for (int i = 0; i < h; ++i) {
        g.AddEdges(i, i + h);
        g.AddEdges(i + h, i);
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

// -----------------------------------------------------------------------------
// Brutal Multigraph Helpers
// -----------------------------------------------------------------------------

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

static Graph BuildHeavyBipartite(int n, int heavy_weight)
{
    int total_nodes = 2 * n;
    Graph g(static_cast<Vertices>(total_nodes));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
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

    cases.push_back(TestCase{"01_accurate_prepared_8_10", []() {
                                 return std::make_pair(BuildPetersen(8), BuildPetersen(10));
                             }});

    cases.push_back(TestCase{"02_accurate_prepared_7_10", []() {
                                 return std::make_pair(BuildClique(7), BuildClique(10));
                             }});

    cases.push_back(TestCase{"03_accurate_prepared_8_12", []() {
                                 return std::make_pair(BuildGrid(2, 4), BuildGrid(3, 4));
                             }});

    cases.push_back(TestCase{"04_accurate_random_10_10", []() {
                                 return GenerateExample({10, 10, 0.6, 1.3, false});
                             }});

    // =========================================================
    // 2. APPROX ALGORITHM TESTS (Large Graphs, N ~ 50-100)
    // =========================================================

    cases.push_back(TestCase{"05_approx_prepared_25_100", []() {
                                 return std::make_pair(BuildGrid(5, 5), BuildGrid(10, 10));
                             }});

    cases.push_back(TestCase{"06_approx_prepared_40_80", []() {
                                 return std::make_pair(BuildLadder(20), BuildLadder(40));
                             }});

    cases.push_back(TestCase{"07_approx_prepared_31_127", []() {
                                 return std::make_pair(BuildBinaryTree(5), BuildBinaryTree(7));
                             }});

    cases.push_back(TestCase{"08_approx_random_30_60", []() {
                                 return GenerateExample({30, 60, 1.4, 3.5, false});
                             }});

    cases.push_back(TestCase{"09_approx_random_40_100", []() {
                                 return GenerateExample({40, 100, 0.3, 2.137, true});
                             }});

    cases.push_back(TestCase{"10_approx_prepared_50_100", []() {
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

    cases.push_back(TestCase{"11_accurate_prepared_8_10", []() {
                                 return std::make_pair(BuildArithmeticClique(8, 20), BuildArithmeticClique(10, 21));
                             }});


    cases.push_back(TestCase{"12_accurate_prepared_20_26", []() {
                                 return std::make_pair(BuildHeavyBipartite(10, 1000), BuildHeavyBipartite(13, 1000));
                             }});

    cases.push_back(TestCase{"13_accurate_prepared_8_8", []() {
                                 Graph g1 = BuildClique(8);

                                 g1.IterateEdges([&](Edges, Vertex u, Vertex v) {
                                     g1.AddEdges(u, v, 9);
                                 });
                                 g1.AddEdges(0, 1, 40);
                                 g1.AddEdges(1, 0, 40);

                                 Graph g2 = BuildClique(8);
                                 g2.IterateEdges([&](Edges, Vertex u, Vertex v) {
                                     g2.AddEdges(u, v, 9);
                                 });

                                 return std::make_pair(std::move(g1), std::move(g2));
                             }});

    cases.push_back(TestCase{"14_approx_prepared_40_100", []() {
                                 Graph g1 = BuildLadder(20);
                                 g1.IterateEdges([&](Edges, Vertex u, Vertex v) {
                                     g1.AddEdges(u, v, 99);
                                 });

                                 Graph g2 = BuildGrid(10, 10);

                                 return std::make_pair(std::move(g1), std::move(g2));
                             }});

    cases.push_back(TestCase{"15_accurate_prepared_9_10", []() {
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
                                 return std::make_pair(BuildModRing(9), BuildModRing(10));
                             }});

    // =========================================================
    // 4. RANDOM ALGORITHM TESTS (From ApproxSpec)
    // =========================================================

    auto add_spec_case = [&](const std::string &name, const GraphSpec &spec) {
        cases.push_back(TestCase{name, [spec]() {
                                     return GenerateExample(spec);
                                 }});
    };

    add_spec_case("16_approx_random_50_70", GraphSpec{50, 70, 1.0, 1.5, true});
    add_spec_case("17_approx_random_60_80", GraphSpec{60, 80, 0.9, 1.8, true});
    add_spec_case("18_approx_random_70_90", GraphSpec{70, 90, 0.75, 1.2, true});
    add_spec_case("19_approx_random_80_100", GraphSpec{80, 100, 0.5, 2.0, true});
    add_spec_case("20_approx_random_90_110", GraphSpec{90, 110, 0.3, 1.0, true});

    add_spec_case("21_approx_random_50_70", GraphSpec{50, 70, 12, 10, false});
    add_spec_case("22_approx_random_60_80", GraphSpec{60, 80, 5, 7, false});
    add_spec_case("23_approx_random_70_90", GraphSpec{70, 90, 6, 5, false});
    add_spec_case("24_approx_random_80_100", GraphSpec{80, 100, 9.42, 6.72, false});
    add_spec_case("25_approx_random_90_110", GraphSpec{90, 110, 5.31, 8.54, false});

    return cases;
}
