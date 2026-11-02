#pragma once
// NeoLibrary の WaveletMatrix<int, false> (neo/data_structure/WaveletMatrix.hpp、値を詰め直さずにそのまま順位として使う形) を、
// 同じ値が前に出た位置 + 1 の列 (0 以上 N 以下) の上に作り、[l, r) のうち l + 1 未満のものを count_lt で数える。前に出た位置は、
// NeoLibrary の compress (neo/sort/compress.hpp) で値を順位に詰めてから、順位ごとの最後の位置の配列で引く (wm4 の _prev.hpp と
// 同じ形)。値を詰め直す WaveletMatrix<int> の形は、7763 の x64-gcc で wm4 より 2 割ほど遅かった (run 40482922246)。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
#include "neo/sort/compress.hpp"
struct Solver {
 WaveletMatrix<int, false> wm;
 static vector<int> prev_plus_one(const vector<int>& a) {
  vector<int> b(a);
  vector<int> last(compress(b).size(), 0), p(a.size());
  for(size_t i= 0; i < a.size(); ++i) {
   int& v= last[b[i]];
   p[i]= v, v= i + 1;
  }
  return p;
 }
 explicit Solver(const vector<int>& a): wm(prev_plus_one(a)) {}
 int count_distinct(int l, int r) const { return wm.count_lt(l, r, l + 1); }
};
