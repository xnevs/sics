#ifndef ADJACENCY_SORTEDLIST_H_
#define ADJACENCY_SORTEDLIST_H_

/*
 * OBSOLETE
 */


#include <algorithm>
#include <vector>

#include <boost/range/iterator_range.hpp>

template <typename Index>
class adjacency_sortedlist {
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
  explicit adjacency_sortedlist(G const & g)
      : nodes(g.num_vertices()) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto v : g.adjacent_vertices(u)) {
        nodes[u].out.push_back(v);
        nodes[v].in.push_back(u);
      }
    }
    for (auto & node : nodes) {
      std::sort(std::begin(node.out), std::end(node.out));
      std::sort(std::begin(node.in), std::end(node.in));
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
    return std::binary_search(
        std::cbegin(nodes[u].out),
        std::cend(nodes[u].out),
        v);
  }
};

#endif  // ADJACENCY_SORTEDLIST_H_
