#ifndef GMCS_BACKJUMPING_IND_H_
#define GMCS_BACKJUMPING_IND_H_

#include <iterator>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backjumping_ind(
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
    
    using x_it_type = typename IndexOrderG::const_iterator;
    x_it_type x_it;
    x_it_type backjump_it;

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
          backjump_it{x_it},
          map(m, n) {
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        bool proceed = true;
        backjump_it = std::next(x_it);
        x_it_type latest_it = std::cbegin(index_order_g);
        for (IndexH y=0; y<n; ++y) {
          x_it_type culprit_it;
          if (!vertex_equiv(x, y)) {
            culprit_it = std::cbegin(index_order_g);
          } else {
            culprit_it = consistency(y);
          }
          if (culprit_it > x_it) {
            map[x] = y;
            ++x_it;
            proceed = explore();
            --x_it;
            map[x] = n;
            if (!proceed || backjump_it <= x_it) {
              break;
            }
          }
          if (culprit_it > latest_it) {
            latest_it = culprit_it;
          }
        }
        if (backjump_it > x_it && latest_it <= x_it) {
          backjump_it = latest_it;
        }
        return proceed;
      }
    }
    
    x_it_type consistency(IndexH y) {
      auto x = *x_it;
      x_it_type it;
      for (it=std::cbegin(index_order_g); it!=x_it; ++it) {
        auto u = *it;
        auto v = map[u];
        if (v == y) {
          break;
        }
        auto x_out = g.edge(x, u);
        if (x_out != h.edge(y, v)) {
          break;
        }
        auto x_in = g.edge(u, x);
        if (x_in != h.edge(v, y)) {
          break;
        }
        if (x_out && !edge_equiv(x, u, y, v)) {
          break;
        }
        if (x_in && !edge_equiv(u, x, v, y)) {
          break;
        }
      }
      return std::next(it);
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);
  
  e.explore();
}

#endif  // GMCS_BACKJUMPING_IND_H_
