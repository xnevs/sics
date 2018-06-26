#ifndef GMCS_BACKMARKING_DEGREEPRUNE_IND_H_
#define GMCS_BACKMARKING_DEGREEPRUNE_IND_H_

#include <iterator>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backmarking_degreeprune_ind(
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
    
    std::vector<IndexG> low;
    std::vector<IndexG> M;
    IndexG M_get(IndexG u, IndexH v) {
      return M[u*n + v];
    }
    void M_set(IndexG u, IndexH v, IndexG level) {
      M[u*n + v] = level;
    }
    void build_M() {
      for (IndexG u=0; u<m; ++u) {
        for (IndexH v=0; v<n; ++v) {
          if (!vertex_equiv(u, v) ||
              g.out_degree(u) > h.out_degree(v) ||
              g.in_degree(u) > h.in_degree(v)) {
            M_set(u, v, 0);
          }
        }
      }
    }
    
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
          low(m, 0),
          M(m * n, m) {
      build_M();
    }
    
    bool explore() {
      if (level == m) {
        return callback();
      } else {
        auto x = index_order_g[level];
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
          if (M_get(x, y) > low[level] &&
              consistency(y)) {
            for (IndexG i=level+1; i<m && level<low[i]; ++i) {
              low[i] = level;
            }
            map[x] = y;
            ++level;
            proceed = explore();
            --level;
            map[x] = n;
            if (!proceed) {
              break;
            }
          }
        }
        low[level] = level;
        return proceed;
      }
    }
    
    bool consistency(IndexH y) {
      auto x = index_order_g[level];
      for (IndexG i=low[level]; i<level; ++i) {
        auto u = index_order_g[i];
        auto v = map[u];
        if (v == y) {
          M_set(x, y, i+1);
          return false;
        }
        auto x_out = g.edge(x, u);
        if (x_out != h.edge(y, v)) {
          M_set(x, y, i+1);
          return false;
        }
        auto x_in = g.edge(u, x);
        if (x_in != h.edge(v, y)) {
          M_set(x, y, i+1);
          return false;
        }
        if (x_out && !edge_equiv(x, u, y, v)) {
          M_set(x, y, i+1);
          return false;
        }
        if (x_in && !edge_equiv(u, x, v, y)) {
          M_set(x, y, i+1);
          return false;
        }
      }
      M_set(x, y, m);
      return true;
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);
  
  e.explore();
}

#endif  // GMCS_BACKMARKING_DEGREEPRUNE_IND_H_
