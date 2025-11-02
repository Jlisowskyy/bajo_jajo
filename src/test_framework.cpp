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
    std::int32_t n1   = g1.GetVertices();

    for (std::int32_t u = 0; u < n1; ++u) {
        std::int32_t mapped_u = mapping.get_mapping_g1_to_g2(u);
        if (mapped_u == -1)
            continue;

        for (std::int32_t v = 0; v < n1; ++v) {
            const std::int32_t mapped_v = mapping.get_mapping_g1_to_g2(v);
            if (mapped_v == -1)
                continue;

            const std::uint32_t edges_in_g1 = g1.GetEdges(u, v);
            std::uint32_t edges_in_g2       = g2.GetEdges(mapped_u, mapped_v);

            if (edges_in_g1 > edges_in_g2) {
                missing_edges += (edges_in_g1 - edges_in_g2);
            }
        }
    }
    return missing_edges;
}

static bool VerifyMapping(const Graph &g1, const Graph &g2, const Mapping &mapping)
{
    std::int32_t n1 = g1.GetVertices();
    std::int32_t n2 = g2.GetVertices();

    // 1. Check if mapping is consistent (g1_to_g2 and g2_to_g1 are inverses)
    for (std::int32_t i = 0; i < n1; ++i) {
        std::int32_t mapped_g2_idx = mapping.get_mapping_g1_to_g2(i);
        if (mapped_g2_idx != -1) {
            if (mapped_g2_idx < 0 || mapped_g2_idx >= n2) {
                std::cerr << "Error: Mapped G2 index out of bounds for G1 vertex " << i << std::endl;
                return false;
            }
            if (mapping.get_mapping_g2_to_g1(mapped_g2_idx) != i) {
                std::cerr << "Error: Inconsistent mapping for G1 vertex " << i << ". G1->" << mapped_g2_idx
                          << " but G2->" << mapping.get_mapping_g2_to_g1(mapped_g2_idx) << std::endl;
                return false;
            }
        }
    }

    for (std::int32_t i = 0; i < n2; ++i) {
        std::int32_t mapped_g1_idx = mapping.get_mapping_g2_to_g1(i);
        if (mapped_g1_idx != -1) {
            if (mapped_g1_idx < 0 || mapped_g1_idx >= n1) {
                std::cerr << "Error: Mapped G1 index out of bounds for G2 vertex " << i << std::endl;
                return false;
            }
            if (mapping.get_mapping_g1_to_g2(mapped_g1_idx) != i) {
                std::cerr << "Error: Inconsistent reverse mapping for G2 vertex " << i << ". G2->" << mapped_g1_idx
                          << " but G1->" << mapping.get_mapping_g1_to_g2(mapped_g1_idx) << std::endl;
                return false;
            }
        }
    }

    // 2. Check mapped_count_ consistency
    int actual_mapped_count = 0;
    for (std::int32_t i = 0; i < n1; ++i) {
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

    std::cout << "--- Testing Correctness: " << algo1_name << " vs " << algo0_name << " ---\n";
    std::cout << std::left << std::setw(6) << "Idx" << std::setw(8) << "G1_S" << std::setw(8) << "G2_S" << std::setw(8)
              << "G1_D" << std::setw(8) << "G2_D" << std::setw(10) << "G1_on_G2" << std::setw(15) << "Algo0_Cost"
              << std::setw(15) << "Algo1_Cost" << std::setw(15) << "Algo0_Map_OK" << std::setw(15) << "Algo1_Map_OK"
              << std::setw(20) << "Algo0_Time (ms)" << std::setw(20) << "Algo1_Time (ms)" << "\n";
    std::cout << std::string(140, '-') << "\n";

    int idx = 0;
    for (const GraphSpec &spec : cases) {
        const auto [g1, g2] = GenerateExample(spec);

        auto start_precise      = std::chrono::high_resolution_clock::now();
        Mapping precise_mapping = algo0_func(g1, g2);
        auto end_precise        = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> precise_time = end_precise - start_precise;
        int precise_cost                                       = CalculateMissingEdges(g1, g2, precise_mapping);
        bool precise_mapping_ok                                = VerifyMapping(g1, g2, precise_mapping);

        auto start_approx      = std::chrono::high_resolution_clock::now();
        Mapping approx_mapping = algo1_func(g1, g2);
        auto end_approx        = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> approx_time = end_approx - start_approx;
        int approx_cost                                       = CalculateMissingEdges(g1, g2, approx_mapping);
        bool approx_mapping_ok                                = VerifyMapping(g1, g2, approx_mapping);

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
// ApproxOnPrecise
// ------------------------------

static constexpr std::array ApproxOnPreciseSpec = {
    GraphSpec{ 3,  3, 1.5, 1.0, false},
    GraphSpec{ 4,  4, 2.0, 0.7, false},
    GraphSpec{ 5,  5, 0.8, 1.2, false},
    GraphSpec{ 6,  6, 2.5, 2.5, false},

    // Different number of vertices (g1 < g2), varying densities
    GraphSpec{ 3,  5, 1.0, 1.5, false},
    GraphSpec{ 4,  6, 1.2, 2.0, false},
    GraphSpec{ 5,  7, 0.9, 1.8, false},

    // G1 based on G2, varying densities and sizes
    GraphSpec{ 3,  5, 0.7, 2.0,  true},
    GraphSpec{ 4,  6, 0.8, 1.2,  true},
    GraphSpec{ 5, 11, 0.4, 1.3,  true},
    GraphSpec{ 5,  9, 0.9, 2.1,  true},

    // Mixed cases with varied densities
    GraphSpec{ 7,  7, 0.5, 2.0, false},
    GraphSpec{ 8,  8, 2.0, 0.5, false},
    GraphSpec{ 4,  4, 0.2, 2.5, false},
    GraphSpec{ 5, 10, 0.9, 1.2,  true},

    // Big graphs
    GraphSpec{11, 13, 0.5, 1.0, false},
    GraphSpec{12, 12, 0.6, 1.3, false},
    GraphSpec{11, 13, 0.8, 2.0,  true},
    GraphSpec{12, 12, 0.9, 2.1,  true},
};

void TestCorrectnessApproxOnPrecise(const ApproxAlgo approx_algo_enum, const PreciseAlgo precise_algo_enum)
{
    RunTest_(
        ApproxOnPreciseSpec, kApproxAlgos[static_cast<size_t>(approx_algo_enum)],
        kPreciseAlgos[static_cast<size_t>(precise_algo_enum)]
    );
}

// ------------------------------
// PreciseOnPrecise
// ------------------------------

void TestCorrectnessPreciseOnPrecise(PreciseAlgo precise_algo, PreciseAlgo precise_algo1) {}

// ------------------------------
// ApproxOnApproxTime
// ------------------------------

void TestApproxOnApproxTime(ApproxAlgo approx_algo, ApproxAlgo approx_algo1) {}

// ------------------------------
// PreciseOnPreciseTime
// ------------------------------

void TestPreciseOnPreciseTime(PreciseAlgo precise_algo, PreciseAlgo precise_algo1) {}
