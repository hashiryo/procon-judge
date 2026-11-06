#pragma once
// 先頭の 64 文字までは AVX2 の 32 byte の比べ 2 回で直接求め、64 文字以上一致したクエリが初めて来たときに、接尾辞配列と LCP 配列と
// 区間最小 (sa_rmq と同じ NeoLibrary の部品、順位の表は huge page) を作る。LCP が短いクエリしか来なければ構造を作らない。
// 文字列は末尾に 0 を 64 個足した写しで読む (英小文字は 0 でないので、短い方の接尾辞の終わりで必ず食い違う)。
#include "pj.hpp"
#include <cstring>
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include "_shared/string/_hugealloc.hpp"
#include "neo/data_structure/RangeMin.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  const string &s;
  int n;
  string t;
  bool built = false;
  string_hp::Vec<int> rnk;
  RangeMin rm;
  explicit Solver(const string &s) : s(s), n((int)s.size()), t(s) { t.append(64, '\0'); }
  void build() {
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_array(s, sa);
    rnk.resize(n);
    for (int i = 0; i < n; ++i) rnk[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
    built = true;
  }
  int lcp(int i, int j) {
    if (i == j) return n - i;
    const char *p = t.data();
    for (int k = 0; k < 64; k += 32) {
      const __m256i x = _mm256_loadu_si256((const __m256i *)(p + i + k)), y = _mm256_loadu_si256((const __m256i *)(p + j + k));
      const unsigned ne = ~(unsigned)_mm256_movemask_epi8(_mm256_cmpeq_epi8(x, y));
      if (ne) return k + __builtin_ctz(ne);
    }
    if (!built) build();
    int l = rnk[i], r = rnk[j];
    if (l > r) swap(l, r);
    return (int)rm.min(l, r);
  }
};
