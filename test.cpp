#include <cstdint>

#include <fstream>
#include <iostream>

#include "include/read_amalfi.h"

#include "include/graph_traits.h"
#include "include/adjacency_list.h"
#include "include/adjacency_listmat.h"

int main(int argc, char * argv[]) {
  char const * g_filename = argv[1];

  std::ifstream in{g_filename,std::ios::in|std::ios::binary};
  auto g_ = read_amalfi<adjacency_list<uint16_t, undirected_tag>>(in);
  in.close();

  adjacency_listmat<decltype(g_)::index_type, undirected_tag> g{g_};

  std::cout << g.degree(3) << std::endl;

  for (auto oe : g.edges(69)) {
    std::cout << oe.target << " ";
  }
  std::cout << std::endl;

  std::cout << g_.edge(89, 69) << " " << g.edge(89, 69) << std::endl;
}
