// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの (2026-11-15、Library e4b31786a)。
// Library を直しても変わらない。答えの配列は checker.cpp が値で比べる。小さい入力では brute.cpp と突き合わせてある
// (チェッカに両方の出力を互いに通す形で、gen.py の seed 1000 から 4299。pj testdata crosscheck はチェッカの問題を扱わない)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// 今の Library の general_matching で解く。帰着は common.hpp で、値の頂点と 0 の区間の頂点の一般グラフの最大マッチングを求める。
#include <vector>
// cf-1615-g の提出で共通の帰着と出力。マッチングを求めるところだけを提出ごとに替える。
//
// 0 の区間 [l, r] (長さ L、左の端の値 x、右の端の値 y。配列の端なら 0) の使い方は、次の 3 通りで足りる。
// - 0 を 2 つずつ、配列に出てこない新しい値の組 (f f) で埋める。floor(L / 2) 個の値が数えられる。新しい値の数は足りる
//   (配列に出てこない値の数は 0 の数以上ある)。
// - L が奇数なら、端の値の 1 つ (x か y) を区間の端の 0 に写して、その値も数える。残りは新しい値の組で埋める。
// - L が偶数なら、両端の値を写して x と y を数え、間の L - 2 個を新しい値の組で埋める (L / 2 - 1 + 2)。片方だけ写すのは、
//   新しい値の組を 1 つ減らして値を 1 つ増やすだけなので、得にならない。
// どの値も 1 回しか数えないので、すでに 0 でない隣り合う組のある値を除いた値を頂点にする。奇数の区間は 1 頂点 (x と y への辺)、
// 偶数の区間は 2 頂点 s1, s2 (s1 - s2、s1 - x、s2 - y の辺) にした一般グラフの最大マッチングの大きさを M とすると、
// 答えは (すでに組のある値の数) + Σ floor(L / 2) + M - (偶数の区間の数) になる。偶数の区間は、s1 と s2 が両方とも値と組んだ
// ときだけ 1 増える。
#include <array>
#include <cstdio>
#include <vector>
namespace cf1615g {
struct Seg {
 int l, r, x, y, s;  // 区間 [l, r]、両端の値 (無ければ 0)、頂点の番号 (偶数の区間は s と s + 1)
};
struct Reduction {
 int n= 0, nv= 0;
 std::vector<int> a;
 std::vector<Seg> segs;
 std::vector<std::array<int, 2>> edges;  // 頂点の数は nv。値 v (1 以上 min(n, 600) 以下) は頂点 v - 1
};
inline Reduction read_and_build() {
 Reduction R;
 int n= 0;
 if(scanf("%d", &n) != 1) return R;
 R.n= n, R.a.resize(n);
 for(int& x: R.a)
  if(scanf("%d", &x) != 1) x= 0;
 const int V= n < 600 ? n : 600;
 std::vector<char> paired(V + 1, 0);
 for(int i= 0; i + 1 < n; ++i)
  if(R.a[i] && R.a[i] == R.a[i + 1]) paired[R.a[i]]= 1;
 auto usable= [&](int v) { return v > 0 && !paired[v]; };
 int nv= V;
 for(int i= 0; i < n;) {
  if(R.a[i]) {
   ++i;
   continue;
  }
  int j= i;
  while(j < n && R.a[j] == 0) ++j;
  const int x= i > 0 ? R.a[i - 1] : 0, y= j < n ? R.a[j] : 0;
  R.segs.push_back({i, j - 1, x, y, nv});
  if((j - i) & 1) {
   if(usable(x)) R.edges.push_back({nv, x - 1});
   if(usable(y) && y != x) R.edges.push_back({nv, y - 1});
   nv+= 1;
  } else {
   R.edges.push_back({nv, nv + 1});
   if(usable(x)) R.edges.push_back({nv, x - 1});
   if(usable(y)) R.edges.push_back({nv + 1, y - 1});
   nv+= 2;
  }
  i= j;
 }
 R.nv= nv;
 return R;
}
// マッチングの相手の配列 (頂点ごと、いなければ -1) から答えの配列を作って出す。
inline void restore_and_print(const Reduction& R, const std::vector<int>& mate) {
 const int n= R.n;
 std::vector<int> b= R.a;
 // 配列に出てこない値を、新しい値として小さい順に使う。
 std::vector<char> used(n + 1, 0);
 for(int x: R.a) used[x]= 1;
 int f= 1;
 auto fill_pairs= [&](int l, int r) {  // [l, r] (長さは偶数) を新しい値の組で埋める
  for(int i= l; i < r; i+= 2) {
   while(used[f]) ++f;
   b[i]= b[i + 1]= f++;
  }
 };
 for(const Seg& sg: R.segs) {
  const int s= sg.s;
  if((sg.r - sg.l + 1) & 1) {
   const int m= mate[s];
   if(m >= 0 && sg.x > 0 && m == sg.x - 1) b[sg.l]= sg.x, fill_pairs(sg.l + 1, sg.r);
   else if(m >= 0 && sg.y > 0 && m == sg.y - 1) b[sg.r]= sg.y, fill_pairs(sg.l, sg.r - 1);
   else fill_pairs(sg.l, sg.r - 1);
  } else if(sg.x > 0 && sg.y > 0 && mate[s] == sg.x - 1 && mate[s + 1] == sg.y - 1) {
   b[sg.l]= sg.x, b[sg.r]= sg.y, fill_pairs(sg.l + 1, sg.r - 1);
  } else fill_pairs(sg.l, sg.r);
 }
 for(int i= 0; i < n; ++i)
  if(b[i] == 0) b[i]= i > 0 ? b[i - 1] : 1;  // 余った 0 は何で埋めてもよい
 std::vector<char> buf;
 buf.reserve((size_t)n * 7 + 2);
 char tmp[12];
 for(int i= 0; i < n; ++i) {
  int len= 0, x= b[i];
  do tmp[len++]= char('0' + x % 10), x/= 10;
  while(x);
  while(len) buf.push_back(tmp[--len]);
  buf.push_back(i + 1 == n ? '\n' : ' ');
 }
 fwrite(buf.data(), 1, buf.size(), stdout);
}
}  // namespace cf1615g

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
// {matching edge ids, partner(-1 if unmatched)}
std::pair<std::vector<int>, std::vector<int>> general_matching(const Graph& g, std::vector<int> partner= {}) {
 auto adj= g.adjacency_vertex(0);
 const int n= adj.size();
 std::vector<int> q, z(n), p(n);
 if(partner.empty()) partner.assign(n, -1);
 assert((int)partner.size() == n);
 std::vector<Edge> fs(n);
 auto rematch= [&](auto&& rc, int u, int v) -> void {
  int w= partner[u];
  if(partner[u]= v; w == -1 || partner[w] != u) return;
  if(auto [x, y]= fs[u]; y == -1) rc(rc, partner[w]= x, w);
  else rc(rc, x, y), rc(rc, y, x);
 };
 int t= 1;
 auto f= [&](auto&& rc, int x) -> int { return z[x] != t || p[x] == -1 ? x : (p[x]= rc(rc, p[x])); };
 auto check= [&](int r) {
  q.clear(), q.push_back(r), fs[r]= {-1, -1}, z[r]= t, p[r]= -1;
  for(int i= 0; i < (int)q.size(); ++i) {
   int x= q[i];
   for(int y: adj[x]) {
    if(y == r) continue;
    if(partner[y] == -1) return rematch(rematch, partner[y]= x, y), true;
    if(z[y] == t) {
     int u= f(f, x), v= f(f, y), w= r;
     if(u == v) continue;
     for(; u != r || v != r; fs[u]= {x, y}, u= f(f, fs[partner[u]].first)) {
      if(v != r) std::swap(u, v);
      if(fs[u].first == x && fs[u].second == y) {
       w= u;
       break;
      }
     }
     for(int a: {f(f, x), f(f, y)})
      for(; a != w; a= f(f, fs[partner[a]].first)) z[a]= t, p[a]= w, q.push_back(a);
    } else if(z[partner[y]] != t) fs[y]= {-1, -1}, fs[partner[y]]= {x, -1}, z[partner[y]]= t, p[partner[y]]= y, q.push_back(partner[y]);
   }
  }
  return false;
 };
 for(int r= n; r--;)
  if(partner[r] == -1) t+= check(r);
 q.clear();
 for(int i= 0, e= g.edge_size(); i < e; ++i)
  if(auto [u, v]= g[i]; partner[u] == v && z[u] >= 0) q.push_back(i), z[u]= z[v]= -1;
 return {q, partner};
}

int main() {
 const cf1615g::Reduction R= cf1615g::read_and_build();
 Graph g(R.nv);
 for(auto [u, v]: R.edges) g.add_edge(u, v);
 const auto [ids, mate]= general_matching(g);
 cf1615g::restore_and_print(R, mate);
}
