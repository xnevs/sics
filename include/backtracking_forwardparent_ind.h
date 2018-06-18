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
    std::vector<IndexG> inv;
    
    typename H::adjacent_vertices_container_type candidates_buffer;
    using adjacent_vertices_range_type = boost::iterator_range<typename H::adjacent_vertices_container_type::const_iterator>;
    std::vector<adjacent_vertices_range_type> candidates;
    
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
          candidates_buffer(),
          candidates(m) {
      
      std::vector<IndexG> index_pos_g(m);
      for (IndexG i=0; i<m; ++i) {
        index_pos_g[index_order_g[i]] = i;
      }
      
      using candidates_buffer_size_type = typename decltype(candidates_buffer)::size_type;
      std::vector<std::tuple<IndexG, candidates_buffer_size_type, candidates_buffer_size_type>> candidates_buffer_offsets;
      for (auto u : index_order_g) {
        candidates_buffer_size_type begin_count;
        
        for (auto i : g.adjacent_vertices(u)) {
          if (index_pos_g[i] < index_pos_g[u]) {
            goto continue_outer;
          }
        }
        for (auto i : g.inv_adjacent_vertices(u)) {
          if (index_pos_g[i] < index_pos_g[u]) {
            goto continue_outer;
          }
        }
          
        begin_count = candidates_buffer.size();
        for (IndexH v=0; v<n; ++v) {
          if (vertex_equiv(u, v) &&
              g.out_degree(u) <= h.out_degree(v) &&
              g.in_degree(u) <= h.in_degree(v)) {
            candidates_buffer.push_back(v);
          }
        }
        candidates_buffer_offsets.emplace_back(u, begin_count, candidates_buffer.size());
        
        continue_outer: ;
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
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        
        bool proceed = true;
        for (auto y : candidates[x]) {
          if (inv[y] == m &&             // uniqueness
              vertex_equiv(x, y) &&      // label
              topology_consistency(y)) { // topology
            map[x] = y;
            inv[y] = x;
            update_candidates(x, y);
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
        return proceed;
      }
    }
    
    void update_candidates(IndexG u, IndexH v) {
      for (auto i : g.adjacent_vertices(u)) {
        if (map[i] == n) {
          candidates[i] = h.adjacent_vertices(v);
        }
      }
      for (auto i : g.inv_adjacent_vertices(u)) {
        if (map[i] == n) {
          candidates[i] = h.inv_adjacent_vertices(v);
        }
      }
    }
    
    bool topology_consistency(IndexH y) {
      auto x = *x_it;
      for (auto it=std::cbegin(index_order_g); it!=x_it; ++it) {
        auto u = *it;
        auto v = map[u];
        if (v != n) {
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
      }
      return true;
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);
  
  e.explore();
}

#endif  // GMCS_BACKTRACKING_FORWARDPARENT_IND_H_
