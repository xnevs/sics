#ifndef GMCS_GRAPH_COMMON_H_
#define GMCS_GRAPH_COMMON_H_

template <typename Index, typename Label=void>
struct half_edge : public half_edge<Index, void> {
  Label label;

  half_edge(Index target, Label label)
      : half_edge<Index, void>::half_edge(target),
        label{label} {
  }
};

template <typename Index>
struct half_edge<Index, void> {
  Index target;

  half_edge(Index target)
      : target{target} {
  }
};

#endif  // GMCS_GRAPH_COMMON_H
