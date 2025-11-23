#ifndef CURATED_GEN_HPP
#define CURATED_GEN_HPP

#include "graph.hpp"

#include <functional>
#include <string>
#include <utility>
#include <vector>

struct TestCase {
    std::string name;
    std::function<std::pair<Graph, Graph>()> generator;
};

// Returns a list of all curated test cases (sanity checks, pathological cases, etc.)
std::vector<TestCase> GenerateAllCurated();

#endif  // CURATED_GEN_HPP
