#pragma once
// duval_w8 で、3 つの手を足した版。(1) k が i に戻っている間は、s[j] > s[i] の位置では k = i のまま j が進むだけなので、s[j] <= s[i]
// になる最初の j を 32 文字ずつ AVX2 で探して飛ぶ (arm では SIMDe)。符号なしの max(s[j], s[i]) = s[i] で s[j] <= s[i] を見る。
// (2) 一致の続く長さを 8 バイトずつ数える前に、1 文字目を比べる (一様な文字列では 1 文字目で外れることが多い)。(3) 切れ目の列は
// n + 1 個分を先に取り、周期 p の因子が続く分は等差の列としてまとめて書く (all_same では因子が n 個ある)。
// Lyndon 分解の実装を比べた記録は algo-notes の notes/lyndon.md にある。
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

namespace duval_skip {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  std::memcpy(&x, p, 8);
  return x;
}
// j 以上で t[j] <= c となる最初の j (無ければ n)。
inline int skip_greater(const unsigned char *t, int j, int n, unsigned char c) {
  const __m256i v = _mm256_set1_epi8(char(c));
  for (; j + 32 <= n; j += 32) {
    const __m256i x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(t + j));
    if (const unsigned m = unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(x, v), v)))) return j + std::countr_zero(m);
  }
  while (j < n && t[j] > c) ++j;
  return j;
}
}  // namespace duval_skip

struct Solver {
  string s;
  vector<int> cut;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using namespace duval_skip;
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    cut.clear();
    cut.reserve(n + 1);
    cut.push_back(0);
    for (int i = 0; i < n;) {
      int j = i + 1, k = i;
      while (j < n) {
        if (k == i) {
          j = skip_greater(t, j, n, t[i]);
          if (j == n) break;
        }
        if (t[k] != t[j]) {
          if (t[k] > t[j]) break;
          k = i, ++j;  // s[k] < s[j]: s[i, j + 1) は Lyndon 語
          continue;
        }
        ++k, ++j;
        // s[k] = s[j] の続く長さを数え、k と j をまとめて進める
        while (j + 8 <= n) {
          const uint64_t x = load(t + k) ^ load(t + j);
          if (x) {
            const int m = std::countr_zero(x) >> 3;
            k += m, j += m;
            break;
          }
          k += 8, j += 8;
        }
        while (j < n && t[k] == t[j]) ++k, ++j;
        if (j == n || t[k] > t[j]) break;
        k = i, ++j;
      }
      const int p = j - k, c = (k - i) / p + 1;  // 周期 p の因子が c 個
      const size_t o = cut.size();
      cut.resize(o + c);
      int *w = cut.data() + o;
      for (int q = 0; q < c; ++q) w[q] = i + (q + 1) * p;
      i += c * p;
    }
  }

  const vector<int> &answer() const { return cut; }
};
