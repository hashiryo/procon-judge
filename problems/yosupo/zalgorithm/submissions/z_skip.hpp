#pragma once
// z_w8b で、s[0] と同じ文字の位置だけを調べる版。i > 0 で z[i] が 1 以上になるのは s[i] = s[0] のときだけで、箱の中でも s[i] =
// s[i - l] なので変わらない。出力は 0 で埋めてあるので、ほかの位置は書かずに済む。s[0] と同じ文字の位置は、32 バイトずつ AVX2 で
// 比べた印 (arm では SIMDe で写した NEON) の下位から拾い、位置の順に、z_w8b と同じく箱の中なら写し、そうでなければ次の 1 文字を
// 比べてから 8 バイトずつ伸ばす。32 位置がすべて一致したら、印を見ずに続けて回す。一様な文字列 (26 文字) では、調べる位置が
// 26 分の 1 になる。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <bit>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "pj.hpp"

namespace z_skip {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  std::memcpy(&x, p, 8);
  return x;
}
inline void z_array(const unsigned char *t, int n, int *z) {
  if (n == 0) return;
  z[0] = n;
  int l = 0, r = 0;
  // s[i] = s[0] の位置 i (> 0) の z[i] を求める。箱 [l, r) は右端が最も右にある一致の区間。
  auto at = [&](int i) __attribute__((always_inline)) {
    int k = 0;
    if (i < r) {
      k = z[i - l];
      if (k < r - i) {
        z[i] = k;
        return;
      }
      k = r - i;
    } else {
      k = 1;  // t[i] = t[0] は分かっている
    }
    if (i + k == n || t[k] != t[i + k]) {  // 8 バイトを読む前に、次の 1 文字を比べる
      z[i] = k, l = i, r = i + k;
      return;
    }
    for (;;) {
      if (i + k + 8 > n) {
        while (i + k < n && t[k] == t[i + k]) ++k;
        break;
      }
      if (const uint64_t x = load(t + k) ^ load(t + i + k)) {
        k += std::countr_zero(x) >> 3;
        break;
      }
      k += 8;
    }
    z[i] = k, l = i, r = i + k;
  };
  const __m256i c0 = _mm256_set1_epi8(char(t[0]));
  int i = 1;
  for (; i + 32 <= n; i += 32) {
    unsigned m = unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(t + i)), c0)));
    if (m == ~0u) {  // 32 位置がすべて s[0] と同じ文字なら、印を見ずに続けて回す
      for (int j = i; j < i + 32; ++j) at(j);
      continue;
    }
    for (; m; m &= m - 1) at(i + std::countr_zero(m));
  }
  for (; i < n; ++i)
    if (t[i] == t[0]) at(i);
}
}  // namespace z_skip

struct Solver {
  string s;
  vector<int> z;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    z.assign(s.size(), 0);
    z_skip::z_array(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), z.data());
  }

  const vector<int> &answer() const { return z; }
};
