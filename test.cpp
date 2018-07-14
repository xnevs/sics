#include <cstdint>

#include <fstream>
#include <iostream>

#include "include/graph_traits.h"
#include "include/adjacency_listmat.h"
#include "include/vertex_order.h"
#include "include/backtracking_ind.h"

int main(int argc, char * argv[]) {

  adjacency_listmat<uint16_t, undirected_tag> g(3);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(2, 1);
  g.add_edge(2, 0);
  adjacency_listmat<uint16_t, undirected_tag> h(4);
  h.add_edge(0, 1);
  h.add_edge(1, 2);
  h.add_edge(2, 0);
  h.add_edge(1, 3);
  h.add_edge(3, 0);
  h.add_edge(0, 3);

  auto index_order_g = vertex_order_DEG(g);

  int count = 0;
  backtracking_ind(
      g,
      h,
      [&count]() {++count; return true;},
      index_order_g);

  std::cout << count << std::endl;
}
