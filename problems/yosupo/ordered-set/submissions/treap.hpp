#pragma once
// treap。節点は {キー, 部分木の大きさ, 左右の子, 優先度} の 20 byte で、1 本の vector に並べて添字でつなぐ。優先度は
// 表を作るときに steady_clock から取った種の xorshift で決める。入れるときは、新しい節点の優先度が上回る位置まで
// 下りて、そこにあった部分木をキーで 2 つに分けて子にする。除くときは、節点を左右の子を併合したもので置き換える。
// どちらも先に入っているかを調べるので、道の上の大きさは 1 ずつ足し引きするだけで済む。初期集合は、昇順に並んだ
// キーに乱数の優先度を付け、スタックで Cartesian tree を作って O(N) で組む。
#include <chrono>
#include <vector>
namespace os_treap {
struct Node {
 int key, sz, ch[2];
 unsigned pri;
};
struct Set {
 std::vector<Node> t;
 int root;
 unsigned long long rng;
 unsigned rand() {
  rng^= rng << 13, rng^= rng >> 7, rng^= rng << 17;
  return unsigned(rng >> 32);
 }
 int make(int key) {
  t.push_back(Node{key, 1, {0, 0}, rand()});
  return int(t.size()) - 1;
 }
 void pull(int x) { t[x].sz= t[t[x].ch[0]].sz + t[t[x].ch[1]].sz + 1; }
 explicit Set(const std::vector<int>& a): root(0), rng(std::chrono::steady_clock::now().time_since_epoch().count() | 1) {
  t.reserve(a.size() + 1);
  t.push_back(Node{0, 0, {0, 0}, 0});
  std::vector<int> st;
  for(int k: a) {
   const int x= make(k);
   int last= 0;
   while(!st.empty() && t[st.back()].pri < t[x].pri) last= st.back(), st.pop_back();
   t[x].ch[0]= last;
   if(!st.empty()) t[st.back()].ch[1]= x;
   st.push_back(x);
  }
  if(!st.empty()) root= st[0];
  fix(root);
 }
 int fix(int x) { return x ? t[x].sz= fix(t[x].ch[0]) + fix(t[x].ch[1]) + 1 : 0; }
 // キーが k 未満のものを l に、残りを r に分ける。
 void split(int x, int k, int& l, int& r) {
  if(!x) return void(l= r= 0);
  if(t[x].key < k) split(t[x].ch[1], k, t[x].ch[1], r), l= x;
  else split(t[x].ch[0], k, l, t[x].ch[0]), r= x;
  pull(x);
 }
 int merge(int a, int b) {
  if(!a || !b) return a | b;
  if(t[a].pri > t[b].pri) return t[a].ch[1]= merge(t[a].ch[1], b), pull(a), a;
  return t[b].ch[0]= merge(a, t[b].ch[0]), pull(b), b;
 }
 bool contains(int k) const {
  for(int x= root; x;) {
   if(t[x].key == k) return true;
   x= t[x].ch[t[x].key < k];
  }
  return false;
 }
 void ins(int& x, int y) {
  if(!x) return void(x= y);
  if(t[y].pri > t[x].pri) {
   split(x, t[y].key, t[y].ch[0], t[y].ch[1]);
   pull(y), x= y;
   return;
  }
  ++t[x].sz;
  ins(t[x].ch[t[x].key < t[y].key], y);
 }
 void del(int& x, int k) {
  if(t[x].key == k) return void(x= merge(t[x].ch[0], t[x].ch[1]));
  --t[x].sz;
  del(t[x].ch[t[x].key < k], k);
 }
 void insert(int k) {
  if(!contains(k)) {
   const int y= make(k);
   ins(root, y);
  }
 }
 void erase(int k) {
  if(contains(k)) del(root, k);
 }
 int kth(int k) const {
  if(k >= t[root].sz) return -1;
  for(int x= root;;) {
   const int ls= t[t[x].ch[0]].sz;
   if(k < ls) x= t[x].ch[0];
   else if(k == ls) return t[x].key;
   else k-= ls + 1, x= t[x].ch[1];
  }
 }
 int count_le(int k) const {
  int c= 0;
  for(int x= root; x;)
   if(t[x].key <= k) c+= t[t[x].ch[0]].sz + 1, x= t[x].ch[1];
   else x= t[x].ch[0];
  return c;
 }
 int prev(int k) const {
  int r= -1;
  for(int x= root; x;)
   if(t[x].key <= k) r= t[x].key, x= t[x].ch[1];
   else x= t[x].ch[0];
  return r;
 }
 int next(int k) const {
  int r= -1;
  for(int x= root; x;)
   if(t[x].key >= k) r= t[x].key, x= t[x].ch[0];
   else x= t[x].ch[1];
  return r;
 }
};
}
struct Solver {
 os_treap::Set s;
 explicit Solver(const vector<int>& a): s(a) {}
 void insert(int x) { s.insert(x); }
 void erase(int x) { s.erase(x); }
 int kth(int k) const { return s.kth(k); }
 int count_le(int x) const { return s.count_le(x); }
 int prev(int x) const { return s.prev(x); }
 int next(int x) const { return s.next(x); }
};
