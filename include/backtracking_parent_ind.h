#ifndef GMCS_BACKTRACKING_PARENT_IND_H_
#define GMCS_BACKTRACKING_PARENT_IND_H_

#include <iterator>
#include <numeric>
#include <utility>
#include <tuple>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_parent_ind(
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
          parents(m, {m, false}) {
      
      std::iota(std::begin(h_vertices), std::end(h_vertices), 0);
      
      std::vector<IndexG> index_pos_g(m);
      for (IndexG i=0; i<m; ++i) {
        index_pos_g[index_order_g[i]] = i;
      }
      
      for (auto it=std::next(std::cbegin(index_order_g)); it!=std::cend(index_order_g); ++it) {
        auto u = *it;
        for (auto i : g.adjacent_vertices(u)) {
          if (index_pos_g[i] < index_pos_g[u]) {
            parents[u] = {i, false};
          }
        }
        for (auto i : g.inv_adjacent_vertices(u)) {
          if (index_pos_g[i] < index_pos_g[u]) {
            parents[u] = {i, true};
          }
        }
      }
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        
        auto candidates = get_candidates(x);
        
        bool proceed = true;
        for (auto y : candidates) {
          if (inv[y] == m &&             // uniqueness
              vertex_equiv(x, y) &&      // label
              topology_consistency(y)) { // topology
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
        return proceed;
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
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback);
  
  e.explore();
}

#endif  // GMCS_BACKTRACKING_PARENT_IND_H_
