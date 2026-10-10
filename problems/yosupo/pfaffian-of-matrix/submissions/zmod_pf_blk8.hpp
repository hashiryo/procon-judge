#pragma once
// 交代行列のパフィアンを、要を 8 本ずつまとめて掃き出す核 (_shared/linalg/lazy_elim.hpp の update_row) で求める。法は 998244353。
// 手順は naive と同じで、列 c の要を位置 c + 1 の行から選び (行と、同じ番号の列を入れ替えると符号が変わる)、c が偶数なら
// -(要の値) を掛け、位置 c + 2 以降の行を消す。位置 0 の行は使わない。行の更新は 8 本ずつ貯めて足すので、列を入れ替えるときは、
// 残りの行と、貯めている要の行の両方で入れ替える。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline u32 run(int N, const vector<vector<u32>>& M_in) {
 using Z= ZMod<MOD>;
 using lazy_elim::K;
 const int n= 2 * N, w= (n + 3) & ~3;
 const u64 R= (u64(1) << 32) % MOD;
 vector<u64> A((size_t)n * w);
 for(int i= 0; i < n; ++i) copy(M_in[i].begin(), M_in[i].end(), &A[(size_t)i * w]);
 vector<int> pos(n);  // pos[t] は位置 t にある行
 iota(pos.begin(), pos.end(), 0);
 vector<u32> F((size_t)n * K), v(n);
 vector<u64> U((size_t)K * w), zero(w);
 const u64* Ub[K];
 Z res= Z::raw(1);
 int bt= 0, first= 1;  // first は残りの行の最初の位置
 auto flush= [&](int col) {
  if(!bt) return;
  for(int s= bt; s < K; ++s) Ub[s]= zero.data();
  for(int t= first; t < n; ++t) {
   u32* fr= &F[(size_t)pos[t] * K];
   u32 any= 0;
   for(int s= 0; s < K; ++s) {
    if(s >= bt) fr[s]= 0;
    any|= fr[s];
   }
   if(any) lazy_elim::update_row(&A[(size_t)pos[t] * w], Ub, fr, col & ~3, w, R);
  }
  bt= 0;
 };
 for(int c= 0; c + 1 < n; ++c) {
  int sel= -1;
  for(int t= first; t < n; ++t) {
   const int r= pos[t];
   const u32* fr= &F[(size_t)r * K];
   u64 x= A[(size_t)r * w + c];
   for(int s= 0; s < bt; ++s) x+= u64(fr[s]) * Ub[s][c];
   v[t]= Z(x).val();
   if(sel < 0 && v[t]) sel= t;
  }
  if(sel < 0) return 0;
  if(sel != c + 1) {
   swap(pos[c + 1], pos[sel]), swap(v[c + 1], v[sel]);
   for(int t= c + 1; t < n; ++t) {
    u64* row= &A[(size_t)pos[t] * w];
    swap(row[c + 1], row[sel]);
   }
   for(int s= 0; s < bt; ++s) swap(U[(size_t)s * w + c + 1], U[(size_t)s * w + sel]);
   res= -res;
  }
  const int p= pos[c + 1];
  if(!(c & 1)) res*= -Z::raw(v[c + 1]);
  u64* up= &U[(size_t)bt * w];
  const u32* fp= &F[(size_t)p * K];
  const u64* ap= &A[(size_t)p * w];
  for(int j= c & ~3; j < w; ++j) {
   u64 x= ap[j];
   for(int s= 0; s < bt; ++s) x+= u64(fp[s]) * Ub[s][j];
   up[j]= Z(x).val();
  }
  const Z iv= Z::raw(v[c + 1]).inv();
  for(int t= c + 2; t < n; ++t) F[(size_t)pos[t] * K + bt]= (-(Z::raw(v[t]) * iv)).val();
  Ub[bt++]= up;
  first= c + 2;
  if(bt == K) flush(c + 1);
 }
 return res.val();
}
