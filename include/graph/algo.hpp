#pragma once
#include <vector>
#include <stack>
#include <queue>
#include <cstddef>
#include "traverse.hpp"

namespace graph::algo {

using size_t = std::size_t;

// ----------------------------------------------------------
// 穿透透明节点的可达性遍历 (使用 Version Tagging 优化 Reset)
// ----------------------------------------------------------
template <graph::traverse::IdOutGraph G, class TransparentPred, class Visitor>
void reach_through_transparent(const G &graph, std::size_t start,
                               TransparentPred &&is_transparent,
                               Visitor &&visitor) {
  const auto n = graph.node_count();
  if (start >= n)
    throw std::out_of_range("start id out of range");

  std::vector<std::size_t> seen(n, 0);
  std::size_t current_token = 1;
  std::vector<std::size_t> stack;
  stack.reserve(n);

  for (const auto &e : graph.out_edges(start)) {
    stack.push_back(e.to);
  }

  while (!stack.empty()) {
    auto u = stack.back();
    stack.pop_back();

    if (u >= n || seen[u] == current_token)
      continue;

    seen[u] = current_token;

    if (!is_transparent(u)) {
      visitor(u);
      continue;
    }
    for (const auto &e : graph.out_edges(u)) {
      stack.push_back(e.to);
    }
  }
}

// -------------------------------------------------------------------------
// Tarjan's SCC 结果的内存优化结构 (Flat Array)
// -------------------------------------------------------------------------
struct SCCFlatResult {
  std::vector<int> component_ids;
  int scc_count = 0;
  bool has_cycle = false;
};

template <typename G> SCCFlatResult tarjan_scc(const G &graph) {
  using size_t = std::size_t;
  size_t n = graph.node_count();
  std::vector<int> dfn(n, -1), low(n, -1);
  std::vector<bool> in_stack(n, false);
  std::stack<size_t> st;
  int timer = 0;
  SCCFlatResult res;
  res.component_ids.assign(n, -1);

  auto dfs = [&](auto &&self, size_t u) -> void {
    dfn[u] = low[u] = ++timer;
    st.push(u);
    in_stack[u] = true;

    for (const auto &edge : graph.out_edges(u)) {
      size_t v = edge.to;
      if (dfn[v] == -1) {
        self(self, v);
        low[u] = std::min(low[u], low[v]);
      } else if (in_stack[v]) {
        low[u] = std::min(low[u], dfn[v]);
      }
    }

    if (low[u] == dfn[u]) {
      size_t component_size = 0;
      while (true) {
        size_t v = st.top();
        st.pop();
        in_stack[v] = false;
        res.component_ids[v] = res.scc_count;
        component_size++;
        if (u == v)
          break;
      }
      if (component_size > 1)
        res.has_cycle = true;
      res.scc_count++;
    }
  };

  for (size_t i = 0; i < n; ++i) {
    if (dfn[i] == -1)
      dfs(dfs, i);
  }

  return res;
}

template <typename IdType>
std::vector<std::vector<IdType>>
group_sccs(const std::vector<int> &component_ids, int scc_count) {
  if (scc_count <= 0)
    return {};

  std::vector<std::vector<IdType>> components(scc_count);

  for (IdType node_id = 0; node_id < component_ids.size(); ++node_id) {
    int scc_id = component_ids[node_id];
    if (scc_id != -1) {
      components[scc_id].push_back(node_id);
    }
  }
  return components;
}

// ----------------------------------------------------------
// I. LayerRange
// ----------------------------------------------------------
struct LayerRange {
  std::span<const std::size_t> nodes;

  auto begin() const { return nodes.begin(); }
  auto end() const { return nodes.end(); }
};

struct KahnResult {
  std::vector<std::size_t> nodes_sorted;
  std::vector<std::size_t> layer_offsets;
  bool has_cycle = false;

  struct LayerIterator {
    const KahnResult *result;
    std::size_t current_layer_index;

    using iterator_category = std::forward_iterator_tag;
    using value_type = LayerRange;
    using difference_type = std::ptrdiff_t;

    LayerRange operator*() const {
      std::size_t start_offset = result->layer_offsets[current_layer_index];
      std::size_t end_offset = result->layer_offsets[current_layer_index + 1];

      return {std::span<const std::size_t>{result->nodes_sorted.data() +
                                               start_offset,
                                           end_offset - start_offset}};
    }

    LayerIterator &operator++() {
      ++current_layer_index;
      return *this;
    }

    bool operator==(const LayerIterator &other) const {
      return current_layer_index == other.current_layer_index;
    }
    bool operator!=(const LayerIterator &other) const {
      return !(*this == other);
    }
  };

  LayerIterator begin() const {
    return {this, 0};
  }

  LayerIterator end() const {
    return {this, layer_offsets.size() - 1};
  }
};

template <typename G> KahnResult kahn_layers(const G &graph) {
  using size_t = std::size_t;
  size_t n = graph.node_count();

  std::vector<size_t> in_degree(n);
  for (size_t i = 0; i < n; ++i) {
    in_degree[i] = graph.in_degree(i);
  }
  std::queue<size_t> q;
  for (size_t i = 0; i < n; ++i) {
    if (in_degree[i] == 0)
      q.push(i);
  }

  KahnResult res;
  res.layer_offsets.push_back(0);
  size_t processed_count = 0;

  while (!q.empty()) {
    size_t layer_size = q.size();

    res.nodes_sorted.reserve(res.nodes_sorted.size() + layer_size);

    for (size_t i = 0; i < layer_size; ++i) {
      size_t u = q.front();
      q.pop();

      res.nodes_sorted.push_back(u);
      processed_count++;

      for (const auto &edge : graph.out_edges(u)) {
        size_t v = edge.to;
        in_degree[v]--;
        if (in_degree[v] == 0)
          q.push(v);
      }
    }

    res.layer_offsets.push_back(res.nodes_sorted.size());
  }

  if (processed_count != n) {
    res.has_cycle = true;
  }

  return res;
}

template <typename G>
std::vector<typename G::id_type> inbound_nodes(const G &graph) {
  using id_type = typename G::id_type;
  const size_t n = graph.node_count();
  std::vector<id_type> result;
  result.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    if (graph.in_degree(i) == 0) {
      result.push_back(static_cast<id_type>(i));
    }
  }
  return result;
}

template <typename G>
std::vector<typename G::id_type> outbound_nodes(const G &graph) {
  using id_type = typename G::id_type;
  const size_t n = graph.node_count();
  std::vector<id_type> result;
  result.reserve(n);

  for (size_t u = 0; u < n; ++u) {
    if (graph.out_edges(u).empty()) {
      result.push_back(static_cast<id_type>(u));
    }
  }
  return result;
}

} // namespace graph::algo
