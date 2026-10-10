#pragma once
// 法が合成数でもよい行列式。要を 8 本ずつまとめて掃き出す核 (_shared/linalg/lazy_elim.hpp の update_row) を使う。
// 列に単元 (法と互いに素な値) があればそれを要にして、体のときと同じように掛ける数を貯める。単元がなければ、貯めている
// 更新を先に足してから、その列の値が 0 でない行どうしに互除法の 2×2 の変換 (行列式 1) をかけて 1 行に集め、その行を
// 要にする (要の値は単元でなくてよい)。値が 1 つだけの列は、変換なしでそのまま要にする。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline u32 run(int n, u32 mod, const vector<vector<u32>>& a) {
 if(mod == 1) return 0;
 using Z= ZMod<0>;
 using lazy_elim::K;
 Z::set_mod(mod);
 const int w= (n + 3) & ~3;
 const u64 R= (u64(1) << 32) % mod;
 vector<u64> A((size_t)n * w);
 for(int i= 0; i < n; ++i) copy(a[i].begin(), a[i].end(), &A[(size_t)i * w]);
 vector<int> rem(n), prow;
 iota(rem.begin(), rem.end(), 0);
 vector<u32> F((size_t)n * K), v(n);
 vector<u64> U((size_t)K * w), zero(w);
 const u64* Ub[K];
 Z d= Z::raw(1);
 int bt= 0;
 // 貯めた bt 本の要の更新を、残りの行の列 [col を 4 の倍数に切り下げた位置, w) に足す
 auto flush= [&](int col) {
  if(!bt) return;
  for(int s= bt; s < K; ++s) Ub[s]= zero.data();
  for(int r : rem) {
   u32* fr= &F[(size_t)r * K];
   u32 any= 0;
   for(int s= 0; s < K; ++s) {
    if(s >= bt) fr[s]= 0;
    any|= fr[s];
   }
   if(any) lazy_elim::update_row(&A[(size_t)r * w], Ub, fr, col & ~3, w, R);
  }
  bt= 0;
 };
 for(int col= 0; col < n; ++col) {
  const int nr= rem.size();
  int pi= -1, nz= -1;
  for(int i= 0; i < nr; ++i) {
   const int r= rem[i];
   const u32* fr= &F[(size_t)r * K];
   u64 x= A[(size_t)r * w + col];
   for(int s= 0; s < bt; ++s) x+= u64(fr[s]) * Ub[s][col];
   v[i]= Z(x).val();
   if(v[i] && nz < 0) nz= i;
   if(pi < 0 && v[i] && gcd(v[i], mod) == 1) pi= i;
  }
  if(nz < 0) return 0;
  if(pi < 0) {
   // 単元がない列。貯めた更新を足してから、値が 0 でない行を互除法で行 p に集める
   flush(col);
   const int p= rem[nz];
   u64* P= &A[(size_t)p * w];
   for(int k= col; k < w; ++k) P[k]= Z(P[k]).val();
   for(int i= nz + 1; i < nr; ++i) {
    if(!v[i]) continue;
    u64* J= &A[(size_t)rem[i] * w];
    for(int k= col; k < w; ++k) J[k]= Z(J[k]).val();
    // x a + y b = g となる整数 x, y を求め、(行 p, 行 j) に [[x, y], [-b/g, a/g]] を掛ける
    i64 g= P[col], b= J[col], x0= 1, x1= 0, y0= 0, y1= 1;
    while(b) {
     const i64 q= g / b;
     g-= q * b, swap(g, b);
     x0-= q * x1, swap(x0, x1);
     y0-= q * y1, swap(y0, y1);
    }
    auto md= [&](i64 t) {
     const i64 r= t % (i64)mod;
     return u64(r < 0 ? r + mod : r);
    };
    const u64 cx= md(x0), cy= md(y0), cu= md(-(i64)J[col] / g), cv= md((i64)P[col] / g);
    for(int k= col; k < w; ++k) {
     const u64 pk= P[k], jk= J[k];
     P[k]= Z(cx * pk + cy * jk).val(), J[k]= Z(cu * pk + cv * jk).val();
    }
   }
   d*= Z::raw(u32(P[col]));
   if(d == Z()) return 0;
   prow.push_back(p);
   rem[nz]= rem.back(), rem.pop_back();
   continue;
  }
  const int p= rem[pi];
  u64* up= &U[(size_t)bt * w];
  const u32* fp= &F[(size_t)p * K];
  const u64* ap= &A[(size_t)p * w];
  for(int j= col & ~3; j < w; ++j) {
   u64 x= ap[j];
   for(int s= 0; s < bt; ++s) x+= u64(fp[s]) * Ub[s][j];
   up[j]= Z(x).val();
  }
  const Z iv= Z::raw(v[pi]).inv();
  for(int i= 0; i < nr; ++i) F[(size_t)rem[i] * K + bt]= (-(Z::raw(v[i]) * iv)).val();
  d*= Z::raw(v[pi]);
  prow.push_back(p);
  Ub[bt++]= up;
  rem[pi]= rem.back(), rem.pop_back();
  if(bt == K) flush(col + 1);
 }
 // 要にした行の並び (t 番目の要が行 prow[t]) の置換の符号
 vector<char> seen(n);
 int cyc= 0;
 for(int i= 0; i < n; ++i)
  if(!seen[i]) {
   ++cyc;
   for(int j= i; !seen[j]; j= prow[j]) seen[j]= 1;
  }
 if((n - cyc) & 1) d= -d;
 return d.val();
}
