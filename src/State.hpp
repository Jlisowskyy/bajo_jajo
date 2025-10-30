#ifndef STATE_HPP
#define STATE_HPP

#include <algorithm>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <unordered_set>

struct State {
    std::unordered_map<int, int> mapping;         // G1 -> G2
    std::unordered_map<int, int> reverseMapping;  // G2 -> G1
    std::unordered_set<int> availableVertices;    // vertices from G2 NOT mapped
    int n2;                                       // Total number of G2 vertices

    State(int n2) : n2(n2)
    {
        for (int i = 0; i < n2; ++i) availableVertices.insert(i);
    }

    State(const State& other)
        : mapping(other.mapping),
          reverseMapping(other.reverseMapping),
          availableVertices(other.availableVertices),
          n2(other.n2)
    {
    }

    State& operator=(const State& other)
    {
        if (this != &other) {
            mapping           = other.mapping;
            reverseMapping    = other.reverseMapping;
            availableVertices = other.availableVertices;
            n2                = other.n2;
        }
        return *this;
    }

    bool SetMapping(int g1_vertex, int g2_vertex)
    {
        // If g1_vertex is mapped to a different g2 vertex, remove that old mapping
        auto it = mapping.find(g1_vertex);
        if (it != mapping.end()) {
            int old_g2 = it->second;
            availableVertices.insert(old_g2);
            reverseMapping.erase(old_g2);
        }

        // If g2_vertex is mapped to a different g1 vertex, remove that old mapping
        auto rev_it = reverseMapping.find(g2_vertex);
        if (rev_it != reverseMapping.end()) {
            int old_g1 = rev_it->second;
            mapping.erase(old_g1);
        }

        mapping[g1_vertex]        = g2_vertex;
        reverseMapping[g2_vertex] = g1_vertex;
        availableVertices.erase(g2_vertex);

        return true;
    }
};

#endif  // STATE_HPP
