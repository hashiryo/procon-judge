#pragma once
// 重み平衡木 (weight-balanced tree)。k 番目と個数を引くために持つ部分木の大きさを、そのまま釣り合いの判定にも使う。
// 重みを大きさ + 1 として、左右の重みの比が 3 を超えたら回転で直す (Δ = 3、Γ = 2)。節点は {キー, 大きさ, 左右の子}
// の 16 byte で、1 本の vector に並べて添字でつなぐ。入れるときと除くときは再帰で下りて、戻りながら大きさを直して
// 釣り合いを見る。どちらも先に入っているかを調べる。除いた節点の位置は使い回す。初期集合は真ん中を根にして O(N) で組む。
#include <vector>
namespace os_wbt {
struct Node {
 int key, sz, l, r;
};
struct Set {
 std::vector<Node> t;
 std::vector<int> fr;  // 空いた節点
 int root;
 int w(int x) const { return t[x].sz + 1; }
 int make(int key) {
  if(!fr.empty()) {
   const int x= fr.back();
   fr.pop_back();
   t[x]= Node{key, 1, 0, 0};
   return x;
  }
  t.push_back(Node{key, 1, 0, 0});
  return int(t.size()) - 1;
 }
 int build(const std::vector<int>& a, int lo, int hi) {
  if(lo == hi) return 0;
  const int m= (lo + hi) / 2, x= make(a[m]);
  const int l= build(a, lo, m), r= build(a, m + 1, hi);
  t[x].l= l, t[x].r= r, t[x].sz= hi - lo;
  return x;
 }
 explicit Set(const std::vector<int>& a) {
  t.reserve(a.size() + 1);
  t.push_back(Node{0, 0, 0, 0});
  root= build(a, 0, a.size());
 }
 int rotl(int x) {
  const int y= t[x].r;
  t[x].r= t[y].l, t[y].l= x;
  t[y].sz= t[x].sz, t[x].sz= t[t[x].l].sz + t[t[x].r].sz + 1;
  return y;
 }
 int rotr(int x) {
  const int y= t[x].l;
  t[x].l= t[y].r, t[y].r= x;
  t[y].sz= t[x].sz, t[x].sz= t[t[x].l].sz + t[t[x].r].sz + 1;
  return y;
 }
 int balance(int x) {
  const int wl= w(t[x].l), wr= w(t[x].r);
  if(wr > 3 * wl) {
   const int y= t[x].r;
   if(w(t[y].l) >= 2 * w(t[y].r)) t[x].r= rotr(y);
   return rotl(x);
  }
  if(wl > 3 * wr) {
   const int y= t[x].l;
   if(w(t[y].r) >= 2 * w(t[y].l)) t[x].l= rotl(y);
   return rotr(x);
  }
  return x;
 }
 int ins(int x, int k) {
  if(!x) return make(k);
  ++t[x].sz;
  if(k < t[x].key) {
   const int c= ins(t[x].l, k);
   t[x].l= c;
  } else {
   const int c= ins(t[x].r, k);
   t[x].r= c;
  }
  return balance(x);
 }
 // いちばん左の節点を外して m に入れる。
 int del_min(int x, int& m) {
  if(!t[x].l) return m= x, t[x].r;
  --t[x].sz;
  t[x].l= del_min(t[x].l, m);
  return balance(x);
 }
 int del(int x, int k) {
  if(t[x].key == k) {
   fr.push_back(x);
   if(!t[x].l) return t[x].r;
   if(!t[x].r) return t[x].l;
   int m;
   const int r= del_min(t[x].r, m);
   t[m].l= t[x].l, t[m].r= r, t[m].sz= t[x].sz - 1;
   return balance(m);
  }
  --t[x].sz;
  if(k < t[x].key) t[x].l= del(t[x].l, k);
  else t[x].r= del(t[x].r, k);
  return balance(x);
 }
 bool contains(int k) const {
  for(int x= root; x;) {
   if(t[x].key == k) return true;
   x= k < t[x].key ? t[x].l : t[x].r;
  }
  return false;
 }
 void insert(int k) {
  if(!contains(k)) root= ins(root, k);
 }
 void erase(int k) {
  if(contains(k)) root= del(root, k);
 }
 int kth(int k) const {
  if(k >= t[root].sz) return -1;
  for(int x= root;;) {
   const int ls= t[t[x].l].sz;
   if(k < ls) x= t[x].l;
   else if(k == ls) return t[x].key;
   else k-= ls + 1, x= t[x].r;
  }
 }
 int count_le(int k) const {
  int c= 0;
  for(int x= root; x;)
   if(t[x].key <= k) c+= t[t[x].l].sz + 1, x= t[x].r;
   else x= t[x].l;
  return c;
 }
 int prev(int k) const {
  int r= -1;
  for(int x= root; x;)
   if(t[x].key <= k) r= t[x].key, x= t[x].r;
   else x= t[x].l;
  return r;
 }
 int next(int k) const {
  int r= -1;
  for(int x= root; x;)
   if(t[x].key >= k) r= t[x].key, x= t[x].l;
   else x= t[x].r;
  return r;
 }
};
}
struct Solver {
 os_wbt::Set s;
 explicit Solver(const vector<int>& a): s(a) {}
 void insert(int x) { s.insert(x); }
 void erase(int x) { s.erase(x); }
 int kth(int k) const { return s.kth(k); }
 int count_le(int x) const { return s.count_le(x); }
 int prev(int x) const { return s.prev(x); }
 int next(int x) const { return s.next(x); }
};
