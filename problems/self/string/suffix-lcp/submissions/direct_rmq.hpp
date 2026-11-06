#pragma once
// 先頭の 32 文字までは 8 byte ずつ排他的論理和で直接比べ、それで決まらないときだけ接尾辞配列と LCP 配列の区間最小 (sa_rmq と同じ
// NeoLibrary の部品) を引く。一様な文字列では LCP がほとんど 8 未満なので、順位の表に触らずに済む。文字列は末尾に 0 を 32 個
// 足した写しで読む (英小文字は 0 でないので、短い方の接尾辞の終わりで必ず食い違う)。
#include "pj.hpp"
#include <cstring>
#include "neo/data_structure/RangeMin.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  int n;
  string t;
  vector<int> rnk;
  RangeMin rm;
  explicit Solver(const string &s) : n((int)s.size()), t(s), rnk(s.size()) {
    t.append(32, '\0');
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_array(s, sa);
    for (int i = 0; i < n; ++i) rnk[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
  }
  int lcp(int i, int j) {
    if (i == j) return n - i;
    const char *p = t.data();
    for (int k = 0; k < 32; k += 8) {
      u64 x, y;
      memcpy(&x, p + i + k, 8), memcpy(&y, p + j + k, 8);
      if (x != y) return k + (__builtin_ctzll(x ^ y) >> 3);
    }
    int l = rnk[i], r = rnk[j];
    if (l > r) swap(l, r);
    return (int)rm.min(l, r);
  }
};
