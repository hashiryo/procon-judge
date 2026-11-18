// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの (2026-11-18、Library 9d0220620)。
// Library を直しても変わらない。小さい入力では brute.cpp (最小カットを全部数え上げる) と、gen.py の seed 1000 から 2999 で
// 一致を確かめてある。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// 今の Library の MaxFlow (Dinic) で s から t へ流し、残余グラフ (残りの容量のある向きの弧) を Graph に組んで
// StronglyConnectedComponents で強連結成分に分ける。辺が飽和していて両端の成分が違えば、どれかの最小カットに入る。さらに
// 始点が s の成分、終点が t の成分にあれば、すべての最小カットに入る。
#include <cstdio>
#include <string>
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

class StronglyConnectedComponents {
 std::vector<int> m, q, b;
public:
 StronglyConnectedComponents(const Graph& g) {
  const int n= g.vertex_size();
  m.assign(n, -2), b.resize(n);
  {
   auto adj= g.adjacency_vertex(1);
   std::vector<int> c(adj.p.begin(), adj.p.begin() + n);
   for(int s= 0, k= n, p; s < n; ++s)
    if(m[s] == -2)
     for(m[p= s]= -1; p >= 0;) {
      if(c[p] == adj.p[p + 1]) b[--k]= p, p= m[p];
      else if(int w= adj.dat[c[p]++]; m[w] == -2) m[w]= p, p= w;
     }
  }
  auto adj= g.adjacency_vertex(-1);
  std::vector<char> z(n);
  int k= 0, p= 0;
  q= {0};
  for(int s: b)
   if(!z[s]) {
    for(z[m[k++]= s]= 1; p < k; ++p)
     for(int u: adj[m[p]])
      if(!z[u]) z[m[k++]= u]= 1;
    q.push_back(k);
   }
  for(int i= q.size() - 1; i--;)
   while(k > q[i]) b[m[--k]]= i;
 }
 size_t size() const { return q.size() - 1; }
 ConstListRange<int> block(int k) const { return {m.cbegin() + q[k], m.cbegin() + q[k + 1]}; }
 int operator()(int i) const { return b[i]; }
 Graph dag(const Graph& g) const {
  Graph ret(size());
  for(auto [s, d]: g)
   if(b[s] != b[d]) ret.add_edge(b[s], b[d]);
  return std::sort(ret.begin(), ret.end()), ret.erase(std::unique(ret.begin(), ret.end()), ret.end()), ret;
 }
};

#include <vector>
#include <numeric>
#include <algorithm>
#include <limits>
#include <queue>
#include <array>
#include <cassert>
template <typename FlowAlgo> struct MaxFlow: public FlowAlgo {
 using FlowAlgo::FlowAlgo;
 using Edge= typename FlowAlgo::Edge;
 using flow_t= decltype(Edge::cap);
 int add_vertex() { return this->adj.resize(++this->n), this->n - 1; }
 std::vector<int> add_vertices(const std::size_t size) {
  std::vector<int> ret(size);
  std::iota(ret.begin(), ret.end(), this->n);
  return this->adj.resize(this->n+= size), ret;
 }
 class EdgePtr {
  friend struct MaxFlow;
  MaxFlow *ins;
  int v, e;
  bool bd;
  Edge &edge() { return ins->adj[v][e]; }
  Edge &rev() {
   Edge &e= edge();
   return ins->adj[e.dst][e.rev];
  }
  EdgePtr(MaxFlow *ins, int v, int e, bool d): ins(ins), v(v), e(e), bd(d) {}
 public:
  EdgePtr()= default;
  int src() const { return v; }
  int dst() { return edge().dst; }
  bool is_direct() const { return !bd; }
  flow_t flow() { return cap() - edge().cap; }
  flow_t cap() { return (edge().cap + rev().cap) / (1 + bd); }
  flow_t change_cap(flow_t new_cap, int s, int t) {
   assert(0 <= new_cap);
   Edge &e= edge(), &re= rev();
   flow_t diff= new_cap - cap(), ext= std::abs(flow()) - new_cap;
   if (ext <= 0) return e.cap+= diff, re.cap+= diff * bd, 0;
   int sr= src(), ds= dst();
   if (flow() < 0) std::swap(sr, ds);
   if (bd) {
    if (sr == src()) re.cap+= 2 * diff + e.cap, e.cap= 0;
    else e.cap+= 2 * diff + re.cap, re.cap= 0;
   } else re.cap+= diff;
   ext-= ins->maxflow(sr, ds, ext);
   if (ds != t) ins->maxflow(t, ds, ext);
   if (sr != s) ins->maxflow(sr, s, ext);
   return ext;
  }
 };
 EdgePtr add_edge(int src, int dst, flow_t cap, bool bidir= false) {
  assert(0 <= src && src < this->n), assert(0 <= dst && dst < this->n), assert(0 <= cap);
  int e= this->adj[src].size(), re= src == dst ? e + 1 : this->adj[dst].size();
  return this->adj[src].push_back(Edge{dst, re, cap}), this->adj[dst].push_back(Edge{src, e, cap * bidir}), this->m++, EdgePtr{this, src, e, bidir};
 }
 flow_t maxflow(int s, int t) { return maxflow(s, t, std::numeric_limits<flow_t>::max()); }
 flow_t maxflow(int s, int t, flow_t flow_lim) { return this->flow(s, t, flow_lim); }
 std::vector<bool> mincut(int s) {
  std::vector<bool> visited(this->n);
  static std::queue<int> que;
  for (que.push(s); !que.empty();) {
   s= que.front(), que.pop(), visited[s]= true;
   for (const auto &e: this->adj[s])
    if (e.cap && !visited[e.dst]) visited[e.dst]= true, que.push(e.dst);
  }
  return visited;
 }
};
template <typename FlowAlgo> class MaxFlowLowerBound: public FlowAlgo {
 using Edge= typename FlowAlgo::Edge;
 using flow_t= decltype(Edge::cap);
 std::vector<flow_t> in;
 int add_edge(int src, int dst, flow_t cap) {
  int e= this->adj[src].size(), re= src == dst ? e + 1 : this->adj[dst].size();
  return this->adj[src].push_back(Edge{dst, re, cap}), this->adj[dst].push_back(Edge{src, e, 0}), this->m++, re;
 }
public:
 MaxFlowLowerBound(std::size_t n= 0): FlowAlgo(n + 2), in(n) {}
 int add_vertex() { return this->adj.resize(++this->n), in.resize(this->n - 2, 0), this->n - 3; }
 std::vector<int> add_vertices(const std::size_t size) {
  std::vector<int> ret(size);
  return std::iota(ret.begin(), ret.end(), this->n - 2), this->adj.resize(this->n+= size), in.resize(this->n - 2, 0), ret;
 }
 class EdgePtr {
  friend class MaxFlowLowerBound;
  MaxFlowLowerBound *ins;
  int v, e;
  flow_t u;
  const Edge &edge() const { return ins->adj[v][e]; }
  const Edge &rev() const {
   const Edge &e= edge();
   return ins->adj[e.dst][e.rev];
  }
  EdgePtr(MaxFlowLowerBound *ins, int v, int e, flow_t u): ins(ins), v(v), e(e), u(u) {}
 public:
  EdgePtr()= default;
  int src() const { return v - 2; }
  int dst() const { return edge().dst - 2; }
  flow_t flow() const { return u - edge().cap; }
  flow_t lower() const { return flow() - rev().cap; }
  flow_t upper() const { return u; }
 };
 EdgePtr add_edge(int src, int dst, flow_t lower, flow_t upper) {
  assert(lower <= upper), src+= 2, dst+= 2, assert(0 <= src && src < this->n), assert(0 <= dst && dst < this->n), ++this->m;
  int e= this->adj[src].size(), re= src == dst ? e + 1 : this->adj[dst].size();
  if (lower * upper <= 0) this->adj[src].push_back(Edge{dst, re, upper}), this->adj[dst].push_back(Edge{src, e, -lower});
  else if (lower > 0) in[src - 2]-= lower, in[dst - 2]+= lower, this->adj[src].push_back(Edge{dst, re, upper - lower}), this->adj[dst].push_back(Edge{src, e, 0});
  else in[src - 2]-= upper, in[dst - 2]+= upper, this->adj[src].push_back(Edge{dst, re, 0}), this->adj[dst].push_back(Edge{src, e, upper - lower});
  return EdgePtr(this, src, e, upper);
 }
 flow_t maxflow(int s, int t) {
  static constexpr flow_t INF= std::numeric_limits<flow_t>::max();
  flow_t sum= 0;
  for (int i= this->n - 2; i--;) {
   if (in[i] > 0) add_edge(0, i + 2, in[i]), sum+= in[i];
   if (in[i] < 0) add_edge(i + 2, 1, -in[i]);
  }
  int re= add_edge(t+= 2, s+= 2, INF);
  if (this->flow(0, 1, INF) < sum) return -1;  // no solution
  return this->flow(s, t, INF) + this->adj[s][re].cap;
 }
};
template <class flow_t> struct Dinic {
 Dinic(std::size_t _n= 0): n(_n), m(0), adj(n) {}
protected:
 struct Edge {
  int dst, rev;
  flow_t cap;
 };
 int n, m;
 std::vector<std::vector<Edge>> adj;
 std::vector<int> level, iter;
 inline void levelize(int s, int t, int u= 0) {
  std::queue<int> que;
  for (que.push(s), level.assign(n, -1), level[s]= 0; !que.empty();) {
   u= que.front(), que.pop();
   for (auto &e: adj[u])
    if (e.cap > 0 && level[e.dst] < 0) {
     if (level[e.dst]= level[u] + 1; e.dst == t) return;
     que.push(e.dst);
    }
  }
 }
 inline flow_t dfs(int u, int s, flow_t cur, flow_t ret= 0, flow_t f= 0) {
  if (u == s) return cur;
  for (int &i= iter[u], ed= adj[u].size(); i < ed; i++) {
   Edge &e= adj[u][i], &re= adj[e.dst][e.rev];
   if (level[u] <= level[e.dst] || re.cap == 0) continue;
   if ((f= dfs(e.dst, s, std::min(cur - ret, re.cap))) <= 0) continue;
   if (e.cap+= f, re.cap-= f, ret+= f; ret == cur) return ret;
  }
  return level[u]= n, ret;
 }
 flow_t flow(int s, int t, flow_t lim, flow_t ret= 0) {
  assert(0 <= s && s < n), assert(0 <= t && t < n), assert(s != t);
  for (flow_t f; ret < lim; ret+= f) {
   if (levelize(s, t), level[t] == -1) break;
   if (iter.assign(n, 0); !(f= dfs(t, s, lim - ret))) break;
  }
  return ret;
 }
};
template <class flow_t, unsigned global_freq= 4, bool use_gap= true, bool freeze= false> struct PushRelabel {
 PushRelabel(std::size_t _n= 0): n(_n), m(0), adj(n) {}
protected:
 struct Edge {
  int dst, rev;
  flow_t cap;
 };
 int n, gap, m;
 struct {
  std::vector<std::array<int, 2>> ev, od;
  int se, so;
  void init(int _n) { ev.resize(_n), od.resize(_n), se= so= 0; };
  void clear() { se= so= 0; }
  inline bool empty() const { return se + so == 0; }
  void push(int i, int h) { (h & 1 ? od[so++] : ev[se++])= {i, h}; }
  inline int highest() const { return std::max(se ? ev[se - 1][1] : -1, so ? od[so - 1][1] : -1); }
  inline int pop() { return !se || (so && od[so - 1][1] > ev[se - 1][1]) ? od[--so][0] : ev[--se][0]; }
 } hque;
 std::vector<std::vector<Edge>> adj;
 std::vector<int> dist, dcnt;
 std::vector<flow_t> exc;
 inline void calc(int t) {
  if constexpr (global_freq != 0) relabel(t);
  for (int tick= m * global_freq; !hque.empty();) {
   int i= hque.pop(), dnxt= n * 2 - 1;
   if constexpr (use_gap)
    if (dist[i] > gap) continue;
   for (auto &e: adj[i])
    if (e.cap) {
     if (dist[e.dst] == dist[i] - 1) {
      if (push(i, e), exc[i] == 0) break;
     } else if (dist[e.dst] + 1 < dnxt) dnxt= dist[e.dst] + 1;
    }
   if (exc[i] > 0) {
    if constexpr (use_gap) {
     if (dnxt != dist[i] && dcnt[dist[i]] == 1 && dist[i] < gap) gap= dist[i];
     if (dnxt == gap) gap++;
     while (hque.highest() > gap) hque.pop();
     if (dnxt > gap) dnxt= n;
     if (dist[i] != dnxt) dcnt[dist[i]]--, dcnt[dnxt]++;
    }
    dist[i]= dnxt, hq_push(i);
   }
   if constexpr (global_freq != 0)
    if (--tick == 0) tick= m * global_freq, relabel(t);
  }
 }
 inline void hq_push(int i) {
  if constexpr (!use_gap) hque.push(i, dist[i]);
  else if (dist[i] < gap) hque.push(i, dist[i]);
 }
 inline void push(int i, Edge &e) {
  flow_t del= std::min(e.cap, exc[i]);
  if (exc[i]-= del, e.cap-= del, exc[e.dst]+= del, adj[e.dst][e.rev].cap+= del; 0 < exc[e.dst] && exc[e.dst] <= del) hq_push(e.dst);
 }
 inline void relabel(int t) {
  dist.assign(n, n), dist[t]= 0;
  static std::queue<int> q;
  q.push(t), hque.clear();
  if constexpr (use_gap) gap= 1, dcnt.assign(n + 1, 0);
  for (int now; !q.empty();) {
   now= q.front(), q.pop();
   if constexpr (use_gap) gap= dist[now] + 1, dcnt[dist[now]]++;
   if (exc[now] > 0) hque.push(now, dist[now]);
   for (const auto &e: adj[now])
    if (adj[e.dst][e.rev].cap && dist[e.dst] == n) dist[e.dst]= dist[now] + 1, q.push(e.dst);
  }
 }
 flow_t flow(int s, int t, flow_t lim, flow_t ret= 0) {
  assert(0 <= s && s < n), assert(0 <= t && t < n), assert(s != t), hque.init(n), exc.assign(n, 0), exc[s]+= lim, exc[t]-= lim, dist.assign(n, 0), dist[s]= n;
  if constexpr (use_gap) gap= 1, dcnt.assign(n + 1, 0), dcnt[0]= n - 1;
  for (auto &e: adj[s]) push(s, e);
  calc(t), ret= exc[t] + lim;
  if constexpr (!freeze) {
   exc[s]+= exc[t], exc[t]= 0;
   if constexpr (global_freq != 0) relabel(s);
   calc(s), assert(exc == std::vector<flow_t>(n, 0));
  }
  return ret;
 }
};
int main() {
 int n, m, s, t;
 if(scanf("%d %d %d %d", &n, &m, &s, &t) != 4) return 1;
 --s, --t;
 using MF= MaxFlow<Dinic<long long>>;
 MF g(n);
 std::vector<MF::EdgePtr> es;
 es.reserve(m);
 for(int i= 0; i < m; ++i) {
  int u, v, c;
  if(scanf("%d %d %d", &u, &v, &c) != 3) return 1;
  es.push_back(g.add_edge(u - 1, v - 1, c));
 }
 g.maxflow(s, t);
 Graph r(n);
 for(auto& e: es) {
  if(e.flow() < e.cap()) r.add_edge(e.src(), e.dst());
  if(e.flow() > 0) r.add_edge(e.dst(), e.src());
 }
 StronglyConnectedComponents scc(r);
 std::string out;
 for(auto& e: es) {
  const bool full= e.flow() == e.cap();
  const int a= scc(e.src()), b= scc(e.dst());
  out+= full && a != b ? '1' : '0', out+= ' ', out+= full && a == scc(s) && b == scc(t) ? '1' : '0', out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
