// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
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

#include <cassert>
#include <tuple>
#include <algorithm>
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

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, M;
 cin >> N >> M;
 Graph g(N, M);
 for(int i= 0; i < M; ++i) cin >> g[i].first;
 for(int i= 0; i < M; ++i) cin >> g[i].second;
 for(auto& e: g) --e;
 auto col= paint_two_colors(g);
 cout << (col.empty() ? "No" : "Yes") << '\n';
 return 0;
}
