#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "defines.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <new>

class Graph
{
    public:
    explicit Graph(const std::uint32_t num_vertices) : vertices_(num_vertices)
    {
        neighbourhood_matrix_ = new (std::align_val_t{64}) std::uint32_t[num_vertices * num_vertices]{};
    }

    ~Graph()
    {
        if (neighbourhood_matrix_ != nullptr) {
            operator delete[](neighbourhood_matrix_, std::align_val_t{64});
        }
    }

    Graph(const Graph &other) : vertices_(other.vertices_), num_edges_(other.num_edges_)
    {
        const std::size_t matrix_size = static_cast<std::size_t>(vertices_) * vertices_;
        neighbourhood_matrix_         = new (std::align_val_t{64}) std::uint32_t[matrix_size];
        std::copy(other.neighbourhood_matrix_, other.neighbourhood_matrix_ + matrix_size, neighbourhood_matrix_);
    }

    Graph &operator=(const Graph &other)
    {
        if (this == &other) {
            return *this;
        }

        operator delete[](neighbourhood_matrix_, std::align_val_t{64});

        vertices_  = other.vertices_;
        num_edges_ = other.num_edges_;

        const std::size_t matrix_size = static_cast<std::size_t>(vertices_) * vertices_;
        neighbourhood_matrix_         = new (std::align_val_t{64}) std::uint32_t[matrix_size];
        std::copy(other.neighbourhood_matrix_, other.neighbourhood_matrix_ + matrix_size, neighbourhood_matrix_);

        return *this;
    }

    Graph(Graph &&g) noexcept
    {
        vertices_               = g.vertices_;
        num_edges_              = g.num_edges_;
        neighbourhood_matrix_   = g.neighbourhood_matrix_;
        g.neighbourhood_matrix_ = nullptr;
    }

    Graph &operator=(Graph &&g) noexcept
    {
        if (this == &g) {
            return *this;
        }

        operator delete[](neighbourhood_matrix_, std::align_val_t{64});

        vertices_               = g.vertices_;
        num_edges_              = g.num_edges_;
        neighbourhood_matrix_   = g.neighbourhood_matrix_;
        g.neighbourhood_matrix_ = nullptr;
        return *this;
    }

    void AddEdges(const std::uint32_t u, const std::uint32_t v, const std::uint32_t num_edges = 1)
    {
        GetEdges_(u, v) += num_edges;
        num_edges_ += num_edges;
    }

    void RemoveEdges(const std::uint32_t u, const std::uint32_t v, const std::uint32_t num_edges = 1)
    {
        assert(GetEdges(u, v) >= num_edges);

        GetEdges_(u, v) -= num_edges;
        num_edges_ -= num_edges;
    }

    NODISCARD FUNC_INLINE std::uint32_t GetEdges(const std::uint32_t u, const std::uint32_t v) const
    {
        return GetEdges_(u, v);
    }

    NODISCARD FUNC_INLINE std::uint32_t GetVertices() const { return static_cast<std::uint32_t>(vertices_); }

    NODISCARD FUNC_INLINE std::uint32_t GetEdges() const { return static_cast<std::uint32_t>(num_edges_); }

    template <class Func>
    void IterateOutEdges(Func func, const std::uint32_t v) const
    {
        assert(v < static_cast<std::uint32_t>(vertices_));

        for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(vertices_); ++u) {
            if (const std::uint32_t edges = GetEdges(v, u); edges != 0) {
                func(edges, u);
            }
        }
    }

    template <class Func>
    void IterateInEdges(Func func, const std::uint32_t v) const
    {
        assert(v < static_cast<std::uint32_t>(vertices_));

        for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(vertices_); ++u) {
            if (const std::uint32_t edges = GetEdges(u, v); edges != 0) {
                func(edges, u);
            }
        }
    }

    template <class Func>
    void IterateEdges(Func func, const std::uint32_t v) const
    {
        assert(v < static_cast<std::uint32_t>(vertices_));

        for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(vertices_); ++u) {
            if (const std::uint32_t edges = GetEdges(v, u); edges != 0) {
                func(edges, v, u);
            }
        }

        for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(vertices_); ++u) {
            if (const std::uint32_t edges = GetEdges(u, v); edges != 0) {
                func(edges, u, v);
            }
        }
    }

    template <class Func>
    void IterateEdges(Func func) const
    {
        for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(vertices_); ++u) {
            for (std::uint32_t v = 0; v < static_cast<std::uint32_t>(vertices_); ++v) {
                if (const std::uint32_t edges = GetEdges(u, v); edges != 0) {
                    func(edges, u, v);
                }
            }
        }
    }

    private:
    NODISCARD FUNC_INLINE std::uint32_t &GetEdges_(const std::uint32_t u, const std::uint32_t v)
    {
        assert(u < static_cast<std::uint32_t>(vertices_));
        assert(v < static_cast<std::uint32_t>(vertices_));

        return neighbourhood_matrix_[u * vertices_ + v];
    }

    NODISCARD FUNC_INLINE const std::uint32_t &GetEdges_(const std::uint32_t u, const std::uint32_t v) const
    {
        assert(u < static_cast<std::uint32_t>(vertices_));
        assert(v < static_cast<std::uint32_t>(vertices_));

        return neighbourhood_matrix_[u * vertices_ + v];
    }

    std::int32_t vertices_{};
    std::int32_t num_edges_{};
    alignas(64) std::uint32_t *neighbourhood_matrix_{};
};

Graph ParseGraph(const char *file);

#endif  // GRAPH_HPP
