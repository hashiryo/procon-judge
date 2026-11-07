#pragma once
// SIMD で候補を絞り、手間が大きければ KMP に切り替える形 (W. Muła の SIMD-friendly algorithms for substring searching の
// generic な形)。テキストの 32 個の位置について、先頭の文字がパターンの先頭と、末尾に当たる文字がパターンの末尾と等しいかを AVX2 で
// まとめて調べる。候補の位置では、パターンの 2 文字目から 32 byte ずつ AVX2 で比べて食い違ったところで止め (最後の 32 byte 未満は
// memcmp)、比べた 32 byte の塊の数を数える。塊の数が n / 2 + m を超えたら、その 32 か所の次の位置から KMP に切り替える。2 文字の
// 一様なテキストでは候補が 4 分の 1 の位置にあるが、ほとんど最初の塊で食い違うので切り替えない。同じ文字だけのテキストと a^1000 の
// ように候補ごとに長く一致する入力だけが KMP に移る。
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
    const int body = m - 2, full = body > 0 ? body / 32 : 0, rest = body > 0 ? body - 32 * full : 0;
    long long budget = n / 2 + m;
    int i = 0;
    for (; i + m + 31 <= n; i += 32) {
      const __m256i a = _mm256_loadu_si256((const __m256i *)(T + i)), b = _mm256_loadu_si256((const __m256i *)(T + i + m - 1));
      unsigned mask = (unsigned)_mm256_movemask_epi8(_mm256_and_si256(_mm256_cmpeq_epi8(a, f), _mm256_cmpeq_epi8(b, l)));
      while (mask) {
        const int k = __builtin_ctz(mask);
        mask &= mask - 1;
        const char *x = T + i + k + 1, *y = P + 1;
        int c = 0;
        for (; c < full; ++c) {
          const __m256i u = _mm256_loadu_si256((const __m256i *)(x + 32 * c)), v = _mm256_loadu_si256((const __m256i *)(y + 32 * c));
          if ((unsigned)_mm256_movemask_epi8(_mm256_cmpeq_epi8(u, v)) != 0xffffffffu) break;
        }
        budget -= c + 1;
        if (c == full && (rest == 0 || memcmp(x + 32 * full, y + 32 * full, rest) == 0)) pos.push_back(i + k);
      }
      if (budget < 0) return kmp_from(i + 32);
    }
    for (; i + m <= n; ++i)
      if (memcmp(T + i, P, m) == 0) pos.push_back(i);
  }
  const vector<int> &answer() const { return pos; }
};
