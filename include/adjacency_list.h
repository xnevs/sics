#ifndef GMCS_ADJACENCY_LIST_H_
#define GMCS_ADJACENCY_LIST_H_

#include <algorithm>
#include <vector>
#include <optional>

#include <boost/range/iterator_range.hpp>

#include "graph_traits.h"
#include "graph_common.h"

template <
    typename Index,
    typename DirectedCategory,
    typename VertexLabel = void,
    typename EdgeLabel = void>
class adjacency_list {
  template <typename T>
  struct always_false {
    static constexpr bool value = false;
  };

  static_assert(always_false<void>::value, "Supplied template parameters not supported.");
};

// TODO implement EdgeLabel

// Vertex labels

template <
    typename Index,
    typename VertexLabel>
class adjacency_list<Index, bidirectional_tag, VertexLabel, void>
    : public adjacency_list<Index, directed_tag, VertexLabel, void> {
 private:
  using base = adjacency_list<Index, directed_tag, VertexLabel, void>;

 public:
  using typename base::index_type;
  using directed_category = bidirectional_tag;
  using typename base::vertex_label_type;
  using typename base::edge_label_type;

  using typename base::half_edge_type;
  using typename base::half_edges_container_type;

 protected:
  using base::m_vertex_labels;
  using base::m_out_edges;

  std::vector<half_edges_container_type> m_in_edges;

 public:
  explicit adjacency_list(index_type n)
      : base(n),
        m_in_edges(n) {
  }

  template <typename G>
  explicit adjacency_list(G const & g)
      : base(g),
        m_in_edges(g.num_vertices()) {
    auto n = num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto oe : m_out_edges[u]) {
        m_in_edges[oe.target].emplace_back(u);
      }
    }
  }

  using base::num_vertices;
  using base::set_vertex_label;
  using base::get_vertex_label;

  void add_edge(index_type u, index_type v) {
    base::add_edge(u, v);
    m_in_edges[v].emplace_back(u);
  }

  using base::out_degree;
  using base::out_edges;
  using base::edge;

  index_type in_degree(index_type u) const {
    return m_in_edges[u].size();
  }

  index_type degree(index_type u) const {
    return out_degree(u) + in_degree(u);
  }

  auto in_edges(index_type u) const {
    return boost::make_iterator_range(m_in_edges[u].cbegin(), m_in_edges[u].cend());
  }
};

template <
    typename Index,
    typename VertexLabel>
class adjacency_list<Index, directed_tag, VertexLabel, void> {
 public:
  using index_type = Index;
  using directed_category = directed_tag;
  using vertex_label_type = VertexLabel;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 protected:
  std::vector<vertex_label_type> m_vertex_labels;
  std::vector<half_edges_container_type> m_out_edges;

 public:
  explicit adjacency_list(index_type n)
      : m_vertex_labels(n),
        m_out_edges(n) {
  }

  template <typename G>
  explicit adjacency_list(G const & g)
      : m_vertex_labels(g.num_vertices()),
        m_out_edges(g.num_vertices) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      m_vertex_labels[u] = g.get_vertex_label(u);
      for (auto oe : g.out_edges(u)) {
        m_out_edges[u].emplace_back(oe.target);
      }
    }
  }

  index_type num_vertices() const {
    return m_vertex_labels.size();
  }

  void set_vertex_label(index_type u, vertex_label_type label) {
    m_vertex_labels[u] = label;
  }

  vertex_label_type get_vertex_label(index_type u) const {
    return m_vertex_labels[u];
  }

  void add_edge(index_type u, index_type v) {
    m_out_edges[u].emplace_back(v);
  }

  index_type out_degree(index_type u) const {
    return m_out_edges[u].size();
  }

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(m_out_edges[u].cbegin(), m_out_edges[u].cend());
  }

  bool edge(index_type u, index_type v) const {
    auto it = std::find_if(
        std::cbegin(m_out_edges[u]),
        std::cend(m_out_edges[u]),
        [v](auto const & oe) {
          return oe.target == v;
        });
    return it != std::cend(m_out_edges[u]);
  }
};

template <
    typename Index,
    typename VertexLabel>
class adjacency_list<Index, undirected_tag, VertexLabel, void> {
 public:
  using index_type = Index;
  using directed_category = undirected_tag;
  using vertex_label_type = VertexLabel;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 protected:
  std::vector<vertex_label_type> m_vertex_labels;
  std::vector<half_edges_container_type> m_edges;

 public:
  explicit adjacency_list(index_type n)
      : m_vertex_labels(n),
        m_edges(n) {
  }

  template <typename G>
  explicit adjacency_list(G const & g)
      : m_vertex_labels(g.num_vertices()),
        m_edges(g.num_vertices) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      m_vertex_labels[u] = g.get_vertex_label(u);
      for (auto oe : g.out_edges(u)) {
        m_edges[u].emplace_back(oe.target);
        m_edges[oe.target].emplace_back(u);
      }
    }
  }

  index_type num_vertices() const {
    return m_vertex_labels.size();
  }

  void set_vertex_label(index_type u, vertex_label_type label) {
    m_vertex_labels[u] = label;
  }

  vertex_label_type get_vertex_label(index_type u) const {
    return m_vertex_labels[u];
  }

  void add_edge(index_type u, index_type v) {
    m_edges[u].emplace_back(v);
    m_edges[v].emplace_back(u);
  }

  index_type degree(index_type u) const {
    return m_edges[u].size();
  }

  auto edges(index_type u) const {
    return boost::make_iterator_range(m_edges[u].cbegin(), m_edges[u].cend());
  }

  bool edge(index_type u, index_type v) const {
    auto it = std::find_if(
        std::cbegin(m_edges[u]),
        std::cend(m_edges[u]),
        [v](auto const & oe) {
          return oe.target == v;
        });
    return it != std::cend(m_edges[u]);
  }
};



// No labels.

template <typename Index>
class adjacency_list<Index, bidirectional_tag, void, void>
    : public adjacency_list<Index, directed_tag, void, void> {
 private:
  using base = adjacency_list<Index, directed_tag, void, void>;

 public:
  using typename base::index_type;
  using directed_category = bidirectional_tag;
  using typename base::vertex_label_type;
  using typename base::edge_label_type;

  using typename base::half_edge_type;
  using typename base::half_edges_container_type;

 protected:
  using base::m_out_edges;

  std::vector<half_edges_container_type> m_in_edges;

 public:
  explicit adjacency_list(index_type n)
      : base(n),
        m_in_edges(n) {
  }

  template <typename G>
  explicit adjacency_list(G const & g)
      : base(g),
        m_in_edges(g.num_vertices()) {
    auto n = num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto oe : m_out_edges[u]) {
        m_in_edges[oe.target].emplace_back(u);
      }
    }
  }

  using base::num_vertices;

  void add_edge(index_type u, index_type v) {
    base::add_edge(u, v);
    m_in_edges[v].emplace_back(u);
  }

  using base::out_degree;
  using base::out_edges;
  using base::edge;

  index_type in_degree(index_type u) const {
    return m_in_edges[u].size();
  }

  index_type degree(index_type u) const {
    return out_degree(u) + in_degree(u);
  }

  auto in_edges(index_type u) const {
    return boost::make_iterator_range(m_in_edges[u].cbegin(), m_in_edges[u].cend());
  }
};

template <typename Index>
class adjacency_list<Index, directed_tag, void, void> {
 public:
  using index_type = Index;
  using directed_category = directed_tag;
  using vertex_label_type = void;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 protected:
  std::vector<half_edges_container_type> m_out_edges;

 public:
  explicit adjacency_list(index_type n)
      : m_out_edges(n) {
  }

  template <typename G>
  explicit adjacency_list(G const & g)
      : m_out_edges(g.num_vertices) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto oe : g.out_edges(u)) {
        m_out_edges[u].emplace_back(oe.target);
      }
    }
  }

  index_type num_vertices() const {
    return m_out_edges.size();
  }

  void add_edge(index_type u, index_type v) {
    m_out_edges[u].emplace_back(v);
  }

  index_type out_degree(index_type u) const {
    return m_out_edges[u].size();
  }

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(m_out_edges[u].cbegin(), m_out_edges[u].cend());
  }

  bool edge(index_type u, index_type v) const {
    auto it = std::find_if(
        std::cbegin(m_out_edges[u]),
        std::cend(m_out_edges[u]),
        [v](auto const & oe) {
          return oe.target == v;
        });
    return it != std::cend(m_out_edges[u]);
  }
};

template <typename Index>
class adjacency_list<Index, undirected_tag, void, void> {
 public:
  using index_type = Index;
  using directed_category = undirected_tag;
  using vertex_label_type = void;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 protected:
  std::vector<half_edges_container_type> m_edges;

 public:
  explicit adjacency_list(index_type n)
      : m_edges(n) {
  }

  template <typename G>
  explicit adjacency_list(G const & g)
      : m_edges(g.num_vertices()) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto oe : g.out_edges(u)) {
        m_edges[u].emplace_back(oe.target);
        m_edges[oe.target].emplace_back(u);
      }
    }
  }

  index_type num_vertices() const {
    return m_edges.size();
  }

  void add_edge(index_type u, index_type v) {
    m_edges[u].emplace_back(v);
    m_edges[v].emplace_back(u);
  }

  index_type degree(index_type u) const {
    return m_edges[u].size();
  }

  auto edges(index_type u) const {
    return boost::make_iterator_range(m_edges[u].cbegin(), m_edges[u].cend());
  }

  bool edge(index_type u, index_type v) const {
    auto it = std::find_if(
        std::cbegin(m_edges[u]),
        std::cend(m_edges[u]),
        [v](auto const & oe) {
          return oe.target == v;
        });
    return it != std::cend(m_edges[u]);
  }
};




/*  OLD (also has implementation of EdgeLabel != void)
template <
    typename Index,
    typename VertexLabel,
    typename EdgeLabel>
class adjacency_list<Index, undirected_tag, VertexLabel, EdgeLabel> {
 public:
  using index_type = Index;
  using vertex_label_type = VertexLabel;
  using edge_label_type = EdgeLabel;

  using half_edge_type = half_edge<index_type, edge_label_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  std::vector<vertex_label_type> vertex_labels;

  std::vector<half_edges_container_type> m_out_edges;

 public:
  template <typename G>
  explicit adjacency_list(G const & g)
      : nodes(g.num_vertices()),
        vertex_labels{g.num_vertices()} {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      vertex_labels[u] = g.get_vertex_label(u);
      for (auto oe : g.out_edges(u)) {
        nodes[u].out.push_back(oe);
        nodes[oe.target].in.emplace_back(u, oe.label);
      }
    }
  }

  index_type num_vertices() const {
    return nodes.size();
  }

  vertex_label_type get_vertex_label(index_type u) const {
    return vertex_labels[u];
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

  // TODO rather than std::optional we should use a sentinel value for no_edge_label
  std::optional<edge_label_type> edge(index_type u, index_type v) const {
    auto it = std::find_if(
        std::cbegin(nodes[u].out),
        std::cend(nodes[u].out),
        [v](auto const & oe) {
          return oe.target == v;
        });
    if (it != std::cend(nodes[u].out)) {
      return {it->label};
    } else {
      return std::nullopt;
    }
  }
};

template <typename Index, typename VertexLabel>
class adjacency_list<Index, VertexLabel, void> {
 public:
  using index_type = Index;
  using vertex_label_type = VertexLabel;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  struct node {
    half_edges_container_type out;
    half_edges_container_type in;
  };
  std::vector<node> nodes;

  std::vector<vertex_label_type> vertex_labels;

 public:
  template <typename G>
  explicit adjacency_list(G const & g)
      : nodes(g.num_vertices()),
        vertex_labels{g.num_vertices()} {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      vertex_labels[u] = g.get_vertex_label(u);
      for (auto oe : g.out_edges(u)) {
        nodes[u].out.push_back(oe);
        nodes[oe.target].in.emplace_back(u);
      }
    }
  }

  index_type num_vertices() const {
    return nodes.size();
  }

  vertex_label_type get_vertex_label(index_type u) const {
    return vertex_labels[u];
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

  bool edge(index_type u, index_type v) const {
    auto it = std::find_if(
        std::cbegin(nodes[u].out),
        std::cend(nodes[u].out),
        [v](auto const & oe) {
          return oe.target == v;
        });
    return it != std::cend(nodes[u].out);
  }
};

template <typename Index>
class adjacency_list<Index, void, void> {
 public:
  using index_type = Index;
  using vertex_label_type = void;
  using edge_label_type = void;

  using half_edge_type = half_edge<index_type>;
  using half_edges_container_type = std::vector<half_edge_type>;

 private:
  struct node {
    half_edges_container_type out;
    half_edges_container_type in;
  };
  std::vector<node> nodes;

 public:
  template <typename G>
  explicit adjacency_list(G const & g)
      : nodes(g.num_vertices()) {
    auto n = g.num_vertices();
    for (index_type u=0; u<n; ++u) {
      for (auto oe : g.out_edges(u)) {
        nodes[u].out.push_back(oe);
        nodes[oe.target].in.emplace_back(u);
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

  auto out_edges(index_type u) const {
    return boost::make_iterator_range(nodes[u].out.cbegin(), nodes[u].out.cend());
  }

  auto in_edges(index_type u) const {
    return boost::make_iterator_range(nodes[u].in.cbegin(), nodes[u].in.cend());
  }

  bool edge(index_type u, index_type v) const {
    auto it = std::find_if(
        std::cbegin(nodes[u].out),
        std::cend(nodes[u].out),
        [v](auto const & oe) {
          return oe.target == v;
        });
    return it != std::cend(nodes[u].out);
  }
};*/

#endif  // GMCS_ADJACENCY_LIST_H_