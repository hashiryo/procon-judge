#pragma once
// 点更新と全体の積だけを持つセグメント木 (葉の数は 2 の冪に切り上げ、余りは単位元で埋める)。左から順に掛ける。
#include "../common.hpp"
template <class Mat> struct Seg {
 int sz;
 vector<Mat> t;
 template <class F> Seg(int n, const Mat& id, const F& leaf): sz(1) {
  while(sz < n) sz<<= 1;
  t.assign(2 * sz, id);
  for(int i= 0; i < n; ++i) t[sz + i]= leaf(i);
  for(int i= sz - 1; i >= 1; --i) t[i]= t[2 * i] * t[2 * i + 1];
 }
 void set(int i, const Mat& m) {
  i+= sz, t[i]= m;
  for(i>>= 1; i; i>>= 1) t[i]= t[2 * i] * t[2 * i + 1];
 }
 const Mat& all() const { return t[1]; }
};
