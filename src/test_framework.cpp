#include "test_framework.hpp"
#include "random_gen.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

// ------------------------------
// Helpers
// ------------------------------

static int CalculateMissingEdges(const Graph &g1, const Graph &g2, const Mapping &mapping)
{
    int missing_edges = 0;
    const Vertices n1 = g1.GetVertices();

    for (Vertex u = 0; u < n1; ++u) {
        const MappedVertex mapped_u = mapping.get_mapping_g1_to_g2(u);
        if (mapped_u == -1) {
            continue;
        }

        for (Vertex v = 0; v < n1; ++v) {
            const MappedVertex mapped_v = mapping.get_mapping_g1_to_g2(v);
            if (mapped_v == -1) {
                continue;
            }

            const Edges edges_in_g1 = g1.GetEdges(u, v);
            const Edges edges_in_g2 = g2.GetEdges(mapped_u, mapped_v);
            if (edges_in_g1 > edges_in_g2) {
                missing_edges += static_cast<int>(edges_in_g1 - edges_in_g2);
            }
        }
    }
    return missing_edges;
}

static bool VerifyMapping(const Graph &g1, const Graph &g2, const Mapping &mapping)
{
    const Vertices n1 = g1.GetVertices();
    const Vertices n2 = g2.GetVertices();

    // 1. Check if mapping is consistent (g1_to_g2 and g2_to_g1 are inverses)
    for (Vertex i = 0; i < n1; ++i) {
        const MappedVertex mapped_g2_idx = mapping.get_mapping_g1_to_g2(i);
        if (mapped_g2_idx != -1) {
            if (mapped_g2_idx < 0 || mapped_g2_idx >= static_cast<MappedVertex>(n2)) {
                std::cerr << "Error: Mapped G2 index out of bounds for G1 vertex " << i << std::endl;
                return false;
            }
            if (mapping.get_mapping_g2_to_g1(mapped_g2_idx) != static_cast<MappedVertex>(i)) {
                std::cerr << "Error: Inconsistent mapping for G1 vertex " << i << ". G1->" << mapped_g2_idx
                          << " but G2->" << mapping.get_mapping_g2_to_g1(mapped_g2_idx) << std::endl;
                return false;
            }
        }
    }

    for (Vertex i = 0; i < n2; ++i) {
        const MappedVertex mapped_g1_idx = mapping.get_mapping_g2_to_g1(i);
        if (mapped_g1_idx != -1) {
            if (mapped_g1_idx < 0 || mapped_g1_idx >= static_cast<MappedVertex>(n1)) {
                std::cerr << "Error: Mapped G1 index out of bounds for G2 vertex " << i << std::endl;
                return false;
            }
            if (mapping.get_mapping_g1_to_g2(mapped_g1_idx) != static_cast<MappedVertex>(i)) {
                std::cerr << "Error: Inconsistent reverse mapping for G2 vertex " << i << ". G2->" << mapped_g1_idx
                          << " but G1->" << mapping.get_mapping_g1_to_g2(mapped_g1_idx) << std::endl;
                return false;
            }
        }
    }

    // 2. Check mapped_count_ consistency
    Vertices actual_mapped_count = 0;
    for (Vertex i = 0; i < n1; ++i) {
        if (mapping.is_g1_mapped(i)) {
            actual_mapped_count++;
        }
    }
    if (actual_mapped_count != mapping.get_mapped_count()) {
        std::cerr << "Error: Mapped count mismatch. Actual: " << actual_mapped_count
                  << ", Stored: " << mapping.get_mapped_count() << std::endl;
        return false;
    }

    // 3. Check if all g1 vertices are mapped if n1 <= n2
    if (n1 <= n2) {
        if (mapping.get_mapped_count() != n1) {
            std::cerr << "Error: Not all G1 vertices mapped when G1 size <= G2 size. Mapped: "
                      << mapping.get_mapped_count() << ", G1 size: " << n1 << std::endl;
            return false;
        }
    } else {  // n1 > n2, max mapped_count can be n2
        if (mapping.get_mapped_count() > n2) {
            std::cerr << "Error: More G1 vertices mapped than available G2 vertices. Mapped: "
                      << mapping.get_mapped_count() << ", G2 size: " << n2 << std::endl;
            return false;
        }
    }

    return true;
}

template <class CollT>
static void RunTest_(const CollT &cases, std::tuple<SigT, const char *> algo0, std::tuple<SigT, const char *> algo1)
{
    const auto &[algo0_func, algo0_name] = algo0;
    const auto &[algo1_func, algo1_name] = algo1;

    std::cout << "--- Testing Correctness: " << algo0_name << " vs " << algo1_name << " ---\n";
    std::cout << std::left << std::setw(6) << "Idx" << std::setw(8) << "G1_S" << std::setw(8) << "G2_S" << std::setw(8)
              << "G1_D" << std::setw(8) << "G2_D" << std::setw(10) << "G1_on_G2" << std::setw(15) << "Algo0_Cost"
              << std::setw(15) << "Algo1_Cost" << std::setw(15) << "Algo0_Map_OK" << std::setw(15) << "Algo1_Map_OK"
              << std::setw(20) << "Algo0_Time (ms)" << std::setw(20) << "Algo1_Time (ms)" << "\n";
    std::cout << std::string(140, '-') << "\n";

    int idx = 0;
    for (const GraphSpec &spec : cases) {
        const auto [g1, g2] = GenerateExample(spec);

        auto start_precise    = std::chrono::high_resolution_clock::now();
        auto precise_mappings = algo0_func(g1, g2, 1);
        auto end_precise      = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> precise_time = end_precise - start_precise;
        int precise_cost        = precise_mappings.empty() ? -1 : CalculateMissingEdges(g1, g2, precise_mappings[0]);
        bool precise_mapping_ok = precise_mappings.empty() ? false : VerifyMapping(g1, g2, precise_mappings[0]);

        auto start_approx    = std::chrono::high_resolution_clock::now();
        auto approx_mappings = algo1_func(g1, g2, 1);
        auto end_approx      = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> approx_time = end_approx - start_approx;
        int approx_cost        = approx_mappings.empty() ? -1 : CalculateMissingEdges(g1, g2, approx_mappings[0]);
        bool approx_mapping_ok = approx_mappings.empty() ? false : VerifyMapping(g1, g2, approx_mappings[0]);

        std::cout << std::left << std::setw(6) << idx++ << std::setw(8) << spec.size_g1 << std::setw(8) << spec.size_g2
                  << std::setw(8) << std::fixed << std::setprecision(1) << spec.density_g1 << std::setw(8) << std::fixed
                  << std::setprecision(1) << spec.density_g2 << std::setw(10)
                  << (spec.create_g1_based_on_g2 ? "Yes" : "No") << std::setw(15) << precise_cost << std::setw(15)
                  << approx_cost << std::setw(15) << (precise_mapping_ok ? "OK" : "FAIL") << std::setw(15)
                  << (approx_mapping_ok ? "OK" : "FAIL") << std::setw(20) << std::fixed << std::setprecision(3)
                  << precise_time.count() << std::setw(20) << std::fixed << std::setprecision(3) << approx_time.count()
                  << "\n";
    }
    std::cout << std::string(140, '-') << "\n";
}

// ------------------------------
// Test cases
// ------------------------------

static constexpr std::array PreciseSpec = {
    GraphSpec{3,  3,  0.0,  0.0,        false              },
    GraphSpec{ 4,  4,  3.0,  0.0,        false},
    GraphSpec{
              4,  4,  0.0,  3.0, // --- Basic Sanity Checks ---
 false        },

    GraphSpec{ 5,  5,  1.0,  2.0,         true},
    GraphSpec{ 4,  6,  1.0,  1.8,         true},

    GraphSpec{ 6,  8,  0.8,  2.0,         true},
    GraphSpec{ 7,  9,  0.4,  1.5,         true},

    GraphSpec{ 8,  8,  2.5,  0.5,        false},
    GraphSpec{ 6,  9,  1.5,  1.5,        false},

    GraphSpec{ 7,  8, 30.0, 35.0,        false},
    GraphSpec{ 6,  9,  0.9, 30.0,         true},

    GraphSpec{ 9,  9,  1.2,  1.5,        false},
    GraphSpec{ 8, 10,  0.9,  1.2,         true},
    GraphSpec{10, 10,   30, 40.0,        false},
    GraphSpec{11, 11,   30, 40.0,        false},
    GraphSpec{ 9, 12,   20, 10.0,        false},
};

static constexpr std::array ApproxSpec = {
    GraphSpec{50,  70,  1.0, 1.5, true},
    GraphSpec{60,  80,  0.9, 1.8, true},
    GraphSpec{70,  90, 0.75, 1.2, true},

    GraphSpec{80, 100,  0.5, 2.0, true},
    GraphSpec{90, 110,  0.3, 1.0, true},

    // GraphSpec{100, 100,  1.5,  1.5, false},
    // GraphSpec{120, 120,  0.8,  2.5, false},
    // GraphSpec{100, 150,  1.2,  1.2, false},

    // GraphSpec{100, 120, 30.0, 25.0, false},
    // GraphSpec{ 80, 100,  0.8, 40.0,  true},
    // GraphSpec{150, 150, 30.0, 30.0, false},
    //
    // GraphSpec{ 30, 150,  0.8,  2.0,  true},
    // GraphSpec{150, 150,  0.5,  3.0, false},
};

// ------------------------------
// ApproxOnPrecise
// ------------------------------

void TestApproxOnPrecise(const ApproxAlgo approx_algo_enum, const PreciseAlgo precise_algo_enum)
{
    RunTest_(
        PreciseSpec, kApproxAlgos[static_cast<size_t>(approx_algo_enum)],
        kPreciseAlgos[static_cast<size_t>(precise_algo_enum)]
    );
}

// ------------------------------
// PreciseOnPrecise
// ------------------------------

void TestPreciseOnPrecise(PreciseAlgo precise_algo, PreciseAlgo precise_algo1)
{
    RunTest_(
        PreciseSpec, kPreciseAlgos[static_cast<size_t>(precise_algo)], kPreciseAlgos[static_cast<size_t>(precise_algo1)]
    );
}

// ------------------------------
// ApproxOnApproxTime
// ------------------------------

void TestApproxOnApprox(ApproxAlgo approx_algo, ApproxAlgo approx_algo1)
{
    RunTest_(
        ApproxSpec, kApproxAlgos[static_cast<size_t>(approx_algo)], kApproxAlgos[static_cast<size_t>(approx_algo1)]
    );
}
