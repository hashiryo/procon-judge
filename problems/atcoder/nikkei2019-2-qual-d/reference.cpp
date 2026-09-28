// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/nikkei2019-2-qual/tasks/nikkei2019_2_qual_d
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
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

template <typename cost_t= int> class RangeToRangeGraph {
 int n;
 inline int to_upper_idx(int i) const { return i >= n ? i - n : n + i; }
 inline int to_lower_idx(int i) const { return i >= n ? i - n : n + n + i; }
public:
 Graph graph;
 std::vector<cost_t> weight;
 RangeToRangeGraph(int n): n(n), graph(n * 3) {
  for(int i= 2; i < n + n; ++i) add(to_upper_idx(i / 2), to_upper_idx(i));
  for(int i= 2; i < n + n; ++i) add(to_lower_idx(i), to_lower_idx(i / 2));
 }
 void add(int s, int t, cost_t w= 0) { graph.add_edge(s, t), weight.emplace_back(w); }
 // [s_l, s_r) -> t
 void add_from_range(int s_l, int s_r, int t, cost_t w= 0) {
  for(int l= s_l + n, r= s_r + n; l < r; l>>= 1, r>>= 1) {
   if(l & 1) add(to_lower_idx(l++), t, w);
   if(r & 1) add(to_lower_idx(--r), t, w);
  }
 }
 // s -> [t_l, t_r)
 void add_to_range(int s, int t_l, int t_r, cost_t w= 0) {
  for(int l= t_l + n, r= t_r + n; l < r; l>>= 1, r>>= 1) {
   if(l & 1) add(s, to_upper_idx(l++), w);
   if(r & 1) add(s, to_upper_idx(--r), w);
  }
 }
 // [s_l, s_r) -> [t_l, t_r)
 void add_from_range_to_range(int s_l, int s_r, int t_l, int t_r, cost_t w= 0) { add_from_range(s_l, s_r, graph.n, w), add_to_range(graph.n, t_l, t_r, 0), ++graph.n; }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, M;
 cin >> N >> M;
 RangeToRangeGraph<int> r2r(N);
 for(int i= 0; i < M; ++i) {
  int L, R, C;
  cin >> L >> R >> C;
  --L;
  r2r.add_from_range_to_range(L, R, L, R, C);
 }
 int n= r2r.graph.vertex_size();
 auto adj= r2r.graph.adjacency_edge(1);
 long long dist[n];
 static constexpr long long INF= 1ll << 60;
 fill_n(dist, n, INF);
 priority_queue<pair<long long, int>> pq;
 pq.emplace(0, 0);
 dist[0]= 0;
 while(!pq.empty()) {
  auto [d, v]= pq.top();
  pq.pop();
  if(-d != dist[v]) continue;
  for(auto e: adj[v]) {
   int u= r2r.graph[e].to(v);
   int w= r2r.weight[e];
   long long cost= dist[v] + w;
   if(dist[u] > cost) {
    dist[u]= cost;
    pq.emplace(-cost, u);
   }
  }
 }
 cout << (dist[N - 1] == INF ? -1 : dist[N - 1]) << '\n';
 return 0;
}
