#pragma once
// 接頭辞の基底の写しを B 個おきにだけ持つ。問い合わせでは、r を切り下げた位置の写しから、残りの元 (B - 1 個以下) を
// 足し直してから調べる。メモリは写しを全部持つ形の B 分の 1 で、問い合わせのたびに挿入が増える。
#include "_basis.hpp"
// RED なら、足し直しをせずに判定する。写しの欄のうち位置が l 以上のもの (区間 [l, kB) の基底) で、x と残りの元
// a[max(kB, l)..r) を上の bit から min の形で消す。消したあとの x と残りの元は写しの要の bit をどれも持たないので、
// x が区間の元の和で書けるのは、消したあとの x が消したあとの残りの元の和で書けるときに限る。残りの元 (B - 1 個以下) は、
// 入った順に min の形で互いに消してから x に当てる。
template <int B, bool MIN= false, bool RED= false> struct SnapSolver {
 const vector<u64>& a;
 vector<PrefixBasis> pre;  // pre[k] は a[0..kB) の基底
 explicit SnapSolver(const vector<u64>& a): a(a), pre(a.size() / B + 1) {
  PrefixBasis cur;
  cur.clear();
  pre[0]= cur;
  for(size_t r= 0; r < a.size(); ++r) {
   cur.insert(a[r], int(r));
   if((r + 1) % B == 0) pre[(r + 1) / B]= cur;
  }
 }
 bool query(int l, int r, u64 x) const {
  const int k= r / B;
  if constexpr(RED) {
   const PrefixBasis& S= pre[k];
   u64 e[B];
   int ne= 0;
   for(int i= max(k * B, l); i < r; ++i) e[ne++]= a[i];
   for(int i= 59; i >= 0; --i) {
    const u64 m= S.p[i] >= l ? S.b[i] : 0;
    x= min(x, x ^ m);
    for(int t= 0; t < ne; ++t) e[t]= min(e[t], e[t] ^ m);
   }
   for(int t= 0; t < ne; ++t) {
    for(int q= 0; q < t; ++q) e[t]= min(e[t], e[t] ^ e[q]);
    x= min(x, x ^ e[t]);
   }
   return x == 0;
  }
  PrefixBasis cur= pre[k];
  for(int i= k * B; i < r; ++i) cur.insert(a[i], i);
  if constexpr(MIN) return cur.contains_min(l, x);
  else return cur.contains(l, x);
 }
};
