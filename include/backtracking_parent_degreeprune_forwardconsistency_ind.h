#ifndef GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_FORWARDCONSISTENCY_IND_H_
#define GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_FORWARDCONSISTENCY_IND_H_

#include <iterator>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_parent_degreeprune_forwardconsistency_ind(
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

    std::vector<std::vector<IndexG>> g_out;
    std::vector<std::vector<IndexG>> g_in;
    void build_g_out_in() {
      std::vector<bool> done(m, false);
      for (auto u : index_order_g) {
        for (auto i : g.adjacent_vertices(u)) {
          if (!done[i]) {
            g_in[i].push_back(u);
          }
        }
        for (auto i : g.inv_adjacent_vertices(u)) {
          if (!done[i]) {
            g_out[i].push_back(u);
          }
        }
        done[u] = true;
      }
    }
    std::vector<std::vector<IndexG>> h_out;
    std::vector<std::vector<IndexG>> h_in;

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
          g_out(m),
          g_in(m),
          h_out(n),
          h_in(n) {
      std::iota(std::begin(h_vertices), std::end(h_vertices), 0);
      build_parents();
      build_g_out_in();
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
              g.out_degree(x) <= h.out_degree(y) &&
              g.in_degree(x) <= h.in_degree(y) &&
              g_out[x] == h_out[y] &&
              g_in[x] == h_in[y]) {
            map[x] = y;
            inv[y] = x;
            update_h_out_in(x, y);
            ++x_it;
            proceed = explore();
            --x_it;
            revert_h_out_in(x, y);
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

    void update_h_out_in(IndexG u, IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        if (inv[j] == m) {
          h_in[j].push_back(u);
        }
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        if (inv[j] == m) {
          h_out[j].push_back(u);
        }
      }
    }

    void revert_h_out_in(IndexG u, IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        if (inv[j] == m) {
          h_in[j].pop_back();
        }
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        if (inv[j] == m) {
          h_out[j].pop_back();
        }
      }
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);

  e.explore();
}

#endif  // GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_FORWARDCONSISTENCY_IND_H_
