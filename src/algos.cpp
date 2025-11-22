#include "algos.hpp"

#include <boost/heap/pairing_heap.hpp>
#include <cassert>
#include <climits>
#include <map>
#include <queue>
#include <unordered_set>
#include <vector>

static bool is_mapping_present(const Mapping &mapping, const std::multimap<int, Mapping> &mappings)
{
    for (const auto &pair : mappings) {
        if (pair.second == mapping) {
            return true;
        }
    }
    return false;
}

static int calculate_incremental_cost(const Graph &g1, const Graph &g2, const Mapping &mapping, const Vertex g1_vertex)
{
    int cost = 0;
    for (Vertex u = 0; u < g1.GetVertices(); ++u) {
        if (!mapping.is_g1_mapped(u)) {
            continue;
        }

        if (const Edges edges_g1_uv = g1.GetEdges(u, g1_vertex); edges_g1_uv > 0) {
            const MappedVertex mapped_u = mapping.get_mapping_g1_to_g2(u);
            const MappedVertex mapped_v = mapping.get_mapping_g1_to_g2(g1_vertex);
            assert(mapped_u != -1 && mapped_v != -1);

            const Edges edges_g2 = g2.GetEdges(mapped_u, mapped_v);
            if (edges_g1_uv > edges_g2) {
                cost += static_cast<int>(edges_g1_uv - edges_g2);
            }
        }

        if (u != g1_vertex) {
            if (const Edges edges_g1_vu = g1.GetEdges(g1_vertex, u); edges_g1_vu > 0) {
                const MappedVertex mapped_v = mapping.get_mapping_g1_to_g2(g1_vertex);
                const MappedVertex mapped_u = mapping.get_mapping_g1_to_g2(u);
                assert(mapped_v != -1 && mapped_u != -1);

                const Edges edges_g2 = g2.GetEdges(mapped_v, mapped_u);
                if (edges_g1_vu > edges_g2) {
                    cost += static_cast<int>(edges_g1_vu - edges_g2);
                }
            }
        }
    }
    return cost;
}

static void BruteForceRecursive(
    const Graph &g1, const Graph &g2, const int k, Mapping &current_mapping, int current_cost,
    std::vector<bool> &used_g2_vertices, const std::int32_t depth, std::multimap<int, Mapping> &best_mappings
)
{
    if (!best_mappings.empty() && best_mappings.size() == static_cast<size_t>(k)) {
        if (current_cost >= best_mappings.rbegin()->first) {
            return;
        }
    }

    if (depth == static_cast<std::int32_t>(g1.GetVertices())) {
        if (is_mapping_present(current_mapping, best_mappings)) {
            return;
        }

        if (best_mappings.size() < static_cast<size_t>(k)) {
            best_mappings.insert({current_cost, current_mapping});
        } else {
            if (current_cost < best_mappings.rbegin()->first) {
                best_mappings.erase(std::prev(best_mappings.end()));
                best_mappings.insert({current_cost, current_mapping});
            }
        }
        return;
    }

    for (Vertex candidate = 0; candidate < g2.GetVertices(); ++candidate) {
        if (used_g2_vertices[candidate]) {
            continue;
        }

        current_mapping.set_mapping(depth, candidate);
        used_g2_vertices[candidate] = true;

        const int incremental_cost = calculate_incremental_cost(g1, g2, current_mapping, depth);
        BruteForceRecursive(
            g1, g2, k, current_mapping, current_cost + incremental_cost, used_g2_vertices, depth + 1, best_mappings
        );

        used_g2_vertices[candidate] = false;
        current_mapping.remove_mapping_g1(depth);
    }
}

// ------------------------------
// Accurate Brute Force
// ------------------------------

std::vector<Mapping> AccurateBruteForce(const Graph &g1, const Graph &g2, int k)
{
    if (g1.GetVertices() > g2.GetVertices()) {
        return {};
    }

    std::multimap<int, Mapping> best_mappings;
    Mapping current_mapping(g1.GetVertices(), g2.GetVertices());
    std::vector<bool> used_g2_vertices(g2.GetVertices(), false);

    BruteForceRecursive(g1, g2, k, current_mapping, 0, used_g2_vertices, 0, best_mappings);

    std::vector<Mapping> result;
    result.reserve(best_mappings.size());
    for (const auto &pair : best_mappings) {
        result.push_back(pair.second);
    }

    return result;
}

// ------------------------------
// A star helpers
// ------------------------------

static Vertex PickNextVertex_(const Graph &g1, const State &state)
{
    // If no vertices are mapped yet, choose the one with the most neighbors
    if (state.mapping.get_mapped_count() == 0) {
        Vertex best_v1         = ~static_cast<Vertex>(0);
        Vertices max_neighbors = 0;

        for (Vertex v1 = 0; v1 < g1.GetVertices(); ++v1) {
            const Vertices neighbor_count = g1.GetNumOfNeighbours(v1);
            if (neighbor_count >= max_neighbors) {
                max_neighbors = neighbor_count;
                best_v1       = v1;
            }
        }

        assert(best_v1 != ~static_cast<Vertex>(0));
        return best_v1;
    }

    Vertex best_v1                = ~static_cast<Vertex>(0);
    Vertices max_mapped_neighbors = 0;
    Vertices max_total_neighbors  = 0;

    for (Vertex v1 = 0; v1 < g1.GetVertices(); ++v1) {
        if (state.mapping.is_g1_mapped(v1)) {
            continue;
        }

        Vertices mapped_neighbours = 0;
        Vertices total_neighbours  = 0;

        g1.IterateNeighbours(
            [&](const Vertex neighbour) {
                if (state.mapping.is_g1_mapped(neighbour)) {
                    mapped_neighbours++;
                }
                total_neighbours++;
            },
            v1
        );

        // FAIL-FIRST:
        // Choose vertex with most mapped neighbors (most constrained)
        // Break ties by choosing vertex with MORE total neighbors (most constraining)
        if (mapped_neighbours > max_mapped_neighbors ||
            (mapped_neighbours == max_mapped_neighbors && total_neighbours >= max_total_neighbors)) {
            best_v1              = v1;
            max_mapped_neighbors = mapped_neighbours;
            max_total_neighbors  = total_neighbours;
        }
    }

    return best_v1;
}

static int CalculateSingleDirectionEdgesAdditions_(
    const Graph &g1, Vertex v1, Vertex u1, const Graph &g2, Vertex v2, Vertex u2
)
{
    int cost                  = 0;
    const Edges edges_g1_v1u1 = g1.GetEdges(v1, u1);
    if (edges_g1_v1u1 > 0) {
        const Edges edges_g2_v2u2 = g2.GetEdges(v2, u2);
        if (edges_g1_v1u1 > edges_g2_v2u2) {
            cost += static_cast<int>(edges_g1_v1u1 - edges_g2_v2u2);
        }
    }
    return cost;
}

static int CalculateEdgesAdditions_(const Graph &g1, Vertex v1, Vertex u1, const Graph &g2, Vertex v2, Vertex u2)
{
    int cost = 0;
    cost += CalculateSingleDirectionEdgesAdditions_(g1, v1, u1, g2, v2, u2);
    cost += CalculateSingleDirectionEdgesAdditions_(g1, u1, v1, g2, u2, v2);
    return cost;
}

static int CalculateAssignmentCost_(const Graph &g1, const Graph &g2, const Mapping &mapping, Vertex v1, Vertex v2)
{
    int cost = 0;

    // Iterate over all neighbors of v1 in G1 that are already mapped
    g1.IterateNeighbours(
        [&](const Vertex neighbour) {
            MappedVertex u2 = -1;

            if (neighbour == v1) {
                u2 = static_cast<MappedVertex>(v2);
            } else {
                if (!mapping.is_g1_mapped(neighbour)) {
                    return;
                }
                u2 = mapping.get_mapping_g1_to_g2(neighbour);
            }
            assert(u2 != -1);

            cost += CalculateSingleDirectionEdgesAdditions_(g1, v1, neighbour, g2, v2, u2);
            if (v1 != neighbour) {
                cost += CalculateSingleDirectionEdgesAdditions_(g1, neighbour, v1, g2, u2, v2);
            }
        },
        v1
    );

    return cost;
}

static int CalculateHeuristic_(const Graph &g1, const Graph &g2, const State &state)
{
    int h = 0;
    // Create a temporary mask of available vertices (inverse of used_mask)
    // Note: This operation creates a copy, but ensures efficient iteration over available vertices
    boost::dynamic_bitset<> available_mask = ~state.used_mask;

    for (Vertex v1 = 0; v1 < g1.GetVertices(); ++v1) {
        if (state.mapping.is_g1_mapped(v1)) {
            continue;
        }
        int min_cost = INT_MAX;

        // Iterate over all available vertices in G2 using the bitset
        for (Vertex v2 = available_mask.find_first(); v2 != static_cast<Vertex>(boost::dynamic_bitset<>::npos);
             v2        = available_mask.find_next(v2)) {
            int cost_candidate = 0;
            g1.IterateNeighbours(
                [&](Vertex neighbour) {
                    if (!state.mapping.is_g1_mapped(neighbour)) {
                        return;
                    }

                    const MappedVertex u2 = state.mapping.get_mapping_g1_to_g2(neighbour);
                    assert(u2 != -1);

                    cost_candidate += CalculateEdgesAdditions_(g1, v1, neighbour, g2, v2, u2);
                },
                v1
            );
            min_cost = std::min(min_cost, cost_candidate);
        }
        h += min_cost;
    }
    return h;
}

// ------------------------------
// A star
// ------------------------------

struct AStarState {
    State state;
    int g;  // Real cost so far
    int f;  // f = g + h (priority)

    AStarState() : state(0, 0), g(0), f(0) {}

    AStarState(std::int32_t size_g1, std::int32_t size_g2) : state(size_g1, size_g2), g(0), f(0) {}

    AStarState(const State &s, int cost_g, int cost_f) : state(s), g(cost_g), f(cost_f) {}

    bool operator>(const AStarState &other) const { return f > other.f; }
};

std::vector<Mapping> AccurateAStar(const Graph &g1, const Graph &g2, const int k)
{
    if (g1.GetVertices() > g2.GetVertices()) {
        return {};
    }
    std::priority_queue<AStarState, std::vector<AStarState>, std::greater<AStarState>> pq;

    // Initialize with empty mapping
    AStarState initial = AStarState(g1.GetVertices(), g2.GetVertices());
    pq.push(initial);

    // To iterate available vertices in the loop, we also use bitset logic
    // though accurate A* state structure is the same.

    while (!pq.empty()) {
        AStarState current = pq.top();
        pq.pop();

        if (current.state.mapping.get_mapped_count() == g1.GetVertices()) {
            return {current.state.mapping};
        }

        // Choose next vertex to map
        const Vertex v1 = PickNextVertex_(g1, current.state);

        // Try mapping v1 to each available vertex in G2
        boost::dynamic_bitset<> available_mask = ~current.state.used_mask;
        for (Vertex v2 = available_mask.find_first(); v2 != static_cast<Vertex>(boost::dynamic_bitset<>::npos);
             v2        = available_mask.find_next(v2)) {
            AStarState next_state;
            next_state.state = current.state;
            next_state.state.set_mapping(v1, v2);

            const int cost_increment = CalculateAssignmentCost_(g1, g2, current.state.mapping, v1, v2);
            next_state.g             = current.g + cost_increment;

            // Calculate heuristic
            const int h  = CalculateHeuristic_(g1, g2, next_state.state);
            next_state.f = next_state.g + h;

            pq.push(next_state);
        }
    }

    return {};
}

// ------------------------------
// Approx A star
// ------------------------------

template <std::uint32_t R>
struct PrioArr {
    bool IsEmpty() { return used_ == 0; }

    AStarState &PeekBest()
    {
        assert(used_ > 0);
        return table_[0];
    }

    AStarState GetBest()
    {
        assert(used_ > 0);

        AStarState rv = table_[0];

        for (size_t idx = 0; idx < used_ - 1; ++idx) {
            table_[idx] = table_[idx + 1];
        }
        used_--;

        return rv;
    }

    void Insert(const AStarState &state)
    {
        if (used_ == R && state.f >= table_[used_ - 1].f) {
            return;
        }

        size_t insert_pos = used_;
        for (size_t i = 0; i < used_; ++i) {
            if (state.f < table_[i].f) {
                insert_pos = i;
                break;
            }
        }

        if (used_ < R) {
            used_++;
        }

        for (size_t i = used_ - 1; i > insert_pos; --i) {
            table_[i] = table_[i - 1];
        }

        table_[insert_pos] = state;
    }

    private:
    AStarState table_[R]{};
    size_t used_{0};
};

// Heap Node for pairing_heap
struct HeapNode {
    int f_val;
    std::uint32_t level_idx;

    // Comparator for Min-Heap behavior in a Priority Queue
    // A < B means A has lower priority. We want smaller f_val to have HIGHER priority.
    // So A < B should return true if A.f_val > B.f_val
    bool operator<(const HeapNode &other) const { return f_val > other.f_val; }
};

template <std::uint32_t R>
struct MasterQueue {
    using heap_t        = boost::heap::pairing_heap<HeapNode, boost::heap::mutable_<true>>;
    using handle_type_t = typename heap_t::handle_type;

    MasterQueue(const size_t size) : state_(size), handles_(size)
    {
        // Initialize handles to null/empty
        for (size_t i = 0; i < size; ++i) {
            handles_[i] = handle_type_t{};
        }
    }

    // Insert state into the specified level and update heap
    void Insert(const std::uint32_t idx, const AStarState &state)
    {
        PrioArr<R> &prio_arr = state_[idx];
        bool was_empty       = prio_arr.IsEmpty();
        int old_best_f       = was_empty ? INT_MAX : prio_arr.PeekBest().f;

        prio_arr.Insert(state);

        int new_best_f = prio_arr.PeekBest().f;

        // If the level was empty, insert into heap
        if (was_empty) {
            handles_[idx] = heap_.push({new_best_f, idx});
        } else {
            // If priority improved, decrease key
            if (new_best_f < old_best_f) {
                // Check if handle is valid (it should be if not empty)
                heap_.decrease(handles_[idx], {new_best_f, idx});
            }
        }
    }

    // Extract the global best state (min f) across all levels
    NODISCARD std::pair<std::uint32_t, AStarState> PopMin()
    {
        assert(!heap_.empty());

        // 1. Get the best level from heap
        HeapNode top = heap_.top();
        heap_.pop();

        const std::uint32_t idx = top.level_idx;

        // 2. Extract best state from that level
        PrioArr<R> &prio_arr = state_[idx];
        AStarState best      = prio_arr.GetBest();

        // 3. If PrioArr still has items, put it back into heap with new best f
        if (!prio_arr.IsEmpty()) {
            int next_best_f = prio_arr.PeekBest().f;
            handles_[idx]   = heap_.push({next_best_f, idx});
        } else {
            // Level effectively removed from heap
            handles_[idx] = handle_type_t{};
        }

        return {idx, best};
    }

    bool IsEmpty() const { return heap_.empty(); }

    private:
    std::vector<PrioArr<R>> state_;
    heap_t heap_;
    std::vector<handle_type_t> handles_;
};

template <std::uint32_t R = 1>
NODISCARD std::vector<Mapping> ApproxAStar_(const Graph &g1, const Graph &g2, int k)
{
    if (g1.GetVertices() > g2.GetVertices()) {
        return {};
    }

    Vertices n1 = g1.GetVertices();

    MasterQueue<R> master_queue(n1);
    const Vertex v_start = PickNextVertex_(g1, AStarState(n1, g2.GetVertices()).state);

    for (Vertex v = 0; v < g2.GetVertices(); ++v) {
        AStarState state(n1, g2.GetVertices());

        state.state.set_mapping(v_start, v);
        state.g = 0;
        state.f = CalculateHeuristic_(g1, g2, state.state);
        master_queue.Insert(0, state);
    }

    while (!master_queue.IsEmpty()) {
        // Extract the globally best state
        auto [idx, best_state] = master_queue.PopMin();

        if (idx == n1 - 1) {
            return {best_state.state.mapping};
        }

        Vertex next_vertex = PickNextVertex_(g1, best_state.state);

        // Iterate available vertices using bitset
        boost::dynamic_bitset<> available_mask = ~best_state.state.used_mask;
        for (Vertex mapping_candidate = available_mask.find_first();
             mapping_candidate != static_cast<Vertex>(boost::dynamic_bitset<>::npos);
             mapping_candidate = available_mask.find_next(mapping_candidate)) {
            AStarState next_state;
            next_state.state = best_state.state;
            next_state.state.set_mapping(next_vertex, mapping_candidate);

            const int cost_increment =
                CalculateAssignmentCost_(g1, g2, best_state.state.mapping, next_vertex, mapping_candidate);
            next_state.g = best_state.g + cost_increment;

            // Calculate heuristic
            const int h  = CalculateHeuristic_(g1, g2, next_state.state);
            next_state.f = next_state.g + h;

            // Insert into next level
            master_queue.Insert(idx + 1, next_state);
        }
    }

    return {};
}

NODISCARD std::vector<Mapping> ApproxAStar(const Graph &g1, const Graph &g2, int k) { return ApproxAStar_(g1, g2, k); }

NODISCARD std::vector<Mapping> ApproxAStar5(const Graph &g1, const Graph &g2, int k)
{
    return ApproxAStar_<5>(g1, g2, k);
}
