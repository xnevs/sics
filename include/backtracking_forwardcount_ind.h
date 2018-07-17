#ifndef GMCS_BACKTRACKING_FORWARDCOUNT_IND_H_
#define GMCS_BACKTRACKING_FORWARDCOUNT_IND_H_

#include <iterator>
#include <vector>

#include "graph_traits.h"
#include "graph_utilities.h"
#include "label_equivalence.h"
#include "consistency_utilities.h"

template <
    typename G,
    typename H,
    typename Callback,
    typename IndexOrderG,
    typename VertexEquiv = default_vertex_label_equiv<G, H>,
    typename EdgeEquiv = default_edge_label_equiv<G, H>>
void backtracking_forwardcount_ind(
    G const & g,
    H const & h,
    Callback const & callback,
    IndexOrderG const & index_order_g,
    VertexEquiv const & vertex_equiv = VertexEquiv(),
    EdgeEquiv const & edge_equiv = EdgeEquiv()) {

  using IndexG = typename G::index_type;
  using IndexH = typename H::index_type;

  struct explorer {

    G const & g;
    H const & h;
    Callback callback;

    IndexOrderG const & index_order_g;

    vertex_equiv_helper<VertexEquiv> vertex_equiv;
    edge_equiv_helper<EdgeEquiv> edge_equiv;

    IndexG m;
    IndexH n;

    typename IndexOrderG::const_iterator x_it;

    std::vector<IndexH> map;

    using g_count_type = std::conditional_t<
        is_directed_v<G>,
        std::pair<IndexG, IndexG>,
        IndexG>;
    std::vector<g_count_type> g_count;
    void build_g_count() {
      std::vector<IndexG> index_pos_g(m);
      for (IndexG i=0; i<m; ++i) {
        index_pos_g[index_order_g[i]] = i;
      }
      for (IndexH u=0; u<m; ++u) {
        for (auto oe : edges_or_out_edges(g, u)) {
          auto i = oe.target;
          if (index_pos_g[u] < index_pos_g[i]) {
            if constexpr (is_directed_v<G>) {
              ++g_count[i].second;
            } else {
              ++g_count[i];
            }
          } else {
            if constexpr (is_directed_v<G>) {
              ++g_count[u].first;
            } else {
              ++g_count[u];
            }
          }
        }
      }
    }

    using h_count_type = std::conditional_t<
        is_directed_v<H>,
        std::pair<IndexH, IndexH>,
        IndexH>;
    std::vector<h_count_type> h_count;

    explorer(
        G const & g,
        H const & h,
        Callback const & callback,
        IndexOrderG const & index_order_g,
        VertexEquiv const & vertex_equiv,
        EdgeEquiv const & edge_equiv)
        : g{g},
          h{h},
          callback{callback},
          index_order_g{index_order_g},
          vertex_equiv{vertex_equiv},
          edge_equiv{edge_equiv},

          m{g.num_vertices()},
          n{h.num_vertices()},
          x_it(std::cbegin(index_order_g)),
          map(m, n),
          g_count(m),
          h_count(n) {
      build_g_count();
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
            update_h_count(y);
            ++x_it;
            proceed = explore();
            --x_it;
            revert_h_count(y);
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

      if (!vertex_equiv(g, x, h, y)) {
        return false;
      }

      if (g_count[x] != h_count[y]) {
        return false;
      }

      for (auto it=std::cbegin(index_order_g); it!=x_it; ++it) {
        auto u = *it;
        auto v = map[u];
        if (v == y) {
          return false;
        }
        auto x_out = g.edge(x, u);
        if (x_out != h.edge(y, v) || (x_out && !edge_equiv(g, x, u, h, y, v))) {
          return false;
        }
        if constexpr (is_directed_v<G>) {
          auto x_in = g.edge(u, x);
          if (x_in != h.edge(v, y) || (x_in && !edge_equiv(g, u, x, h, v, y))) {
            return false;
          }
        }
      }
      return true;
    }

    void update_h_count(IndexH v) {
      if constexpr (is_directed_v<H>) {
        for (auto oe : h.out_edges(v)) {
          ++h_count[oe.target].second;
        }
        for (auto ie : h.in_edges(v)) {
          ++h_count[ie.target].first;
        }
      } else {
        for (auto e : h.edges(v)) {
          ++h_count[e.target];
        }
      }
    }

    void revert_h_count(IndexH v) {
      if constexpr (is_directed_v<H>) {
        for (auto oe : h.out_edges(v)) {
          --h_count[oe.target].second;
        }
        for (auto ie : h.in_edges(v)) {
          --h_count[ie.target].first;
        }
      } else {
        for (auto e : h.edges(v)) {
          --h_count[e.target];
        }
      }
    }
  } e(g, h, callback, index_order_g, vertex_equiv, edge_equiv);

  e.explore();
}

#endif  // GMCS_BACKTRACKING_FORWARDCOUNT_IND_H_
