// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
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

#include <algorithm>
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

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 Graph g(N, N);
 for(int i= 0, X; i < N; ++i) cin >> X, g[i]= {i, --X};
 vector<int> C(N);
 for(int i= 0; i < N; ++i) cin >> C[i];
 StronglyConnectedComponents scc(g);
 long long ans= 0;
 for(int i= scc.size(); i--;) {
  auto blk= scc.block(i);
  if(blk.size() == 1) continue;
  int mn= 1 << 30;
  for(int v: blk) mn= min(mn, C[v]);
  ans+= mn;
 }
 cout << ans << '\n';
 return 0;
}
