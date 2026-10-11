#pragma once
// 値ごとに、出てくる位置を昇順に並べる (値の順位で区切った 1 本の配列)。x の順位を二分探索で求め、その値の位置の
// うち [l, r) に入るものの数を、もう 2 回の二分探索で数える。wavelet matrix と比べる相手で、値が 1 つに決まる問い
// 合わせならこれで足りる。(値, 添字) の並べ替えは wm と同じ基数ソート。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 vector<int> vals, start, pos;
 explicit Solver(const vector<int>& a) {
  const int n= a.size();
  const auto k= wavelet::sorted_keys(a);
  pos.resize(n);
  for(int j= 0; j < n; ++j) {
   const int v= int(k[j] >> 32);
   if(vals.empty() || vals.back() != v) vals.push_back(v), start.push_back(j);
   pos[j]= int(unsigned(k[j]));
  }
  start.push_back(n);
 }
 int freq(int l, int r, int x) const {
  const int id= wavelet::lower(vals, x);
  if(id == int(vals.size()) || vals[id] != x) return 0;
  const int *b= pos.data() + start[id], m= start[id + 1] - start[id];
  return wavelet::lower(b, m, r) - wavelet::lower(b, m, l);
 }
};
