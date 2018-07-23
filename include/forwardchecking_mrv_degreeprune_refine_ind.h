#ifndef SICS_FORWARDCHECKING_MRV_DEGREEPRUNE_REFINE_IND_H_
#define SICS_FORWARDCHECKING_MRV_DEGREEPRUNE_REFINE_IND_H_

#include <iterator>
#include <numeric>
#include <vector>

#include "graph_traits.h"
#include "label_equivalence.h"
#include "multi_stack.h"

#include "stats.h"

template <
    typename G,
    typename H,
    typename Callback,
    typename VertexEquiv = default_vertex_label_equiv<G, H>,
    typename EdgeEquiv = default_edge_label_equiv<G, H>>
void forwardchecking_mrv_degreeprune_refine_ind(
    G const & g,
    H const & h,
    Callback const & callback,
    VertexEquiv const & vertex_equiv = VertexEquiv(),
    EdgeEquiv const & edge_equiv = EdgeEquiv()) {

  using IndexG = typename G::index_type;
  using IndexH = typename H::index_type;

  struct explorer {

    G const & g;
    H const & h;
    Callback callback;

    vertex_equiv_helper<VertexEquiv> vertex_equiv;
    edge_equiv_helper<EdgeEquiv> edge_equiv;

    IndexG m;
    IndexH n;

    IndexG level;

    std::vector<IndexG> index_order_g;

    std::vector<IndexH> map;

    std::vector<IndexH> num_candidates;
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
          if (vertex_equiv(g, u, h, v) &&
              degree_condition(g, u, h, v)) {
            M_set(u, v);
            ++num_candidates[u];
          }
        }
      }
    }
    multi_stack<std::pair<IndexG,IndexH>> M_mst;

    explorer(
        G const & g,
        H const & h,
        Callback const & callback,
        VertexEquiv const & vertex_equiv,
        EdgeEquiv const & edge_equiv)
        : g{g},
          h{h},
          callback{callback},
          vertex_equiv{vertex_equiv},
          edge_equiv{edge_equiv},

          m{g.num_vertices()},
          n{h.num_vertices()},
          level{0},
          index_order_g(m),
          map(m, n),
          num_candidates(m, 0),
          M(m * n, false),
          M_mst(m*n, m) {
      std::iota(index_order_g.begin(), index_order_g.end(), 0);
      build_M();
      refine();
    }

    bool explore() {
      SICS_STATS_STATE;
      if (level == m) {
        return callback();
      } else {
        auto it = std::min_element(
            std::next(index_order_g.begin(), level),
            index_order_g.end(),
            [this](auto a, auto b) {
              return num_candidates[a] < num_candidates[b];
            });
        std::swap(index_order_g[level], *it);
        auto x = index_order_g[level];
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
          if (M_get(x, y)) {
            M_mst.push_level();
            if (forward_check(y) &&
                (level > 0 || refine())) {
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

        if constexpr (is_directed_v<G>) {
          bool x_out = g.edge(x, u);
          bool x_in = g.edge(u, x);
          for (IndexH v=0; v<n; ++v) {
            if (M_get(u, v)) {
              if (v == y ||
                  (x_out != h.edge(y, v) || (x_out && !edge_equiv(g, x, u, h, y, v))) ||
                  (x_in != h.edge(v, y) || (x_in && !edge_equiv(g, u, x, h, v, y)))) {
                M_unset(u, v);
                --num_candidates[u];
                M_mst.push({u, v});
              } else {
                not_empty = true;
              }
            }
          }
        } else {
          bool x_edge = g.edge(x, u);
          for (IndexH v=0; v<n; ++v) {
            if (M_get(u, v)) {
              if (v == y ||
                  (x_edge != h.edge(y, v) || (x_edge && !edge_equiv(g, x, u, h, y, v)))) {
                M_unset(u, v);
                --num_candidates[u];
                M_mst.push({u, v});
              } else {
                not_empty = true;
              }
            }
          }
        }
      }
      return not_empty;
    }

    bool refine() {
      bool change = true;
      while (change) {
        change = false;
        for (IndexG i=level+1; i<m; ++i) {
          auto u0 = index_order_g[i];
          for (IndexH v0=0; v0<n; ++v0) {
            if (M_get(u0, v0)) {
              for (auto he : edges_or_out_edges(g, u0)) {
                auto u1 = he.target;
                auto hfs = edges_or_out_edges(h, v0);
                if (!std::any_of(
                        std::cbegin(hfs),
                        std::cend(hfs),
                        [this, u1](auto hf) {
                          return M_get(u1, hf.target);
                        })) {
                  M_unset(u0, v0);
                  --num_candidates[u0];
                  M_mst.push({u0, v0});
                  if (num_candidates[u0] == 0) {
                    return false;
                  }
                  change = true;
                  goto continue_v0;
                }
              }
              if constexpr (is_directed_v<G>) {
                for (auto ie : g.in_edges(u0)) {
                  auto u1 = ie.target;
                  auto hfs = h.in_edges(v0);
                  if (!std::any_of(
                          std::cbegin(hfs),
                          std::cend(hfs),
                          [this, u1](auto hf) {
                            return M_get(u1, hf.target);
                          })) {
                    M_unset(u0, v0);
                    --num_candidates[u0];
                    M_mst.push({u0, v0});
                    if (num_candidates[u0] == 0) {
                      return false;
                    }
                    change = true;
                    goto continue_v0;
                  }
                }
              }
            }
            continue_v0: ;
          }
        }
      }
      return true;
    }

    void revert_M() {
      while (!M_mst.level_empty()) {
        auto [u, v] = M_mst.top();
        M_mst.pop();
        M_set(u, v);
        ++num_candidates[u];
      }
    }
  } e(g, h, callback, vertex_equiv, edge_equiv);

  e.explore();
}

#endif  // SICS_FORWARDCHECKING_MRV_DEGREEPRUNE_REFINE_IND_H_
