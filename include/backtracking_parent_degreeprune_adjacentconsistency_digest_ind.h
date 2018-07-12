#ifndef GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_DIGEST_IND_H_
#define GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_DIGEST_IND_H_

#include <cstdint>

#include <iterator>
#include <algorithm>
#include <vector>
#include <random>

template <
    typename G,
    typename H,
    typename VertexEquiv,
    typename EdgeEquiv,
    typename IndexOrderG,
    typename Callback,
    typename URBG>
void backtracking_parent_degreeprune_adjacentconsistency_digest_ind(
    G const & g,
    H const & h,
    VertexEquiv const & vertex_equiv,
    EdgeEquiv const & edge_equiv,
    IndexOrderG const & index_order_g,
    Callback const & callback,
    URBG && rnd_gen) {
    
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
    
    std::vector<unsigned long> g_hash;
    void build_g_hash(URBG && rnd_gen) {
      std::uniform_int_distribution<unsigned long> dis;
      std::generate(
          std::begin(g_hash),
          std::end(g_hash), 
          [&rnd_gen, &dis]() {
            return dis(rnd_gen);
          });
    }
    
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
    
    std::vector<unsigned long> g_out_digest;
    std::vector<unsigned long> g_in_digest;
    std::vector<unsigned long> h_out_digest;
    std::vector<unsigned long> h_in_digest;
    void build_g_digest() {
      std::vector<IndexG> index_pos_g(m);
      for (IndexG i=0; i<m; ++i) {
        index_pos_g[index_order_g[i]] = i;
      }
      for (IndexG u=0; u<m; ++u) {
        for (auto i : g.adjacent_vertices(u)) {
          if (index_pos_g[u] < index_pos_g[i]) {
            g_in_digest[i] ^= g_hash[u];
          } else {
            g_out_digest[u] ^= g_hash[i];
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
        Callback const & callback,
        URBG && rnd_gen)
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
          g_hash(m),
          h_vertices(n),
          h_vertices_range(std::cbegin(h_vertices), std::cend(h_vertices)),
          parents(m, {m, false}),
          g_out_digest(m, 0),
          g_in_digest(m, 0),
          h_out_digest(n, 0),
          h_in_digest(n, 0) {
      
      std::generate(std::begin(g_hash), std::end(g_hash), rnd_gen);
      std::iota(std::begin(h_vertices), std::end(h_vertices), 0);
      build_parents();
      build_g_digest();
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
              g_out_digest[x] == h_out_digest[y] &&
              g_in_digest[x] == h_in_digest[y] &&
              g.out_degree(x) <= h.out_degree(y) &&
              g.in_degree(x) <= h.in_degree(y) &&
              topology_consistency(x, y)) {
            map[x] = y;
            inv[y] = x;
            update_h_digest(y);
            ++x_it;
            proceed = explore();
            --x_it;
            revert_h_digest(y);
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
      for (auto j : h.adjacent_vertices(v)) {
        auto i = inv[j];
        if (i != m) {
          if (!g.edge(u, i)) {
            return false;
          }
        }
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        auto i = inv[j];
        if (i != m) {
          if (!g.edge(i, u)) {
            return false;
          }
        }
      }
      return true;
    }
    
    void update_h_digest(IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        h_in_digest[j] ^= g_hash[inv[v]];
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        h_out_digest[j] ^= g_hash[inv[v]];
      }
    }
    
    void revert_h_digest(IndexH v) {
      for (auto j : h.adjacent_vertices(v)) {
        h_in_digest[j] ^= g_hash[inv[v]];
      }
      for (auto j : h.inv_adjacent_vertices(v)) {
        h_out_digest[j] ^= g_hash[inv[v]];
      }
    }
  } e(g, h, vertex_equiv, edge_equiv, index_order_g, callback, rnd_gen);
  
  e.explore();
}

#endif  // GMCS_BACKTRACKING_PARENT_DEGREEPRUNE_ADJACENTCONSISTENCY_DIGEST_IND_H_
