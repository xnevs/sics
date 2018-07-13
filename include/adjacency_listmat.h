#ifndef GMCS_ADJACENCY_LISTMAT_H_
#define GMCS_ADJACENCY_LISTMAT_H_

#include <vector>

#include <boost/range/iterator_range.hpp>

#include "graph_traits.h"
#include "graph_common.h"

template <
    typename Index,
    typename DirectedCategory,
    typename VertexLabel = void,
    typename EdgeLabel = void>
class adjacency_listmat
    : public adjacency_list<Index, DirectedCategory, VertexLabel, EdgeLabel> {
};

// TODO implement EdgeLabel != void

template <
    typename Index,
    typename DirectedCategory,
    typename VertexLabel>
class adjacency_listmat<Index, DirectedCategory, VertexLabel, void>
    : public adjacency_list<Index, DirectedCategory, VertexLabel, void> {
 private:
  using base = adjacency_list<Index, DirectedCategory, VertexLabel, void>;

 protected:
  typename base::index_type n;
  std::vector<bool> m_mat;

 public:
  template <
      typename G,
      typename Temp = typename base::directed_category,  // needed to make enable_if work
      typename std::enable_if_t<std::is_base_of<directed_tag, Temp>::value, int> = 0>
  explicit adjacency_listmat(G const & g)
      : base(g),
        n{base::num_vertices()},
        m_mat(n * n, false){
    for (typename base::index_type u=0; u<n; ++u) {
      for (auto oe : g.out_edges(u)) {
        m_mat[u*n + oe.target] = true;
      }
    }
  }

  template <
      typename G,
      typename Temp = typename base::directed_category,  // needed to make enable_if work
      typename std::enable_if_t<std::is_base_of<undirected_tag, Temp>::value, int> = 0>
  explicit adjacency_listmat(G const & g)
      : base(g),
        n{base::num_vertices()},
        m_mat((n * (n+1)) / 2, false){
    for (typename base::index_type u=0; u<n; ++u) {
      for (auto e : g.edges(u)) {
        auto v = e.target;
        if (u <= v) {
          m_mat[(u * (u+1)) / 2 + v] = true;
        }
        else {
          m_mat[(v * (v+1)) / 2 + u] = true;
        }
      }
    }
  }

  bool edge(typename base::index_type u, typename base::index_type v) {
    if constexpr (std::is_base_of<undirected_tag, typename base::directed_category>::value) {
      if (u <= v) {
        return m_mat[(u * (u+1)) / 2 + v];
      } else {
        return m_mat[(v * (v+1)) / 2 + u];
      }
    } else { // directed
      return m_mat[u*n + v];
    }
  }
};


/* OLD
template <
    typename Index,
    typename VertexLabel = void,
    typename EdgeLabel = void>
class adjacency_listmat {
  // TODO
};

template <typename Index, typename VertexLabel>
class adjacency_listmat<Index, VertexLabel, void> {
 public:
  using index_type = Index;
  using vertex_label_type = VertexLabel;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  index_type n;

  struct node {
    half_edges_container_type out;
    half_edges_container_type in;
  };
  std::vector<node> nodes;

  std::vector<vertex_label_type> vertex_labels;

  std::vector<bool> mat;

  void set(index_type i, index_type j) {
    mat[i*n + j] = true;
  }

  bool get(index_type i, index_type j) const {
    return mat[i*n + j];
  }

 public:
  template <typename G>
  explicit adjacency_listmat(G const & g)
      : n{g.num_vertices()},
        nodes(n),
        vertex_labels(n),
        mat(n * n) {
    for (index_type u=0; u<n; ++u) {
      vertex_labels[u] = g.get_vertex_label(u);
      for (auto oe : g.out_edges(u)) {
        nodes[u].out.push_back(oe);
        nodes[oe.target].in.emplace_back(u);
        set(u, oe.target);
      }
    }
  }

  index_type num_vertices() const {
    return nodes.size();
  }

  vertex_label_type get_vertex_label(index_type u) const {
    return vertex_labels[u];
  }

  bool edge(index_type u, index_type v) const {
    return get(u, v);
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

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(nodes[u].out.cbegin(), nodes[u].out.cend());
  }

  auto in_edges(index_type u) const {
    return boost::make_iterator_range(nodes[u].in.cbegin(), nodes[u].in.cend());
  }
};

template <typename Index>
class adjacency_listmat<Index, void, void> {
 public:
  using index_type = Index;
  using vertex_label_type = void;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  index_type n;

  struct node {
    half_edges_container_type out;
    half_edges_container_type in;
  };
  std::vector<node> nodes;

  std::vector<bool> mat;

  void set(index_type i, index_type j) {
    mat[i*n + j] = true;
  }

  bool get(index_type i, index_type j) const {
    return mat[i*n + j];
  }

 public:
  template <typename G>
  explicit adjacency_listmat(G const & g)
      : n{g.num_vertices()},
        nodes(n),
        mat(n * n) {
    for (index_type u=0; u<n; ++u) {
      for (auto oe : g.out_edges(u)) {
        nodes[u].out.push_back(oe);
        nodes[oe.target].in.emplace_back(u);
        set(u, oe.target);
      }
    }
  }

  index_type num_vertices() const {
    return nodes.size();
  }

  bool edge(index_type u, index_type v) const {
    return get(u, v);
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

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(nodes[u].out.cbegin(), nodes[u].out.cend());
  }

  auto in_edges(index_type u) const {
    return boost::make_iterator_range(nodes[u].in.cbegin(), nodes[u].in.cend());
  }
};*/

#endif  // GMCS_ADJACENCY_LISTMAT_H_
