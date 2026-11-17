// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの (2026-11-17、Library e4b31786a)。
// Library を直しても変わらない。推移閉包と König の定理で解く形で、確かめたい lib-neo とは中身が違う。小さい入力では brute.cpp と
// 突き合わせてある (チェッカに両方の出力を通す形で、gen.py の seed 1000 から 3999。pj testdata crosscheck はチェッカの問題を扱わない)。
// 固定したあとに 1 か所だけ直した。DulmageMendelsohn::min_vertex_cover の z を char から signed char にした。arm の Linux では char が
// 符号なしで、-1 で埋めて -1 と比べる判定が働かず、最小点被覆を誤るため (CI の arm で K が 1 や -98 になった。今の Library も 9d0220620 で
// 同じように直した)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// 今の Library の bipartite_matching と DulmageMendelsohn で解く。推移閉包を bitset で作り、頂点を出る側と入る側に分けた
// 二部グラフの最小点被覆 C を取ると、出る側も入る側も C に入らない頂点が最大反鎖になる (König の定理と Dilworth の定理)。
// 3 行目は、頂点 v と比べられる頂点と v 自身を除いた残りで、最大反鎖の大きさ (頂点の数 - 最大マッチング) が K - 1 なら 1。
#include <bitset>
#include <cstdio>
#include <string>
#include <vector>
#include <cassert>
#include <tuple>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <type_traits>
#define _LR(name, IT, CT) \
 template <class T> struct name { \
  using Iterator= typename std::vector<T>::IT; \
  Iterator bg, ed; \
  Iterator begin() const { return bg; } \
  Iterator end() const { return ed; } \
  size_t size() const { return std::distance(bg, ed); } \
  CT &operator[](int i) const { return bg[i]; } \
 }
_LR(ListRange, iterator, T);
_LR(ConstListRange, const_iterator, const T);
#undef _LR
template <class T> struct CSRArray {
 std::vector<T> dat;
 std::vector<int> p;
 size_t size() const { return p.size() - 1; }
 ListRange<T> operator[](int i) { return {dat.begin() + p[i], dat.begin() + p[i + 1]}; }
 ConstListRange<T> operator[](int i) const { return {dat.cbegin() + p[i], dat.cbegin() + p[i + 1]}; }
};
template <template <class> class F, class T> std::enable_if_t<std::disjunction_v<std::is_same<F<T>, ListRange<T>>, std::is_same<F<T>, ConstListRange<T>>, std::is_same<F<T>, CSRArray<T>>>, std::ostream &> operator<<(std::ostream &os, const F<T> &r) {
 os << '[';
 for (int _= 0, __= r.size(); _ < __; ++_) os << (_ ? ", " : "") << r[_];
 return os << ']';
}
struct Edge: std::pair<int, int> {
 using std::pair<int, int>::pair;
 Edge& operator--() { return --first, --second, *this; }
 int to(int v) const { return first ^ second ^ v; }
 friend std::istream& operator>>(std::istream& is, Edge& e) { return is >> e.first >> e.second, is; }
};
struct Graph: std::vector<Edge> {
 size_t n;
 Graph(size_t n= 0, size_t m= 0): vector(m), n(n) {}
 size_t vertex_size() const { return n; }
 size_t edge_size() const { return size(); }
 size_t add_vertex() { return n++; }
 size_t add_edge(int s, int d) { return emplace_back(s, d), size() - 1; }
 size_t add_edge(Edge e) { return emplace_back(e), size() - 1; }
#define _ADJ_FOR(a, b) \
 for(auto [u, v]: *this) a; \
 for(size_t i= 0; i < n; ++i) p[i + 1]+= p[i]; \
 for(int i= size(); i--;) { \
  auto [u, v]= (*this)[i]; \
  b; \
 }
#define _ADJ(a, b) \
 vector<int> p(n + 1), c(size() << !dir); \
 if(!dir) { \
  _ADJ_FOR((++p[u], ++p[v]), (c[--p[u]]= a, c[--p[v]]= b)) \
 } else if(dir > 0) { \
  _ADJ_FOR(++p[u], c[--p[u]]= a) \
 } else { \
  _ADJ_FOR(++p[v], c[--p[v]]= b) \
 } \
 return {c, p}
 CSRArray<int> adjacency_vertex(int dir) const { _ADJ(v, u); }
 CSRArray<int> adjacency_edge(int dir) const { _ADJ(i, i); }
#undef _ADJ
#undef _ADJ_FOR
};

// [0, L) is left, [L, n) is right
struct BipartiteGraph: Graph {
 size_t L;
 BipartiteGraph() {}
 BipartiteGraph(size_t L, size_t R, size_t m= 0): Graph(L + R, m), L(L) {}
 size_t left_size() const { return L; }
 size_t right_size() const { return this->n - L; }
};
std::vector<int> paint_two_colors(const CSRArray<int>& adj) {
 const int n= adj.size();
 std::vector<int> col(n, -1);
 for(int s= n; s--;)
  if(col[s] == -1) {
   std::vector<int> q= {s};
   for(int i= col[s]= 0, v; i < (int)q.size(); ++i)
    for(int u: adj[v= q[i]])
     if(int c= col[v]; col[u] == c) return {};
     else if(col[u] == -1) col[u]= c ^ 1, q.push_back(u);
  }
 return col;
}
std::vector<int> paint_two_colors(const Graph& g) { return paint_two_colors(g.adjacency_vertex(0)); }
// { BipartiteGraph , original to new, new to original }
// {{},{},{}} if not bipartite
std::tuple<BipartiteGraph, std::vector<int>, std::vector<int>> graph_to_bipartite(const Graph& g, std::vector<int> color= {}) {
 if(color.empty()) color= paint_two_colors(g);
 if(color.empty()) return {};
 const int n= g.vertex_size(), m= g.edge_size();
 std::vector<int> a(n), b(n);
 int l= 0, r= n;
 for(int i= n; i--;) b[a[i]= color[i] ? --r : l++]= i;
 BipartiteGraph bg(l, n - l, m);
 for(int i= m; i--;) {
  auto [u, v]= g[i];
  bg[i]= std::minmax(a[u], a[v]);
 }
 return {bg, a, b};
}
namespace _bg_internal {
std::vector<int> _bm(int L, const CSRArray<int>& adj, std::vector<int>& m) {
 std::vector<int> a, p, q(L);
 for(bool u= true; u;) {
  u= false, a.assign(L, -1), p.assign(L, -1);
  int t= 0;
  for(int l= L; l--;)
   if(m[l] == -1) q[t++]= a[l]= p[l]= l;
  for(int i= 0; i < t; ++i)
   if(int l= q[i], x; m[a[l]] == -1)
    for(int r: adj[l]) {
     if(x= m[r]; x == -1) {
      for(u= true; r != -1; l= p[l]) m[r]= l, std::swap(m[l], r);
      break;
     }
     if(p[x] == -1) a[q[t++]= x]= a[p[x]= l];
    }
 }
 return a;
}
}
template <bool lexical= false> std::pair<std::vector<int>, std::vector<int>> bipartite_matching(const BipartiteGraph& bg, std::vector<int> partner= {}) {
 const int L= bg.left_size(), M= bg.edge_size();
 if(partner.empty()) partner.assign(bg.vertex_size(), -1);
 assert(partner.size() == bg.vertex_size());
 {
  CSRArray<int> adj{std::vector<int>(M), std::vector<int>(L + 1)};
  for(auto [l, r]: bg) ++adj.p[l];
  for(int i= 0; i < L; ++i) adj.p[i + 1]+= adj.p[i];
  for(auto [l, r]: bg) adj.dat[--adj.p[l]]= r;
  if constexpr(lexical) {
   for(int l= L; l--;) std::sort(adj[l].begin(), adj[l].end());
   _bg_internal::_bm(L, adj, partner);
   std::vector<char> a(L, 1);
   for(int l= 0; l < L; ++l)
    if(int r= partner[l], v= l; r != -1) {
     std::vector<int> p(L, partner[v]= partner[r]= -1), c(adj.p.begin(), adj.p.begin() + L);
     for(p[v]= -2;;) {
      if(c[v] == adj.p[v + 1]) v= p[v];
      else if(int u= partner[r= adj.dat[c[v]++]]; u == -1) {
       for(; r != -1; v= p[v]) partner[r]= v, std::swap(partner[v], r);
       break;
      } else if(a[u] && p[u] == -1) p[u]= v, v= u;
     }
     a[l]= 0;
    }
  } else _bg_internal::_bm(L, adj, partner);
 }
 std::vector<int> c;
 std::vector<char> p(L);
 for(int i= 0; i < M; ++i)
  if(auto [l, r]= bg[i]; partner[l] == r && !p[l]) c.push_back(i), p[l]= 1;
 return {c, partner};
}

#include <algorithm>
#include <numeric>
class DulmageMendelsohn {
 int L;
 std::vector<int> b, m, a;
 CSRArray<int> dag[2];
public:
 DulmageMendelsohn(const BipartiteGraph& bg): L(bg.left_size()) {
  auto adj= bg.adjacency_vertex(0);
  const int n= adj.size();
  m.assign(n, -1), b.assign(n, -3), a= _bg_internal::_bm(L, adj, m);
  std::vector<int> q(n - L);
  int t= 0, k= 0;
  for(int l= L; l--;)
   if(a[l] != -1)
    if(b[l]= -1; m[l] != -1) b[m[l]]= -1;
  for(int r= n; r-- > L;)
   if(m[r] == -1) b[q[t++]= r]= 0;
  for(int i= 0, r, w; i < t; ++i)
   for(int l: adj[r= q[i]])
    if(b[l] == -3)
     if(b[l]= 0, w= m[l]; w != -1 && b[w] == -3) b[q[t++]= w]= 0;
  t= 0;
  {
   std::vector<int> c(adj.p.begin(), adj.p.begin() + L);
   for(int l= L; l--;)
    if(int v= l; b[v] == -3)
     for(b[v]= -2; v >= 0;) {
      if(c[v] == adj.p[v + 1]) a[t++]= v, v= b[v];
      else if(int w= m[adj.dat[c[v]++]]; b[w] == -3) b[w]= v, v= w;
     }
  }
  for(int i= 0, e= 0, r; t--;)
   if(int s= a[t], p= m[s]; b[p] == -3)
    for(b[q[e++]= p]= b[s]= ++k; i < e; ++i)
     for(int l: adj[r= q[i]])
      if(b[m[l]] == -3) b[q[e++]= m[l]]= b[l]= k;
  ++k;
  for(int l= L; l--;)
   if(b[l] == -1)
    if(b[l]= k; m[l] != -1) b[m[l]]= k;
  a.assign(k + 2, 0);
  for(int i= n; i--;) ++a[b[i]];
  for(int i= 0; i <= k; ++i) a[i + 1]+= a[i];
  for(int i= n; i--;) m[--a[b[i]]]= i;
  Graph h(k + 1);
  for(auto [l, r]: bg)
   if(b[l] != b[r]) h.add_edge(b[l], b[r]);
  std::sort(h.begin(), h.end()), h.erase(std::unique(h.begin(), h.end()), h.end()), dag[0]= h.adjacency_vertex(1), dag[1]= h.adjacency_vertex(-1);
 }
 size_t size() const { return a.size() - 1; }
 ConstListRange<int> block(int k) const { return {m.cbegin() + a[k], m.cbegin() + a[k + 1]}; }
 int operator()(int i) const { return b[i]; }
 std::vector<int> min_vertex_cover(std::vector<int> ord= {}) const {
  if(ord.empty()) ord.resize(b.size()), std::iota(ord.begin(), ord.end(), 0);
  std::vector<signed char> z(size(), -1);
  std::vector<int> q(size()), vc;
  z[0]= 1, z.back()= 0;
  for(int v: ord) {
   int c= (v >= L), k= b[v], s= z[k];
   if(s == -1) {
    auto& adj= dag[z[q[0]= k]= s= !c];
    for(int i= 0, t= 1; i < t; ++i)
     for(int u: adj[q[i]])
      if(z[u] == -1) z[u]= s, q[t++]= u;
   }
   if(c ^ s) vc.push_back(v);
  }
  return vc;
 }
};

int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::vector<int>> g(n);
 std::vector<int> indeg(n, 0), ord;
 for(int i= 0, u, v; i < m; ++i) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  g[u - 1].push_back(v - 1), ++indeg[v - 1];
 }
 for(int v= 0; v < n; ++v)
  if(!indeg[v]) ord.push_back(v);
 for(int i= 0; i < (int)ord.size(); ++i)
  for(int w: g[ord[i]])
   if(!--indeg[w]) ord.push_back(w);
 std::vector<std::bitset<128>> reach(n);
 for(int i= n; i--;)
  for(int w: g[ord[i]]) reach[ord[i]][w]= 1, reach[ord[i]]|= reach[w];
 BipartiteGraph bg(n, n);
 for(int u= 0; u < n; ++u)
  for(int w= 0; w < n; ++w)
   if(reach[u][w]) bg.add_edge(u, n + w);
 DulmageMendelsohn dm(bg);
 const std::vector<int> vc= dm.min_vertex_cover();
 const int k= n - (int)vc.size();
 std::vector<char> in(2 * n, 0);
 for(int x: vc) in[x]= 1;
 std::string pick(n, '0'), can(n, '0');
 for(int v= 0; v < n; ++v)
  if(!in[v] && !in[n + v]) pick[v]= '1';
 for(int v= 0; v < n; ++v) {
  std::vector<int> id(n, -1), rest;
  for(int u= 0; u < n; ++u)
   if(u != v && !reach[u][v] && !reach[v][u]) id[u]= rest.size(), rest.push_back(u);
  const int r= rest.size();
  BipartiteGraph h(r, r);
  for(int a: rest)
   for(int b: rest)
    if(reach[a][b]) h.add_edge(id[a], r + id[b]);
  auto [mc, mate]= bipartite_matching(h);
  if(r - (int)mc.size() == k - 1) can[v]= '1';
 }
 printf("%d\n%s\n%s\n", k, pick.c_str(), can.c_str());
}
