#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp) を、同じ値が前に出た位置 + 1 の列の上に作り、
// [l, r) のうち l + 1 未満のものを count_lt で数える。前に出た位置は、NeoLibrary の compress (neo/sort/compress.hpp) で値を
// 順位に詰めてから、順位ごとの最後の位置の配列で引く (wm4 の _prev.hpp と同じ形)。HashMap で引いた形 (run 40477233432) とほぼ
// 同じ時間で、wm4 との 2 割ほどの差は残った (run 40482922246、どちらも 7763 の x64-gcc)。wm4 は前に出た位置の列 (0 以上 N 以下) を
// 順位に詰めずに使い、l + 1 も順位として渡すが、WaveletMatrix は中で値を詰め直し、クエリのたびに値から順位を引くため。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
#include "neo/sort/compress.hpp"
struct Solver {
 WaveletMatrix<int> wm;
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
