#pragma once
// pos_cm の値の順位を、上位 16 bit の表 (problems/_shared/wavelet/wm.hpp の Table) で区間に狭めてから lower_cm で探す形。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 vector<int> vals, start, pos;
 wavelet::Table tbl;
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
  tbl= wavelet::Table(vals);
 }
 int freq(int l, int r, int x) const {
  const int id= tbl.lower_cm(x);
  if(id == int(vals.size()) || vals[id] != x) return 0;
  const int *b= pos.data() + start[id], m= start[id + 1] - start[id];
  return wavelet::lower_cm(b, m, r) - wavelet::lower_cm(b, m, l);
 }
};
