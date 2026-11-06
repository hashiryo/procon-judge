#pragma once
// simd_kmp2 を 2 つ直したもの。1 つ目は候補の残りの比べで、パターンの 2 文字目からの残りを 32 byte ずつ AVX2 で比べ、最後の
// 塊も範囲の外を印で消して同じ形で比べる (memcmp を呼ばない)。パターンは末尾に 64 byte 足した写しで読み、テキストは読み込みが
// 範囲に収まる位置 (i + m + 63 <= n) までを SIMD で調べて、残りは 1 か所ずつ比べる。2 つ目は答えの列で、最大の長さを
// reserve してから、その領域の 2 MB 境界の内側に huge page を頼む (Linux のみ)。出現が 10^6 個近いと、4 KB のページを
// 初めて触る手間が答えを書く時間の大半になるため。
#include "pj.hpp"
#include <cstdint>
#include <cstring>
#ifdef __linux__
#include <sys/mman.h>
#endif
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
  void reserve_out(size_t cap) {
    pos.clear();
    pos.reserve(cap);
#ifdef __linux__
    const uintptr_t H = uintptr_t(1) << 21, b = (uintptr_t)pos.data(), e = b + cap * sizeof(int);
    const uintptr_t lo = (b + H - 1) & ~(H - 1), hi = e & ~(H - 1);
    if (lo < hi) madvise((void *)lo, hi - lo, MADV_HUGEPAGE);
#endif
  }
  void run() {
    const int n = (int)t.size(), m = (int)p.size();
    const char *T = t.data();
    reserve_out(n - m + 1);
    string pb = p;
    pb.append(64, '\0');
    const char *P = pb.data();
    const __m256i f = _mm256_set1_epi8(P[0]), l = _mm256_set1_epi8(P[m - 1]);
    const int body = m > 2 ? m - 2 : 0, nc = (body + 31) / 32;
    const unsigned last = body % 32 ? (1u << (body % 32)) - 1 : 0xffffffffu;
    long long budget = n / 2 + m;
    int i = 0;
    for (; i + m + 63 <= n; i += 32) {
      const __m256i a = _mm256_loadu_si256((const __m256i *)(T + i)), b = _mm256_loadu_si256((const __m256i *)(T + i + m - 1));
      unsigned mask = (unsigned)_mm256_movemask_epi8(_mm256_and_si256(_mm256_cmpeq_epi8(a, f), _mm256_cmpeq_epi8(b, l)));
      while (mask) {
        const int k = __builtin_ctz(mask);
        mask &= mask - 1;
        const char *x = T + i + k + 1, *y = P + 1;
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
    for (; i + m <= n; ++i)
      if (memcmp(T + i, P, m) == 0) pos.push_back(i);
  }
  const vector<int> &answer() const { return pos; }
};
