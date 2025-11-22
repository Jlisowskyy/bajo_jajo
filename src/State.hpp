#ifndef STATE_HPP
#define STATE_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "graph.hpp"

using MappedVertex                            = std::int32_t;
static constexpr MappedVertex kUnmappedVertex = -1;

class Mapping
{
    public:
    Mapping(const Vertices size_g1, const Vertices size_g2) : size_g1_(size_g1), size_g2_(size_g2), mapped_count_(0)
    {
        mapping_.resize(size_g1, kUnmappedVertex);
        reverse_mapping_.resize(size_g2, kUnmappedVertex);
    }

    // Default copy/move/destructor are fine now because of std::vector
    Mapping(const Mapping &other)                = default;
    Mapping(Mapping &&other) noexcept            = default;
    Mapping &operator=(const Mapping &other)     = default;
    Mapping &operator=(Mapping &&other) noexcept = default;
    ~Mapping()                                   = default;

    bool operator==(const Mapping &other) const
    {
        if (this == &other) {
            return true;
        }
        if (size_g1_ != other.size_g1_ || size_g2_ != other.size_g2_ || mapped_count_ != other.mapped_count_) {
            return false;
        }
        // Vector comparison is fast
        return mapping_ == other.mapping_;
    }

    void set_mapping(const Vertex g1_index, const Vertex g2_index)
    {
        assert(g1_index < size_g1_);
        assert(g2_index < size_g2_);

        const bool was_g1_mapped = (mapping_[g1_index] != kUnmappedVertex);
        if (was_g1_mapped) {
            if (mapping_[g1_index] != static_cast<MappedVertex>(g2_index)) {
                reverse_mapping_[mapping_[g1_index]] = kUnmappedVertex;
            }
        }

        const bool was_g2_mapped = (reverse_mapping_[g2_index] != kUnmappedVertex);
        if (was_g2_mapped) {
            if (reverse_mapping_[g2_index] != static_cast<MappedVertex>(g1_index)) {
                mapping_[reverse_mapping_[g2_index]] = kUnmappedVertex;
            }
        }

        mapping_[g1_index]         = g2_index;
        reverse_mapping_[g2_index] = g1_index;
        if (!was_g1_mapped && !was_g2_mapped) {
            mapped_count_++;
        }
    }

    bool remove_mapping_g1(const Vertex g1_index)
    {
        assert(g1_index < size_g1_);

        if (mapping_[g1_index] == kUnmappedVertex) {
            return false;
        }

        const MappedVertex g2_index = mapping_[g1_index];
        assert(g2_index >= 0);
        mapping_[g1_index]         = kUnmappedVertex;
        reverse_mapping_[g2_index] = kUnmappedVertex;
        mapped_count_--;
        assert(mapped_count_ >= 0);
        return true;
    }

    NODISCARD MappedVertex get_mapping_g1_to_g2(const Vertex g1_index) const
    {
        assert(g1_index < size_g1_);
        return mapping_[g1_index];
    }

    NODISCARD MappedVertex get_mapping_g2_to_g1(const Vertex g2_index) const
    {
        assert(g2_index < size_g2_);
        return reverse_mapping_[g2_index];
    }

    bool is_g1_mapped(const Vertex g1_index) const
    {
        assert(g1_index < size_g1_);
        return mapping_[g1_index] != kUnmappedVertex;
    }

    bool is_g2_mapped(const Vertex g2_index) const
    {
        assert(g2_index < size_g2_);
        return reverse_mapping_[g2_index] != kUnmappedVertex;
    }

    Vertices get_mapped_count() const
    {
        assert(mapped_count_ >= 0);
        return static_cast<Vertices>(mapped_count_);
    }

    private:
    std::vector<MappedVertex> mapping_;         /* g1 -> g2 */
    std::vector<MappedVertex> reverse_mapping_; /* g2 -> g1 */

    Vertices size_g1_;
    Vertices size_g2_;
    std::int32_t mapped_count_;
};

struct State {
    Mapping mapping;
    boost::dynamic_bitset<> used_mask;
    Vertices size_g1_;
    Vertices size_g2_;

    State(const Vertices size_g1, const Vertices size_g2)
        : mapping(size_g1, size_g2), used_mask(size_g2), size_g1_(size_g1), size_g2_(size_g2)
    {
    }

    State(const State &other)                = default;
    State &operator=(const State &other)     = default;
    State(State &&other) noexcept            = default;
    State &operator=(State &&other) noexcept = default;

    void set_mapping(const Vertex g1_vertex, const Vertex g2_vertex)
    {
        const MappedVertex old_g2 = mapping.get_mapping_g1_to_g2(g1_vertex);
        if (old_g2 != -1) {
            used_mask[old_g2] = 0;
        }

        mapping.set_mapping(g1_vertex, g2_vertex);
        used_mask[g2_vertex] = 1;
    }
};

#endif  // STATE_HPP
