#include <io.hpp>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// ------------------------------
// statics
// ------------------------------

static int CalculateMissingEdges(const Graph &g1, const Graph &g2, const Mapping &mapping)
{
    int missing_edges = 0;

    g1.IterateEdges([&](const Edges edges_in_g1, const Vertex u, const Vertex v) {
        const MappedVertex mapped_u = mapping.get_mapping_g1_to_g2(u);
        const MappedVertex mapped_v = mapping.get_mapping_g1_to_g2(v);

        if (mapped_u != -1 && mapped_v != -1) {
            const Edges edges_in_g2 = g2.GetEdges(mapped_u, mapped_v);
            if (edges_in_g1 > edges_in_g2) {
                missing_edges += static_cast<int>(edges_in_g1 - edges_in_g2);
            }
        }
    });

    return missing_edges;
}

// ------------------------------
// implementations
// ------------------------------

std::pair<Graph, Graph> Read(const std::filesystem::path &file)
{
    std::ifstream file_stream(file);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + file.string());
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
    std::cout << "Number of solutions found: " << mappings.size() << "\n";

    // --- Results ---
    if (mappings.empty()) {
        std::cout << "No valid mapping found.\n";
        return;
    }

    std::cout << "--- Best Mapping Results ---\n";
    int result_idx = 1;
    for (const auto &mapping : mappings) {
        const int cost = CalculateMissingEdges(g1, g2, mapping);

        std::cout << "\nResult " << result_idx++ << ":\n";
        std::cout << "  - Cost (Missing Edges): " << cost << "\n";
        std::cout << "  - Mapping (G1 -> G2):\n";

        for (Vertex i = 0; i < g1.GetVertices(); ++i) {
            std::cout << "    " << i << " -> " << mapping.get_mapping_g1_to_g2(i) << "\n";
        }
    }
    std::cout << "--------------------------\n";
}

void Write(const std::filesystem::path &file, const std::tuple<Graph, Graph> &graphs)
{
    std::ofstream file_stream(file);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Error: Could not open file for writing: " + file.string());
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
