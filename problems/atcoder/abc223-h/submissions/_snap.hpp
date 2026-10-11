#pragma once
// 接頭辞の基底の写しを B 個おきにだけ持つ。問い合わせでは、r を切り下げた位置の写しから、残りの元 (B - 1 個以下) を
// 足し直してから調べる。メモリは写しを全部持つ形の B 分の 1 で、問い合わせのたびに挿入が増える。
#include "_basis.hpp"
template <int B> struct SnapSolver {
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
  PrefixBasis cur= pre[k];
  for(int i= k * B; i < r; ++i) cur.insert(a[i], i);
  return cur.contains(l, x);
 }
};
