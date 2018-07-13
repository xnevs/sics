#ifndef GMCS_BACKTRACKING_IND_H_
#define GMCS_BACKTRACKING_IND_H_

#include <iterator>
#include <vector>

#include "graph_traits.h"

template <
    typename G,
    typename H,
    typename Callback,
    typename IndexOrderG>
void backtracking_ind(
    G const & g,
    H const & h,
    Callback const & callback,
    IndexOrderG const & index_order_g) {

  using IndexG = typename G::index_type;
  using IndexH = typename H::index_type;

  struct explorer {

    G const & g;
    H const & h;
    IndexOrderG const & index_order_g;
    Callback callback;


    IndexG m;
    IndexH n;

    typename IndexOrderG::const_iterator x_it;

    std::vector<IndexH> map;

    explorer(
        G const & g,
        H const & h,
        IndexOrderG const & index_order_g,
        Callback const & callback)
        : g{g},
          h{h},
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
          if (consistency(y)) {
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

      if constexpr (is_vertex_labelled_v<G>) {
        // TODO maybe use a vertex_equiv function?
        if (g.get_vertex_label(x) != h.get_vertex_label(y)) {
          return false;
        }
      }

      for (auto it=std::cbegin(index_order_g); it!=x_it; ++it) {
        auto u = *it;
        auto v = map[u];
        if (v == y) {
          return false;
        }
        auto x_out = g.edge(x, u);
        if (x_out != h.edge(y, v)) { // TODO maybe use an edge_equiv function?
          return false;
        }
        auto x_in = g.edge(u, x);
        if (x_in != h.edge(v, y)) { // TODO maybe use an edge_equiv function?
          return false;
        }
      }
      return true;
    }
  } e(g, h, index_order_g, callback);

  e.explore();
}

#endif  // GMCS_BACKTRACKING_IND_H_
