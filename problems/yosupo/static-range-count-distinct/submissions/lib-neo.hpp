#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp) を、同じ値が前に出た位置 + 1 の列の上に作り、
// [l, r) のうち l + 1 未満のものを count_lt で数える。前に出た位置は NeoLibrary の HashMap で引く。
#include "pj.hpp"
#include "neo/data_structure/HashMap.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 static vector<int> prev_plus_one(const vector<int>& a) {
  HashMap<int> last;
  vector<int> p(a.size());
  for(size_t i= 0; i < a.size(); ++i) {
   int& v= last[a[i]];
   p[i]= v, v= i + 1;
  }
  return p;
 }
 explicit Solver(const vector<int>& a): wm(prev_plus_one(a)) {}
 int count_distinct(int l, int r) const { return wm.count_lt(l, r, l + 1); }
};
