#ifndef SIMPLE_ADJACENCY_LIST_H_
#define SIMPLE_ADJACENCY_LIST_H_

#include <vector>

#include <boost/range/iterator_range.hpp>

template <typename Index>
class simple_adjacency_list {
 public:
  using index_type = Index;
  using adjacent_vertices_container_type = std::vector<index_type>;
  
 private:
  std::vector<std::vector<index_type>> out;
  
 public:
  explicit simple_adjacency_list(index_type n)
      : out(n) {
  }
  
  /*
  template <typename G>
  explicit simple_adjacency_list(G const & g)
      : simple_adjacency_list(g.num_vertices()) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto v : g.adjacent_vertices(u)) {
        out[u].push_back(v);
      }
    }
  }
  */
  
  void add_edge(index_type u, index_type v) {
    out[u].push_back(v);
  }
  
  index_type num_vertices() const {
    return out.size();
  }
  
  index_type out_degree(index_type u) const {
    return out[u].size();
  }
  
  auto adjacent_vertices(index_type u) const {
    return boost::make_iterator_range(out[u].cbegin(), out[u].cend());
  }
  
  void print() {
    for (index_type u=0; u<out.size(); ++u) {
      std::cout << u << ":";
      for (auto v : out[u]) {
        std::cout << " " << v;
      }
      std::cout << std::endl;
    }
  } 
};

#endif  // SIMPLE_ADJACENCY_LIST_H_
