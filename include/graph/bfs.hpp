#pragma once
#include <vector>
#include <cstddef>
#include "traverse.hpp"

namespace graph::algo {

using graph::traverse::BFSRange;

template<class G>
class BFSTrait {
public:
    using id_type = std::size_t;

    explicit BFSTrait(const G& g) : g_(g) {}

    std::vector<int> build_depth_map(id_type start) const {
        const auto n = g_.node_count();
        if (start >= n) throw std::out_of_range("start id out of range");

        std::vector<int> depth(n, -1);
        depth[start] = 0;

        for (auto u : BFSRange<G>(g_, start)) {
            int du = depth[u];
            for (auto& e : g_.out_edges(u)) {
                auto v = e.to;
                if (depth[v] == -1) depth[v] = du + 1;
            }
        }
        return depth;
    }

    bool has_cycle_reachable(id_type start) const {
        const auto n = g_.node_count();
        if (start >= n) throw std::out_of_range("start id out of range");

        std::vector<unsigned char> color(n, 0);
        std::vector<std::pair<id_type,std::size_t>> st;
        st.reserve(n);

        auto push_node = [&](id_type u){
            color[u] = 1;
            st.emplace_back(u, 0);
        };

        push_node(start);

        while (!st.empty()) {
            auto& [u, idx] = st.back();
            auto& edges = g_.out_edges(u);

            if (idx == edges.size()) {
                color[u] = 2;
                st.pop_back();
                continue;
            }

            id_type v = edges[idx++].to;

            if (color[v] == 1) {
                return true;
            }
            if (color[v] == 0) {
                push_node(v);
            }
        }
        return false;
    }

    bool is_arborescence(id_type start) const {
        const auto n = g_.node_count();
        if (start >= n) throw std::out_of_range("start id out of range");

        std::vector<unsigned char> reachable(n, 0);
        std::vector<int> indeg(n, 0);

        for (auto u : BFSRange<G>(g_, start)) {
            reachable[u] = 1;
            for (auto& e : g_.out_edges(u)) {
                id_type v = e.to;
                if (!reachable[v]) {
                    // will be marked by BFS eventually
                }
                indeg[v] += 1;
            }
        }

        for (auto r : reachable) if (!r) return false;

        if (has_cycle_reachable(start)) return false;

        if (indeg[start] != 0) return false;
        for (id_type u = 0; u < n; ++u) {
            if (u == start) continue;
            if (indeg[u] != 1) return false;
        }
        return true;
    }

private:
    const G& g_;
};

} // namespace graph::algo
