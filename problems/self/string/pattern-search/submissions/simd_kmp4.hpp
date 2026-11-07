#pragma once
// simd_kmp3 の候補の絞り方を変えたもの。パターンが 8 文字以下なら、32 か所の位置について全部の文字を AVX2 で比べて、
// その and を答えにする (候補の確かめが要らない)。9 文字以上なら、先頭の 4 文字と末尾の 1 文字の 5 回の比べで候補を絞り
// (2 文字の一様なテキストで候補が 1/32)、残りを 32 byte ずつ比べる。比べた塊の数が n / 2 + m を超えたら KMP に切り替える。
// 答えの列は最大の長さを reserve するだけにする (simd_kmp3 のように huge page を頼むと、出現が 10^6 個近いケースで x64 では
// 遅くなった)。
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
  template <int M> int exact(const char *T, const char *P, int n) {
    __m256i f[M];
    for (int j = 0; j < M; ++j) f[j] = _mm256_set1_epi8(P[j]);
    int i = 0;
    for (; i + M + 31 <= n; i += 32) {
      __m256i acc = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(T + i)), f[0]);
      for (int j = 1; j < M; ++j) acc = _mm256_and_si256(acc, _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(T + i + j)), f[j]));
      for (unsigned mask = (unsigned)_mm256_movemask_epi8(acc); mask; mask &= mask - 1) pos.push_back(i + __builtin_ctz(mask));
    }
    return i;
  }
  void run() {
    const int n = (int)t.size(), m = (int)p.size();
    const char *T = t.data();
    pos.clear();
    pos.reserve(n - m + 1);
    string pb = p;
    pb.append(64, '\0');
    const char *P = pb.data();
    int i = 0;
    switch (m) {
      case 1: i = exact<1>(T, P, n); break;
      case 2: i = exact<2>(T, P, n); break;
      case 3: i = exact<3>(T, P, n); break;
      case 4: i = exact<4>(T, P, n); break;
      case 5: i = exact<5>(T, P, n); break;
      case 6: i = exact<6>(T, P, n); break;
      case 7: i = exact<7>(T, P, n); break;
      case 8: i = exact<8>(T, P, n); break;
      default: {
        const __m256i f0 = _mm256_set1_epi8(P[0]), f1 = _mm256_set1_epi8(P[1]), f2 = _mm256_set1_epi8(P[2]), f3 = _mm256_set1_epi8(P[3]);
        const __m256i l = _mm256_set1_epi8(P[m - 1]);
        const int body = m - 5, nc = (body + 31) / 32;
        const unsigned last = body % 32 ? (1u << (body % 32)) - 1 : 0xffffffffu;
        long long budget = n / 2 + m;
        for (; i + m + 63 <= n; i += 32) {
          __m256i acc = _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(T + i)), f0),
                                         _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(T + i + m - 1)), l));
          acc = _mm256_and_si256(acc, _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(T + i + 1)), f1));
          acc = _mm256_and_si256(acc, _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(T + i + 2)), f2));
          acc = _mm256_and_si256(acc, _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(T + i + 3)), f3));
          unsigned mask = (unsigned)_mm256_movemask_epi8(acc);
          while (mask) {
            const int k = __builtin_ctz(mask);
            mask &= mask - 1;
            const char *x = T + i + k + 4, *y = P + 4;
            int c = 0;
            for (; c < nc; ++c) {
              const __m256i u = _mm256_loadu_si256((const __m256i *)(x + 32 * c)), v = _mm256_loadu_si256((const __m256i *)(y + 32 * c));
              const unsigned eq = (unsigned)_mm256_movemask_epi8(_mm256_cmpeq_epi8(u, v)) | ~(c + 1 == nc ? last : 0xffffffffu);
              if (eq != 0xffffffffu) break;
            }
            budget -= c + 1;
            if (c == nc) pos.push_back(i + k);
          }
          if (budget < 0) return kmp_from(i + 32);
        }
      }
    }
    for (; i + m <= n; ++i)
      if (memcmp(T + i, P, m) == 0) pos.push_back(i);
  }
  const vector<int> &answer() const { return pos; }
};
