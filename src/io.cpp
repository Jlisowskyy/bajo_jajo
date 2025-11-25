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

static void PrintMappingTable(const Graph &g1, const Graph &g2, const Mapping &mapping)
{
    std::vector<std::string> headers = {"G1", "G2", "deg1", "deg2"};
    std::vector<std::vector<std::string>> rows;
    rows.reserve(g1.GetVertices());

    for (Vertex i = 0; i < g1.GetVertices(); ++i) {
        std::vector<std::string> row;

        // G1 index
        row.push_back([&]() {
            std::ostringstream os;
            os << i;
            return os.str();
        }());

        // mapped G2 vertex
        Vertex mapped = mapping.get_mapping_g1_to_g2(i);
        row.push_back([&]() {
            std::ostringstream os;
            os << mapped;
            return os.str();
        }());

        // deg1
        row.push_back([&]() {
            std::ostringstream os;
            os << g1.GetDegree(i);
            return os.str();
        }());

        // deg2
        row.push_back([&]() {
            std::ostringstream os;
            os << g2.GetDegree(mapped);
            return os.str();
        }());

        rows.push_back(std::move(row));
    }

    // Compute dynamic column widths
    size_t cols = headers.size();
    std::vector<size_t> widths(cols, 0);

    for (size_t c = 0; c < cols; ++c) widths[c] = headers[c].size();

    for (const auto &r : rows)
        for (size_t c = 0; c < cols; ++c) widths[c] = std::max(widths[c], r[c].size());

    for (size_t c = 0; c < cols; ++c) widths[c] += 2;  // left + right padding

    // Top border
    std::cout << "  +";
    for (size_t c = 0; c < cols; ++c) std::cout << std::string(widths[c], '-') << "+";
    std::cout << "\n";

    // Header row
    std::cout << "  |";
    for (size_t c = 0; c < cols; ++c) {
        std::ostringstream cell;
        cell << " " << headers[c];
        std::cout << std::left << std::setw((int)widths[c]) << cell.str() << "|";
    }
    std::cout << "\n";

    // Header separator
    std::cout << "  +";
    for (size_t c = 0; c < cols; ++c) std::cout << std::string(widths[c], '-') << "+";
    std::cout << "\n";

    // Table rows
    for (const auto &r : rows) {
        std::cout << "  |";
        for (size_t c = 0; c < cols; ++c) {
            std::string cell = " " + r[c];
            std::cout << std::left << std::setw((int)widths[c]) << cell << "|";
        }
        std::cout << "\n";
    }

    // Bottom border
    std::cout << "  +";
    for (size_t c = 0; c < cols; ++c) std::cout << std::string(widths[c], '-') << "+";
    std::cout << "\n\n";
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
        std::cout << "Cost (Added Edges): " << cost << "\n";
        std::cout << "\n  === " << "Vertices mapping" << " === \n";
        PrintMappingTable(g1, g2, mapping);
        if (cost > 0) {
            std::cout << "  === " << "Minimal Ege Extension" << " === ";
            std::vector<std::string> headers = {"#", "G1 edge", "Mapped to G2 edge", "cost"};

            std::vector<std::vector<std::string>> rows;
            int idx = 1;
            for (const auto &ext : extensions) {
                std::vector<std::string> row;

                // index
                row.push_back([&]() {
                    std::ostringstream os;
                    os << idx;
                    return os.str();
                }());

                // G1 edge "(u,v) [weight_needed]"
                row.push_back([&]() {
                    std::ostringstream os;
                    os << "(" << ext.u << "," << ext.v << ") [" << ext.weight_needed << "]";
                    return os.str();
                }());

                // Mapped pair "(mapped_u,mapped_v) [weight_found]"
                row.push_back([&]() {
                    std::ostringstream os;
                    os << "(" << ext.mapped_u << "," << ext.mapped_v << ") [" << ext.weight_found << "]";
                    return os.str();
                }());

                // cost delta with sign
                row.push_back([&]() {
                    int delta = static_cast<int>(ext.weight_needed) - static_cast<int>(ext.weight_found);
                    std::ostringstream os;
                    if (delta >= 0)
                        os << "+";
                    os << delta;
                    return os.str();
                }());

                rows.push_back(std::move(row));
                ++idx;
            }

            // Compute column widths (max of header and all rows)
            const size_t cols = headers.size();
            std::vector<size_t> widths(cols, 0);
            for (size_t c = 0; c < cols; ++c) widths[c] = headers[c].size();
            for (const auto &r : rows) {
                for (size_t c = 0; c < cols; ++c) {
                    if (c < r.size())
                        widths[c] = std::max(widths[c], r[c].size());
                }
            }

            // Add minimal padding (1 space on left and right inside the cell)
            for (size_t c = 0; c < cols; ++c) widths[c] += 2;  // one leading, one trailing space

            // Print top border
            std::cout << "\n  +";
            for (size_t c = 0; c < cols; ++c) {
                std::cout << std::string(widths[c], '-') << "+";
            }
            std::cout << "\n";

            // Print header row
            std::cout << "  |";
            for (size_t c = 0; c < cols; ++c) {
                // print header centered-ish: we'll left-align the text with one leading space included already in
                // widths
                std::ostringstream cell;
                cell << " " << headers[c];  // leading space
                std::cout << std::left << std::setw((int)widths[c]) << cell.str() << "|";
            }
            std::cout << "\n";

            // Print header-bottom border
            std::cout << "  +";
            for (size_t c = 0; c < cols; ++c) {
                std::cout << std::string(widths[c], '-') << "+";
            }
            std::cout << "\n";

            // Print rows
            for (const auto &r : rows) {
                std::cout << "  |";
                for (size_t c = 0; c < cols; ++c) {
                    std::string cell = (c < r.size() ? r[c] : "");
                    // add one leading space to visually separate from border
                    cell = " " + cell;

                    // For numeric-looking "cost" column, right-align; others left-align
                    if (c == 3) {  // cost column (0-based index)
                        std::cout << std::right << std::setw((int)widths[c]) << cell << "|";
                        std::cout << std::left;  // reset to left for next columns
                    } else {
                        std::cout << std::left << std::setw((int)widths[c]) << cell << "|";
                    }
                }
                std::cout << "\n";
            }

            // Print bottom border
            std::cout << "  +";
            for (size_t c = 0; c < cols; ++c) {
                std::cout << std::string(widths[c], '-') << "+";
            }
            std::cout << "\n\n";
        }
    }
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
