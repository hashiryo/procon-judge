#pragma once
// manacher_w8b で、答えが自明な中心を調べない版。文字 s[i] の中心 (c = 2i) は s[i - 1] != s[i + 1] なら長さ 1、文字 s[i] と s[i + 1]
// の間の中心 (c = 2i + 1) は s[i] != s[i + 1] なら長さ 0 で、箱の中でも変わらない (対称な中心も同じく自明になる)。出力を 1, 0, 1,
// 0, ... で埋めておき、両隣が一致する中心だけを、中心の順に manacher_w8b と同じ手で調べる。両隣の一致は、32 文字ずつ AVX2 で比べた
// 2 つの印 (arm では SIMDe) を 1 bit おきに組み合わせた 64 bit の印の下位から拾う。自明な中心では箱を更新しないが、その箱は後ろの
// 中心の役に立たないので、答えは変わらない。一様な文字列 (26 文字) では、調べる中心が 13 分の 1 ほどになる。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>
#include "pj.hpp"

namespace pal_skip {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  std::memcpy(&x, p, 8);
  return x;
}
// 32 bit の x の各 bit を、64 bit の偶数番目の bit に広げる。
inline uint64_t spread(uint32_t x) {
  uint64_t v = x;
  v = (v | v << 16) & 0x0000FFFF0000FFFFull;
  v = (v | v << 8) & 0x00FF00FF00FF00FFull;
  v = (v | v << 4) & 0x0F0F0F0F0F0F0F0Full;
  v = (v | v << 2) & 0x3333333333333333ull;
  v = (v | v << 1) & 0x5555555555555555ull;
  return v;
}
// 1, 0, 1, 0, ... を出す入力イテレータ (出力の配列を、自明な答えで 1 回だけ書いて作る)。
struct Alt {
  using iterator_category = std::forward_iterator_tag;
  using value_type = int;
  using difference_type = std::ptrdiff_t;
  using pointer = const int *;
  using reference = int;
  std::ptrdiff_t c;
  int operator*() const { return int(~c & 1); }
  Alt &operator++() { return ++c, *this; }
  Alt operator++(int) { Alt r = *this; ++c; return r; }
  bool operator==(const Alt &o) const { return c == o.c; }
  bool operator!=(const Alt &o) const { return c != o.c; }
  std::ptrdiff_t operator-(const Alt &o) const { return c - o.c; }
};
inline void palindromes(const unsigned char *t, int n, int *len) {
  int C = 0, H = 0;
  auto at = [&](int c) __attribute__((always_inline)) {
    int h = c / 2 + 1;
    if (h < H) {
      const int c2 = 2 * C - c, hm = c - C + ((len[c2] + c2 + 1) >> 1);
      if (hm < H) {
        len[c] = 2 * hm - c - 1;
        return;
      }
      h = H;
    }
    int lo = c + 1 - h;
    if (lo > 0 && h < n && t[lo - 1] == t[h]) {
      --lo, ++h;
      for (;;) {
        if (lo < 8 || h + 8 > n) {
          while (lo > 0 && h < n && t[lo - 1] == t[h]) --lo, ++h;
          break;
        }
        if (const uint64_t x = load(t + h) ^ __builtin_bswap64(load(t + lo - 8))) {
          const int m = std::countr_zero(x) >> 3;
          lo -= m, h += m;
          break;
        }
        lo -= 8, h += 8;
      }
    }
    len[c] = h - lo;
    C = c, H = h;
  };
  // 文字の中心 c = 2i (1 <= i <= n - 2) は t[i - 1] = t[i + 1] のとき、間の中心 c = 2i + 1 (0 <= i <= n - 2) は t[i] = t[i + 1] のとき
  // に調べる。i が 0 の文字の中心と n - 1 の文字の中心は長さ 1 で決まる。
  int i = 1;
  if (n >= 2 && t[0] == t[1]) at(1);
  for (; i + 33 <= n; i += 32) {
    const __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(t + i - 1)),
                  b = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(t + i)),
                  d = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(t + i + 1));
    const uint32_t odd = uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(a, d))), even = uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(b, d)));
    uint64_t m = spread(odd) | spread(even) << 1;
    if (m == ~0ull) {  // 64 個の中心をすべて調べるなら、印を見ずに続けて回す
      for (int c = 2 * i; c < 2 * i + 64; ++c) at(c);
      continue;
    }
    for (; m; m &= m - 1) at(2 * i + std::countr_zero(m));
  }
  for (; i + 1 < n; ++i) {
    if (t[i - 1] == t[i + 1]) at(2 * i);
    if (t[i] == t[i + 1]) at(2 * i + 1);
  }
}
}  // namespace pal_skip

struct Solver {
  string s;
  vector<int> len;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    len.assign(pal_skip::Alt{0}, pal_skip::Alt{n > 0 ? 2 * n - 1 : 0});
    pal_skip::palindromes(reinterpret_cast<const unsigned char *>(s.data()), n, len.data());
  }

  const vector<int> &answer() const { return len; }
};
