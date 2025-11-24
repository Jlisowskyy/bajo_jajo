#include <io.hpp>

#include "algos.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// ------------------------------
// implementations
// ------------------------------

std::pair<Graph, Graph> Read(const char *file)
{
    std::ifstream file_stream(file);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + std::string(file));
    }

    auto read_single_graph = [](std::ifstream &fs) {
        Vertices size{};
        fs >> size;
        if (fs.fail() || size == 0) {
            throw std::runtime_error("Error reading or invalid graph size.");
        }

        Graph g(size);
        for (Vertex i = 0; i < size; ++i) {
            for (Vertex j = 0; j < size; ++j) {
                Edges edges{};
                fs >> edges;
                if (fs.fail()) {
                    throw std::runtime_error("Error reading adjacency matrix.");
                }
                if (edges > 0) {
                    g.AddEdges(i, j, edges);
                }
            }
        }
        return g;
    };

    auto g1 = read_single_graph(file_stream);
    auto g2 = read_single_graph(file_stream);
    return std::make_pair(std::move(g1), std::move(g2));
}

void Write(const Graph &g1, const Graph &g2, const std::vector<Mapping> &mappings, std::uint64_t time_spent_ns)
{
    std::cout << "Execution Time: " << std::fixed << std::setprecision(4) << time_spent_ns / 1'000'000.0 << " ms\n";

    // --- Results ---
    if (mappings.empty()) {
        std::cout << "No valid mapping found.\n";
        return;
    }
    for (const auto &mapping : mappings) {
        const std::vector<EdgeExtension> extensions = GetMinimalEdgeExtension(g1, g2, mapping);

        int cost = 0;
        for (const auto &ext : extensions) {
            cost += static_cast<int>(ext.weight_needed - ext.weight_found);
        }

        std::cout << "Result " << ":\n";
        std::cout << "  - Cost (Added Edges): " << cost << "\n";
        std::cout << "  - Mapping (G1 -> G2):\n";
        for (Vertex i = 0; i < g1.GetVertices(); ++i) {
            std::cout << "    " << i << " -> " << mapping.get_mapping_g1_to_g2(i) << "\n";
        }
        if (cost > 0) {
            std::cout << "\n  --- Minimal Edge Extension ---\n";
            for (const auto &ext : extensions) {
                // Format: G1 Edge (2, 3) [Weight 10] -> Maps to G2 (5, 8) [Weight 2] : Add Weight +8
                std::cout << "    G1 Edge (" << ext.u << ", " << ext.v << ") [Weight " << ext.weight_needed
                          << "] -> Maps to G2 (" << ext.mapped_u << ", " << ext.mapped_v << ") [Weight "
                          << ext.weight_found << "] : Add Weight +" << (ext.weight_needed - ext.weight_found) << "\n";
            }
        }
    }
    std::cout << "--------------------------\n";
}

void Write(const char *file, const std::tuple<Graph, Graph> &graphs)
{
    std::ofstream file_stream(file);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Error: Could not open file for writing: " + std::string(file));
    }

    const auto &[g1, g2] = graphs;

    const auto size1 = g1.GetVertices();
    file_stream << size1 << "\n";
    for (Vertex i = 0; i < size1; ++i) {
        for (Vertex j = 0; j < size1; ++j) {
            file_stream << g1.GetEdges(i, j) << (j == size1 - 1 ? "" : " ");
        }
        file_stream << "\n";
    }

    const auto size2 = g2.GetVertices();
    file_stream << size2 << "\n";
    for (Vertex i = 0; i < size2; ++i) {
        for (Vertex j = 0; j < size2; ++j) {
            file_stream << g2.GetEdges(i, j) << (j == size2 - 1 ? "" : " ");
        }
        file_stream << "\n";
    }
}

void WriteResult(const char *file, const Graph &g)
{
    // Create parent directories if they don't exist
    std::filesystem::path file_path(file);
    if (file_path.has_parent_path()) {
        std::filesystem::create_directories(file_path.parent_path());
    }

    std::ofstream file_stream(file);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Error: Could not open file for writing: " + std::string(file));
    }

    const auto size = g.GetVertices();
    file_stream << size << "\n";
    for (Vertex i = 0; i < size; ++i) {
        for (Vertex j = 0; j < size; ++j) {
            file_stream << g.GetEdges(i, j) << (j == size - 1 ? "" : " ");
        }
        file_stream << "\n";
    }
}
