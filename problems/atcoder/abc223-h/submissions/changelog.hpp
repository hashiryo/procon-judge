#pragma once
// 接頭辞の基底の bit ごとの変更の履歴を持つ。bit i の欄 (b[i], p[i]) が変わるたびに、(変わった時刻, 新しい値) を
// その bit の列の末尾に足す。問い合わせ (l, r) では、x の最上位の bit ごとに、時刻 r 以下で最後の変更を二分探索で引く。
// メモリは変更の回数に比例する。
#include "_basis.hpp"
struct Solver {
 struct Entry {
  int t;  // a[0..t) を入れ終えたときの値
  int p;
  u64 b;
 };
 vector<Entry> log[60];
 explicit Solver(const vector<u64>& a) {
  PrefixBasis cur;
  cur.clear();
  for(size_t r= 0; r < a.size(); ++r) {
   u64 x= a[r];
   int pos= int(r);
   const int t= int(r) + 1;
   while(x) {
    const int i= 63 - __builtin_clzll(x);
    if(!cur.b[i]) {
     cur.b[i]= x, cur.p[i]= pos;
     log[i].push_back({t, pos, x});
     break;
    }
    if(cur.p[i] < pos) {
     swap(cur.b[i], x), swap(cur.p[i], pos);
     log[i].push_back({t, cur.p[i], cur.b[i]});
    }
    x^= cur.b[i];
   }
  }
 }
 bool query(int l, int r, u64 x) const {
  while(x) {
   const int i= 63 - __builtin_clzll(x);
   const auto& v= log[i];
   // 時刻 r 以下で最後の変更
   int lo= 0, hi= int(v.size());
   while(lo < hi) {
    const int mid= (lo + hi) / 2;
    if(v[mid].t <= r) lo= mid + 1;
    else hi= mid;
   }
   if(lo == 0) return false;
   const Entry& e= v[lo - 1];
   if(e.p < l) return false;
   x^= e.b;
  }
  return true;
 }
};
