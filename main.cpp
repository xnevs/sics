#include <cstdint>

#include <iostream>

#include "include/simple_adjacency_list.h"
#include "include/adjacency_listmat.h"

int main() {
  simple_adjacency_list<uint16_t> g_(4);
  g_.add_edge(0,1);
  g_.add_edge(1,2);
  g_.add_edge(2,0);
  g_.add_edge(0,3);
  
  adjacency_listmat<uint16_t> g{g_};
  
  for (auto i : g.adjacent_vertices(0)) {
    std::cout << " " << i;
  }
  std::cout << std::endl;
  for (auto i : g.inv_adjacent_vertices(0)) {
    std::cout << " " << i;
  }
  std::cout << std::endl;
}
