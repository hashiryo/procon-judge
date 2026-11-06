#pragma once
// 参照実装。接尾辞配列 (_shared/string/_ref_lcp.hpp の SA-IS) の上で、p で始まる接尾辞の範囲を二分探索する。
#include <algorithm>
#include "_shared/string/_ref_lcp.hpp"

struct Solver {
  const string &s;
  vector<int> sa;
  explicit Solver(const string &s) : s(s), sa(string_ref::suffix_array(s)) {}
  int count(const string &p) {
    const int n = (int)s.size(), m = (int)p.size();
    auto cmp = [&](int i) {  // s[i, n) の先頭 m 文字と p の比べ
      const int l = min(n - i, m);
      const int c = s.compare(i, l, p, 0, l);
      return c != 0 ? c : (l < m ? -1 : 0);
    };
    const auto lo = partition_point(sa.begin(), sa.end(), [&](int i) { return cmp(i) < 0; });
    const auto hi = partition_point(lo, sa.end(), [&](int i) { return cmp(i) == 0; });
    return int(hi - lo);
  }
};
