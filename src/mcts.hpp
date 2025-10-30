#ifndef MCTS_HPP
#define MCTS_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <vector>
#include "State.hpp"
#include "graph.hpp"

// Action represents mapping a G1 vertex to a G2 vertex
struct Action {
    int g1_vertex;
    int g2_vertex;

    Action(int g1, int g2) : g1_vertex(g1), g2_vertex(g2) {}
};

// Forward declaration
class MCTSNode;

class MCTS
{
    public:
    MCTS(const Graph& g1, const Graph& g2, double exploration_constant = std::sqrt(2.0))
        : g1_(g1), g2_(g2), exploration_constant_(exploration_constant), rng_(std::random_device{}())
    {
        n1_ = g1_.GetVertices();
        n2_ = g2_.GetVertices();
    }

    // Run MCTS for a given number of iterations and return the best mapping found
    State Search(int num_iterations);

    // Calculate the number of edges that need to be added to G2 for a given mapping
    int CalculateMissingEdges(const State& state) const;

    private:
    const Graph& g1_;
    const Graph& g2_;
    int n1_;
    int n2_;
    double exploration_constant_;
    std::mt19937 rng_;

    // MCTS phases
    MCTSNode* Select(MCTSNode* node);
    void Expand(MCTSNode* node, const State& state);
    int Simulate(State state);
    void Backpropagate(MCTSNode* node, int reward);

    // Helper functions
    std::vector<Action> GetPossibleActions(const State& state) const;
    bool IsTerminal(const State& state) const;
    int GetUnmappedG1Vertex(const State& state) const;
};

class MCTSNode
{
    public:
    MCTSNode(MCTSNode* parent = nullptr, Action action = Action(-1, -1))
        : parent_(parent), action_(action), visits_(0), total_reward_(0.0)
    {
    }

    ~MCTSNode()
    {
        for (auto child : children_) {
            delete child;
        }
    }

    bool IsFullyExpanded() const { return untried_actions_.empty(); }

    bool IsLeaf() const { return children_.empty(); }

    MCTSNode* BestChild(double exploration_constant) const
    {
        MCTSNode* best    = nullptr;
        double best_value = -std::numeric_limits<double>::infinity();

        for (auto child : children_) {
            if (child->visits_ == 0)
                continue;

            // UCB1 formula: average reward + exploration term
            double exploitation = child->total_reward_ / child->visits_;
            double exploration  = exploration_constant * std::sqrt(std::log(visits_) / child->visits_);
            double ucb_value    = exploitation + exploration;

            if (ucb_value > best_value) {
                best_value = ucb_value;
                best       = child;
            }
        }

        return best;
    }

    MCTSNode* MostVisitedChild() const
    {
        MCTSNode* best = nullptr;
        int max_visits = -1;

        for (auto child : children_) {
            if (child->visits_ > max_visits) {
                max_visits = child->visits_;
                best       = child;
            }
        }

        return best;
    }

    void AddChild(MCTSNode* child) { children_.push_back(child); }

    MCTSNode* parent_;
    Action action_;
    std::vector<MCTSNode*> children_;
    std::vector<Action> untried_actions_;
    int visits_;
    double total_reward_;
};

// Calculate the number of edges that need to be added to G2
inline int MCTS::CalculateMissingEdges(const State& state) const
{
    int missing_edges = 0;

    // For each edge in G1, check if it exists in G2 under the current mapping
    for (std::uint32_t u = 0; u < n1_; ++u) {
        // Check if u is mapped
        auto it_u = state.mapping.find(u);
        if (it_u == state.mapping.end())
            continue;
        int mapped_u = it_u->second;

        for (std::uint32_t v = 0; v < n1_; ++v) {
            // Check if v is mapped
            auto it_v = state.mapping.find(v);
            if (it_v == state.mapping.end())
                continue;
            int mapped_v = it_v->second;

            // Count missing edges
            std::uint32_t edges_in_g1 = g1_.GetEdges(u, v);
            std::uint32_t edges_in_g2 = g2_.GetEdges(mapped_u, mapped_v);

            if (edges_in_g1 > edges_in_g2) {
                missing_edges += (edges_in_g1 - edges_in_g2);
            }
        }
    }

    return missing_edges;
}

// Get possible actions from current state
inline std::vector<Action> MCTS::GetPossibleActions(const State& state) const
{
    std::vector<Action> actions;

    // Find the first unmapped G1 vertex
    int unmapped_g1 = GetUnmappedG1Vertex(state);
    if (unmapped_g1 == -1) {
        return actions;  // All vertices mapped
    }

    // Try mapping it to each available G2 vertex
    for (int g2_vertex : state.availableVertices) {
        actions.emplace_back(unmapped_g1, g2_vertex);
    }

    return actions;
}

// Check if all G1 vertices are mapped
inline bool MCTS::IsTerminal(const State& state) const { return state.mapping.size() == n1_; }

// Get the first unmapped G1 vertex
inline int MCTS::GetUnmappedG1Vertex(const State& state) const
{
    for (std::uint32_t i = 0; i < n1_; ++i) {
        if (state.mapping.find(i) == state.mapping.end()) {
            return i;
        }
    }
    return -1;  // All mapped
}

// Selection phase: traverse tree using UCB1
inline MCTSNode* MCTS::Select(MCTSNode* node)
{
    while (!node->IsLeaf()) {
        if (!node->IsFullyExpanded()) {
            return node;  // Found a node to expand
        }
        node = node->BestChild(exploration_constant_);
    }
    return node;
}

// Expansion phase: add one child node
inline void MCTS::Expand(MCTSNode* node, const State& state)
{
    if (IsTerminal(state)) {
        return;  // Terminal state, no expansion
    }

    if (node->untried_actions_.empty()) {
        node->untried_actions_ = GetPossibleActions(state);
    }

    if (!node->untried_actions_.empty()) {
        // Pick a random untried action
        std::uniform_int_distribution<size_t> dist(0, node->untried_actions_.size() - 1);
        size_t idx    = dist(rng_);
        Action action = node->untried_actions_[idx];
        node->untried_actions_.erase(node->untried_actions_.begin() + idx);

        // Create new child node
        MCTSNode* child = new MCTSNode(node, action);
        node->AddChild(child);
    }
}

// Simulation phase: random playout
inline int MCTS::Simulate(State state)
{
    while (!IsTerminal(state)) {
        std::vector<Action> actions = GetPossibleActions(state);
        if (actions.empty())
            break;

        // Pick a random action
        std::uniform_int_distribution<size_t> dist(0, actions.size() - 1);
        Action action = actions[dist(rng_)];

        state.SetMapping(action.g1_vertex, action.g2_vertex);
    }

    // Reward is negative of missing edges (we want to minimize)
    // Higher reward = fewer missing edges
    int missing = CalculateMissingEdges(state);
    return -missing;
}

// Backpropagation phase: update statistics
inline void MCTS::Backpropagate(MCTSNode* node, int reward)
{
    while (node != nullptr) {
        node->visits_++;
        node->total_reward_ += reward;
        node = node->parent_;
    }
}

// Main MCTS search
inline State MCTS::Search(int num_iterations)
{
    State initial_state(n2_);
    MCTSNode* root         = new MCTSNode();
    root->untried_actions_ = GetPossibleActions(initial_state);

    for (int i = 0; i < num_iterations; ++i) {
        // 1. Selection
        MCTSNode* node = Select(root);

        // Reconstruct state by following path from root to node
        State state(n2_);
        MCTSNode* path_node = node;
        std::vector<Action> path;
        while (path_node->parent_ != nullptr) {
            path.push_back(path_node->action_);
            path_node = path_node->parent_;
        }
        std::reverse(path.begin(), path.end());
        for (const Action& action : path) {
            state.SetMapping(action.g1_vertex, action.g2_vertex);
        }

        // 2. Expansion
        Expand(node, state);
        if (!node->children_.empty()) {
            node = node->children_.back();
            state.SetMapping(node->action_.g1_vertex, node->action_.g2_vertex);
        }

        // 3. Simulation
        int reward = Simulate(state);

        // 4. Backpropagation
        Backpropagate(node, reward);
    }

    // Extract best path
    State best_state(n2_);
    MCTSNode* current = root;
    while (!current->children_.empty()) {
        current = current->MostVisitedChild();
        if (current == nullptr)
            break;
        best_state.SetMapping(current->action_.g1_vertex, current->action_.g2_vertex);
    }

    delete root;
    return best_state;
}

#endif  // MCTS_HPP
