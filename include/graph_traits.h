#ifndef GMCS_GRAPH_TRAITS_H_
#define GMCS_GRAPH_TRAITS_H_

#include <type_traits>

struct directed_category_tag {};
struct directed_tag : public directed_category_tag {};
struct undirected_tag : public directed_category_tag {};
struct bidirectional_tag : public directed_tag {};

template <
    typename DirectedCategoryTag,
    typename G>
struct is_directed_category_of {
  using G_directed_category = typename G::directed_category;
  static_assert(std::is_base_of<directed_category_tag, G_directed_category>::value);
  static constexpr bool value = std::is_base_of<DirectedCategoryTag, G_directed_category>::value;
  constexpr operator bool() {
    return value;
  }
};

template <typename G>
struct is_vertex_labelled {
  using vertex_label_type = typename G::vertex_label_type;
  static constexpr bool value = ! std::is_same<void, vertex_label_type>::value;
  constexpr operator bool() {
    return value;
  }
};

template <typename G>
struct is_edge_labelled {
  using edge_label_type = typename G::edge_label_type;
  static constexpr bool value = ! std::is_same<void, edge_label_type>::value;
  constexpr operator bool() {
    return value;
  }
};

#endif  // GMCS_GRAPH_TRAITS_H_
