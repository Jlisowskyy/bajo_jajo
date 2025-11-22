#ifndef IO_HPP
#define IO_HPP

#include "State.hpp"
#include "graph.hpp"

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>

std::pair<Graph, Graph> Read(const std::filesystem::path &file);
void Write(const Graph &g1, const Graph &g2, const std::vector<Mapping> &mappings, std::uint64_t time_spent);
void Write(const std::filesystem::path &file, const std::tuple<Graph, Graph> &graphs);

#endif  // IO_HPP
