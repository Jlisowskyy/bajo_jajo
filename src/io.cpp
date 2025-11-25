#include <io.hpp>

#include "algos.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// ------------------------------
// Helpers Implementations
// ------------------------------

static void PrintExtensionTable(std::ostream &os, const std::vector<EdgeExtension> &extensions)
{
    if (extensions.empty()) {
        return;
    }

    os << "\n=== Minimal Edge Extension ===\n";
    std::vector<std::string> headers = {"#", "G1 edge", "Mapped to G2 edge", "Cost"};

    std::vector<std::vector<std::string>> rows;
    int idx = 1;
    for (const auto &ext : extensions) {
        std::vector<std::string> row;

        row.push_back(std::to_string(idx++));

        {
            std::ostringstream ss;
            ss << "(" << ext.u << "," << ext.v << ") [" << ext.weight_needed << "]";
            row.push_back(ss.str());
        }

        {
            std::ostringstream ss;
            ss << "(" << ext.mapped_u << "," << ext.mapped_v << ") [" << ext.weight_found << "]";
            row.push_back(ss.str());
        }

        {
            int delta = static_cast<int>(ext.weight_needed) - static_cast<int>(ext.weight_found);
            std::ostringstream ss;
            if (delta > 0)
                ss << "+";  // should always be positive in extension context
            ss << delta;
            row.push_back(ss.str());
        }

        rows.push_back(std::move(row));
    }

    const size_t cols = headers.size();
    std::vector<size_t> widths(cols, 0);
    for (size_t c = 0; c < cols; ++c) widths[c] = headers[c].size();
    for (const auto &r : rows) {
        for (size_t c = 0; c < cols; ++c) {
            if (c < r.size())
                widths[c] = std::max(widths[c], r[c].size());
        }
    }
    for (size_t c = 0; c < cols; ++c) widths[c] += 2;

    auto print_border = [&](char junction = '+', char dash = '-') {
        os << "  " << junction;
        for (size_t c = 0; c < cols; ++c) {
            os << std::string(widths[c], dash) << junction;
        }
        os << "\n";
    };

    print_border();
    os << "  |";
    for (size_t c = 0; c < cols; ++c) {
        std::ostringstream cell;
        cell << " " << headers[c];
        os << std::left << std::setw((int)widths[c]) << cell.str() << "|";
    }
    os << "\n";
    print_border();

    for (const auto &r : rows) {
        os << "  |";
        for (size_t c = 0; c < cols; ++c) {
            std::string cell = (c < r.size() ? r[c] : "");
            cell             = " " + cell;
            if (c == 3)
                os << std::right << std::setw((int)widths[c]) << cell << "|";
            else
                os << std::left << std::setw((int)widths[c]) << cell << "|";
        }
        os << "\n";
    }
    print_border();
    os << "\n";
}

static void PrintVisualMatrix(std::ostream &os, const Graph &g_orig, const Graph &g_ext)
{
    const Vertices size = g_orig.GetVertices();

    std::vector<std::vector<std::string>> grid(size, std::vector<std::string>(size));

    for (Vertex i = 0; i < size; ++i) {
        for (Vertex j = 0; j < size; ++j) {
            Edges old_w = g_orig.GetEdges(i, j);
            Edges new_w = g_ext.GetEdges(i, j);
            Edges added = (new_w > old_w) ? (new_w - old_w) : 0;

            if (added > 0) {
                std::ostringstream ss;
                ss << "(" << old_w << "+" << added << ")";
                grid[i][j] = ss.str();
            } else {
                grid[i][j] = std::to_string(old_w);
            }
        }
    }

    std::vector<size_t> col_widths(size, 0);
    for (Vertex j = 0; j < size; ++j) {
        for (Vertex i = 0; i < size; ++i) {
            col_widths[j] = std::max(col_widths[j], grid[i][j].length());
        }
        col_widths[j] += 1;
    }

    for (Vertex i = 0; i < size; ++i) {
        for (Vertex j = 0; j < size; ++j) {
            os << std::right << std::setw((int)col_widths[j]) << grid[i][j] << " ";
        }
        os << "\n";
    }
}

static void PrintMappingTable(std::ostream &os, const Mapping &mapping, const Vertices g1_size)
{
    os << "\n=== Vertex Mapping ===\n";

    const int col_width  = 12;
    auto print_separator = [&]() {
        os << "  +" << std::string(col_width, '-') << "+" << std::string(col_width, '-') << "+\n";
    };

    print_separator();
    os << "  |" << std::left << std::setw(col_width) << " G1 Vertex"
       << "|" << std::left << std::setw(col_width) << " G2 Vertex" << "|\n";
    print_separator();

    for (Vertex i = 0; i < g1_size; ++i) {
        MappedVertex mapped_to = mapping.get_mapping_g1_to_g2(i);

        std::string g1_str = " " + std::to_string(i);
        std::string g2_str = (mapped_to != kUnmappedVertex) ? " " + std::to_string(mapped_to) : " -";

        os << "  |" << std::left << std::setw(col_width) << g1_str << "|" << std::left << std::setw(col_width) << g2_str
           << "|\n";
    }
    print_separator();
    os << "\n";
}

// ------------------------------
// Public API Implementations
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

void Write(const char *file, const std::tuple<Graph, Graph> &graphs)
{
    std::ofstream file_stream(file);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Error: Could not open file for writing: " + std::string(file));
    }

    const auto &[g1, g2] = graphs;

    auto write_g = [&](const Graph &g) {
        const auto size = g.GetVertices();
        file_stream << size << "\n";
        for (Vertex i = 0; i < size; ++i) {
            for (Vertex j = 0; j < size; ++j) {
                file_stream << g.GetEdges(i, j) << (j == size - 1 ? "" : " ");
            }
            file_stream << "\n";
        }
    };

    write_g(g1);
    write_g(g2);
}

// --- Console Output Implementation (Requirement A) ---
void Write(const Graph &g1, const Graph &g2, const std::vector<Mapping> &mappings, std::uint64_t time_spent_ns)
{
    double time_ms = time_spent_ns / 1'000'000.0;

    // 1. Time
    std::cout << "\n=== Execution Summary ===\n";
    std::cout << "Execution Time: " << std::fixed << std::setprecision(4) << time_ms << " ms\n";

    if (mappings.empty()) {
        std::cout << "No valid mapping found.\n";
        return;
    }

    const Mapping &mapping                      = mappings[0];
    const std::vector<EdgeExtension> extensions = GetMinimalEdgeExtension(g1, g2, mapping);

    // 2. Cost
    int cost = 0;
    for (const auto &ext : extensions) {
        cost += static_cast<int>(ext.weight_needed - ext.weight_found);
    }
    std::cout << "Cost (Added Edges): " << cost << "\n";

    // 3. Visual Matrix (only if size < 15)
    if (g2.GetVertices() < 15) {
        Graph g_extended = GetMinimalExtension(g1, g2, mapping);
        std::cout << "\n=== Modified G2 Adjacency Matrix ===\n";
        std::cout << "(Legend: 'old' or '(old + added)')\n\n";
        PrintVisualMatrix(std::cout, g2, g_extended);
    }

    // 4. Minimal Edge Extension
    if (!extensions.empty()) {
        PrintExtensionTable(std::cout, extensions);
    }

    // 5. Mapping Table
    PrintMappingTable(std::cout, mapping, g1.GetVertices());
}

void WriteResult(
    const char *file, const Graph &g1, const Graph &g2, const Mapping &mapping, std::uint64_t time_spent_ns
)
{
    std::filesystem::path file_path(file);
    if (file_path.has_parent_path()) {
        std::filesystem::create_directories(file_path.parent_path());
    }

    std::ofstream fs(file);
    if (!fs.is_open()) {
        throw std::runtime_error("Error: Could not open file for writing: " + std::string(file));
    }

    Graph g_extended                            = GetMinimalExtension(g1, g2, mapping);
    const std::vector<EdgeExtension> extensions = GetMinimalEdgeExtension(g1, g2, mapping);
    int cost                                    = 0;
    for (const auto &ext : extensions) {
        cost += static_cast<int>(ext.weight_needed - ext.weight_found);
    }
    double time_ms = time_spent_ns / 1'000'000.0;

    auto write_g = [&](const Graph &g) {
        const auto size = g.GetVertices();
        fs << size << "\n";
        for (Vertex i = 0; i < size; ++i) {
            for (Vertex j = 0; j < size; ++j) {
                fs << g.GetEdges(i, j) << (j == size - 1 ? "" : " ");
            }
            fs << "\n";
        }
    };

    // 1. Standard Adjacency Matrix (Extended)
    write_g(g1);
    write_g(g2);
    write_g(g_extended);

    // 2. Visual Matrix (Regardless of size)
    fs << "\n=== Visual Representation of Changes ===\n";
    fs << "(Legend: 'old' or '(old + added)')\n\n";
    PrintVisualMatrix(fs, g2, g_extended);

    // 3. Time & Cost
    fs << "\n=== Execution Summary ===\n";
    fs << "\nExecution Time: " << std::fixed << std::setprecision(4) << time_ms << " ms\n";
    fs << "Cost (Added Edges): " << cost << "\n";

    // 4. Minimal Edge Extension Table
    PrintExtensionTable(fs, extensions);

    // 5. Mapping Table
    PrintMappingTable(fs, mapping, g1.GetVertices());
}
