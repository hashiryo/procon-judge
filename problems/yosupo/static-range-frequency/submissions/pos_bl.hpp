#pragma once
// pos と同じく値ごとの位置の列を持ち、3 回の二分探索を、マスクを通して分岐なしに固定した lower_bl で行う (pos の
// lower は GCC 15 だと cmov、clang だと分岐になり、x64-clang で 2.5 倍遅かった)。
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
  const int id= wavelet::lower_bl(vals.data(), vals.size(), x);
  if(id == int(vals.size()) || vals[id] != x) return 0;
  const int *b= pos.data() + start[id], m= start[id + 1] - start[id];
  return wavelet::lower_bl(b, m, r) - wavelet::lower_bl(b, m, l);
 }
};
