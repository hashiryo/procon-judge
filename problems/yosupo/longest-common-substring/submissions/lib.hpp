#pragma once
// 今の Library (mylib) の SuffixArray と LCPArray で求める (raw のときの lib.cpp と同じ求め方)。s と t を '$' でつないだ文字列の接尾辞
// 配列と LCP 配列を作り、隣り合う接尾辞が別の文字列から始まる所の LCP の最大を取る。LCPArray は Kasai の方法のあと、区間最小のための
// 疎テーブル (O(n log n)) も作る。この問題では疎テーブルは使わない。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <algorithm>
#include <array>
#include <string>
#include "pj.hpp"
#include "mylib/string/SuffixArray.hpp"

struct Solver {
  string s, t;
  array<int, 4> ans{};

  Solver(const string &s, const string &t) : s(s), t(t) {}

  void run() {
    const int n = int(s.size());
    const string u = s + "$" + t;
    const int N = int(u.size());
    SuffixArray sa(u);
    LCPArray lcp(sa);
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
