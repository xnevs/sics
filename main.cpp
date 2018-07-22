#include <cstdint>

#include <fstream>
#include <iostream>

#include "include/read_gal.h"
#include "include/read_amalfi.h"
#include "include/read_ldgraphs.h"

#include "include/adjacency_listmat.h"

#include "include/vertex_order.h"

int state_count = 0;

#include "include/backtracking_ind.h"
#include "include/backjumping_ind.h"
#include "include/backmarking_ind.h"
#include "include/backmarking_degreeprune_ind.h"
#include "include/backtracking_degreeprune_ind.h"
#include "include/backtracking_adjacentconsistency_ind.h"
#include "include/backtracking_degreeprune_adjacentconsistency_ind.h"
#include "include/backtracking_adjacentconsistency_forwardcount_ind.h"
#include "include/backtracking_degreeprune_adjacentconsistency_forwardcount_ind.h"
#include "include/backtracking_forwardcount_ind.h"
#include "include/backtracking_parent_ind.h"
#include "include/backtracking_parent_degreeprune_ind.h"
#include "include/backtracking_parent_adjacentconsistency_ind.h"
#include "include/backtracking_parent_degreeprune_adjacentconsistency_ind.h"
#include "include/backtracking_parent_forwardcount_ind.h"
#include "include/backtracking_parent_adjacentconsistency_forwardcount_ind.h"
#include "include/backtracking_parent_degreeprune_adjacentconsistency_forwardcount_ind.h"
#include "include/backtracking_adjacentconsistency_precount_ind.h"
#include "include/backtracking_degreeprune_adjacentconsistency_precount_ind.h"
#include "include/backtracking_parent_adjacentconsistency_precount_ind.h"
#include "include/backtracking_parent_degreeprune_adjacentconsistency_precount_ind.h"
#include "include/forwardchecking_ind.h"
#include "include/forwardchecking_degreeprune_ind.h"
#include "include/lazyforwardchecking_ind.h"
#include "include/lazyforwardchecking_degreeprune_ind.h"
#include "include/lazyforwardchecking_low_ind.h"
#include "include/lazyforwardchecking_low_degreeprune_ind.h"
#include "include/lazyforwardchecking_parent_ind.h"

#include "include/forwardchecking_mrv_degreeprune_ind.h"
#include "include/forwardchecking_mrv_degreeprune_refine_ind.h"

#include "include/conflictbackjumping_ind.h"
#include "include/conflictbackjumping_degreeprune_ind.h"

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
  lazyforwardchecking_parent_ind(
      g,
      h,
      [&count]() {++count; return true;},
      index_order_g);

  std::cout << count << std::endl;
  std::cout << state_count << std::endl;
}
