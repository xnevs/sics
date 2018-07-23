#ifndef SICS_BACKTRACKING_PARENT_DEGREEPRUNE_ORDEREDADJACENTCONSISTENCY_FORWARDCOUNT_IND_H_
#define SICS_BACKTRACKING_PARENT_DEGREEPRUNE_ORDEREDADJACENTCONSISTENCY_FORWARDCOUNT_IND_H_

#include <iterator>
#include <vector>

namespace sics {

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_parent_degreeprune_orderedadjacentconsistency_forwardcount_ind(
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

    typename H::adjacent_vertices_container_type h_vertices;
    using adjacent_vertices_range_type = boost::iterator_range<typename H::adjacent_vertices_container_type::const_iterator>;
    adjacent_vertices_range_type h_vertices_range;
    std::vector<std::pair<IndexG, bool>> parents;
    void build_parents() {
      std::vector<bool> done(m, false);
      auto end = std::prev(std::cend(index_order_g));
      for (auto it=std::cbegin(index_order_g); it!=end; ++it) {
        auto u = *it;
        done[u] = true;
        for (auto i : g.adjacent_vertices(u)) {
          if (parents[i].first == m && !done[i]) {
            parents[i] = {u, true};
          }
        }
        for (auto i : g.inv_adjacent_vertices(u)) {
          if (parents[i].first == m && !done[i]) {
            parents[i] = {u, false};
          }
        }
      }
    }

    std::vector<IndexG> g_out_count;
    std::vector<IndexG> g_in_count;
    std::vector<IndexH> h_out_count;
    std::vector<IndexH> h_in_count;
    void build_g_count() {
      std::vector<IndexG> index_pos_g(m);
      for (IndexG i=0; i<m; ++i) {
        index_pos_g[index_order_g[i]] = i;
      }
      for (IndexG u=0; u<m; ++u) {
        for (auto i : g.adjacent_vertices(u)) {
          if (index_pos_g[u] < index_pos_g[i]) {
            ++g_in_count[i];
          } else {
            ++g_out_count[u];
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
          x_it(std::cbegin(index_order_g)),
          map(m, n),
          inv(n, m),
          h_vertices(n),
          h_vertices_range(std::cbegin(h_vertices), std::cend(h_vertices)),
          parents(m, {m, false}),
          g_out_count(m, 0),
          g_in_count(m, 0),
          h_out_count(n, 0),
          h_in_count(n, 0) {
      std::iota(std::begin(h_vertices), std::end(h_vertices), 0);
      build_parents();
      build_g_count();
    }

    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        bool proceed = true;
        for (auto y : get_candidates(x)) {
          if (vertex_equiv(x, y) &&
              inv[y] == m &&
              g_out_count[x] == h_out_count[y] &&
              g_in_count[x] == h_in_count[y] &&
              g.out_degree(x) <= h.out_degree(y) &&
              g.in_degree(x) <= h.in_degree(y) &&
              topology_consistency(x, y)) {
            map[x] = y;
            inv[y] = x;
            update_h_count(y);
            ++x_it;
            proceed = explore();
            --x_it;
            revert_h_count(y);
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

    adjacent_vertices_range_type get_candidates(IndexG u) {
      IndexG parent;
      bool out;
      std::tie(parent, out) = parents[u];
      if (parent != m) {
        if (out) {
          return h.adjacent_vertices(map[parent]);
        } else {
          return h.inv_adjacent_vertices(map[parent]);
        }
      } else {
        return h_vertices_range;
      }
    }

    bool topology_consistency(IndexG u, IndexH v) {
      for (auto i : g.adjacent_vertices_before(u)) {
        auto j = map[i];
        if (j != n) {
          if (!h.edge(v, j) || !edge_equiv(u, i, v, j)) {
            return false;
          }
        }
      }
      for (auto i : g.inv_adjacent_vertices_before(u)) {
        auto j = map[i];
        if (j != n) {
          if (!h.edge(j, v) || !edge_equiv(i, u, j, v)) {
            return false;
          }
        }
      }
      return true;
    }

    void update_h_count(IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        ++h_in_count[j];
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        ++h_out_count[j];
      }
    }

    void revert_h_count(IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        --h_in_count[j];
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        --h_out_count[j];
      }
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);

  e.explore();
}

}  // namespace sics

#endif  // SICS_BACKTRACKING_PARENT_DEGREEPRUNE_ORDEREDADJACENTCONSISTENCY_FORWARDCOUNT_IND_H_
