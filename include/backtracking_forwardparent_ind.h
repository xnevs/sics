#ifndef GMCS_BACKTRACKING_FORWARDPARENT_IND_H_
#define GMCS_BACKTRACKING_FORWARDPARENT_IND_H_

#include <iterator>
#include <numeric>
#include <utility>
#include <tuple>
#include <vector>

#include <boost/range/iterator_range.hpp>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_forwardparent_ind(
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
          g_children_buffer(),
          g_adj_children(m),
          g_inv_adj_children(m),
          candidates_buffer(),
          candidates(m) {
      build_children_and_candidates();
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        
        bool proceed = true;
        for (auto y : candidates[x]) {
          if (vertex_equiv(x, y) &&
              consistency(y)) {
            map[x] = y;
            update_candidates(x, y);
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
    
    void update_candidates(IndexG u, IndexH v) {
      for (auto i : g_adj_children[u]) {
        candidates[i] = h.adjacent_vertices(v);
      }
      for (auto i : g_inv_adj_children[u]) {
        candidates[i] = h.inv_adjacent_vertices(v);
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

#endif  // GMCS_BACKTRACKING_FORWARDPARENT_IND_H_
