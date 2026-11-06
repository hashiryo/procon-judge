#pragma once
// NeoLibrary の接尾辞配列 (neo/string/suffix_array.hpp) の上で、p で始まる接尾辞の範囲を二分探索する。比べるのは memcmp。
#include "pj.hpp"
#include <algorithm>
#include <cstring>
#include "neo/string/suffix_array.hpp"

struct Solver {
  const string &s;
  vector<int> sa;
  explicit Solver(const string &s) : s(s), sa(suffix_array(s)) {}
  int count(const string &p) {
    const int n = (int)s.size(), m = (int)p.size();
    const char *t = s.data(), *q = p.data();
    const auto lo = partition_point(sa.begin(), sa.end(), [&](int i) {
      const int l = min(n - i, m), c = memcmp(t + i, q, l);
      return c < 0 || (c == 0 && l < m);
    });
    const auto hi = partition_point(lo, sa.end(), [&](int i) { return memcmp(t + i, q, min(n - i, m)) == 0; });
    return int(hi - lo);
  }
};
