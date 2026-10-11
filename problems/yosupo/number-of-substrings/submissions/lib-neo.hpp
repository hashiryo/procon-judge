#pragma once
// NeoLibrary の suffix_array と lcp_array (neo/string/suffix_array.hpp、LCP 配列は Φ 配列を経由する方法) で数える。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  string s;
  long long ans = 0;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_array(s, sa);
    const int n = int(s.size());
    ans = (long long)n * (n + 1) / 2;
    for (const int x : lcp) ans -= x;
  }

  long long answer() const { return ans; }
};
