#ifndef SICS_BACKTRACKING_PARENT_ADJACENTCONSISTENCY_IND_H_
#define SICS_BACKTRACKING_PARENT_ADJACENTCONSISTENCY_IND_H_

#include <iterator>
#include <tuple>
#include <vector>

#include "graph_traits.h"
#include "label_equivalence.h"
#include "consistency_utilities.h"

#include "stats.h"

namespace sics {

template <
    typename G,
    typename H,
    typename Callback,
    typename IndexOrderG,
    typename VertexEquiv = default_vertex_label_equiv<G, H>,
    typename EdgeEquiv = default_edge_label_equiv<G, H>>
void backtracking_parent_adjacentconsistency_ind(
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
    std::vector<IndexG> inv;

    using parent_type = std::conditional_t<
        is_directed_v<H>,
        std::tuple<IndexG, bool>,
        std::tuple<IndexG>>;
    std::vector<parent_type> parents;
    void build_parents() {
      for (IndexG u=0; u<m; ++u) {
        std::get<0>(parents[u]) = m;
      }
      std::vector<bool> done(m, false);
      auto end = std::prev(std::cend(index_order_g));
      for (auto it=std::cbegin(index_order_g); it!=end; ++it) {
        auto u = *it;
        done[u] = true;
        if constexpr (is_directed_v<G>) {
          for (auto oe : g.out_edges(u)) {
            auto i = oe.target;
            if (std::get<0>(parents[i]) == m && !done[i]) {
              parents[i] = {u, true};
            }
          }
          for (auto ie : g.in_edges(u)) {
            auto i = ie.target;
            if (std::get<0>(parents[i]) == m && !done[i]) {
              parents[i] = {u, false};
            }
          }
        } else {
          for (auto e : g.edges(u)) {
            auto i = e.target;
            if (std::get<0>(parents[i]) == m && !done[i]) {
              parents[i] = {u};
            }
          }
        }
      }
    }

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
          inv(n, m),
          parents(m) {
      build_parents();
    }

    bool explore() {
      SICS_STATS_STATE;
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        bool proceed = true;

        parent_type p = parents[x];
        if (std::get<0>(p) == m) {
          for (IndexH y=0; y<n; ++y) {
            if (consistency(x, y)) {
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
        } else {
          for (auto he : get_parent_edges(p)) {
            // TODO maybe add edge_equiv() check for parent
            auto y = he.target;
            if (consistency(x, y)) {
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
        }

        return proceed;
      }
    }

    auto get_parent_edges(parent_type p) {
      if constexpr (is_directed_v<H>) {
        if (std::get<1>(p)) {
          return h.out_edges(map[std::get<0>(p)]);
        } else {
          return h.in_edges(map[std::get<0>(p)]);
        }
      } else {
        return h.edges(map[std::get<0>(p)]);
      }
    }

    bool consistency(IndexG u, IndexH v) {
      return
          vertex_equiv(g, u, h, v) &&
          inv[v] == m &&
          adjacent_consistency_ind(g, u, h, v, map, inv, edge_equiv);
    }
  } e(g, h, callback, index_order_g, vertex_equiv, edge_equiv);

  e.explore();
}

}  // namespace sics

#endif  // SICS_BACKTRACKING_PARENT_ADJACENTCONSISTENCY_IND_H_
