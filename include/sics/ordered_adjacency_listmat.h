#ifndef SICS_ORDRED_ADJACENCY_LISTMAT_H_
#define SICS_ORDRED_ADJACENCY_LISTMAT_H_

/*
 * OBSOLETE
 */

#include <vector>

#include <boost/range/iterator_range.hpp>

template <typename Index>
class ordered_adjacency_listmat {
 public:
  using index_type = Index;

 private:
  index_type n;

  struct node {
    std::vector<index_type> out;
    std::vector<index_type> in;
    typename decltype(out)::const_iterator out_mid;
    typename decltype(in)::const_iterator in_mid;
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
  template <
      typename G,
      typename IndexOrder>
  ordered_adjacency_listmat(
      G const & g,
      IndexOrder const & index_order)
      : n{g.num_vertices()},
        nodes(n),
        mat(n * n) {
    for (index_type u=0; u<n; ++u) {
      for (auto v : g.adjacent_vertices(u)) {
        nodes[u].out.push_back(v);
        nodes[v].in.push_back(u);
        set(u, v);
      }
    }
    std::vector<index_type> index_pos(n);
    for (index_type i=0; i<n; ++i) {
      index_pos[index_order[i]] = i;
    }
    for (index_type u=0; u<n; ++u) {
      nodes[u].out_mid = std::partition(
          std::begin(nodes[u].out),
          std::end(nodes[u].out),
          [&index_pos, u](auto a) {
            return index_pos[a] < index_pos[u];
          });
      nodes[u].in_mid = std::partition(
          std::begin(nodes[u].in),
          std::end(nodes[u].in),
          [&index_pos, u](auto a) {
            return index_pos[a] < index_pos[u];
          });
    }
  }

  index_type num_vertices() const {
    return n;
  }

  bool edge(index_type u, index_type v) const {
    return get(u, v);
  }

  index_type out_degree_before(index_type u) const {
    return adjacent_vertices_before(u).size();
  }
  index_type in_degree_before(index_type u) const {
    return inv_adjacent_vertices_before(u).size();
  }
  index_type degree_before(index_type u) const {
    return out_degree_before(u) + in_degree_before(u);
  }

  index_type out_degree_after(index_type u) const {
    return adjacent_vertices_after(u).size();
  }
  index_type in_degree_after(index_type u) const {
    return inv_adjacent_vertices_after(u).size();
  }
  index_type degree_after(index_type u) const {
    return out_degree_after(u) + in_degree_after(u);
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
    return boost::make_iterator_range(
        std::begin(nodes[u].out),
        std::end(nodes[u].out));
  }

  auto adjacent_vertices_before(index_type u) const {
    return boost::make_iterator_range(
        std::begin(nodes[u].out),
        nodes[u].out_mid);
  }

  auto adjacent_vertices_after(index_type u) const {
    return boost::make_iterator_range(
        nodes[u].out_mid,
        std::end(nodes[u].out));
  }

  auto inv_adjacent_vertices(index_type u) const {
    return boost::make_iterator_range(
        std::begin(nodes[u].in),
        std::end(nodes[u].in));
  }

  auto inv_adjacent_vertices_before(index_type u) const {
    return boost::make_iterator_range(
        std::begin(nodes[u].in),
        nodes[u].in_mid);
  }

  auto inv_adjacent_vertices_after(index_type u) const {
    return boost::make_iterator_range(
        nodes[u].in_mid,
        std::end(nodes[u].in));
  }
};

#endif  // SICS_ORDERED_ADJACENCY_LISTMAT_H_
