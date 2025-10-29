#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "defines.hpp"

#include <cstdint>
#include <new>

class Graph
{
    public:
    explicit Graph(const std::uint32_t num_vertices): vertices_(num_vertices)
    {
        neighbourhood_matrix_ = new (std::align_val_t{64}) std::uint32_t[num_vertices*num_vertices]{};
    }

    ~Graph()
    {
        operator delete[](neighbourhood_matrix_, std::align_val_t{64});
    }

    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    void AddEdge(const std::uint32_t u, const std::uint32_t v)
    {
        GetEdges(u, v) += 1;
    }

    void RemoveEdge(const std::uint32_t u, const std::uint32_t v)
    {
        GetEdges(u, v) -= 1;
    }

    NODISCARD FUNC_INLINE const std::uint32_t& GetEdges(const std::uint32_t u, const std::uint32_t v) const
    {
        return neighbourhood_matrix_[u*vertices_ + v];
    }

    NODISCARD FUNC_INLINE std::uint32_t& GetEdges(const std::uint32_t u, const std::uint32_t v)
    {
        return neighbourhood_matrix_[u*vertices_ + v];
    }

    NODISCARD FUNC_INLINE std::uint32_t GetVertices() const
    {
        return vertices_;
    }

    private:
    std::uint32_t vertices_{};
    alignas(64) std::uint32_t* neighbourhood_matrix_{};
};

Graph ParseGraph(const char* file);

#endif //GRAPH_HPP