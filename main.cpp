#include <cstdint>

#include <fstream>
#include <iostream>
#include <string>

#include <sics/read_gal.h>
#include <sics/read_amalfi.h>
#include <sics/read_ldgraphs.h>
#include <sics/read_gf.h>

#include <sics/adjacency_listmat.h>

#include <sics/vertex_order.h>

#include <sics/stats.h>

#include <sics/backtracking_ind.h>
#include <sics/backtracking_degreeprune_ind.h>
#include <sics/backtracking_adjacentconsistency_ind.h>
#include <sics/backtracking_degreeprune_adjacentconsistency_ind.h>
#include <sics/backtracking_adjacentconsistency_forwardcount_ind.h>
#include <sics/backtracking_degreeprune_adjacentconsistency_forwardcount_ind.h>
#include <sics/backtracking_forwardcount_ind.h>
#include <sics/backtracking_parent_ind.h>
#include <sics/backtracking_parent_degreeprune_ind.h>
#include <sics/backtracking_parent_adjacentconsistency_ind.h>
#include <sics/backtracking_parent_degreeprune_adjacentconsistency_ind.h>
#include <sics/backtracking_parent_forwardcount_ind.h>
#include <sics/backtracking_parent_adjacentconsistency_forwardcount_ind.h>
#include <sics/backtracking_parent_degreeprune_adjacentconsistency_forwardcount_ind.h>
#include <sics/backtracking_adjacentconsistency_precount_ind.h>
#include <sics/backtracking_degreeprune_adjacentconsistency_precount_ind.h>
#include <sics/backtracking_parent_adjacentconsistency_precount_ind.h>
#include <sics/backtracking_parent_degreeprune_adjacentconsistency_precount_ind.h>
#include <sics/backjumping_ind.h>
#include <sics/conflictbackjumping_ind.h>
#include <sics/conflictbackjumping_degreeprune_ind.h>
#include <sics/backmarking_ind.h>
#include <sics/backmarking_degreeprune_ind.h>
#include <sics/forwardchecking_ind.h>
#include <sics/forwardchecking_degreeprune_ind.h>
#include <sics/lazyforwardchecking_ind.h>
#include <sics/lazyforwardchecking_degreeprune_ind.h>
#include <sics/lazyforwardchecking_low_ind.h>
#include <sics/lazyforwardchecking_low_degreeprune_ind.h>
#include <sics/lazyforwardchecking_parent_ind.h>
#include <sics/lazyforwardchecking_parent_degreeprune_ind.h>
#include <sics/lazyforwardchecking_low_parent_ind.h>
#include <sics/lazyforwardchecking_low_parent_degreeprune_ind.h>

#include <sics/forwardchecking_mrv_degreeprune_ind.h>

#include <sics/forwardchecking_bitset_degreeprune_ind.h>
#include <sics/forwardchecking_bitset_degreesequenceprune_ind.h>

#include <sics/forwardchecking_bitset_mrv_degreeprune_ind.h>
#include <sics/forwardchecking_bitset_mrv_degreeprune_ac1_ind.h>
#include <sics/forwardchecking_bitset_mrv_degreeprune_countingalldifferent_ind.h>
#include <sics/forwardchecking_bitset_mrv_degreesequenceprune_ind.h>
#include <sics/forwardchecking_bitset_mrv_degreesequenceprune_countingalldifferent_ind.h>

#include <sics/lazyforwardchecking_parent_degreesequenceprune_ind.h>

#include <sics/lazyforwardchecking_low_bitset_degreeprune_ind.h>
#include <sics/lazyforwardcheckingbackjumping_low_bitset_degreeprune_ind.h>

#include <sics/backtracking_bitset_degreeprune_ind.h>

int main(int argc, char * argv[]) {
  using namespace sics;

  char const * g_filename = argv[1];
  char const * h_filename = argv[2];

  std::ifstream in{g_filename,std::ios::in|std::ios::binary};
  auto g = read_gf<adjacency_listmat<uint16_t, undirected_tag, std::string>>(in);
  in.close();
  in.open(h_filename,std::ios::in|std::ios::binary);
  auto h = read_gf<adjacency_listmat<uint16_t, undirected_tag, std::string>>(in);
  in.close();

  auto index_order_g = vertex_order_GreatestConstraintFirst(g);

  int count = 0;
  lazyforwardchecking_low_bitset_degreeprune_ind(
      g,
      h,
      [&count]() {++count; return true;},
      index_order_g);

  std::cout << count << std::endl;

  SICS_STATS_PRINT(std::cout);
}
