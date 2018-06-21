#ifndef GMCS_ADJACENCY_LIST_H_
#define GMCS_ADJACENCY_LIST_H_

#include <algorithm>
#include <vector>

#include <boost/range/iterator_range.hpp>

template <typename Index>
class adjacency_list {
 public:
  using index_type = Index;
  using adjacent_vertices_container_type = std::vector<index_type>;
  
 private:
  struct node {
    adjacent_vertices_container_type out;
    adjacent_vertices_container_type in;
  };
  std::vector<node> nodes;
  
 public:
  template <typename G>
  explicit adjacency_list(G const & g)
      : nodes(g.num_vertices()) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto v : g.adjacent_vertices(u)) {
        nodes[u].out.push_back(v);
        nodes[v].in.push_back(u);
      }
    }
  }
  
  index_type num_vertices() const {
    return nodes.size();
  }
  
  index_type out_degree(index_type u) const {
    return nodes[u].out.size();
  }
  
  index_type in_degree(index_type u) const {
    return nodes[u].in.size();
  }
  
  index_type degree(index_type u) const {
    return out_degree(u) + in_degree(u);
  }
  
  auto adjacent_vertices(index_type u) const {
    return boost::make_iterator_range(nodes[u].out.cbegin(), nodes[u].out.cend());
  }
  
  auto inv_adjacent_vertices(index_type u) const {
    return boost::make_iterator_range(nodes[u].in.cbegin(), nodes[u].in.cend());
  }
  
  bool edge(index_type u, index_type v) const {
    return std::find(
        std::cbegin(nodes[u].out),
        std::cend(nodes[u].out),
        v) != std::cend(nodes[u].out);
  }
};

#endif  // GMCS_ADJACENCY_LIST_H_
