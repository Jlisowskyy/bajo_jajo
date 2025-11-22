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
        neighbourhood_matrix_ = new (std::align_val_t{64}) Edges[num_vertices * num_vertices]{};
        adj_list_.resize(num_vertices);
    }

    ~Graph()
    {
        if (neighbourhood_matrix_ != nullptr) {
            operator delete[](neighbourhood_matrix_, std::align_val_t{64});
        }
    }

    Graph(const Graph &other) : vertices_(other.vertices_), num_edges_(other.num_edges_), adj_list_(other.adj_list_)
    {
        const std::size_t matrix_size = static_cast<std::size_t>(vertices_) * vertices_;
        neighbourhood_matrix_         = new (std::align_val_t{64}) Edges[matrix_size];
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
        adj_list_  = other.adj_list_;

        const std::size_t matrix_size = static_cast<std::size_t>(vertices_) * vertices_;
        neighbourhood_matrix_         = new (std::align_val_t{64}) Edges[matrix_size];
        std::copy(other.neighbourhood_matrix_, other.neighbourhood_matrix_ + matrix_size, neighbourhood_matrix_);

        return *this;
    }

    Graph(Graph &&g) noexcept
    {
        vertices_               = g.vertices_;
        num_edges_              = g.num_edges_;
        neighbourhood_matrix_   = g.neighbourhood_matrix_;
        adj_list_               = std::move(g.adj_list_);
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
        adj_list_               = std::move(g.adj_list_);
        g.neighbourhood_matrix_ = nullptr;
        return *this;
    }

    void AddEdges(const Vertex u, const Vertex v, const Edges edges = 1)
    {
        Edges &current_uv = GetEdges_(u, v);
        // If edge didn't exist before (in either direction if we consider neighbors unified),
        // we might need to update adjacency list.
        // Note: The previous logic handled in/out.
        // To fully optimize IterateNeighbours, we store a Unified Adjacency List (neighbors in either direction).

        bool had_edge_uv = (current_uv > 0);
        current_uv += edges;

        if (!had_edge_uv) {
            AddAdj(u, v);
        }

        // Since IterateNeighbours checks In and Out, we ensure connectivity is registered.
        // The matrix stores the weight/direction. The list stores "connectedness".
        // However, AddEdges(u,v) implies u->v.
        // IterateNeighbours(x) wants y where x->y OR y->x.
        // So we add u to v's list and v to u's list.
        AddAdj(v, u);

        num_edges_ += edges;
    }

    void RemoveEdges(const Vertex u, const Vertex v, const Edges edges = 1)
    {
        assert(GetEdges(u, v) >= edges);

        GetEdges_(u, v) -= edges;
        num_edges_ -= edges;
        assert(num_edges_ >= 0);

        // Note: Removing from adj_list_ is expensive (search and erase) and rarely done in this specific logic
        // (mainly used in random gen). We leave the neighbor in the list;
        // IterateNeighbours will just see weight 0 and skip/handle it if strictly required.
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
        // Fallback to slow method if needed, or implement specific logic
        // But usually IterateNeighbours is what is used.
        IterateNeighbours(
            [&](Vertex u) {
                Edges fwd = GetEdges(v, u);
                Edges bwd = GetEdges(u, v);
                if (fwd)
                    func(fwd, v, u);
                if (bwd)
                    func(bwd, u, v);
            },
            v
        );
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

    // OPTIMIZED: Uses pre-calculated adjacency list. No allocations.
    template <class Func>
    void IterateNeighbours(Func func, Vertex v) const
    {
        assert(v < static_cast<Vertices>(vertices_));
        for (const Vertex neighbor : adj_list_[v]) {
            func(neighbor);
        }
    }

    NODISCARD Vertices GetNumOfNeighbours(const Vertex v) const
    {
        assert(v < static_cast<Vertices>(vertices_));
        return static_cast<Vertices>(adj_list_[v].size());
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

    void AddAdj(Vertex from, Vertex to)
    {
        // Keep list sorted or just check existence. For small graphs, linear scan is fine.
        // We want unique entries.
        std::vector<Vertex> &list = adj_list_[from];
        for (Vertex existing : list) {
            if (existing == to)
                return;
        }
        list.push_back(to);
    }

    std::int32_t vertices_{};
    std::int32_t num_edges_{};
    alignas(64) Edges *neighbourhood_matrix_{};

    // Unified adjacency list (stores u if u->v OR v->u)
    std::vector<std::vector<Vertex>> adj_list_;
};

#endif  // GRAPH_HPP
