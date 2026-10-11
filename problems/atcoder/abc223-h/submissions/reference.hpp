#pragma once
// 期待出力を作る参照実装。接頭辞ごとに基底 (_basis.hpp) の写しを全部持つ。4 × 10^5 個で 288 MB ほど。
// brute.hpp と小さい入力で突き合わせてある (pj testdata crosscheck --brute brute.hpp)。
#include "_basis.hpp"
struct Solver {
 vector<PrefixBasis> pre;  // pre[r] は a[0..r) の基底
 explicit Solver(const vector<u64>& a): pre(a.size() + 1) {
  pre[0].clear();
  for(size_t r= 0; r < a.size(); ++r) pre[r + 1]= pre[r], pre[r + 1].insert(a[r], int(r));
 }
 bool query(int l, int r, u64 x) const { return pre[r].contains(l, x); }
};
