// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <algorithm>
#include <iostream>
#include <utility>
#include <type_traits>
#include <cassert>
template <class Uint> constexpr inline Uint mod_inv(Uint a, Uint mod) {
 std::make_signed_t<Uint> x= 1, y= 0, z= 0;
 for (Uint q= 0, b= mod, c= 0; b;) z= x, x= y, y= z - y * (q= a / b), c= a, a= b, b= c - b * q;
 return assert(a == 1), x < 0 ? mod - (-x) % mod : x % mod;
}
namespace math_internal {
using namespace std;
using u8= unsigned char;
using u32= unsigned;
using i64= long long;
using u64= unsigned long long;
using u128= __uint128_t;
struct MP_Na {  // mod < 2^32
 u32 mod;
 constexpr MP_Na(): mod(0) {}
 constexpr MP_Na(u32 m): mod(m) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return u64(l) * r % mod; }
 constexpr inline u32 set(u32 n) const { return n; }
 constexpr inline u32 get(u32 n) const { return n; }
 constexpr inline u32 norm(u32 n) const { return n; }
 constexpr inline u32 plus(u64 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 diff(u64 l, u32 r) const { return l-= r, l >> 63 ? l + mod : l; }
};
template <class u_t, class du_t, u8 B> struct MP_Mo {  // mod < 2^32, mod < 2^62
 u_t mod;
 constexpr MP_Mo(): mod(0), iv(0), r2(0) {}
 constexpr MP_Mo(u_t m): mod(m), iv(inv(m)), r2(-du_t(mod) % mod) {}
 constexpr inline u_t mul(u_t l, u_t r) const { return reduce(du_t(l) * r); }
 constexpr inline u_t set(u_t n) const { return mul(n, r2); }
 constexpr inline u_t get(u_t n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u_t norm(u_t n) const { return n >= mod ? n - mod : n; }
 constexpr inline u_t plus(u_t l, u_t r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u_t diff(u_t l, u_t r) const { return l-= r, l >> (B - 1) ? l + (mod << 1) : l; }
private:
 u_t iv, r2;
 static constexpr u_t inv(u_t n, int e= 6, u_t x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u_t reduce(const du_t &w) const { return u_t(w >> B) + mod - ((du_t(u_t(w) * iv) * mod) >> B); }
};
using MP_Mo32= MP_Mo<u32, u64, 32>;
using MP_Mo64= MP_Mo<u64, u128, 64>;
struct MP_Br {  // 2^20 < mod <= 2^41
 u64 mod;
 constexpr MP_Br(): mod(0), x(0) {}
 constexpr MP_Br(u64 m): mod(m), x((u128(1) << 84) / m) {}
 constexpr inline u64 mul(u64 l, u64 r) const { return rem(u128(l) * r); }
 static constexpr inline u64 set(u64 n) { return n; }
 constexpr inline u64 get(u64 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u64 norm(u64 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u64 plus(u64 l, u64 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u64 diff(u64 l, u64 r) const { return l-= r, l >> 63 ? l + (mod << 1) : l; }
private:
 u64 x;
 constexpr inline u128 quo(const u128 &n) const { return (n * x) >> 84; }
 constexpr inline u64 rem(const u128 &n) const { return n - quo(n) * mod; }
};
template <class du_t, u8 B> struct MP_D2B1 {  // mod < 2^63, mod < 2^64
 u64 mod;
 constexpr MP_D2B1(): mod(0), s(0), d(0), v(0) {}
 constexpr MP_D2B1(u64 m): mod(m), s(__builtin_clzll(m)), d(m << s), v(u128(-1) / d) {}
 constexpr inline u64 mul(u64 l, u64 r) const { return rem((u128(l) * r) << s) >> s; }
 constexpr inline u64 set(u64 n) const { return n; }
 constexpr inline u64 get(u64 n) const { return n; }
 constexpr inline u64 norm(u64 n) const { return n; }
 constexpr inline u64 plus(du_t l, u64 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u64 diff(du_t l, u64 r) const { return l-= r, l >> B ? l + mod : l; }
private:
 u8 s;
 u64 d, v;
 constexpr inline u64 rem(const u128 &u) const {
  u128 q= (u >> 64) * v + u;
  u64 r= u64(u) - (q >> 64) * d - d;
  if (r > u64(q)) r+= d;
  if (r >= d) r-= d;
  return r;
 }
};
using MP_D2B1_1= MP_D2B1<u64, 63>;
using MP_D2B1_2= MP_D2B1<u128, 127>;
template <class u_t, class MP> constexpr u_t pow(u_t x, u64 k, const MP &md) {
 for (u_t ret= md.set(1);; x= md.mul(x, x))
  if (k & 1 ? ret= md.mul(ret, x) : 0; !(k>>= 1)) return ret;
}
}
#include <type_traits>
namespace math_internal {
struct m_b {};
struct s_b: m_b {};
}
template <class mod_t> constexpr bool is_modint_v= std::is_base_of_v<math_internal::m_b, mod_t>;
template <class mod_t> constexpr bool is_staticmodint_v= std::is_base_of_v<math_internal::s_b, mod_t>;
namespace math_internal {
template <class MP, u64 MOD> struct SB: s_b {
protected:
 static constexpr MP md= MP(MOD);
};
template <class U, class B> struct MInt: public B {
 using Uint= U;
 static constexpr inline auto mod() { return B::md.mod; }
 constexpr MInt(): x(0) {}
 template <class T, typename= enable_if_t<is_modint_v<T> && !is_same_v<T, MInt>>> constexpr MInt(T v): x(B::md.set(v.val() % B::md.mod)) {}
 constexpr MInt(__int128_t n): x(B::md.set((n < 0 ? ((n= (-n) % B::md.mod) ? B::md.mod - n : n) : n % B::md.mod))) {}
 constexpr MInt operator-() const { return MInt() - *this; }
#define FUNC(name, op) \
 constexpr MInt name const { \
  MInt ret; \
  return ret.x= op, ret; \
 }
 FUNC(operator+(const MInt & r), B::md.plus(x, r.x))
 FUNC(operator-(const MInt & r), B::md.diff(x, r.x))
 FUNC(operator*(const MInt & r), B::md.mul(x, r.x))
 FUNC(pow(u64 k), math_internal::pow(x, k, B::md))
#undef FUNC
 constexpr MInt operator/(const MInt& r) const { return *this * r.inv(); }
 constexpr MInt& operator+=(const MInt& r) { return *this= *this + r; }
 constexpr MInt& operator-=(const MInt& r) { return *this= *this - r; }
 constexpr MInt& operator*=(const MInt& r) { return *this= *this * r; }
 constexpr MInt& operator/=(const MInt& r) { return *this= *this / r; }
 constexpr bool operator==(const MInt& r) const { return B::md.norm(x) == B::md.norm(r.x); }
 constexpr bool operator!=(const MInt& r) const { return !(*this == r); }
 constexpr bool operator<(const MInt& r) const { return B::md.norm(x) < B::md.norm(r.x); }
 constexpr inline MInt inv() const { return mod_inv<U>(val(), B::md.mod); }
 constexpr inline Uint val() const { return B::md.get(x); }
 friend ostream& operator<<(ostream& os, const MInt& r) { return os << r.val(); }
 friend istream& operator>>(istream& is, MInt& r) {
  i64 v;
  return is >> v, r= MInt(v), is;
 }
private:
 Uint x;
};
template <u64 MOD> using MP_B= conditional_t < (MOD < (1 << 30)) & MOD, MP_Mo32, conditional_t < MOD < (1ull << 32), MP_Na, conditional_t<(MOD < (1ull << 62)) & MOD, MP_Mo64, conditional_t<MOD<(1ull << 41), MP_Br, conditional_t<MOD<(1ull << 63), MP_D2B1_1, MP_D2B1_2>>>>>;
template <u64 MOD> using ModInt= MInt < conditional_t<MOD<(1 << 30), u32, u64>, SB<MP_B<MOD>, MOD>>;
}
using math_internal::ModInt;

#include <cassert>
#include <vector>
template <class mod_t> class FactorialPrecalculation {
 static_assert(is_modint_v<mod_t>);
 static inline std::vector<mod_t> iv, fct, fiv;
public:
 static void reset() { iv.clear(), fct.clear(), fiv.clear(); }
 static inline mod_t inv(int n) {
  assert(0 < n);
  if(int k= iv.size(); k <= n) {
   if(iv.resize(n + 1); !k) iv[1]= 1, k= 2;
   for(unsigned long long mod= mod_t::mod(), q; k <= n; ++k) q= (mod + k - 1) / k, iv[k]= iv[k * q - mod] * q;
  }
  return iv[n];
 }
 static inline mod_t fact(int n) {
  assert(0 <= n);
  if(int k= fct.size(); k <= n) {
   if(fct.resize(n + 1); !k) fct[0]= 1, k= 1;
   for(; k <= n; ++k) fct[k]= fct[k - 1] * k;
  }
  return fct[n];
 }
 static inline mod_t finv(int n) {
  assert(0 <= n);
  if(int k= fiv.size(); k <= n) {
   if(fiv.resize(n + 1); !k) fiv[0]= 1, k= 1;
   for(; k <= n; ++k) fiv[k]= fiv[k - 1] * inv(k);
  }
  return fiv[n];
 }
 static inline mod_t nPr(int n, int r) { return r < 0 || n < r ? mod_t(0) : fact(n) * finv(n - r); }
 // [x^r] (1 + x)^n
 static inline mod_t nCr(int n, int r) { return r < 0 || n < r ? mod_t(0) : fact(n) * finv(n - r) * finv(r); }
 // [x^r] (1 - x)^{-n}
 static inline mod_t nHr(int n, int r) { return !r ? mod_t(1) : nCr(n + r - 1, r); }
};

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
 using Mint= ModInt<int(1e9 + 7)>;
 using F= FactorialPrecalculation<Mint>;
 int N;
 cin >> N;
 Graph g(N, N - 1);
 for(int i= 0; i < N - 1; ++i) cin >> g[i], --g[i];
 using Data= pair<int, Mint>;
 auto put_edge= [&](int, int, const Data& d) { return d; };
 auto op= [&](const Data& l, const Data& r) { return Data{l.first + r.first, F::nCr(l.first + r.first, l.first) * l.second * r.second}; };
 auto put_vertex= [&](int, const Data& d) { return Data{d.first + 1, d.second}; };
 for(auto [_, x]: Rerooting<Data>(g, put_edge, op, Data{0, 1}, put_vertex)) cout << x << '\n';
 return 0;
}
