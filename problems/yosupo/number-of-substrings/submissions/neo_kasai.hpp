#pragma once
// NeoLibrary の suffix_array で接尾辞配列を作り、LCP 配列を Kasai の方法 (ACL の lcp_array と同じ手順を byte のまま比べる形、核は
// _shared/lcp/kasai.hpp) で求めて数える。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "_shared/lcp/kasai.hpp"

struct Solver {
  string s;
  long long ans = 0;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_kasai::lcp_array(s, sa);
    const int n = int(s.size());
    ans = (long long)n * (n + 1) / 2;
    for (const int x : lcp) ans -= x;
  }

  long long answer() const { return ans; }
};
