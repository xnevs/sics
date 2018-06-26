#ifndef GMCS_LAZYFORWARDCHECKING_PARENT_DEGREEPRUNE_IND_H_
#define GMCS_LAZYFORWARDCHECKING_PARENT_DEGREEPRUNE_IND_H_

#include <iterator>
#include <vector>
#include <stack>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void lazyforwardchecking_parent_degreeprune_ind(
    G const & g,
    H const & h,
    VertexEquiv const & vertex_equiv,
    EdgeEquiv const & edge_equiv,
    IndexOrderG const & index_order_g,
    Callback const & callback) {
    
  using IndexG = typename G::index_type;
  using IndexH = typename H::index_type;
  
  struct explorer {
  
    G const & g;
    H const & h;
    VertexEquiv vertex_equiv;
    EdgeEquiv edge_equiv;
    IndexOrderG const & index_order_g;
    Callback callback;
    
  
    IndexG m;
    IndexH n;
    
    IndexG level;

    std::vector<IndexH> map;
    
    typename H::adjacent_vertices_container_type h_vertices;
    using adjacent_vertices_range_type = boost::iterator_range<typename H::adjacent_vertices_container_type::const_iterator>;
    adjacent_vertices_range_type h_vertices_range;
    std::vector<std::pair<IndexG, bool>> parents;
    void build_parents() {
      std::vector<bool> done(m, false);
      auto end = std::prev(std::cend(index_order_g));
      for (auto it=std::cbegin(index_order_g); it!=end; ++it) {
        auto u = *it;
        done[u] = true;
        for (auto i : g.adjacent_vertices(u)) {
          if (parents[i].first == m && !done[i]) {
            parents[i] = {u, true};
          }
        }
        for (auto i : g.inv_adjacent_vertices(u)) {
          if (parents[i].first == m && !done[i]) {
            parents[i] = {u, false};
          }
        }
      }
    }
    
    std::vector<char> M;
    bool M_get(IndexG u, IndexH v) {
      return M[u*n + v];
    }
    void M_set(IndexG u, IndexH v) {
      M[u*n + v] = true;
    }
    void M_unset(IndexG u, IndexH v) {
      M[u*n + v] = false;
    }
    void build_M() {
      for (IndexG u=0; u<m; ++u) {
        for (IndexH v=0; v<n; ++v) {
          if (vertex_equiv(u, v) &&
              g.out_degree(u) <= h.out_degree(v) &&
              g.in_degree(u) <= h.in_degree(v)) {
            M_set(u, v);
          }
        }
      }
    }
    std::vector<std::stack<std::pair<IndexG,IndexH>>> M_sts;
    
    explorer(
        G const & g,
        H const & h,
        VertexEquiv const & vertex_equiv,
        EdgeEquiv const & edge_equiv,
        IndexOrderG const & index_order_g,
        Callback const & callback)
        : g{g},
          h{h},
          vertex_equiv{vertex_equiv},
          edge_equiv{edge_equiv},
          index_order_g{index_order_g},
          callback{callback},
          
          m{g.num_vertices()},
          n{h.num_vertices()},
          level{0},
          map(m, n),
          h_vertices(n),
          h_vertices_range(std::cbegin(h_vertices), std::cend(h_vertices)),
          parents(m, {m, false}),
          M(m * n, false),
          M_sts(m) {
      std::iota(std::begin(h_vertices), std::end(h_vertices), 0);
      build_parents();
      build_M();
    }
    
    bool explore() {
      if (level == m) {
        return callback();
      } else {
        auto x = index_order_g[level];
        bool proceed = true;
        for (auto y : get_candidates(x)) {
          if (M_get(x, y) &&
              consistency(y)) {
            map[x] = y;
            ++level;
            proceed = explore();
            --level;
            map[x] = n;
            revert_M();
            if (!proceed) {
              break;
            }
          }
        }
        return proceed;
      }
    }
    
    adjacent_vertices_range_type get_candidates(IndexG u) {
      IndexG parent;
      bool out;
      std::tie(parent, out) = parents[u];
      if (parent != m) {
        if (out) {
          return h.adjacent_vertices(map[parent]);
        } else {
          return h.inv_adjacent_vertices(map[parent]);
        }
      } else {
        return h_vertices_range;
      }
    }
    
    bool consistency(IndexH y) {
      auto x = index_order_g[level];
      for (IndexG i=0; i<level; ++i) {
        auto u = index_order_g[i];
        auto v = map[u];
        if (v == y) {
          M_unset(x, y);
          M_sts[i].push({x, y});
          return false;
        }
        auto x_out = g.edge(x, u);
        if (x_out != h.edge(y, v)) {
          M_unset(x, y);
          M_sts[i].push({x, y});
          return false;
        }
        auto x_in = g.edge(u, x);
        if (x_in != h.edge(v, y)) {
          M_unset(x, y);
          M_sts[i].push({x, y});
          return false;
        }
        if (x_out && !edge_equiv(x, u, y, v)) {
          M_unset(x, y);
          M_sts[i].push({x, y});
          return false;
        }
        if (x_in && !edge_equiv(u, x, v, y)) {
          M_unset(x, y);
          M_sts[i].push({x, y});
          return false;
        }
      }
      return true;
    }
    
    void revert_M() {
      while (!M_sts[level].empty()) {
        IndexG u;
        IndexH v;
        std::tie(u, v) = M_sts[level].top();
        M_sts[level].pop();
        M_set(u, v);
      }
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);
  
  e.explore();
}

#endif  // GMCS_LAZYFORWARDCHECKING_PARENT_DEGREEPRUNE_IND_H_
