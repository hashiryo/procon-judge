#pragma once
// 今の Library (mylib) の SuffixArray と LCPArray で数える (raw のときの lib.cpp と同じ数え方)。LCPArray は Kasai の方法で LCP 配列を
// 作ったあと、任意の 2 つの接尾辞の LCP を引くための疎テーブル (O(n log n)) も作る。この問題では疎テーブルは使わない。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <string>
#include "pj.hpp"
#include "mylib/string/SuffixArray.hpp"

struct Solver {
  string s;
  long long ans = 0;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    SuffixArray sa(s);
    LCPArray lcp(sa);
    const int n = int(s.size());
    ans = (long long)n * (n + 1) / 2;
    for (int i = n; --i;) ans -= lcp[i - 1];
  }

  long long answer() const { return ans; }
};
