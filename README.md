# Subgraph Isomorphism Constraint Satisfaction

A header only C++ library for solving subgraph isomorphism constraint satisfaction probem.

## Requirements

A C++ compiler with C++17 support is required.

The library depends on `boost::dynamic_bitset` and `boost::iterator_range` from the Boost C++ library.

## Usage

The library includes many variations of algorithms for solving the subgraphs isomorphism problem. The list of all available algorithms can be obrained by looking in the `include/sics` directory for the files ending with the suffix `_ind.h`.

The two recommended algorithms to use are:
  - `lazyforwardcheckingbackjumping_low_bitset_degreeprune_ind` and
  - `forwardchecking_bitset_mrv_degreeprune_ind`.
  
An example is shown in `main.cpp`.
