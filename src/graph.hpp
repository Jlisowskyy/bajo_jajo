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

    Graph(const Graph &)            = delete;
    Graph &operator=(const Graph &) = delete;

    Graph(Graph &&g) noexcept
    {
        vertices_               = g.vertices_;
        num_edges_              = g.num_edges_;
        neighbourhood_matrix_   = g.neighbourhood_matrix_;
        g.neighbourhood_matrix_ = nullptr;
    }

    Graph &operator=(Graph &&g) noexcept
    {
        vertices_  = g.vertices_;
        num_edges_ = g.num_edges_;
        operator delete[](neighbourhood_matrix_, std::align_val_t{64});
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
    void IterateOutEdges(Func func, const std::uint32_t v)
    {
        assert(v < vertices_);

        for (std::uint32_t u = 0; u < vertices_; ++u) {
            if (const std::uint32_t edges = GetEdges(v, u); edges != 0) {
                func(edges, u);
            }
        }
    }

    template <class Func>
    void IterateInEdges(Func func, const std::uint32_t v)
    {
        assert(v < vertices_);

        for (std::uint32_t u = 0; u < vertices_; ++u) {
            if (const std::uint32_t edges = GetEdges(u, v); edges != 0) {
                func(edges, u);
            }
        }
    }

    template <class Func>
    void IterateEdges(Func func, const std::uint32_t v)
    {
        assert(v < vertices_);

        for (std::uint32_t u = 0; u < vertices_; ++u) {
            if (const std::uint32_t edges = GetEdges(v, u); edges != 0) {
                func(edges, v, u);
            }
        }

        for (std::uint32_t u = 0; u < vertices_; ++u) {
            if (const std::uint32_t edges = GetEdges(u, v); edges != 0) {
                func(edges, u, v);
            }
        }
    }

    template <class Func>
    void IterateEdges(Func func)
    {
        for (std::uint32_t u = 0; u < vertices_; ++u) {
            for (std::uint32_t v = 0; v < vertices_; ++v) {
                if (const std::uint32_t edges = GetEdges(u, v); edges != 0) {
                    func(edges, u, v);
                }
            }
        }
    }

    private:
    NODISCARD FUNC_INLINE std::uint32_t &GetEdges_(const std::uint32_t u, const std::uint32_t v)
    {
        assert(u < vertices_);
        assert(v < vertices_);
        assert(u != v);

        return neighbourhood_matrix_[u * vertices_ + v];
    }

    NODISCARD FUNC_INLINE const std::uint32_t &GetEdges_(const std::uint32_t u, const std::uint32_t v) const
    {
        assert(u < vertices_);
        assert(v < vertices_);
        assert(u != v);

        return neighbourhood_matrix_[u * vertices_ + v];
    }

    std::int32_t vertices_{};
    std::int32_t num_edges_{};
    alignas(64) std::uint32_t *neighbourhood_matrix_{};
};

Graph ParseGraph(const char *file);

#endif  // GRAPH_HPP
