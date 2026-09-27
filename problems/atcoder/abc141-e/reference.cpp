// 期待出力を作る参照実装。submissions/lib-suffixtree.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <string>
#include <vector>
template <class T, class F> class SparseTable {
 std::vector<std::vector<T>> dat;
 F f;
public:
 SparseTable() {}
 SparseTable(const std::vector<T> &v, const F &f): f(f) {
  int n= v.size(), log= n > 1 ? 31 - __builtin_clz(n - 1) : 0;
  dat.resize(log + 1), dat[0].assign(v.begin(), v.end());
  for (int i= 0, I= 1, j; i < log; ++i, I<<= 1)
   for (dat[i + 1].resize(j= dat[i].size() - I); j--;) dat[i + 1][j]= f(dat[i][j], dat[i][j + I]);
 }
 // [l, r)
 T prod(int l, int r) const {
  if (r == l + 1) return dat[0][l];
  int k= 31 - __builtin_clz(r - l - 1);
  return f(dat[k][l], dat[k][r - (1 << k)]);
 }
};
#include <string>
#include <vector>
#include <algorithm>
template <class String> struct SuffixArray {
 String s;
 std::vector<int> sa;
 static inline std::vector<int> sa_is(const std::vector<int> &s, int K) {
  const int n= s.size();
  std::vector<char> t(n);
  std::vector<int> bkt(K, 0), bkt_l(K), bkt_r(K), sa(n), p1;
  t.back()= true;
  for (int i= n; --i;)
   if (t[i - 1]= (s[i - 1] < s[i] || (t[i] && s[i - 1] == s[i])); t[i] && !t[i - 1]) p1.push_back(i);
  std::reverse(p1.begin(), p1.end());
  const int n1= p1.size();
  for (int i= n; i--;) ++bkt[s[i]];
  for (int i= 0, sum= 0; i < K; ++i) sum+= bkt[i], bkt_r[i]= sum, bkt_l[i]= sum - bkt[i];
  std::vector<int> s1(n1), sa1(n1);
  std::fill_n(sa.begin(), n, -1), std::copy_n(bkt_r.begin(), K, bkt.begin());
  for (int i= n1; i--;) sa[--bkt[s[p1[i]]]]= p1[i];
  std::copy_n(bkt_l.begin(), K, bkt.begin());
  for (int i= 0, j; i < n; ++i)
   if ((j= sa[i] - 1) >= 0 && !t[j]) sa[bkt[s[j]]++]= j;
  std::copy_n(bkt_r.begin(), K, bkt.begin());
  for (int i= n, j; i--;)
   if ((j= sa[i] - 1) >= 0 && t[j]) sa[--bkt[s[j]]]= j;
  for (int i= 0, j= 0; i < n; ++i)
   if (t[sa[i]] && sa[i] > 0 && !t[sa[i] - 1]) sa1[j++]= sa[i];
  int name= 0;
  for (int i= 0, prev= -1, j, pos; i < n1; ++i, sa[pos]= name - 1)
   for (j= 0, pos= sa1[i];; ++j)
    if (prev == -1 || s[pos + j] != s[prev + j] || t[pos + j] != t[prev + j]) {
     prev= pos, ++name;
     break;
    } else if (j && ((t[pos + j] && !t[pos + j - 1]) || (t[prev + j] && !t[prev + j - 1]))) break;
  for (int i= n1; i--;) s1[i]= sa[p1[i]];
  if (name != n1) sa1= sa_is(s1, name);
  else
   for (int i= n1; i--;) sa1[s1[i]]= i;
  std::copy_n(bkt_r.begin(), K, bkt.begin()), std::fill_n(sa.begin(), n, -1);
  for (int i= n1; i--;) sa[--bkt[s[p1[sa1[i]]]]]= p1[sa1[i]];
  for (int i= 0, j; i < n; ++i)
   if ((j= sa[i] - 1) >= 0 && !t[j]) sa[bkt_l[s[j]]++]= j;
  for (int i= n, j; i--;)
   if ((j= sa[i] - 1) >= 0 && t[j]) sa[--bkt_r[s[j]]]= j;
  return sa;
 }
public:
 SuffixArray(const String &S): s(S) {
  std::vector<int> s_cpy(s.size() + 1);
  if constexpr (std::is_convertible_v<String, std::string>) std::copy(s.begin(), s.end(), s_cpy.begin()), sa= sa_is(s_cpy, 128), sa.erase(sa.begin());
  else {
   auto v= s;
   sort(v.begin(), v.end()), v.erase(unique(v.begin(), v.end()), v.end());
   for (int i= s.size(); i--;) s_cpy[i]= std::lower_bound(v.begin(), v.end(), s[i]) - v.begin() + 1;
   sa= sa_is(s_cpy, v.size() + 1), sa.erase(sa.begin());
  }
 }
 int operator[](int i) const { return sa[i]; }
 size_t size() const { return sa.size(); }
 auto begin() const { return sa.begin(); }
 auto end() const { return sa.end(); }
 // return {l,r} s.t. P is a prefix of S[sa[i]:] ( i in [l,r) )
 // l == r if P is not a substring of S
 // O(|P|log|S|)
 std::pair<int, int> pattern_matching(const String &P) const {
  const int n= s.size(), m= P.size();
  if (n < m) return {0, 0};
  auto f1= [&](int h) {
   auto t= s.begin() + h;
   for (int j= 0, e= std::min(n - h, m); j < e; ++j) {
    if (t[j] < P[j]) return true;
    if (t[j] > P[j]) return false;
   }
   return n - h < m;
  };
  auto f2= [&](int h) {
   auto t= s.begin() + h;
   for (int j= 0, e= std::min(n - h, m); j < e; ++j)
    if (t[j] > P[j]) return false;
   return true;
  };
  auto L= std::partition_point(sa.begin(), sa.end(), f1), R= std::partition_point(L, sa.end(), f2);
  return {L - sa.begin(), R - sa.begin()};
 }
};
struct LCPArray {
 std::vector<int> rnk;
 template <class String> LCPArray(const SuffixArray<String> &sa): rnk(sa.size()) {
  const int n= sa.size(), log= n > 2 ? 31 - __builtin_clz(n - 2) : 0;
  dat.resize(log + 1), dat[0].resize(n - 1);
  auto &lcp= dat[0];
  for (int i= n; i--;) rnk[sa[i]]= i;
  for (int i= 0, h= 0; i < n; ++i) {
   if (rnk[i] == n - 1) {
    h= 0;
    continue;
   }
   for (int j= sa[rnk[i] + 1]; i + h < n && j + h < n && sa.s[i + h] == sa.s[j + h];) ++h;
   if ((lcp[rnk[i]]= h)) --h;
  }
  for (int i= 0, I= 1, j; i < log; ++i, I<<= 1)
   for (dat[i + 1].resize(j= dat[i].size() - I); j--;) dat[i + 1][j]= std::min(dat[i][j], dat[i][j + I]);
 }
 int operator[](int i) const { return dat[0][i]; }
 size_t size() const { return dat[0].size(); }
 auto begin() const { return dat[0].begin(); }
 auto end() const { return dat[0].end(); }
 int operator()(int i, int j) const {
  if (i == j) return rnk.size() - i;
  auto [l, r]= std::minmax(rnk[i], rnk[j]);
  if (r == l + 1) return dat[0][l];
  int k= 31 - __builtin_clz(r - l - 1);
  return std::min(dat[k][l], dat[k][r - (1 << k)]);
 }
private:
 std::vector<std::vector<int>> dat;
};
#include <array>
#include <cassert>
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

#include <vector>
#include <array>
class CartesianTree {
 std::vector<std::array<int, 2>> rg, ch;
 std::vector<int> par;
 int rt;
public:
 template <class Vec> CartesianTree(const Vec &a, bool is_min= 1): rg(a.size()), ch(a.size(), std::array{-1, -1}), par(a.size(), -1) {
  const int n= a.size();
  auto comp= [&](int l, int r) { return (is_min ? a[l] < a[r] : a[l] > a[r]) || (a[l] == a[r] && l < r); };
  int st[n], t= 0;
  for (int i= n; i--; rg[i][1]= (t ? st[t - 1] : n), st[t++]= i)
   while (t && comp(i, st[t - 1])) ch[i][1]= st[--t];
  for (int i= t= 0; i < n; rg[i][0]= (t ? st[t - 1] + 1 : 0), st[t++]= i++)
   while (t && comp(i, st[t - 1])) ch[i][0]= st[--t];
  for (int i= 0; i < n; ++i)
   for (int b= 2; b--;)
    if (ch[i][b] != -1) par[ch[i][b]]= i;
  for (int i= 0; i < n; ++i)
   if (par[i] == -1) rt= i;
 }
 std::array<int, 2> children(int i) const { return ch[i]; }
 int parent(int i) const { return par[i]; }
 int root() const { return rt; }
 // [l,r)
 std::array<int, 2> range(int i) const { return rg[i]; }
};
struct SuffixTree {
 Graph graph;
 HeavyLightDecomposition tree;
 std::vector<std::tuple<int, int, int, int>> node;
 std::vector<int> suf;
 template <class String> SuffixTree(const SuffixArray<String>& sa, const LCPArray& lcp): suf(sa.size()) {
  const int n= sa.size();
  node.emplace_back(0, n, 0, 0);
  if(n == 1) {
   graph.add_edge(0, 1), graph.n= 2, tree= HeavyLightDecomposition(graph.adjacency_vertex(1), 0), node.emplace_back(0, 1, 0, 1), suf[0]= 1;
   return;
  }
  CartesianTree ct(lcp);
  auto dfs= [&](auto dfs, int p, int idx, int h) -> void {
   auto [l, r]= ct.range(idx);
   ++r;
   int hh= lcp[idx];
   if(h < hh) graph.add_edge(p, node.size()), p= node.size(), node.emplace_back(l, r, h, hh);
   auto [lch, rch]= ct.children(idx);
   if(lch == -1) {
    if(hh < n - sa[idx]) graph.add_edge(p, node.size()), suf[sa[idx]]= node.size(), node.emplace_back(idx, idx + 1, hh, n - sa[idx]);
    else suf[sa[idx]]= p;
   } else dfs(dfs, p, lch, hh);
   if(rch == -1) {
    if(hh < n - sa[idx + 1]) graph.add_edge(p, node.size()), suf[sa[idx + 1]]= node.size(), node.emplace_back(idx + 1, idx + 2, hh, n - sa[idx + 1]);
    else suf[sa[idx + 1]]= p;
   } else dfs(dfs, p, rch, hh);
  };
  if(int r= ct.root(); lcp[r] > 0) graph.add_edge(0, 1), node.emplace_back(0, n, 0, lcp[r]), dfs(dfs, 1, r, lcp[r]);
  else dfs(dfs, 0, r, 0);
  graph.n= node.size(), tree= HeavyLightDecomposition(graph.adjacency_vertex(1), 0);
 }
 int size() const { return node.size(); }
 auto& operator[](int i) const { return node[i]; }
 auto begin() const { return node.begin(); }
 auto end() const { return node.end(); }
 int substr(int l) const { return suf[l]; }
 int substr(int l, int n) const {
  for(int v= suf[l], u, w;; v= w)
   if(u= tree.head(v), w= tree.parent(u); w == -1 || std::get<3>(node[w]) < n) {
    int ok= tree.to_seq(v), ng= tree.to_seq(u) - 1;
    for(int m; ok - ng > 1;) m= (ok + ng) / 2, (n <= std::get<3>(node[tree.to_vertex(m)]) ? ok : ng)= m;
    return tree.to_vertex(ok);
   }
 }
 template <class String> std::string debug_output(const SuffixArray<String>& sa) const {
  std::string res= "\n";
  for(int i= 0; i < node.size(); ++i) {
   auto [l, r, h, hh]= node[i];
   res+= std::to_string(i) + ": (" + std::to_string(l) + "," + std::to_string(r) + "," + std::to_string(h) + "," + std::to_string(hh) + ") ";
   res+= sa.s.substr(sa[l] + h, hh - h);
   res+= "\n";
  }
  for(int i= 0; i < sa.size(); ++i) {
   res+= " " + sa.s.substr(sa[i]) + "\n";
  }
  return res;
 }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 string S;
 cin >> S;
 SuffixArray sa(S);
 LCPArray lcp(sa);
 SuffixTree st(sa, lcp);
 SparseTable sa_mn(sa.sa, [&](int i, int j) { return min(i, j); });
 SparseTable sa_mx(sa.sa, [&](int i, int j) { return max(i, j); });
 int ans= 0;
 for(auto [l, r, h, hh]: st) {
  int i= sa_mn.prod(l, r), j= sa_mx.prod(l, r);
  ans= max(ans, min(j - i, hh));
 }
 cout << ans << '\n';
 return 0;
}
