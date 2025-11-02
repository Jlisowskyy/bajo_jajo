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
    Mapping(const size_t size_g1, const size_t size_g2) : size_g1_(size_g1), size_g2_(size_g2), mapped_count_(0)
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

    Mapping(const Mapping &other)
        : size_g1_(other.size_g1_), size_g2_(other.size_g2_), mapped_count_(other.mapped_count_)
    {
        mapping_         = new std::int32_t[size_g1_];
        reverse_mapping_ = new std::int32_t[size_g2_];

        std::copy(other.mapping_, other.mapping_ + size_g1_, mapping_);
        std::copy(other.reverse_mapping_, other.reverse_mapping_ + size_g2_, reverse_mapping_);
    }

    Mapping &operator=(const Mapping &other)
    {
        if (this != &other) {
            if (size_g1_ != other.size_g1_ || size_g2_ != other.size_g2_) {
                delete[] mapping_;
                delete[] reverse_mapping_;
                size_g1_         = other.size_g1_;
                size_g2_         = other.size_g2_;
                mapping_         = new std::int32_t[size_g1_];
                reverse_mapping_ = new std::int32_t[size_g2_];
            }
            std::copy(other.mapping_, other.mapping_ + size_g1_, mapping_);
            std::copy(other.reverse_mapping_, other.reverse_mapping_ + size_g2_, reverse_mapping_);
            mapped_count_ = other.mapped_count_;
        }
        return *this;
    }

    bool set_mapping(const std::int32_t g1_index, const std::int32_t g2_index)
    {
        if (g1_index < 0 || g1_index >= size_g1_ || g2_index < 0 || g2_index >= size_g2_) {
            return false;
        }

        bool was_g1_mapped = (mapping_[g1_index] != -1);
        bool was_g2_mapped = (reverse_mapping_[g2_index] != -1);

        if (mapping_[g1_index] != -1) {
            if (mapping_[g1_index] != g2_index) {
                reverse_mapping_[mapping_[g1_index]] = -1;
            }
        }

        if (reverse_mapping_[g2_index] != -1) {
            if (reverse_mapping_[g2_index] != g1_index) {
                mapping_[reverse_mapping_[g2_index]] = -1;
            }
        }

        mapping_[g1_index]         = g2_index;
        reverse_mapping_[g2_index] = g1_index;

        if (!was_g1_mapped && !was_g2_mapped) {
            mapped_count_++;
        }

        return true;
    }

    bool remove_mapping_g1(const std::int32_t g1_index)
    {
        if (g1_index >= size_g1_ || mapping_[g1_index] == -1) {
            return false;
        }

        const std::int32_t g2_index = mapping_[g1_index];
        mapping_[g1_index]          = -1;
        reverse_mapping_[g2_index]  = -1;
        mapped_count_--;
        return true;
    }

    bool remove_mapping_g2(const std::int32_t g2_index)
    {
        if (g2_index >= size_g2_ || reverse_mapping_[g2_index] == -1) {
            return false;
        }

        const std::int32_t g1_index = reverse_mapping_[g2_index];
        mapping_[g1_index]          = -1;
        reverse_mapping_[g2_index]  = -1;
        mapped_count_--;
        return true;
    }

    std::int32_t get_mapping_g1_to_g2(const std::int32_t g1_index) const
    {
        if (g1_index >= size_g1_) {
            return -1;
        }
        return mapping_[g1_index];
    }

    std::int32_t get_mapping_g2_to_g1(const std::int32_t g2_index) const
    {
        if (g2_index >= size_g2_) {
            return -1;
        }
        return reverse_mapping_[g2_index];
    }

    bool is_g1_mapped(const std::int32_t g1_index) const { return g1_index < size_g1_ && mapping_[g1_index] != -1; }

    bool is_g2_mapped(const std::int32_t g2_index) const
    {
        return g2_index < size_g2_ && reverse_mapping_[g2_index] != -1;
    }

    std::int32_t get_mapped_count() const { return mapped_count_; }

    private:
    std::int32_t *mapping_;         /* g1 -> g2. Stores -1 if no mapping. */
    std::int32_t *reverse_mapping_; /* g2 -> g1. Stores -1 if no mapping. */

    std::int32_t size_g1_;
    std::int32_t size_g2_;
    std::int32_t mapped_count_;
};

struct State {
    Mapping mapping;
    std::unordered_set<std::int32_t> availableVertices;
    std::int32_t size_g1_;
    std::int32_t size_g2_;

    State(const std::int32_t size_g1, const std::int32_t size_g2)
        : mapping(size_g1, size_g2), size_g1_(size_g1), size_g2_(size_g2)
    {
        for (std::int32_t i = 0; i < size_g2_; ++i) {
            availableVertices.insert(i);
        }
    }

    State(const State &other)            = default;
    State &operator=(const State &other) = default;

    bool set_mapping(const std::int32_t g1_vertex, const std::int32_t g2_vertex)
    {
        const std::int32_t old_g2 = mapping.get_mapping_g1_to_g2(g1_vertex);
        if (old_g2 != -1) {
            availableVertices.insert(old_g2);
        }

        const bool result = mapping.set_mapping(g1_vertex, g2_vertex);
        if (result) {
            availableVertices.erase(g2_vertex);
        }

        return result;
    }
};

#endif  // STATE_HPP
