#pragma once
// neo_phi で、LCP 配列へ並べ替えずに、文字列の順の PLCP の和をそのまま引く版 (核は _shared/lcp/phi.hpp)。隣り合う接尾辞の LCP の和は、
// どの位置の PLCP も 1 回ずつ足したものと同じなので、最後の並べ替え (飛び飛びの読み込み n 回と、LCP 配列の確保) が要らない。この問題に
// 特有の近道で、LCP 配列を返す関数の形 (neo_phi) と比べる。
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
    const vector<int> plcp = lcp_phi::plcp_array(s, sa);
    const int n = int(s.size());
    ans = (long long)n * (n + 1) / 2;
    for (const int x : plcp) ans -= x;
  }

  long long answer() const { return ans; }
};
