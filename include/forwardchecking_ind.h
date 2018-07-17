#ifndef GMCS_FORWARDCHECKING_IND_H_
#define GMCS_FORWARDCHECKING_IND_H_

#include <iterator>
#include <vector>

#include "multi_stack.h"

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void forwardchecking_ind(
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
    multi_stack<std::pair<IndexG,IndexH>> M_mst;

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
          M_mst(m*n, m) {
      build_M();
    }

    bool explore() {
      if (level == m) {
        return callback();
      } else {
        auto x = index_order_g[level];
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
          if (M_get(x, y)) {
            M_mst.push_level();
            if (forward_check(y)) {
              map[x] = y;
              ++level;
              proceed = explore();
              --level;
              map[x] = n;
            }
            revert_M();
            M_mst.pop_level();
            if (!proceed) {
              break;
            }
          }
        }
        return proceed;
      }
    }

    bool forward_check(IndexH y) {
      auto x = index_order_g[level];

      bool not_empty = true;
      for (IndexG i=level+1; i<m && not_empty; ++i) {
        auto u = index_order_g[i];
        not_empty = false;
        bool x_out = g.edge(x, u);
        bool x_in = g.edge(u, x);
        for (IndexH v=0; v<n; ++v) {
          if (M_get(u, v)) {
            if (v == y ||
                x_out != h.edge(y, v) ||
                x_in != h.edge(v, y)) {
              M_unset(u, v);
              M_mst.push({u, v});
            } else {
              not_empty = true;
            }
          }
        }
      }
      return not_empty;
    }

    void revert_M() {
      while (!M_mst.level_empty()) {
        IndexG u;
        IndexH v;
        std::tie(u, v) = M_mst.top();
        M_mst.pop();
        M_set(u, v);
      }
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);

  e.explore();
}

#endif  // GMCS_FORWARDCHECKING_IND_H_
