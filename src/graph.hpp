#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "defines.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <new>
#include <vector>

using Vertex   = std::uint32_t;
using Edges    = std::uint32_t;
using Vertices = std::uint32_t;

class Graph
{
    public:
    explicit Graph(const Vertices num_vertices) : vertices_(num_vertices)
    {
        neighbourhood_matrix_ = new Edges[num_vertices * num_vertices]{};
    }

    ~Graph()
    {
        if (neighbourhood_matrix_ != nullptr) {
            operator delete[](neighbourhood_matrix_);
        }
    }

    Graph(const Graph &other) : vertices_(other.vertices_), num_edges_(other.num_edges_)
    {
        const std::size_t matrix_size = static_cast<std::size_t>(vertices_) * vertices_;
        neighbourhood_matrix_         = new Edges[matrix_size];
        std::copy(other.neighbourhood_matrix_, other.neighbourhood_matrix_ + matrix_size, neighbourhood_matrix_);
    }

    Graph &operator=(const Graph &other)
    {
        if (this == &other) {
            return *this;
        }

        operator delete[](neighbourhood_matrix_);

        vertices_  = other.vertices_;
        num_edges_ = other.num_edges_;

        const std::size_t matrix_size = static_cast<std::size_t>(vertices_) * vertices_;
        neighbourhood_matrix_         = new Edges[matrix_size];
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

        operator delete[](neighbourhood_matrix_);

        vertices_               = g.vertices_;
        num_edges_              = g.num_edges_;
        neighbourhood_matrix_   = g.neighbourhood_matrix_;
        g.neighbourhood_matrix_ = nullptr;
        return *this;
    }

    void AddEdges(const Vertex u, const Vertex v, const Edges edges = 1)
    {
        GetEdges_(u, v) += edges;
        num_edges_ += edges;
    }

    void RemoveEdges(const Vertex u, const Vertex v, const Edges edges = 1)
    {
        assert(GetEdges(u, v) >= edges);

        GetEdges_(u, v) -= edges;
        num_edges_ -= edges;
        assert(num_edges_ >= 0);
    }

    NODISCARD FUNC_INLINE Edges GetEdges(const Vertex u, const Vertex v) const { return GetEdges_(u, v); }

    NODISCARD FUNC_INLINE Vertices GetVertices() const { return static_cast<Vertices>(vertices_); }

    NODISCARD FUNC_INLINE Edges GetEdges() const { return static_cast<Edges>(num_edges_); }

    template <class Func>
    void IterateOutEdges(Func func, const Vertex v) const
    {
        assert(v < static_cast<Vertices>(vertices_));

        for (Vertex u = 0; u < static_cast<Vertex>(vertices_); ++u) {
            if (const Edges edges = GetEdges(v, u); edges != 0) {
                func(edges, u);
            }
        }
    }

    template <class Func>
    void IterateInEdges(Func func, const Vertex v) const
    {
        assert(v < static_cast<Vertex>(vertices_));

        for (Vertex u = 0; u < static_cast<Vertex>(vertices_); ++u) {
            if (const Edges edges = GetEdges(u, v); edges != 0) {
                func(edges, u);
            }
        }
    }

    template <class Func>
    void IterateEdges(Func func, const Vertex v) const
    {
        assert(v < static_cast<Vertex>(vertices_));

        for (Vertex u = 0; u < static_cast<Vertex>(vertices_); ++u) {
            if (const Edges edges = GetEdges(v, u); edges != 0) {
                func(edges, v, u);
            }
        }

        for (Vertex u = 0; u < static_cast<Vertex>(vertices_); ++u) {
            if (const Edges edges = GetEdges(u, v); edges != 0) {
                func(edges, u, v);
            }
        }
    }

    template <class Func>
    void IterateEdges(Func func) const
    {
        for (Vertex u = 0; u < static_cast<Vertex>(vertices_); ++u) {
            for (Vertex v = 0; v < static_cast<Vertex>(vertices_); ++v) {
                if (const Edges edges = GetEdges(u, v); edges != 0) {
                    func(edges, u, v);
                }
            }
        }
    }

    template <class Func>
    void IterateNeighbours(Func func, Vertex v) const
    {
        IterateOutEdges(
            [&](Edges, Vertex neighbour) {
                func(neighbour);
            },
            v
        );

        IterateInEdges(
            [&](Edges, Vertex neighbour) {
                if (GetEdges(v, neighbour) > 0) {
                    return;
                }
                func(neighbour);
            },
            v
        );
    }

    NODISCARD Vertices GetNumOfNeighbours(const Vertex v) const
    {
        Vertices num_of_neighbours = 0;
        IterateNeighbours(
            [&](Vertex) {
                num_of_neighbours++;
            },
            v
        );
        return num_of_neighbours;
    }

    NODISCARD Vertices GetDegree(const Vertex v) const
    {
        Vertices degree = 0;
        IterateOutEdges(
            [&](Edges edges, Vertex neighbour) {
                degree += edges;
            },
            v
        );
        return degree;
    }

    private:
    NODISCARD FUNC_INLINE Edges &GetEdges_(const Vertex u, const Vertex v)
    {
        assert(u < static_cast<Vertex>(vertices_));
        assert(v < static_cast<Vertex>(vertices_));

        return neighbourhood_matrix_[u * vertices_ + v];
    }

    NODISCARD FUNC_INLINE const Edges &GetEdges_(const Vertex u, const Vertex v) const
    {
        assert(u < static_cast<Vertex>(vertices_));
        assert(v < static_cast<Vertex>(vertices_));

        return neighbourhood_matrix_[u * vertices_ + v];
    }

    std::int32_t vertices_{};
    std::int32_t num_edges_{};
    alignas(64) Edges *neighbourhood_matrix_{};
};

#endif  // GRAPH_HPP
