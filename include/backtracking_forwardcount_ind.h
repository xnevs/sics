#ifndef GMCS_BACKTRACKING_FORWARDCOUNT_IND_H_
#define GMCS_BACKTRACKING_FORWARDCOUNT_IND_H_

#include <iterator>
#include <vector>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback>
void backtracking_forwardcount_ind(
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
          g_out_count(m, 0),
          g_in_count(m, 0),
          h_out_count(n, 0),
          h_in_count(n, 0) {
      build_g_count();
    }
    
    bool explore() {
      if (x_it == std::cend(index_order_g)) {
        return callback();
      } else {
        auto x = *x_it;
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
          if (vertex_equiv(x, y) &&
              g_out_count[x] == h_out_count[y] &&
              g_in_count[x] == h_in_count[y] &&
              consistency(y)) {
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

#endif  // GMCS_BACKTRACKING_FORWARDCOUNT_IND_H_
