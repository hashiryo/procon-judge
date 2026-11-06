#pragma once
// simd の候補の絞り込みに、手間の上限を付けたもの。候補ごとに memcmp で比べる長さを足していき、2 n + 64 m を超えたら、その塊の
// 次の位置から KMP に切り替える (それより前から始まる出現は、もう調べ終えている)。KMP の失敗関数は切り替えるときだけ作る。
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
  void kmp_from(int i) {
    const int n = (int)t.size(), m = (int)p.size();
    const unsigned char *T = (const unsigned char *)t.data(), *P = (const unsigned char *)p.data();
    vector<int> pi(m);
    pi[0] = 0;
    for (int j = 1, k = 0; j < m; ++j) {
      while (k > 0 && P[j] != P[k]) k = pi[k - 1];
      k += P[j] == P[k];
      pi[j] = k;
    }
    for (int k = 0; i < n; ++i) {
      while (k > 0 && T[i] != P[k]) k = pi[k - 1];
      k += T[i] == P[k];
      if (k == m) pos.push_back(i - m + 1), k = pi[m - 1];
    }
  }
  void run() {
    const int n = (int)t.size(), m = (int)p.size();
    const char *T = t.data(), *P = p.data();
    pos.clear();
    pos.reserve(n - m + 1);
    const __m256i f = _mm256_set1_epi8(P[0]), l = _mm256_set1_epi8(P[m - 1]);
    long long budget = 2LL * n + 64LL * m;
    int i = 0;
    for (; i + m + 31 <= n; i += 32) {
      const __m256i a = _mm256_loadu_si256((const __m256i *)(T + i)), b = _mm256_loadu_si256((const __m256i *)(T + i + m - 1));
      unsigned mask = (unsigned)_mm256_movemask_epi8(_mm256_and_si256(_mm256_cmpeq_epi8(a, f), _mm256_cmpeq_epi8(b, l)));
      if (m > 2) budget -= (long long)__builtin_popcount(mask) * (m - 2);
      while (mask) {
        const int k = __builtin_ctz(mask);
        mask &= mask - 1;
        if (m <= 2 || memcmp(T + i + k + 1, P + 1, m - 2) == 0) pos.push_back(i + k);
      }
      if (budget < 0) return kmp_from(i + 32);
    }
    for (; i + m <= n; ++i)
      if (memcmp(T + i, P, m) == 0) pos.push_back(i);
  }
  const vector<int> &answer() const { return pos; }
};
