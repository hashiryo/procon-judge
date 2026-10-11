#pragma once
// pos と同じく値ごとの位置の列を持ち、3 回の二分探索を lower_cm (clang にも cmov を選ばせる形) で行う。
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
  const int id= wavelet::lower_cm(vals.data(), vals.size(), x);
  if(id == int(vals.size()) || vals[id] != x) return 0;
  const int *b= pos.data() + start[id], m= start[id + 1] - start[id];
  return wavelet::lower_cm(b, m, r) - wavelet::lower_cm(b, m, l);
 }
};
