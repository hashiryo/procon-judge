#pragma once
// NeoLibrary の suffix_array で接尾辞配列を作り、LCP 配列を Φ 配列を経由する方法 (Kärkkäinen、Manzini、Puglisi 2009 の PLCP、核は
// _shared/lcp/phi.hpp) で求めて数える。PLCP を求めてから lcp[i] = plcp[sa[i + 1]] と並べ替えるので、LCP 配列を返す関数の形になる。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "_shared/lcp/phi.hpp"

struct Solver {
  string s;
  long long ans = 0;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = lcp_phi::lcp_array(s, sa);
    const int n = int(s.size());
    ans = (long long)n * (n + 1) / 2;
    for (const int x : lcp) ans -= x;
  }

  long long answer() const { return ans; }
};
