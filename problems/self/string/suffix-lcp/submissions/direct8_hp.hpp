#pragma once
// direct_rmq の直接の比べを先頭 8 文字 (u64 の読み込み 1 回ずつ) だけにし、順位の表を huge page に置いたもの
// (_shared/string/_hugealloc.hpp。RangeMin の表はもともと huge page)。
#include "pj.hpp"
#include <cstring>
#include "_shared/string/_hugealloc.hpp"
#include "neo/data_structure/RangeMin.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  int n;
  string t;
  string_hp::Vec<int> rnk;
  RangeMin rm;
  explicit Solver(const string &s) : n((int)s.size()), t(s), rnk(s.size()) {
    t.append(8, '\0');
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_array(s, sa);
    for (int i = 0; i < n; ++i) rnk[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
  }
  int lcp(int i, int j) {
    if (i == j) return n - i;
    u64 x, y;
    memcpy(&x, t.data() + i, 8), memcpy(&y, t.data() + j, 8);
    if (x != y) return __builtin_ctzll(x ^ y) >> 3;
    int l = rnk[i], r = rnk[j];
    if (l > r) swap(l, r);
    return (int)rm.min(l, r);
  }
};
