#pragma once
// neo_phi で、LCP 配列へ並べ替えずに、文字列の順に PLCP を求めながら、位置 i と、接尾辞配列でその 1 つ前の接尾辞 k が別の文字列から
// 始まる所の最大を取る版 (核は _shared/lcp/phi.hpp の scan_plcp)。隣り合う接尾辞の組は、どれも PLCP のどこかの位置 i と k の組として
// 1 回ずつ現れるので、並べ替え (飛び飛びの読み込み n 回と、LCP 配列の確保) が要らない。この問題に特有の近道で、LCP 配列を返す関数の形
// (neo_phi) と比べる。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
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
    const vector<int> sa = suffix_array(u);
    vector<int> buf(u.size());
    int a = 0, c = 0, len = 0;
    lcp_phi::scan_plcp(u, sa, buf.data(), [&](int i, int k, int h) {
      // k = |u| (先頭の接尾辞) と k = n (区切り) は、どちらの条件にも当たらない
      if (h > len && ((i < n && n < k && k < int(u.size())) || (k < n && n < i))) len = h, a = i < n ? i : k, c = (i < n ? k : i) - n - 1;
    });
    ans = {a, a + len, c, c + len};
  }

  array<int, 4> answer() const { return ans; }
};
