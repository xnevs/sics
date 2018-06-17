#ifndef GMCS_READ_AMALFI_H_
#define GMCS_READ_AMALFI_H_

#include <cstdint>

#include <istream>

uint16_t read_word(std::istream & in) {
  uint16_t x = static_cast<unsigned char>(in.get());
  x |= static_cast<uint16_t>(in.get()) << 8;
  return x;
}

template <typename G>
G read_amalfi(std::istream & in) {
  auto n = read_word(in);
  G g(n);
  for (decltype(n) u=0; u<n; ++u) {
    auto cnt = read_word(in);
    for (decltype(cnt) j=0; j<cnt; ++j) {
      auto v = read_word(in);
      g.add_edge(u, v);
    }
  }
  return g;
}

#endif  // GMCS_READ_AMALFI_H_
