#pragma once
// 乱択を使わない形。NeoLibrary の接尾辞配列と LCP 配列 (neo/string/suffix_array.hpp) と区間最小 (neo/data_structure/RangeMin.hpp) で、
// s[a, n) と s[b, n) の LCP が len 以上かを見る。LCP は 2 つの接尾辞の順位の間の LCP 配列の最小。
#include "pj.hpp"
#include "neo/data_structure/RangeMin.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  vector<int> rnk;
  RangeMin rm;
  explicit Solver(const string &s) : rnk(s.size()) {
    const int n = (int)s.size();
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_array(s, sa);
    for (int i = 0; i < n; ++i) rnk[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
  }
  bool equal(int a, int b, int len) {
    if (len == 0 || a == b) return true;
    int l = rnk[a], r = rnk[b];
    if (l > r) swap(l, r);
    return (int)rm.min(l, r) >= len;
  }
};
