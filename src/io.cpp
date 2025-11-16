#include <io.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

std::pair<Graph, Graph> Read(const char *file)
{
    std::ifstream file_stream(file);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + std::string(file));
    }

    auto read_single_graph = [](std::ifstream &fs) {
        std::uint32_t size;
        fs >> size;
        if (fs.fail() || size == 0) {
            throw std::runtime_error("Error reading or invalid graph size.");
        }

        Graph g(size);
        for (std::uint32_t i = 0; i < size; ++i) {
            for (std::uint32_t j = 0; j < size; ++j) {
                std::uint32_t edges;
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

    Graph g1 = read_single_graph(file_stream);
    Graph g2 = read_single_graph(file_stream);

    return std::make_pair(std::move(g1), std::move(g2));
}

void Write(const Mapping &mapping) {}

void Write(const char *file, const std::tuple<Graph, Graph> &graphs) {}
