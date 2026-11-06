#pragma once
// NeoLibrary の接尾辞配列と LCP 配列 (neo/string/suffix_array.hpp) と区間最小 (neo/data_structure/RangeMin.hpp) をつないだもの。
// lcp(i, j) は 2 つの接尾辞の順位の間の LCP 配列の最小。
#include "pj.hpp"
#include "neo/data_structure/RangeMin.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  int n;
  vector<int> rnk;
  RangeMin rm;
  explicit Solver(const string &s) : n((int)s.size()), rnk(s.size()) {
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_array(s, sa);
    for (int i = 0; i < n; ++i) rnk[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
  }
  int lcp(int i, int j) {
    if (i == j) return n - i;
    int l = rnk[i], r = rnk[j];
    if (l > r) swap(l, r);
    return (int)rm.min(l, r);
  }
};
