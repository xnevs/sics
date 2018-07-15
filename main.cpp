#include <cstdint>

#include <fstream>
#include <iostream>

#include "include/read_amalfi.h"
#include "include/read_ldgraphs.h"

#include "include/adjacency_listmat.h"

#include "include/vertex_order.h"

#include "include/backtracking_ind.h"
#include "include/backjumping_ind.h"
#include "include/backmarking_ind.h"
#include "include/backmarking_degreeprune_ind.h"
#include "include/backtracking_degreeprune_ind.h"
#include "include/backtracking_adjacentconsistency_ind.h"
#include "include/backtracking_degreeprune_adjacentconsistency_ind.h"
#include "include/backtracking_adjacentconsistency_forwardcount_ind.h"

int main(int argc, char * argv[]) {
  char const * g_filename = argv[1];
  char const * h_filename = argv[2];

  std::ifstream in{g_filename,std::ios::in|std::ios::binary};
  auto g = read_amalfi<adjacency_listmat<uint16_t, bidirectional_tag>>(in);
  in.close();
  in.open(h_filename,std::ios::in|std::ios::binary);
  auto h = read_amalfi<adjacency_listmat<uint16_t, bidirectional_tag>>(in);
  in.close();

  auto index_order_g = vertex_order_GreatestConstraintFirst(g);

  int count = 0;
  backtracking_degreeprune_adjacentconsistency_ind(
      g,
      h,
      [&count]() {++count; return true;},
      index_order_g);

  std::cout << count << std::endl;
}
