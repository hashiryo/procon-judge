#pragma once
// 期待出力を作る参照実装 (提出としても測る)。マージソート木で、位置のセグメント木の各節点に、その範囲の値を
// 並べた列と累積を持つ。[l, r) を O(log N) 個の節点に分け、各節点で x 以下の値の数を二分探索して累積を足す
// (1 クエリ O(log^2 N))。wavelet matrix も Library も使わない。brute.hpp と小さい入力で突き合わせてある
// (pj testdata crosscheck --brute brute.hpp)。
#include <algorithm>
#include "pj.hpp"
struct Solver {
 int n= 1;
 vector<vector<int>> v;
 vector<vector<i64>> acc;
 explicit Solver(const vector<int>& a) {
  while(n < int(a.size())) n*= 2;
  v.assign(2 * n, {}), acc.assign(2 * n, {});
  for(size_t i= 0; i < a.size(); ++i) v[n + i]= {a[i]};
  for(int i= n - 1; i >= 1; --i) merge(v[2 * i].begin(), v[2 * i].end(), v[2 * i + 1].begin(), v[2 * i + 1].end(), back_inserter(v[i]));
  for(int i= 1; i < 2 * n; ++i) {
   acc[i].assign(v[i].size() + 1, 0);
   for(size_t j= 0; j < v[i].size(); ++j) acc[i][j + 1]= acc[i][j] + v[i][j];
  }
 }
 i64 node(int i, int x) const { return acc[i][upper_bound(v[i].begin(), v[i].end(), x) - v[i].begin()]; }
 i64 sum_le(int l, int r, int x) const {
  i64 s= 0;
  for(l+= n, r+= n; l < r; l>>= 1, r>>= 1) {
   if(l & 1) s+= node(l++, x);
   if(r & 1) s+= node(--r, x);
  }
  return s;
 }
};
