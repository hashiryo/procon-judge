#pragma once
// B+ 木。葉はキーを 63 個まで昇順に持ち、前後の葉とつなぐ。内部の節点は子を 63 個まで持ち、子ごとに部分木のキーの
// 最大と個数を持つ。キーで下りるときは、最大の列のうち x 以上のものの数を AVX2 で 8 個ずつ比べて数え、子を選ぶ。
// 部分木の個数を持つので、k 番目と x 以下の個数も 1 回下りるだけで引ける。入れて 64 個になった節点は半分に割り、
// 除いて空になった節点は親から外す (それ以外のときは隣と併せない)。初期集合は、葉と節点を 48 個ずつ詰めて下から組む。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <cstring>
#include <vector>
namespace os_bptree {
constexpr int B= 64, FILL= 48;
struct alignas(64) Leaf {
 int key[B];
 int n, prv, nxt;
};
struct alignas(64) Inner {
 int mx[B], cnt[B], ch[B];
 int n;
};
// 昇順の a[0, n) のうち x 未満のものの個数。
inline int count_lt(const int* a, int n, int x) {
 const __m256i vx= _mm256_set1_epi32(x);
 int c= 0;
 for(int i= 0; i < n; i+= 8) {
  unsigned m= _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(vx, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i)))));
  if(n - i < 8) m&= (1u << (n - i)) - 1;
  c+= __builtin_popcount(m);
 }
 return c;
}
// 昇順の a[0, n) のうち x 以下のものの個数。
inline int count_le(const int* a, int n, int x) {
 const __m256i vx= _mm256_set1_epi32(x);
 int c= 0;
 for(int i= 0; i < n; i+= 8) {
  unsigned m= ~_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i)), vx))) & 0xff;
  if(n - i < 8) m&= (1u << (n - i)) - 1;
  c+= __builtin_popcount(m);
 }
 return c;
}
inline int sum(const int* a, int n) {
 int s= 0;
 for(int i= 0; i < n; ++i) s+= a[i];
 return s;
}
struct Set {
 std::vector<Leaf> lf;
 std::vector<Inner> in;
 std::vector<int> flf, fin;  // 空いた葉と節点
 int root, h, total;         // h は根の高さ (0 なら根が葉)
 int new_leaf() {
  if(!flf.empty()) {
   const int v= flf.back();
   flf.pop_back();
   return v;
  }
  lf.emplace_back();
  return int(lf.size()) - 1;
 }
 int new_inner() {
  if(!fin.empty()) {
   const int v= fin.back();
   fin.pop_back();
   return v;
  }
  in.emplace_back();
  return int(in.size()) - 1;
 }
 explicit Set(const std::vector<int>& a): h(0), total(a.size()) {
  const int n= a.size();
  lf.reserve(n / FILL + 2), in.reserve(n / FILL / FILL + 8);
  std::vector<int> ids, mxs, cnts;
  for(int i= 0; i < n || i == 0; i+= FILL) {
   const int v= new_leaf(), m= std::min(FILL, n - i);
   Leaf& L= lf[v];
   L.n= m, L.prv= ids.empty() ? -1 : ids.back(), L.nxt= -1;
   if(m > 0) std::memcpy(L.key, a.data() + i, m * sizeof(int));
   if(!ids.empty()) lf[ids.back()].nxt= v;
   ids.push_back(v), mxs.push_back(m ? L.key[m - 1] : 0), cnts.push_back(m);
  }
  while(ids.size() > 1) {
   std::vector<int> ids2, mxs2, cnts2;
   for(size_t i= 0; i < ids.size(); i+= FILL) {
    const int v= new_inner(), m= std::min<int>(FILL, ids.size() - i);
    Inner& t= in[v];
    t.n= m;
    for(int j= 0; j < m; ++j) t.ch[j]= ids[i + j], t.mx[j]= mxs[i + j], t.cnt[j]= cnts[i + j];
    ids2.push_back(v), mxs2.push_back(t.mx[m - 1]), cnts2.push_back(sum(t.cnt, m));
   }
   ids.swap(ids2), mxs.swap(mxs2), cnts.swap(cnts2), ++h;
  }
  root= ids[0];
 }
 int kth(int k) const {
  if(k >= total) return -1;
  int v= root;
  for(int d= h; d > 0; --d) {
   const Inner& t= in[v];
   int i= 0;
   while(k >= t.cnt[i]) k-= t.cnt[i++];
   v= t.ch[i];
  }
  return lf[v].key[k];
 }
 int count_le(int x) const {
  int c= 0, v= root;
  for(int d= h; d > 0; --d) {
   const Inner& t= in[v];
   const int i= os_bptree::count_le(t.mx, t.n, x);
   if(i == t.n) return c + sum(t.cnt, t.n);
   c+= sum(t.cnt, i), v= t.ch[i];
  }
  return c + os_bptree::count_le(lf[v].key, lf[v].n, x);
 }
 int prev(int x) const {
  int v= root;
  for(int d= h; d > 0; --d) {
   const Inner& t= in[v];
   const int i= os_bptree::count_le(t.mx, t.n, x);
   if(i == t.n) return t.mx[t.n - 1];
   v= t.ch[i];
  }
  const Leaf& L= lf[v];
  if(const int j= os_bptree::count_le(L.key, L.n, x)) return L.key[j - 1];
  return L.prv >= 0 ? lf[L.prv].key[lf[L.prv].n - 1] : -1;
 }
 int next(int x) const {
  int v= root;
  for(int d= h; d > 0; --d) {
   const Inner& t= in[v];
   const int i= count_lt(t.mx, t.n, x);
   if(i == t.n) return -1;
   v= t.ch[i];
  }
  const Leaf& L= lf[v];
  const int j= count_lt(L.key, L.n, x);
  return j == L.n ? -1 : L.key[j];
 }
 int path[16], pos[16];  // 段 d の通った節点と、その中で選んだ子
 void insert(int x) {
  int v= root;
  for(int d= h; d > 0; --d) {
   const Inner& t= in[v];
   int i= count_lt(t.mx, t.n, x);
   if(i == t.n) i= t.n - 1;
   path[d]= v, pos[d]= i, v= t.ch[i];
  }
  Leaf& L= lf[v];
  const int j= count_lt(L.key, L.n, x);
  if(j < L.n && L.key[j] == x) return;
  std::memmove(L.key + j + 1, L.key + j, (L.n - j) * sizeof(int));
  L.key[j]= x, ++L.n, ++total;
  for(int d= 1; d <= h; ++d) {
   Inner& t= in[path[d]];
   ++t.cnt[pos[d]];
   if(t.mx[pos[d]] < x) t.mx[pos[d]]= x;
  }
  if(L.n == B) split_leaf(v);
 }
 void split_leaf(int v) {
  const int u= new_leaf();
  Leaf &L= lf[v], &R= lf[u];
  R.n= B / 2, L.n= B / 2;
  std::memcpy(R.key, L.key + B / 2, B / 2 * sizeof(int));
  R.prv= v, R.nxt= L.nxt, L.nxt= u;
  if(R.nxt >= 0) lf[R.nxt].prv= u;
  push_up(1, v, B / 2, L.key[B / 2 - 1], u, B / 2, R.key[B / 2 - 1]);
 }
 // 段 d - 1 の節点 l を割って r を作ったので、段 d の親に r を足す。
 void push_up(int d, int l, int lc, int lm, int r, int rc, int rm) {
  if(d > h) {
   const int v= new_inner();
   Inner& t= in[v];
   t.n= 2, t.ch[0]= l, t.ch[1]= r, t.cnt[0]= lc, t.cnt[1]= rc, t.mx[0]= lm, t.mx[1]= rm;
   root= v, ++h;
   return;
  }
  const int v= path[d], i= pos[d];
  {
   Inner& t= in[v];
   const int k= t.n - i - 1;
   std::memmove(t.ch + i + 2, t.ch + i + 1, k * sizeof(int));
   std::memmove(t.cnt + i + 2, t.cnt + i + 1, k * sizeof(int));
   std::memmove(t.mx + i + 2, t.mx + i + 1, k * sizeof(int));
   t.cnt[i]= lc, t.mx[i]= lm, t.ch[i + 1]= r, t.cnt[i + 1]= rc, t.mx[i + 1]= rm, ++t.n;
   if(t.n < B) return;
  }
  const int u= new_inner();
  Inner &T= in[v], &U= in[u];
  T.n= U.n= B / 2;
  std::memcpy(U.ch, T.ch + B / 2, B / 2 * sizeof(int));
  std::memcpy(U.cnt, T.cnt + B / 2, B / 2 * sizeof(int));
  std::memcpy(U.mx, T.mx + B / 2, B / 2 * sizeof(int));
  push_up(d + 1, v, sum(T.cnt, B / 2), T.mx[B / 2 - 1], u, sum(U.cnt, B / 2), U.mx[B / 2 - 1]);
 }
 void erase(int x) {
  int v= root;
  for(int d= h; d > 0; --d) {
   const Inner& t= in[v];
   const int i= count_lt(t.mx, t.n, x);
   if(i == t.n) return;
   path[d]= v, pos[d]= i, v= t.ch[i];
  }
  Leaf& L= lf[v];
  const int j= count_lt(L.key, L.n, x);
  if(j == L.n || L.key[j] != x) return;
  std::memmove(L.key + j, L.key + j + 1, (L.n - j - 1) * sizeof(int));
  --L.n, --total;
  for(int d= 1; d <= h; ++d) --in[path[d]].cnt[pos[d]];
  if(L.n > 0) {
   if(j == L.n) fix_max(1, L.key[L.n - 1]);
   return;
  }
  if(h == 0) return;
  if(L.prv >= 0) lf[L.prv].nxt= L.nxt;
  if(L.nxt >= 0) lf[L.nxt].prv= L.prv;
  flf.push_back(v);
  remove_child(1);
 }
 // 段 d の pos[d] 番目の子の部分木の最大が m に変わったので、上へ直す。
 void fix_max(int d, int m) {
  for(; d <= h; ++d) {
   Inner& t= in[path[d]];
   t.mx[pos[d]]= m;
   if(pos[d] != t.n - 1) return;
  }
 }
 // 段 d の pos[d] 番目の子 (空になった) を外す。
 void remove_child(int d) {
  const int v= path[d], i= pos[d];
  Inner& t= in[v];
  const int k= t.n - i - 1;
  std::memmove(t.ch + i, t.ch + i + 1, k * sizeof(int));
  std::memmove(t.cnt + i, t.cnt + i + 1, k * sizeof(int));
  std::memmove(t.mx + i, t.mx + i + 1, k * sizeof(int));
  --t.n;
  if(t.n == 0) {
   fin.push_back(v);
   if(d < h) return remove_child(d + 1);
   root= new_leaf(), h= 0;
   lf[root].n= 0, lf[root].prv= lf[root].nxt= -1;
   return;
  }
  if(i == t.n) fix_max(d + 1, t.mx[t.n - 1]);
  while(h > 0 && in[root].n == 1) fin.push_back(root), root= in[root].ch[0], --h;
 }
};
}
struct Solver {
 os_bptree::Set s;
 explicit Solver(const vector<int>& a): s(a) {}
 void insert(int x) { s.insert(x); }
 void erase(int x) { s.erase(x); }
 int kth(int k) const { return s.kth(k); }
 int count_le(int x) const { return s.count_le(x); }
 int prev(int x) const { return s.prev(x); }
 int next(int x) const { return s.next(x); }
};
