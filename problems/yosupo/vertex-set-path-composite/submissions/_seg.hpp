#pragma once
#include "_hld.hpp"
// HLD の位置に 1 次関数を置いたセグメント木。道の区間ごとに、u 側の上る区間は位置の大きい方から、v 側の下りる区間は
// 位置の小さい方から、値 x に直接当てる。木は葉を [n, 2n) に置く非再帰の形 (n は 2 の冪に揃えない) で、区間の節は
// point-set-range-composite の query_bl_apply と同じく分岐なしで集めてから当てる。値は Montgomery (_mont.hpp)。
//   Seg2: 前向きの木と、位置を逆に並べた木の 2 本。上る区間は逆の木の前向きの当て方で済む。1 点の更新は 2 本とも直す。
//   Seg1: 1 本の木の節に、掛ける数 m と、前向きの足す数 af と後ろ向きの足す数 ab を持つ (1 次関数は合成の向きで
//         掛ける数が変わらない)。合成は掛け算 3 回。
namespace comp_seg {
struct F {
 u32 m, a;
};
inline F op(F l, F r) { return {mont::mul(r.m, l.m), mont::mulsum(r.m, l.a, r.a, mont::R1)}; }  // l を先に当てる
inline u32 apply(u32 m, u32 a, u32 x) { return mont::mulsum(m, x, a, mont::R1); }
// 前向きの木。fwd(l, r, x) は閉区間 [l, r] を l から r の順に x へ当てる。
struct Seg {
 int n;
 HugeArray<F> d;
 Seg()= default;
 Seg(int n, const F* leaf): n(n), d(2 * size_t(n)) {
  for(int i= 0; i < n; ++i) d[n + i]= leaf[i];
  for(int k= n - 1; k >= 1; --k) d[k]= op(d[2 * k], d[2 * k + 1]);
 }
 void set(int p, F f) {
  p+= n, d[p]= f;
  for(p>>= 1; p >= 1; p>>= 1) d[p]= op(d[p << 1], d[p << 1 | 1]);
 }
 u32 fwd(int l, int r, u32 x) const {
  int L[32], R[32], nl= 0, nr= 0;
  for(l+= n, r+= n + 1; l < r; l>>= 1, r>>= 1) {
   L[nl]= l, nl+= l & 1, l+= l & 1;
   R[nr]= r - 1, nr+= r & 1, r-= r & 1;
  }
  for(int i= 0; i < nl; ++i) x= apply(d[L[i]].m, d[L[i]].a, x);
  while(nr) --nr, x= apply(d[R[nr]].m, d[R[nr]].a, x);
  return x;
 }
};
struct G {
 u32 m, af, ab, pad;
};
// l が位置の小さい側。前向きは l を先に、後ろ向きは r を先に当てる。
inline G op2(const G& l, const G& r) { return {mont::mul(l.m, r.m), mont::mulsum(r.m, l.af, r.af, mont::R1), mont::mulsum(l.m, r.ab, l.ab, mont::R1), 0}; }
struct Seg1 {
 int n;
 HugeArray<G> d;
 Seg1()= default;
 Seg1(int n, const F* leaf): n(n), d(2 * size_t(n)) {
  for(int i= 0; i < n; ++i) d[n + i]= {leaf[i].m, leaf[i].a, leaf[i].a, 0};
  for(int k= n - 1; k >= 1; --k) d[k]= op2(d[2 * k], d[2 * k + 1]);
 }
 void set(int p, F f) {
  p+= n, d[p]= {f.m, f.a, f.a, 0};
  for(p>>= 1; p >= 1; p>>= 1) d[p]= op2(d[p << 1], d[p << 1 | 1]);
 }
 // [l, r] を l から r の順に当てる。左の節は見つけた順、右の節は見つけた逆順。
 u32 fwd(int l, int r, u32 x) const {
  int L[32], R[32], nl= 0, nr= 0;
  for(l+= n, r+= n + 1; l < r; l>>= 1, r>>= 1) {
   L[nl]= l, nl+= l & 1, l+= l & 1;
   R[nr]= r - 1, nr+= r & 1, r-= r & 1;
  }
  for(int i= 0; i < nl; ++i) x= apply(d[L[i]].m, d[L[i]].af, x);
  while(nr) --nr, x= apply(d[R[nr]].m, d[R[nr]].af, x);
  return x;
 }
 // [l, r] を r から l の順に当てる。右の節は見つけた順、左の節は見つけた逆順。
 u32 bwd(int l, int r, u32 x) const {
  int L[32], R[32], nl= 0, nr= 0;
  for(l+= n, r+= n + 1; l < r; l>>= 1, r>>= 1) {
   L[nl]= l, nl+= l & 1, l+= l & 1;
   R[nr]= r - 1, nr+= r & 1, r-= r & 1;
  }
  for(int i= 0; i < nr; ++i) x= apply(d[R[i]].m, d[R[i]].ab, x);
  while(nl) --nl, x= apply(d[L[nl]].m, d[L[nl]].ab, x);
  return x;
 }
};
inline vector<F> leaves(const comp_hld::Tree& t, const vector<array<i64, 2>>& f) {
 vector<F> v(t.n);
 for(int u= 0; u < t.n; ++u) v[t.pos[u]]= {mont::enc(f[u][0]), mont::enc(f[u][1])};
 return v;
}
}
