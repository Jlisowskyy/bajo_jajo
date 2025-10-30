#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include "graph.hpp"
#include "mcts.hpp"
#include "random_gen.hpp"

void PrintState(const State& state, const MCTS& mcts)
{
    std::cout << "\n=== Best Mapping Found ===" << std::endl;
    std::cout << "G1 Vertex -> G2 Vertex" << std::endl;
    std::cout << "----------------------" << std::endl;

    for (const auto& [g1_vertex, g2_vertex] : state.mapping) {
        std::cout << std::setw(9) << g1_vertex << " -> " << std::setw(9) << g2_vertex << std::endl;
    }

    int missing_edges = mcts.CalculateMissingEdges(state);
    std::cout << "\n=== Result ===" << std::endl;
    std::cout << "Minimum edges to add to G2: " << missing_edges << std::endl;
}

void PrintGraphInfo(const Graph& g, const std::string& name)
{
    std::cout << name << ": " << g.GetVertices() << " vertices, " << g.GetEdges() << " edges" << std::endl;
}

extern int LibMain()
{
    // Default parameters
    std::uint32_t size_g1      = 5;
    std::uint32_t size_g2      = 8;
    double density_g1          = 0.3;
    double density_g2          = 0.4;
    bool create_g1_based_on_g2 = false;
    int mcts_iterations        = 10000;

    std::cout << "=== MCTS Subgraph Isomorphism Solver ===" << std::endl;
    std::cout << "\nGenerating random graphs..." << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  G1 size: " << size_g1 << std::endl;
    std::cout << "  G2 size: " << size_g2 << std::endl;
    std::cout << "  G1 density: " << density_g1 << std::endl;
    std::cout << "  G2 density: " << density_g2 << std::endl;
    std::cout << "  Create G1 based on G2: " << (create_g1_based_on_g2 ? "yes" : "no") << std::endl;
    std::cout << "  MCTS iterations: " << mcts_iterations << std::endl;
    std::cout << std::endl;

    // Generate graphs
    auto [g1, g2] = GenerateExample(size_g1, size_g2, density_g1, density_g2, create_g1_based_on_g2);

    PrintGraphInfo(g1, "G1");
    PrintGraphInfo(g2, "G2");

    std::cout << "\nRunning MCTS search..." << std::endl;

    // Create MCTS solver
    MCTS mcts(g1, g2);

    // Run search
    State best_state = mcts.Search(mcts_iterations);

    // Print results
    PrintState(best_state, mcts);

    std::cout << "\n=== Interpretation ===" << std::endl;
    if (mcts.CalculateMissingEdges(best_state) == 0) {
        std::cout << "G1 is already a subgraph of G2 with this mapping!" << std::endl;
    } else {
        std::cout << "To make G1 a subgraph of G2, add the missing edges to G2" << std::endl;
        std::cout << "according to the mapping above." << std::endl;
    }

    return 0;
}
