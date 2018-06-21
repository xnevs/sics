#ifndef GMCS_BACKTRACKING_FORWARDPARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_FORWARDCOUNT_IND_H_
#define GMCS_BACKTRACKING_FORWARDPARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_FORWARDCOUNT_IND_H_

#include <iterator>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_forwardparent_degreeprune_adjacentconsistency_forwardcount_ind(
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
    
    typename G::adjacent_vertices_container_type g_children_buffer;
    using G_adjacent_vertices_range_type = boost::iterator_range<typename G::adjacent_vertices_container_type::const_iterator>;
    std::vector<G_adjacent_vertices_range_type> g_adj_children;
    std::vector<G_adjacent_vertices_range_type> g_inv_adj_children;
    typename H::adjacent_vertices_container_type candidates_buffer;
    using H_adjacent_vertices_range_type = boost::iterator_range<typename H::adjacent_vertices_container_type::const_iterator>;
    std::vector<H_adjacent_vertices_range_type> candidates;
    void build_children_and_candidates() {
      std::vector<bool> has_parent(m, false);
      
      using g_children_buffer_size_type = typename decltype(g_children_buffer)::size_type;
      std::vector<std::tuple<IndexG, g_children_buffer_size_type, g_children_buffer_size_type>> g_adj_children_buffer_offsets;
      std::vector<std::tuple<IndexG, g_children_buffer_size_type, g_children_buffer_size_type>> g_inv_adj_children_buffer_offsets;
      using candidates_buffer_size_type = typename decltype(candidates_buffer)::size_type;
      std::vector<std::tuple<IndexG, candidates_buffer_size_type, candidates_buffer_size_type>> candidates_buffer_offsets;
      
      for (auto u : index_order_g) {
        if (!has_parent[u]) {
          auto begin_count = candidates_buffer.size();
          for (IndexH v=0; v<n; ++v) {
            if (vertex_equiv(u, v) &&
                g.out_degree(u) <= h.out_degree(v) &&
                g.in_degree(u) <= h.in_degree(v)) {
              candidates_buffer.push_back(v);
            }
          }
          candidates_buffer_offsets.emplace_back(u, begin_count, candidates_buffer.size());
          has_parent[u] = true;
        }
        {
          auto begin_count = g_children_buffer.size();
          for (auto i : g.adjacent_vertices(u)) {
            if (!has_parent[i]) {
              g_children_buffer.push_back(i);
              has_parent[i] = true;
            }
          }
          g_adj_children_buffer_offsets.emplace_back(u, begin_count, g_children_buffer.size());
        }
        {
          auto begin_count = g_children_buffer.size();
          for (auto i : g.inv_adjacent_vertices(u)) {
            if (!has_parent[i]) {
              g_children_buffer.push_back(i);
              has_parent[i] = true;
            }
          }
          g_inv_adj_children_buffer_offsets.emplace_back(u, begin_count, g_children_buffer.size());
        }
      }
      for (auto const & p : candidates_buffer_offsets) {
        IndexG u;
        candidates_buffer_size_type begin_count;
        candidates_buffer_size_type end_count;
        std::tie(u, begin_count, end_count) = p;
        candidates[u] = boost::make_iterator_range(
            std::next(candidates_buffer.cbegin(), begin_count),
            std::next(candidates_buffer.cbegin(), end_count));
      }
      for (auto const & p : g_adj_children_buffer_offsets) {
        IndexG u;
        g_children_buffer_size_type begin_count;
        g_children_buffer_size_type end_count;
        std::tie(u, begin_count, end_count) = p;
        g_adj_children[u] = boost::make_iterator_range(
            std::next(g_children_buffer.cbegin(), begin_count),
            std::next(g_children_buffer.cbegin(), end_count));
      }
      for (auto const & p : g_inv_adj_children_buffer_offsets) {
        IndexG u;
        g_children_buffer_size_type begin_count;
        g_children_buffer_size_type end_count;
        std::tie(u, begin_count, end_count) = p;
        g_inv_adj_children[u] = boost::make_iterator_range(
            std::next(g_children_buffer.cbegin(), begin_count),
            std::next(g_children_buffer.cbegin(), end_count));
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
          g_children_buffer(),
          g_adj_children(m),
          g_inv_adj_children(m),
          candidates_buffer(),
          candidates(m),
          g_out_count(m, 0),
          g_in_count(m, 0),
          h_out_count(n, 0),
          h_in_count(n, 0) {
      build_children_and_candidates();
      build_g_count();
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        bool proceed = true;
        for (auto y : candidates[x]) {
          if (vertex_equiv(x, y) &&
              inv[y] == m &&
              g_out_count[x] == h_out_count[y] &&
              g_in_count[x] == h_in_count[y] &&
              g.out_degree(x) <= h.out_degree(y) &&
              g.in_degree(x) <= h.in_degree(y) &&
              topology_consistency(x, y)) {
            map[x] = y;
            inv[y] = x;
            update_candidates(x, y);
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
    
    void update_candidates(IndexG u, IndexH v) {
      for (auto i : g_adj_children[u]) {
        candidates[i] = h.adjacent_vertices(v);
      }
      for (auto i : g_inv_adj_children[u]) {
        candidates[i] = h.inv_adjacent_vertices(v);
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

#endif  // GMCS_BACKTRACKING_FORWARDPARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_FORWARDCOUNT_IND_H_
