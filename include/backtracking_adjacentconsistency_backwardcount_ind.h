#ifndef GMCS_BACKTRACKING_ADJACENTCONSISTENCY_BACKWARDCOUNT_IND_H_
#define GMCS_BACKTRACKING_ADJACENTCONSISTENCY_BACKWARDCOUNT_IND_H_

#include <iterator>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_adjacentconsistency_backwardcount_ind(
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
    
    typename IndexOrderG::const_iterator x_it;

    std::vector<IndexH> map;
    std::vector<IndexG> inv;
    
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
          x_it(std::cbegin(index_order_g)),
          map(m, n),
          inv(n, m) {
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
          if (inv[y] == m &&
              vertex_equiv(x, y) &&
              topology_consistency(x, y)) {
            map[x] = y;
            inv[y] = x;
            ++x_it;
            proceed = explore();
            --x_it;
            inv[y] = m;
            map[x] = n;
            if (!proceed) {
              break;
            }
          }
        }
        return proceed;
      }
    }
    
    bool topology_consistency(IndexG u, IndexH v) {
      IndexG u_out_count = 0;
      for (auto i : g.adjacent_vertices(u)) {
        auto j = map[i];
        if (j != n) {
          if (!h.edge(v, j) || !edge_equiv(u, i, v, j)) {
            return false;
          }
          ++u_out_count;
        }
      }
      IndexG u_in_count = 0;
      for (auto i : g.inv_adjacent_vertices(u)) {
        auto j = map[i];
        if (j != n) {
          if (!h.edge(j, v) || !edge_equiv(i, u, j, v)) {
            return false;
          }
          ++u_in_count;
        }
      }
      IndexH v_out_count = 0;
      for (auto j : h.adjacent_vertices(v)) {
        if (inv[j] != m) {
          ++v_out_count;
        }
      }
      if (u_out_count != v_out_count) {
        return false;
      }
      IndexH v_in_count = 0;
      for (auto j : h.inv_adjacent_vertices(v)) {
        if (inv[j] != m) {
          ++v_in_count;
        }
      }
      return u_in_count == v_in_count;
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);
  
  e.explore();
}

#endif  // GMCS_BACKTRACKING_ADJACENTCONSISTENCY_BACKWARDCOUNT_IND_H_
