#pragma once
// abc281-e の愚直解。窓ごとに値を写して並べ替え、先頭の k 個を足す。O(N M log M) なので小さい入力でだけ使う
// (pj testdata crosscheck --brute brute.hpp)。
#include <algorithm>
#include "pj.hpp"
struct Solver {
 vector<int> a;
 explicit Solver(const vector<int>& a_): a(a_) {}
 i64 sum_smallest(int l, int r, int k) const {
  vector<int> w(a.begin() + l, a.begin() + r);
  sort(w.begin(), w.end());
  i64 s= 0;
  for(int j= 0; j < k; ++j) s+= w[j];
  return s;
 }
};
