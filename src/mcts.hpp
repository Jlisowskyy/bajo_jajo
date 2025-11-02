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

struct Action {
    std::int32_t g1_vertex;
    std::int32_t g2_vertex;

    Action(std::int32_t g1, std::int32_t g2) : g1_vertex(g1), g2_vertex(g2) {}
};

class MCTSNode;

class MCTS
{
    public:
    MCTS(const Graph &g1, const Graph &g2, double exploration_constant = std::sqrt(2.0))
        : g1_(g1), g2_(g2), exploration_constant_(exploration_constant), rng_(std::random_device{}())
    {
        n1_ = g1_.GetVertices();
        n2_ = g2_.GetVertices();
    }

    State Search(int num_iterations);

    int CalculateMissingEdges(const State &state) const;

    private:
    const Graph &g1_;
    const Graph &g2_;
    std::int32_t n1_;
    std::int32_t n2_;
    double exploration_constant_;
    std::mt19937 rng_;

    MCTSNode *Select(MCTSNode *node);
    void Expand(MCTSNode *node, const State &state);
    int Simulate(State state);
    void Backpropagate(MCTSNode *node, int reward);

    std::vector<Action> GetPossibleActions(const State &state) const;
    bool IsTerminal(const State &state) const;
    std::int32_t GetUnmappedG1Vertex(const State &state) const;
};

class MCTSNode
{
    public:
    MCTSNode(MCTSNode *parent = nullptr, Action action = Action(-1, -1))
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

    MCTSNode *BestChild(double exploration_constant) const
    {
        MCTSNode *best    = nullptr;
        double best_value = -std::numeric_limits<double>::infinity();

        for (auto child : children_) {
            if (child->visits_ == 0)
                continue;

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

    MCTSNode *MostVisitedChild() const
    {
        MCTSNode *best = nullptr;
        int max_visits = -1;

        for (auto child : children_) {
            if (child->visits_ > max_visits) {
                max_visits = child->visits_;
                best       = child;
            }
        }

        return best;
    }

    void AddChild(MCTSNode *child) { children_.push_back(child); }

    MCTSNode *parent_;
    Action action_;
    std::vector<MCTSNode *> children_;
    std::vector<Action> untried_actions_;
    int visits_;
    double total_reward_;
};

inline int MCTS::CalculateMissingEdges(const State &state) const
{
    int missing_edges = 0;

    for (std::int32_t u = 0; u < n1_; ++u) {
        std::int32_t mapped_u = state.mapping.get_mapping_g1_to_g2(u);
        if (mapped_u == -1)
            continue;

        for (std::int32_t v = 0; v < n1_; ++v) {
            std::int32_t mapped_v = state.mapping.get_mapping_g1_to_g2(v);
            if (mapped_v == -1)
                continue;

            std::uint32_t edges_in_g1 = g1_.GetEdges(u, v);
            std::uint32_t edges_in_g2 = g2_.GetEdges(mapped_u, mapped_v);

            if (edges_in_g1 > edges_in_g2) {
                missing_edges += (edges_in_g1 - edges_in_g2);
            }
        }
    }

    return missing_edges;
}

inline std::vector<Action> MCTS::GetPossibleActions(const State &state) const
{
    std::vector<Action> actions;

    std::int32_t unmapped_g1 = GetUnmappedG1Vertex(state);
    if (unmapped_g1 == -1) {
        return actions;
    }

    for (std::int32_t g2_vertex : state.availableVertices) {
        actions.emplace_back(unmapped_g1, g2_vertex);
    }

    return actions;
}

inline bool MCTS::IsTerminal(const State &state) const { return state.mapping.get_mapped_count() == n1_; }

inline std::int32_t MCTS::GetUnmappedG1Vertex(const State &state) const
{
    for (std::int32_t i = 0; i < n1_; ++i) {
        if (!state.mapping.is_g1_mapped(i)) {
            return i;
        }
    }
    return -1;
}

inline MCTSNode *MCTS::Select(MCTSNode *node)
{
    while (!node->IsLeaf()) {
        if (!node->IsFullyExpanded()) {
            return node;
        }
        node = node->BestChild(exploration_constant_);
    }
    return node;
}

inline void MCTS::Expand(MCTSNode *node, const State &state)
{
    if (IsTerminal(state)) {
        return;
    }

    if (node->untried_actions_.empty()) {
        node->untried_actions_ = GetPossibleActions(state);
    }

    if (!node->untried_actions_.empty()) {
        std::uniform_int_distribution<size_t> dist(0, node->untried_actions_.size() - 1);
        size_t idx    = dist(rng_);
        Action action = node->untried_actions_[idx];
        node->untried_actions_.erase(node->untried_actions_.begin() + idx);

        MCTSNode *child = new MCTSNode(node, action);
        node->AddChild(child);
    }
}

inline int MCTS::Simulate(State state)
{
    while (!IsTerminal(state)) {
        std::vector<Action> actions = GetPossibleActions(state);
        if (actions.empty())
            break;

        std::uniform_int_distribution<size_t> dist(0, actions.size() - 1);
        Action action = actions[dist(rng_)];

        state.set_mapping(action.g1_vertex, action.g2_vertex);
    }

    int missing = CalculateMissingEdges(state);
    return -missing;
}

inline void MCTS::Backpropagate(MCTSNode *node, int reward)
{
    while (node != nullptr) {
        node->visits_++;
        node->total_reward_ += reward;
        node = node->parent_;
    }
}

inline State MCTS::Search(int num_iterations)
{
    State initial_state(n1_, n2_);
    MCTSNode *root         = new MCTSNode();
    root->untried_actions_ = GetPossibleActions(initial_state);

    for (int i = 0; i < num_iterations; ++i) {
        MCTSNode *node = Select(root);

        State state(n1_, n2_);
        MCTSNode *path_node = node;
        std::vector<Action> path;
        while (path_node->parent_ != nullptr) {
            path.push_back(path_node->action_);
            path_node = path_node->parent_;
        }
        std::reverse(path.begin(), path.end());
        for (const Action &action : path) {
            state.set_mapping(action.g1_vertex, action.g2_vertex);
        }

        Expand(node, state);
        if (!node->children_.empty()) {
            node = node->children_.back();
            state.set_mapping(node->action_.g1_vertex, node->action_.g2_vertex);
        }

        int reward = Simulate(state);

        Backpropagate(node, reward);
    }

    State best_state(n1_, n2_);
    MCTSNode *current = root;
    while (!current->children_.empty()) {
        current = current->MostVisitedChild();
        if (current == nullptr)
            break;
        best_state.set_mapping(current->action_.g1_vertex, current->action_.g2_vertex);
    }

    delete root;
    return best_state;
}

#endif  // MCTS_HPP
