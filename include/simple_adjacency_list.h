#ifndef GMCS_SIMPLE_ADJACENCY_LIST_H_
#define GMCS_SIMPLE_ADJACENCY_LIST_H_

/* OLD
#include <vector>

#include <boost/range/iterator_range.hpp>

#include "graph_traits.h"
#include "graph_common.h"

template <
    typename Index,
    typename DirectedCategory,
    typename VertexLabel=void,
    typename EdgeLabel=void>
class simple_adjacency_list {
  static_assert(false, "DirectedCategory not supported.");
}

template <
    typename Index,
    typename DirectedCategory,
    typename VertexLabel=void,
    typename EdgeLabel=void>
class simple_adjacency_list {
 public:
  using index_type = Index;
  using directed_category = DirectedCategory;
  using vertex_label_type = VertexLabel;
  using edge_label_type = EdgeLabel;

  using half_edge_type = half_edge<index_type, edge_label_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  std::vector<vertex_label_type> vertex_labels;
  std::vector<half_edges_container_type> m_edges;

 public:
  explicit simple_adjacency_list(index_type n)
      : out(n) {
  }

  vertex_label_type get_vertex_label(index_type u) const {
    return vertex_labels[u];
  }

  void set_vertex_label(index_type u, vertex_label_type const & label) {
    vertex_labels[u] = label;
  }

  void add_edge(index_type u, index_type v, edge_label_type label) {
    out[u].emplace_back(v, label);
  }

  index_type num_vertices() const {
    return out.size();
  }

  index_type out_degree(index_type u) const {
    return out[u].size();
  }

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(out[u].cbegin(), out[u].cend());
  }
};

template <
    typename Index,
    typename DirectedCategory,
    typename VertexLabel>
class simple_adjacency_list<Index, DirectedCategory, VertexLabel, void> {
 public:
  using index_type = Index;
  using directed_category = DirectedCategory;
  using vertex_label_type = VertexLabel;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  std::vector<half_edges_container_type> out;
  std::vector<vertex_label_type> vertex_labels;

 public:
  explicit simple_adjacency_list(index_type n)
      : out(n) {
  }

  vertex_label_type get_vertex_label(index_type u) const {
    return vertex_labels[u];
  }

  void set_vertex_label(index_type u, vertex_label_type const & label) {
    vertex_labels[u] = label;
  }

  void add_edge(index_type u, index_type v) {
    out[u].emplace_back(v);
  }

  index_type num_vertices() const {
    return out.size();
  }

  index_type out_degree(index_type u) const {
    return out[u].size();
  }

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(out[u].cbegin(), out[u].cend());
  }
};

template <
    typename Index,
    typename DirectedCategory>
class simple_adjacency_list<Index, DirectedCategory, void, void> {
 public:
  using index_type = Index;
  using directed_category = DirectedCategory;
  using vertex_label_type = void;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  std::vector<half_edges_container_type> out;

 public:
  explicit simple_adjacency_list(index_type n)
      : out(n) {
  }

  void add_edge(index_type u, index_type v) {
    out[u].emplace_back(v);
  }

  index_type num_vertices() const {
    return out.size();
  }

  index_type out_degree(index_type u) const {
    return out[u].size();
  }

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(out[u].cbegin(), out[u].cend());
  }

  void print() {
    for (index_type u=0; u<out.size(); ++u) {
      std::cout << u << ":";
      for (auto oe : out[u]) {
        std::cout << " " << oe.target;
      }
      std::cout << std::endl;
    }
  }
};*/

#endif  // GMCS_SIMPLE_ADJACENCY_LIST_H_
