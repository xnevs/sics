#ifndef READ_MIVIA_LDGRAPHS_H_
#define READ_MIVIA_LDGRAPHS_H_


#include <cstdint>

#include <istream>

uint16_t read_word(std::istream & in) {
  uint16_t x = static_cast<unsigned char>(in.get());
  x |= static_cast<uint16_t>(in.get()) << 8;
  return x;
}

template <typename G>
G read_ldgraphs(std::istream & in) {
  uint16_t n;
  in >> n;
  
  G g(n);
  
  uint16_t tmp;
  for (decltype(n) i=0; i<n; ++i) {
    in >> tmp >> tmp;
  }
  
  for (decltype(n) u=0; u<n; ++u) {
    int cnt;
    in >> cnt;
    for (decltype(cnt) j=0; j<cnt; ++j) {
      decltype(n) v;
      in >> tmp >> v;
      g.add_edge(u, v);
    }
  }
  return g;
}

#endif  // READ_MIVIA_LDGRAPHS_H_
