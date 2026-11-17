// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの (2026-11-17、Library 9d0220620)。
// Library を直しても変わらない。答えは checker.cpp が組の数と表との一致で確かめるので、期待出力からは組の数だけを使う。小さい入力
// では brute.cpp と突き合わせてある (チェッカに両方の出力を通す形で、gen.py の seed 1000 から 3999。pj testdata crosscheck は
// チェッカの問題を扱わない)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// 今の Library の bipartite_matching で解く。文書の包含の順序 (common.hpp) は推移的なので、文書を出る側と入る側に分けた
// 二部グラフの最大マッチングで、組になった j の次を j' にすれば、頂点を共有しない道がそのまま最小の数の鎖になる。
#include <vector>
// cf-1765-a の提出で共有する、入力の読み込みと、文書の鎖から答えを組む部分。
//
// 文書 j に、その文書を見られる開発者の集合 S_j を対応させる。同じ組の文書は、水準の低い順に集合が包含の向きで並ぶ
// (水準 c の文書を見られる開発者は、それより低い水準の文書も見られる) ので、組は包含の順序の鎖になる。逆に、鎖になって
// いれば水準を付けられるので、最小の組の数は、包含の順序を覆う最小の鎖の数になる。順序は、S_j が S_j' を真に含むか、
// 同じ集合で j < j' のときに j を前にする (推移的)。
#include <bitset>
#include <cstdio>
#include <string>
#include <vector>
struct CF1765A {
 int n= 0, m= 0;
 std::vector<std::string> a;
 std::vector<std::bitset<500>> col;
 bool read() {
  if(scanf("%d %d", &n, &m) != 2) return false;
  a.assign(n, std::string());
  static char buf[512];
  for(auto& s: a) {
   if(scanf("%511s", buf) != 1) return false;
   s= buf;
  }
  col.assign(m, {});
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < m; ++j)
    if(a[i][j] == '1') col[j][i]= 1;
  return true;
 }
 // j を j' の前にするか。
 bool before(int j, int k) const {
  if(col[j] == col[k]) return j < k;
  return (col[j] & col[k]) == col[k];
 }
 // 包含の順序の組を全部 (推移的な関係のまま) 列挙する。
 template <class F> void for_each_pair(F&& f) const {
  for(int j= 0; j < m; ++j)
   for(int k= 0; k < m; ++k)
    if(j != k && before(j, k)) f(j, k);
 }
 // 鎖 (前の文書の集合が後の文書の集合を含む) の列から答えを組んで書く。鎖の p 番目 (0 始まり) の水準を p + 2 にし、
 // 開発者の水準は、見られる文書のうち最も後ろの位置 p の p + 2 (見られる文書が無ければ 1) にする。
 void print(const std::vector<std::vector<int>>& chains) const {
  const int k= chains.size();
  std::vector<int> grp(m), lev(m), pos(m);
  for(int g= 0; g < k; ++g)
   for(int p= 0; p < (int)chains[g].size(); ++p) grp[chains[g][p]]= g, lev[chains[g][p]]= p + 2;
  std::string out= std::to_string(k) + '\n';
  for(int j= 0; j < m; ++j) out+= std::to_string(grp[j] + 1), out+= j + 1 < m ? ' ' : '\n';
  for(int j= 0; j < m; ++j) out+= std::to_string(lev[j]), out+= j + 1 < m ? ' ' : '\n';
  for(int i= 0; i < n; ++i) {
   std::vector<int> v(k, 1);
   for(int j= 0; j < m; ++j)
    if(a[i][j] == '1' && lev[j] > v[grp[j]]) v[grp[j]]= lev[j];
   for(int g= 0; g < k; ++g) out+= std::to_string(v[g]), out+= g + 1 < k ? ' ' : '\n';
  }
  fputs(out.c_str(), stdout);
 }
};

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

int main() {
 CF1765A p;
 if(!p.read()) return 1;
 const int m= p.m;
 BipartiteGraph bg(m, m);
 p.for_each_pair([&](int j, int k) { bg.add_edge(j, m + k); });
 auto [mc, mate]= bipartite_matching(bg);
 std::vector<int> nx(m, -1);
 std::vector<char> head(m, 1);
 for(int j= 0; j < m; ++j)
  if(mate[j] >= 0) nx[j]= mate[j] - m, head[mate[j] - m]= 0;
 std::vector<std::vector<int>> chains;
 for(int j= 0; j < m; ++j)
  if(head[j]) {
   auto& c= chains.emplace_back();
   for(int x= j; x >= 0; x= nx[x]) c.push_back(x);
  }
 p.print(chains);
}
