#pragma once
// 書き比べ: 1 点への加算と部分集合の和の問いの構造体 (ds1、ds2 が読む)。bit を下の L 個と上の残りに分け、
// t[h][l] = sum_{l' ⊆ l} a[h][l'] を持つ (下の bit だけゼータ変換)。加算は 2^{L - popcount(x の下)}、問いは 2^{popcount(x の上)}。
// SS_L は L の決め方で、0 なら n / 2、1 なら 2n / 3 (問いを軽く、加算を重くする)。上位集合は、使う側で添字を補集合にする。
// 比べ終えたら消す。
#include "pj.hpp"
#ifndef SS_L
#define SS_L 0
#endif
template <class T> struct SubsetSumDyn {
 int n, L;
 u32 lm;
 vector<T> t;
 explicit SubsetSumDyn(int n_): n(n_), L(SS_L == 0 ? n_ / 2 : (2 * n_) / 3), lm((u32(1) << L) - 1), t(size_t(1) << n_) {}
 void add(u32 x, T v) {
  T* row= t.data() + (size_t(x >> L) << L);
  const u32 lo= x & lm, fr= lm ^ lo;
  for(u32 s= fr;; s= (s - 1) & fr) {
   row[lo | s]+= v;
   if(!s) break;
  }
 }
 T sum(u32 x) const {
  const u32 h= x >> L, lo= x & lm;
  T r= T();
  for(u32 s= h;; s= (s - 1) & h) {
   r+= t[(size_t(s) << L) | lo];
   if(!s) break;
  }
  return r;
 }
};
struct Solver {
 int k;
 vector<int> a;
 vector<pair<int, int>> ops;
 vector<int> ans;
 Solver(int k_, const vector<int>& a_, const vector<pair<int, int>>& ops_): k(k_), a(a_), ops(ops_) {}
 void run() {
  const u32 full= (u32(1) << k) - 1;
  SubsetSumDyn<int> st(k);  // 補集合 ~v を数える (上位集合の個数 = 補集合の部分集合の個数)
  for(int v: a) st.add(full ^ u32(v), 1);
  ans.reserve(ops.size());
  for(auto [t, v]: ops) {
   if(t == 1) st.add(full ^ u32(v), 1);
   else if(t == 2) st.add(full ^ u32(v), -1);
   else {
    u32 m= 0;
    for(int b= k - 1; b >= 0; --b)
     if(st.sum(full ^ (m | (u32(1) << b))) >= v) m|= u32(1) << b;
    ans.push_back(int(m));
   }
  }
 }
 vector<int> answer() const { return ans; }
};
