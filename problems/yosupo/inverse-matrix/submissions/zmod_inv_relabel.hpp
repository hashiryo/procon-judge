#pragma once
// zmod_inv_blk8t の前進消去で、[A | I] の右半分の 0 の部分を更新しない形。法は 998244353。
// 右半分の列は「行 r の単位行列の 1 の位置」として付け替える。行を t 番目の要にするとき、その行の 1 を位置 t に移し、
// 位置 t にあった行の 1 を空いた位置に移す (その 2 つの位置は、まだ持ち主の行にしか値がない)。すると要の行の右半分は位置 t
// 以下にしか値がなく、残りの行の更新は列 [今の列, n + 要の数) で済む。前進消去の手間は 5n^3/6 から n^3/2 に減る。
// 後退代入は tri_solve_blk で、X の位置 q の列は A^{-1} の列 (位置 q に 1 を持っていた元の行) になる。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline InverseResult run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 using namespace lazy_elim;
 const int w= (2 * n + 3) & ~3;
 const u64 R= (u64(1) << 32) % MOD;
 Elim<Z> e(n, w);
 for(int i= 0; i < n; ++i) copy(a[i].begin(), a[i].end(), e.row(i)), e.row(i)[n + i]= 1;
 e.U.assign((size_t)n * w, 0);
 vector<u32> F((size_t)n * K), v(n);
 vector<u64> zero(w);
 vector<int> pos2row(n), row2pos(n);
 iota(pos2row.begin(), pos2row.end(), 0), iota(row2pos.begin(), row2pos.end(), 0);
 const u64* Ub[K];
 for(int col= 0; col < n;) {
  int bt= 0;
  for(; bt < K && col < n; ++col) {
   const int nr= e.rem.size();
   int pi= -1;
   for(int i= 0; i < nr; ++i) {
    const int r= e.rem[i];
    const u32* fr= &F[(size_t)r * K];
    u64 x= e.a[(size_t)r * w + col];
    for(int s= 0; s < bt; ++s) x+= u64(fr[s]) * Ub[s][col];
    v[i]= Z(x).val();
    if(pi < 0 && v[i]) pi= i;
   }
   if(pi < 0) return {false, {}};
   const int p= e.rem[pi], t= e.prow.size(), q= row2pos[p];
   if(q != t) {  // 行 p の 1 を位置 t に、位置 t にあった行 r2 の 1 を位置 q に移す
    const int r2= pos2row[t];
    u64 *ap= e.row(p), *ar= e.row(r2);
    ap[n + q]= 0, ap[n + t]= 1, ar[n + t]= 0, ar[n + q]= 1;
    pos2row[q]= r2, row2pos[r2]= q, pos2row[t]= p, row2pos[p]= t;
   }
   u64* up= &e.U[(size_t)t * w];
   const u32* fp= &F[(size_t)p * K];
   const u64* ap= e.row(p);
   const int je= min(w, (n + t + 1 + 3) & ~3);
   for(int j= col & ~3; j < je; ++j) {
    u64 x= ap[j];
    for(int s= 0; s < bt; ++s) x+= u64(fp[s]) * Ub[s][j];
    up[j]= Z(x).val();
   }
   const Z iv= Z::raw(v[pi]).inv();
   for(int i= 0; i < nr; ++i) F[(size_t)e.rem[i] * K + bt]= (-(Z::raw(v[i]) * iv)).val();
   e.prow.push_back(p), e.pcol.push_back(col), e.pval.push_back(v[pi]);
   Ub[bt++]= up;
   e.rem[pi]= e.rem.back(), e.rem.pop_back();
  }
  for(int s= bt; s < K; ++s) Ub[s]= zero.data();
  const int jb= col & ~3, je= min(w, (n + (int)e.prow.size() + 3) & ~3);
  for(int r : e.rem) {
   u32* fr= &F[(size_t)r * K];
   for(int s= bt; s < K; ++s) fr[s]= 0;
   u32 any= 0;
   for(int s= 0; s < K; ++s) any|= fr[s];
   if(any) update_row(e.row(r), Ub, fr, jb, je, R);
  }
 }
 vector<int> cols(n);
 iota(cols.begin(), cols.end(), n);
 int wx;
 const auto X= tri_solve_blk(e, cols, wx);
 InverseResult res{true, vector<vector<u32>>(n, vector<u32>(n))};
 for(int t= 0; t < n; ++t)
  for(int q= 0; q < n; ++q) res.mat[t][pos2row[q]]= u32(X[(size_t)t * wx + q]);
 return res;
}
