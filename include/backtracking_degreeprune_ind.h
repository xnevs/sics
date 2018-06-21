#ifndef GMCS_BACKTRACKING_DEGREEPRUNE_IND_H_
#define GMCS_BACKTRACKING_DEGREEPRUNE_IND_H_

#include <iterator>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_degreeprune_ind(
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
          map(m, n) {
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
          if (vertex_equiv(x, y) &&
              g.out_degree(x) <= h.out_degree(y) &&
              g.in_degree(x) <= h.in_degree(y) &&
              consistency(y)) {
            map[x] = y;
            ++x_it;
            proceed = explore();
            --x_it;
            map[x] = n;
            if (!proceed) {
              break;
            }
          }
        }
        return proceed;
      }
    }
    
    bool consistency(IndexH y) {
      auto x = *x_it;
      for (auto it=std::cbegin(index_order_g); it!=x_it; ++it) {
        auto u = *it;
        auto v = map[u];
        if (v == y) {
          return false;
        }
        auto x_out = g.edge(x, u);
        if (x_out != h.edge(y, v)) {
          return false;
        }
        auto x_in = g.edge(u, x);
        if (x_in != h.edge(v, y)) {
          return false;
        }
        if (x_out && !edge_equiv(x, u, y, v)) {
          return false;
        }
        if (x_in && !edge_equiv(u, x, v, y)) {
          return false;
        }
      }
      return true;
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);
  
  e.explore();
}

#endif  // GMCS_BACKTRACKING_DEGREEPRUNE_IND_H_
