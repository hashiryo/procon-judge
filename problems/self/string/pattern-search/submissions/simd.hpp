#pragma once
// SIMD で候補を絞る形 (W. Muła の SIMD-friendly algorithms for substring searching の generic な形)。テキストの 32 個の位置について、
// 先頭の文字がパターンの先頭と、末尾に当たる文字がパターンの末尾と等しいかを AVX2 でまとめて調べ、両方が等しい位置だけ
// memcmp で残りを比べる。一様なテキストでは候補がほとんど無いが、候補が密な入力 (同じ文字の列と長いパターン) では O(n m)。
#include "pj.hpp"
#include <cstring>
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif

struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() {
    const int n = (int)t.size(), m = (int)p.size();
    const char *T = t.data(), *P = p.data();
    pos.clear();
    pos.reserve(n - m + 1);
    const __m256i f = _mm256_set1_epi8(P[0]), l = _mm256_set1_epi8(P[m - 1]);
    int i = 0;
    for (; i + m + 31 <= n; i += 32) {
      const __m256i a = _mm256_loadu_si256((const __m256i *)(T + i)), b = _mm256_loadu_si256((const __m256i *)(T + i + m - 1));
      unsigned mask = (unsigned)_mm256_movemask_epi8(_mm256_and_si256(_mm256_cmpeq_epi8(a, f), _mm256_cmpeq_epi8(b, l)));
      while (mask) {
        const int k = __builtin_ctz(mask);
        mask &= mask - 1;
        if (m <= 2 || memcmp(T + i + k + 1, P + 1, m - 2) == 0) pos.push_back(i + k);
      }
    }
    for (; i + m <= n; ++i)
      if (memcmp(T + i, P, m) == 0) pos.push_back(i);
  }
  const vector<int> &answer() const { return pos; }
};
