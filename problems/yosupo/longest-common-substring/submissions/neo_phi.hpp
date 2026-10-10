#pragma once
// NeoLibrary の suffix_array で s と t を '$' でつないだ文字列の接尾辞配列を作り、LCP 配列を Φ 配列を経由する方法 (核は
// _shared/lcp/phi.hpp) で求めて、隣り合う接尾辞が別の文字列から始まる所の LCP の最大を取る。LCP 配列を返す関数の形。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "_shared/lcp/phi.hpp"

struct Solver {
  string s, t;
  array<int, 4> ans{};

  Solver(const string &s, const string &t) : s(s), t(t) {}

  void run() {
    const int n = int(s.size());
    const string u = s + "$" + t;
    const int N = int(u.size());
    const vector<int> sa = suffix_array(u);
    const vector<int> lcp = lcp_phi::lcp_array(u, sa);
    int a = 0, c = 0, len = 0;
    for (int i = 0; i + 1 < N; ++i) {
      int x = sa[i], y = sa[i + 1];
      if (x > y) swap(x, y);
      if (x < n && n < y && len < lcp[i]) len = lcp[i], a = x, c = y - n - 1;
    }
    ans = {a, a + len, c, c + len};
  }

  array<int, 4> answer() const { return ans; }
};
