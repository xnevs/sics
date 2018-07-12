#ifndef GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_COLOR_IND_H_
#define GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_COLOR_IND_H_

#include <cstdint>

#include <iterator>
#include <algorithm>
#include <vector>

#include <boost/range/iterator_range.hpp>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_parent_degreeprune_adjacentconsistency_color_ind(
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

    using color_type = IndexG;
    color_type num_colors = 2;
    std::vector<color_type> g_color;
    void build_g_color() {
      std::vector<IndexG> adj_colors(num_colors, 0);
      for (IndexG u=0; u<m; ++u) {
        for (auto i : g.adjacent_vertices(u)) {
          auto i_color = g_color[i];
          if (i_color != num_colors) {
            ++adj_colors[i_color];
          }
        }
        for (auto i : g.inv_adjacent_vertices(u)) {
          auto i_color = g_color[i];
          if (i_color != num_colors) {
            ++adj_colors[i_color];
          }
        }
        g_color[u] = 0;
        for (color_type color=1; color < num_colors; ++color) {
          if (adj_colors[color] < adj_colors[g_color[u]]) {
            g_color[u] = color;
          }
        }
      }
    }

    std::vector<IndexG> g_out_color_count;
    std::vector<IndexG> g_in_color_count;
    void build_g_color_count() {
      std::vector<IndexG> index_pos_g(m);
      for (IndexG i=0; i<m; ++i) {
        index_pos_g[index_order_g[i]] = i;
      }
      for (IndexG u=0; u<m; ++u) {
        for (auto i : g.adjacent_vertices(u)) {
          if (index_pos_g[u] < index_pos_g[i]) {
            ++g_in_color_count[i*num_colors + g_color[u]];
          } else {
            ++g_out_color_count[u*num_colors + g_color[i]];
          }
        }
      }
    }

    std::vector<IndexH> h_out_color_count;
    std::vector<IndexH> h_in_color_count;

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
          g_color(m, num_colors),
          g_out_color_count(m * num_colors, 0),
          g_in_color_count(m * num_colors, 0),
          h_out_color_count(n * num_colors, 0),
          h_in_color_count(n * num_colors, 0) {
      std::iota(std::begin(h_vertices), std::end(h_vertices), 0);
      build_parents();
      build_g_color();
      build_g_color_count();
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
              memcmp(&g_out_color_count[x*num_colors], &h_out_color_count[y*num_colors], num_colors) == 0 &&
              memcmp(&g_in_color_count[x*num_colors], &h_in_color_count[y*num_colors], num_colors) == 0 &&
              g.out_degree(x) <= h.out_degree(y) &&
              g.in_degree(x) <= h.in_degree(y) &&
              topology_consistency(x, y)) {
            map[x] = y;
            inv[y] = x;
            update_h_color_count(x, y);
            ++x_it;
            proceed = explore();
            --x_it;
            revert_h_color_count(x, y);
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
      for (auto i : g.adjacent_vertices(u)) {
        auto j = map[i];
        if (j != n) {
          if (!h.edge(v, j) || !edge_equiv(u, i, v, j)) {
            return false;
          }
        }
      }
      for (auto i : g.inv_adjacent_vertices(u)) {
        auto j = map[i];
        if (j != n) {
          if (!h.edge(j, v) || !edge_equiv(i, u, j, v)) {
            return false;
          }
        }
      }
      return true;
    }

    void update_h_color_count(IndexG u, IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        ++h_in_color_count[j*num_colors + g_color[u]];
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        ++h_out_color_count[j*num_colors + g_color[u]];
      }
    }

    void revert_h_color_count(IndexG u, IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        --h_in_color_count[j*num_colors + g_color[u]];
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        --h_out_color_count[j*num_colors + g_color[u]];
      }
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);

  e.explore();
}

#endif  // GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_COLOR_IND_H_
