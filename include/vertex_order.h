#ifndef GMCS_VERTEX_ORDER_H_
#define GMCS_VERTEX_ORDER_H_

#include <utility>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <vector>
#include <set>

template <typename G>
std::vector<typename G::index_type> vertex_order_DEG(G const & g) {
  std::vector<typename G::index_type> order(g.num_vertices());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&g](auto i, auto j) {
    return g.degree(i) > g.degree(j);
  });
  return order;
}

template <typename G>
std::vector<typename G::index_type> vertex_order_RDEG(G const & g) {
  using Index = typename G::index_type;
  
  auto n = g.num_vertices();
  
  std::vector<Index> vertex_order(n);
  
  std::vector<bool> avail(n, true);
  for (Index idx=0; idx<n; ++idx) {
    auto bestn = n;
    int bestv = -1;
    for (Index i=0; i<n; ++i) {
      if (avail[i]) {
        auto const & i_adj = g.adjacent_vertices(i);
        auto const & i_inv_adj = g.inv_adjacent_vertices(i);
        int rdeg =
            std::count_if(std::begin(i_adj), std::end(i_adj), [&avail](auto p) {
              return !avail[p];
            })
            +
            std::count_if(std::begin(i_inv_adj), std::end(i_inv_adj), [&avail](auto p) {
              return !avail[p];
            });
        if (bestn == n || (rdeg > bestv)) {
          bestn = i;
          bestv = rdeg;
        }
      }
    }
    avail[bestn] = false;
    vertex_order[idx] = bestn;
  }
  return vertex_order;
}

template <typename G>
typename G::index_type clustering1(typename G::index_type i, G const & g) {
  using Index = typename G::index_type;
  Index c = 0;
  for (auto w : g.adjacent_vertices(i)) {
    auto const & w_adj = g.adjacent_vertices(w);
    c += std::count_if(std::begin(w_adj), std::end(w_adj), [i, &g](auto r) {
      return g.edge(r, i) || g.edge(i, r);
    });
    auto const & w_inv_adj = g.inv_adjacent_vertices(w);
    c += std::count_if(std::begin(w_inv_adj), std::end(w_inv_adj), [i, &g](auto r) {
      return g.edge(r, i) || g.edge(i, r);
    });
  }
  for (auto w : g.inv_adjacent_vertices(i)) {
    auto const & w_adj = g.adjacent_vertices(w);
    c += std::count_if(std::begin(w_adj), std::end(w_adj), [i, &g](auto r) {
      return g.edge(r, i) || g.edge(i, r);
    });
    auto const & w_inv_adj = g.inv_adjacent_vertices(w);
    c += std::count_if(std::begin(w_inv_adj), std::end(w_inv_adj), [i, &g](auto r) {
      return g.edge(r, i) || g.edge(i, r);
    });
  }
  return c;
}

template <typename G>
std::vector<typename G::index_type> vertex_order_RDEG_CNC(G const & g) {
  using Index = typename G::index_type;
  
  auto n = g.num_vertices();
  
  std::vector<Index> vertex_order(n);
  
  std::vector<Index> clustdeg(n);
  for (Index i=0; i<n; ++i) {
    clustdeg[i] = clustering1(i, g) + g.degree(i);
  }
  
  std::vector<bool> avail(n, true);
  for (Index idx=0; idx<n; ++idx) {
    auto bestn = n;
    int bestv = -1;
    for (Index i=0; i<n; ++i) {
      if (avail[i]) {
        auto const & i_adj = g.adjacent_vertices(i);
        auto const & i_inv_adj = g.inv_adjacent_vertices(i);
        int rdeg =
            std::count_if(std::begin(i_adj), std::end(i_adj), [&avail](auto p) {
              return !avail[p];
            })
            +
            std::count_if(std::begin(i_inv_adj), std::end(i_inv_adj), [&avail](auto p) {
              return !avail[p];
            });
        if (bestn == n || (rdeg > bestv || (rdeg == bestv && clustdeg[i] > clustdeg[bestn]))) {
          bestn = i;
          bestv = rdeg;
        }
      }
    }
    avail[bestn] = false;
    vertex_order[idx] = bestn;
  }
  return vertex_order;
}

template <typename G>
std::vector<typename G::index_type> vertex_order_GreatestConstraintFirst(G const & g) {
  using Index = typename G::index_type;
  
  auto n = g.num_vertices();
  
  std::vector<Index> vertex_order(n);
  std::iota(std::begin(vertex_order), std::end(vertex_order), 0);
  
  enum struct Flag {
    vis,
    neigh,
    unv
  };
  
  std::vector<Flag> flags(n, Flag::unv);
  std::vector<std::tuple<Index,Index,Index,Index>> ranks(n);
  
  for (Index i=0; i<n; ++i) {
    std::get<2>(ranks[i]) = g.degree(i);
    std::get<3>(ranks[i]) = i;
  }
  
  for (Index m=0; m<n; ++m) {
    auto u_it = std::max_element(
        std::next(std::begin(vertex_order), m),
        std::end(vertex_order),
        [&ranks](auto u, auto v) {
          return ranks[u] < ranks[v];
        });
    Index u = *u_it;
    
    if (flags[u] == Flag::unv) {
      for (auto v : g.adjacent_vertices(u)) {
        --std::get<2>(ranks[v]);
      }
      for (auto v : g.inv_adjacent_vertices(u)) {
        --std::get<2>(ranks[v]);
      }
    } else if (flags[u] == Flag::neigh) {
      for (auto v : g.adjacent_vertices(u)) {
        --std::get<1>(ranks[v]);
      }
      for (auto v : g.inv_adjacent_vertices(u)) {
        --std::get<1>(ranks[v]);
      }
    }
    
    std::swap(vertex_order[m], *u_it);
    flags[u] = Flag::vis;
    
    for (auto v : g.adjacent_vertices(u)) {
      ++std::get<0>(ranks[v]);
      
      if (flags[v] == Flag::unv) {
        flags[v] = Flag::neigh;
        for (auto w : g.adjacent_vertices(v)) {
          ++std::get<1>(ranks[w]);
        }
        for (auto w : g.inv_adjacent_vertices(v)) {
          ++std::get<1>(ranks[w]);
        }
      }
    }
    for (auto v : g.inv_adjacent_vertices(u)) {
      ++std::get<0>(ranks[v]);
      if (flags[v] == Flag::unv) {
        flags[v] = Flag::neigh;
        for (auto w : g.adjacent_vertices(v)) {
          ++std::get<1>(ranks[w]);
        }
        for (auto w : g.inv_adjacent_vertices(v)) {
          ++std::get<1>(ranks[w]);
        }
      }
    }
  }
  return vertex_order;
}

template <typename G>
std::vector<typename G::index_type> vertex_order_GreatestConstraintKth(G const & g) {
  using Index = typename G::index_type;
  
  auto n = g.num_vertices();
  
  std::vector<Index> vertex_order(n);
  std::iota(vertex_order.begin(), vertex_order.end(), 0);
  
  std::vector<bool> vis(n, false);
  std::vector<Index> vis_count(n, 0);
  std::vector<Index> neigh_count(n, 0);
  std::vector<Index> unv_count(n);
  
  struct Rank {
    Index vis;
    Index neigh;
    Index unv;
    
    Rank(): vis{0}, neigh{0}, unv{0} {}
  };
  std::vector<Rank> ranks(n); 
  for (Index u=0; u<n; ++u) {
    ranks[u].unv = g.degree(u);
  }
  
  struct MaxElement {
    using score_type = std::vector<Index>;
  
    G const & g;
    typename std::vector<Index>::iterator first;
    typename std::vector<Index>::iterator last;
    
    std::vector<Rank> & ranks;
    
    Index level;
    
    typename std::vector<Index>::iterator max_it;
    score_type max_score;
    
    std::set<Index> visited;
    std::vector<typename std::vector<Index>::iterator> working_order;
    
    MaxElement(
        G const & g,
        typename std::vector<Index>::iterator first,
        typename std::vector<Index>::iterator last,
        std::vector<Rank> & ranks,
        Index level)
        : g{g},
          first{first},
          last{last},
          ranks{ranks},
          level{level} {
    }
    
    typename std::vector<Index>::iterator operator()() {
      max_it = last;
      compute(level);
      return max_it;
    }
    
    void compute(Index level) {
      if (level == 0) {
        score_type working_score;
        for (auto v_it : working_order) {
          working_score.push_back(ranks[*v_it].vis);
        }
        working_score.push_back(ranks[*working_order.front()].neigh);
        working_score.push_back(ranks[*working_order.front()].unv);
        if (max_it == last || working_score > max_score) {
          max_it = working_order.front();
          max_score = working_score;
        }
      } else {
        for (auto u_it=first; u_it!=last; ++u_it) {
          if (visited.find(*u_it) == visited.end()) {
            auto u = *u_it;
            
            if (ranks[u].vis == 0 && ranks[u].neigh == 0) {
              for (auto v : g.adjacent_vertices(u)) {
                --ranks[v].unv;
              }
              for (auto v : g.inv_adjacent_vertices(u)) {
                --ranks[v].unv;
              }
            } else if (ranks[u].vis == 0) {
              for (auto v : g.adjacent_vertices(u)) {
                --ranks[v].neigh;
              }
              for (auto v : g.inv_adjacent_vertices(u)) {
                --ranks[v].neigh;
              }
            }
            for (auto v : g.adjacent_vertices(u)) {
              ++ranks[v].vis;
              if (ranks[v].vis == 1) {
                for (auto w : g.adjacent_vertices(v)) {
                  ++ranks[w].neigh;
                }
                for (auto w : g.inv_adjacent_vertices(v)) {
                  ++ranks[w].neigh;
                }
              }
            }
            for (auto v : g.inv_adjacent_vertices(u)) {
              ++ranks[v].vis;
              if (ranks[v].vis == 1) {
                for (auto w : g.adjacent_vertices(v)) {
                  ++ranks[w].neigh;
                }
                for (auto w : g.inv_adjacent_vertices(v)) {
                  ++ranks[w].neigh;
                }
              }
            }
            
            working_order.push_back(u_it);
            visited.insert(*u_it);
            compute(level - 1);
            
            working_order.pop_back();
            visited.erase(*u_it);
            for (auto v : g.adjacent_vertices(u)) {
              --ranks[v].vis;
              if (ranks[v].vis == 0) {
                for (auto w : g.adjacent_vertices(v)) {
                  --ranks[w].neigh;
                }
                for (auto w : g.inv_adjacent_vertices(v)) {
                  --ranks[w].neigh;
                }
              }
            }
            for (auto v : g.inv_adjacent_vertices(u)) {
              --ranks[v].vis;
              if (ranks[v].vis == 0) {
                for (auto w : g.adjacent_vertices(v)) {
                  --ranks[w].neigh;
                }
                for (auto w : g.inv_adjacent_vertices(v)) {
                  --ranks[w].neigh;
                }
              }
            }
            if (ranks[u].vis == 0 && ranks[u].neigh == 0) {
              for (auto v : g.adjacent_vertices(u)) {
                ++ranks[v].unv;
              }
              for (auto v : g.inv_adjacent_vertices(u)) {
                ++ranks[v].unv;
              }
            } else if (ranks[u].vis == 0) {
              for (auto v : g.adjacent_vertices(u)) {
                ++ranks[v].neigh;
              }
              for (auto v : g.inv_adjacent_vertices(u)) {
                ++ranks[v].neigh;
              }
            }
          }
        }
      }
    }
  };
  
  for (Index m=0; m<n; ++m) {
    MaxElement max_element(g, std::next(std::begin(vertex_order), m), std::end(vertex_order), ranks, std::min(1, n-m));
    auto u_it = max_element();
    auto u = *u_it;
    
    if (ranks[u].vis == 0 && ranks[u].neigh == 0) {
      for (auto v : g.adjacent_vertices(u)) {
        --ranks[v].unv;
      }
      for (auto v : g.inv_adjacent_vertices(u)) {
        --ranks[v].unv;
      }
    } else if (ranks[u].vis == 0) {
      for (auto v : g.adjacent_vertices(u)) {
        --ranks[v].neigh;
      }
      for (auto v : g.inv_adjacent_vertices(u)) {
        --ranks[v].neigh;
      }
    }
  
    std::swap(vertex_order[m], *u_it);
    vis[u] = true;
    
    for (auto v : g.adjacent_vertices(u)) {
      ++ranks[v].vis;
      if (ranks[v].vis == 1) {
        for (auto w : g.adjacent_vertices(v)) {
          ++ranks[w].neigh;
        }
        for (auto w : g.inv_adjacent_vertices(v)) {
          ++ranks[w].neigh;
        }
      }
    }
    for (auto v : g.inv_adjacent_vertices(u)) {
      ++ranks[v].vis;
      if (ranks[v].vis == 1) {
        for (auto w : g.adjacent_vertices(v)) {
          ++ranks[w].neigh;
        }
        for (auto w : g.inv_adjacent_vertices(v)) {
          ++ranks[w].neigh;
        }
      }
    }
  }
  return vertex_order;
}

#endif  // GMCS_VERTEX_ORDER_H_
