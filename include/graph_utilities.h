#ifndef GMCS_GRAPH_UTILITIES_H_
#define GMCS_GRAPH_UTILITIES_H_

#include "graph_traits.h"

template <typename G>
auto edges_or_out_edges(G const & g, typename G::index_type u) {
  if constexpr (is_directed_v<G>) {
    return g.out_edges(u);
  } else {
    return g.edges(u);
  }
}

#endif  // GMCS_GRAPH_UTILITIES_H_
