#ifndef GMCS_LAZYFORWARDCHECKING_IND_H_
#define GMCS_LAZYFORWARDCHECKING_IND_H_

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
void lazyforwardchecking_ind(
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
          if (vertex_equiv(u, v)) {
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
          M(m * n, false),
          M_sts(m) {
      build_M();
    }
    
    bool explore() {
      if (level == m) {
        return callback();
      } else {
        auto x = index_order_g[level];
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
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

#endif  // GMCS_LAZYFORWARDCHECKING_IND_H_
