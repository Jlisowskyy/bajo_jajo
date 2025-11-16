#ifndef IO_HPP
#define IO_HPP

#include "State.hpp"
#include "graph.hpp"

#include <tuple>

std::pair<Graph, Graph> Read(const char *file);
void Write(const Mapping &mapping);
void Write(const char *file, const std::tuple<Graph, Graph> &graphs);

#endif  // IO_HPP
