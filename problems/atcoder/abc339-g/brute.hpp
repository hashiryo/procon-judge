#pragma once
// abc339-g の愚直解。区間の値を 1 つずつ見て x 以下のものを足す。区間を木やバケットに分けないので、提出とは別の
// 考え方になる。O(NQ) なので小さい入力でだけ使う (pj testdata crosscheck --brute brute.hpp)。
#include "pj.hpp"
struct Solver {
 vector<int> a;
 explicit Solver(const vector<int>& a_): a(a_) {}
 i64 sum_le(int l, int r, int x) const {
  i64 s= 0;
  for(int i= l; i < r; ++i) s+= a[i] <= x ? a[i] : 0;
  return s;
 }
};
