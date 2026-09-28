// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
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

#include <valarray>
#include <array>
#include <cassert>
class HeavyLightDecomposition {
 std::vector<int> P, PP, D, I, L, R;
public:
 HeavyLightDecomposition()= default;
 HeavyLightDecomposition(const Graph& g, int root= 0): HeavyLightDecomposition(g.adjacency_vertex(0), root) {}
 HeavyLightDecomposition(const CSRArray<int>& adj, int root= 0) {
  const int n= adj.size();
  P.assign(n, -2), PP.resize(n), D.resize(n), I.resize(n), L.resize(n), R.resize(n);
  auto f= [&, i= 0, v= 0, t= 0](int r) mutable {
   for(P[r]= -1, I[t++]= r; i < t; ++i)
    for(int u: adj[v= I[i]])
     if(P[v] != u) P[I[t++]= u]= v;
  };
  f(root);
  for(int r= 0; r < n; ++r)
   if(P[r] == -2) f(r);
  std::vector<int> Z(n, 1), nx(n, -1);
  for(int i= n, v; i--;) {
   if(P[v= I[i]] == -1) continue;
   if(Z[P[v]]+= Z[v]; nx[P[v]] == -1) nx[P[v]]= v;
   if(Z[nx[P[v]]] < Z[v]) nx[P[v]]= v;
  }
  for(int v= n; v--;) PP[v]= v;
  for(int v: I)
   if(nx[v] != -1) PP[nx[v]]= v;
  for(int v: I)
   if(P[v] != -1) PP[v]= PP[PP[v]], D[v]= D[P[v]] + 1;
  for(int i= n; i--;) L[I[i]]= i;
  for(int v: I) {
   int ir= R[v]= L[v] + Z[v];
   for(int u: adj[v])
    if(u != P[v] && u != nx[v]) L[u]= (ir-= Z[u]);
   if(nx[v] != -1) L[nx[v]]= L[v] + 1;
  }
  for(int i= n; i--;) I[L[i]]= i;
 }
 int to_seq(int v) const { return L[v]; }
 int to_vertex(int i) const { return I[i]; }
 size_t size() const { return P.size(); }
 int parent(int v) const { return P[v]; }
 int head(int v) const { return PP[v]; }
 int root(int v) const {
  for(v= PP[v];; v= PP[P[v]])
   if(P[v] == -1) return v;
 }
 bool connected(int u, int v) const { return root(u) == root(v); }
 // u is in v
 bool in_subtree(int u, int v) const { return L[v] <= L[u] && L[u] < R[v]; }
 int subtree_size(int v) const { return R[v] - L[v]; }
 int lca(int u, int v) const {
  for(;; v= P[PP[v]]) {
   if(L[u] > L[v]) std::swap(u, v);
   if(PP[u] == PP[v]) return u;
  }
 }
 int la(int v, int k) const {
  assert(k <= D[v]);
  for(int u;; k-= L[v] - L[u] + 1, v= P[u])
   if(L[v] - k >= L[u= PP[v]]) return I[L[v] - k];
 }
 int jump(int u, int v, int k) const {
  if(!k) return u;
  if(u == v) return -1;
  if(k == 1) return in_subtree(v, u) ? la(v, D[v] - D[u] - 1) : P[u];
  int w= lca(u, v), d_uw= D[u] - D[w], d_vw= D[v] - D[w];
  return k > d_uw + d_vw ? -1 : k <= d_uw ? la(u, k) : la(v, d_uw + d_vw - k);
 }
 int depth(int v) const { return D[v]; }
 int dist(int u, int v) const { return D[u] + D[v] - D[lca(u, v)] * 2; }
 // half-open interval [l,r)
 std::pair<int, int> subtree(int v) const { return {L[v], R[v]}; }
 // sequence of closed intervals [l,r]
 std::vector<std::pair<int, int>> path(int u, int v, bool edge= 0) const {
  std::vector<std::pair<int, int>> up, down;
  while(PP[u] != PP[v]) {
   if(L[u] < L[v]) down.emplace_back(L[PP[v]], L[v]), v= P[PP[v]];
   else up.emplace_back(L[u], L[PP[u]]), u= P[PP[u]];
  }
  if(L[u] < L[v]) down.emplace_back(L[u] + edge, L[v]);
  else if(L[v] + edge <= L[u]) up.emplace_back(L[u], L[v] + edge);
  return up.insert(up.end(), down.rbegin(), down.rend()), up;
 }
};

// put_edge(int v, int e, T t) -> U
// op(U l, U r) -> U
// ui(:U) is the identity element of op
// put_vertex(int v, U sum) -> T
template <class T> class Rerooting {
 HeavyLightDecomposition hld;
 std::valarray<T> dp, dp1, dp2;
public:
 template <class U, class F1, class F2, class F3> Rerooting(const Graph& g, const CSRArray<int>& adje, const HeavyLightDecomposition& hld, const F1& put_edge, const F2& op, const U& ui, const F3& put_vertex): hld(hld) {
  static_assert(std::is_invocable_r_v<U, F1, int, int, T>, "put_edge(int,int,T) is not invocable");
  static_assert(std::is_invocable_r_v<U, F2, U, U>, "op(U,U) is not invocable");
  static_assert(std::is_invocable_r_v<T, F3, int, U>, "put_vertex(int,U) is not invocable");
  const int n= g.vertex_size();
  dp.resize(n), dp1.resize(n), dp2.resize(n);
  for(int i= n, v; i--;) {
   U sum= ui;
   for(int e: adje[v= hld.to_vertex(i)])
    if(int u= g[e].to(v); u != hld.parent(v)) sum= op(sum, put_edge(v, e, dp1[u]));
   dp1[v]= put_vertex(v, sum);
  }
  for(int i= 0, v; i < n; ++i) {
   auto gv= adje[v= hld.to_vertex(i)];
   int dg= gv.size();
   std::valarray<U> f(dg + 1), b(dg + 1);
   for(int j= 0, e, u; j < dg; ++j) u= g[e= gv[j]].to(v), f[j + 1]= put_edge(v, e, u == hld.parent(v) ? dp2[v] : dp1[u]);
   f[0]= b[dg]= ui;
   for(int j= dg; j--;) b[j]= op(f[j + 1], b[j + 1]);
   for(int j= 0; j < dg; ++j) f[j + 1]= op(f[j], f[j + 1]);
   for(int j= 0; j < dg; ++j)
    if(int u= g[gv[j]].to(v); u != hld.parent(v)) dp2[u]= put_vertex(v, op(f[j], b[j + 1]));
   dp[v]= put_vertex(v, f[dg]);
  }
 }
 template <class U, class F1, class F2, class F3> Rerooting(const Graph& g, const CSRArray<int>& adje, const F1& put_edge, const F2& op, const U& ui, const F3& put_vertex): Rerooting(g, adje, HeavyLightDecomposition(g), put_edge, op, ui, put_vertex) {}
 template <class U, class F1, class F2, class F3> Rerooting(const Graph& g, const HeavyLightDecomposition& hld, const F1& put_edge, const F2& op, const U& ui, const F3& put_vertex): Rerooting(g, g.adjacency_edge(0), hld, put_edge, op, ui, put_vertex) {}
 template <class U, class F1, class F2, class F3> Rerooting(const Graph& g, const F1& put_edge, const F2& op, const U& ui, const F3& put_vertex): Rerooting(g, g.adjacency_edge(0), HeavyLightDecomposition(g), put_edge, op, ui, put_vertex) {}
 const T& operator[](int v) const { return dp[v]; }
 auto begin() const { return std::begin(dp); }
 auto end() const { return std::end(dp); }
 const T& operator()(int root, int v) const { return root == v ? dp[v] : hld.in_subtree(root, v) ? dp2[hld.jump(v, root, 1)] : dp1[v]; }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 Graph g(N, N - 1);
 vector<int> C(N - 1);
 for(int i= 0; i < N - 1; ++i) cin >> g[i] >> C[i], --g[i];
 vector<long long> D(N);
 for(int i= 0; i < N; ++i) cin >> D[i];
 auto put_edge= [&](int v, int e, long long d) { return max(d, D[g[e].to(v)]) + C[e]; };
 auto op= [&](long long l, long long r) { return max(l, r); };
 auto put_vertex= [&](int, long long d) { return d; };
 for(long long x: Rerooting<long long>(g, put_edge, op, 0ll, put_vertex)) cout << x << '\n';
 return 0;
}
