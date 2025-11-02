#ifndef STATE_HPP
#define STATE_HPP

#include <algorithm>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <unordered_set>

class Mapping
{
    public:
    Mapping(const size_t size_g1, const size_t size_g2) : size_g1_(size_g1), size_g2_(size_g2)
    {
        mapping_         = new std::int32_t[size_g1_];
        reverse_mapping_ = new std::int32_t[size_g2_];

        std::fill(mapping_, mapping_ + size_g1_, -1);
        std::fill(reverse_mapping_, reverse_mapping_ + size_g2_, -1);
    }

    ~Mapping()
    {
        delete[] mapping_;
        delete[] reverse_mapping_;
    }

    Mapping(const Mapping &)            = delete;
    Mapping &operator=(const Mapping &) = delete;

    bool set_mapping(const std::int32_t g1_index, const std::int32_t g2_index)
    {
        if (g1_index >= size_g1_ || g2_index >= size_g2_) {
            return false;
        }

        if (mapping_[g1_index] != -1) {
            if (mapping_[g1_index] != g2_index) {
                reverse_mapping_[mapping_[g1_index]] = -1;
            }
        } else {
            if (reverse_mapping_[g2_index] != -1) {
                mapping_[reverse_mapping_[g2_index]] = -1;
            }
        }

        mapping_[g1_index]         = g2_index;
        reverse_mapping_[g2_index] = g1_index;

        return true;
    }

    bool remove_mapping_g1(const size_t g1_index)
    {
        if (g1_index >= size_g1_ || mapping_[g1_index] == -1) {
            return false;
        }

        const std::int32_t g2_index = mapping_[g1_index];
        mapping_[g1_index]          = -1;
        reverse_mapping_[g2_index]  = -1;
        return true;
    }

    bool remove_mapping_g2(const size_t g2_index)
    {
        if (g2_index >= size_g2_ || reverse_mapping_[g2_index] == -1) {
            return false;
        }

        const std::int32_t g1_index = reverse_mapping_[g2_index];
        mapping_[g1_index]          = -1;
        reverse_mapping_[g2_index]  = -1;
        return true;
    }

    std::int32_t get_mapping_g1_to_g2(const size_t g1_index) const
    {
        if (g1_index >= size_g1_) {
            return -1;
        }
        return mapping_[g1_index];
    }

    std::int32_t get_mapping_g2_to_g1(const size_t g2_index) const
    {
        if (g2_index >= size_g2_) {
            return -1;
        }
        return reverse_mapping_[g2_index];
    }

    bool is_g1_mapped(const size_t g1_index) const { return g1_index < size_g1_ && mapping_[g1_index] != -1; }

    bool is_g2_mapped(const size_t g2_index) const { return g2_index < size_g2_ && reverse_mapping_[g2_index] != -1; }

    private:
    std::int32_t *mapping_;         /* g1 -> g2. Stores -1 if no mapping. */
    std::int32_t *reverse_mapping_; /* g2 -> g1. Stores -1 if no mapping. */

    const size_t size_g1_;
    const size_t size_g2_;
};

struct State {
    std::unordered_map<int, int> mapping;         // G1 -> G2
    std::unordered_map<int, int> reverseMapping;  // G2 -> G1
    std::unordered_set<int> availableVertices;    // vertices from G2 NOT mapped
    int n2;                                       // Total number of G2 vertices

    State(int n2) : n2(n2)
    {
        for (int i = 0; i < n2; ++i) availableVertices.insert(i);
    }

    State(const State &other)
        : mapping(other.mapping),
          reverseMapping(other.reverseMapping),
          availableVertices(other.availableVertices),
          n2(other.n2)
    {
    }

    State &operator=(const State &other)
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
