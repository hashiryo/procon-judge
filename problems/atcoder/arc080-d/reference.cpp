// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/arc080/tasks/arc080_d
#include <iostream>
#include <vector>
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
namespace math_internal {
template <class Uint, class MP, u32... args> constexpr bool miller_rabin(Uint n) {
 const MP md(n);
 const Uint s= __builtin_ctzll(n - 1), d= n >> s, one= md.set(1), n1= md.norm(md.set(n - 1));
 for(u32 a: (u32[]){args...})
  if(Uint b= a % n; b)
   if(Uint p= md.norm(pow(md.set(b), d, md)); p != one)
    for(int i= s; p != n1; p= md.norm(md.mul(p, p)))
     if(!(--i)) return 0;
 return 1;
}
}
constexpr bool is_prime(unsigned long long n) {
 if(n < 2 || n % 6 % 4 != 1) return (n | 1) == 3;
 if(n < (1 << 30)) return math_internal::miller_rabin<unsigned, math_internal::MP_Mo32, 2, 7, 61>(n);
 if(n < (1ull << 62)) return math_internal::miller_rabin<unsigned long long, math_internal::MP_Mo64, 2, 325, 9375, 28178, 450775, 9780504, 1795265022>(n);
 if(n < (1ull << 63)) return math_internal::miller_rabin<unsigned long long, math_internal::MP_D2B1_1, 2, 325, 9375, 28178, 450775, 9780504, 1795265022>(n);
 return math_internal::miller_rabin<unsigned long long, math_internal::MP_D2B1_2, 2, 325, 9375, 28178, 450775, 9780504, 1795265022>(n);
}

#include <limits>
#include <iterator>
#include <vector>
#include <queue>
#include <algorithm>
template <class cost_t, bool min_perfect= false> class WeightedMatching {
 static constexpr cost_t INF= std::numeric_limits<cost_t>::max() / 4;
 struct E {
  int16_t u, v;
  cost_t w;
 };
 template <class T> struct Mat {
  int n;
  std::vector<T> dat;
  Mat(int n): n(n), dat(n * n){};
  auto operator[](int i) { return std::next(dat.begin(), n * i); }
  const auto operator[](int i) const { return std::next(dat.cbegin(), n * i); }
 };
 cost_t mx;
 Mat<cost_t> adj;
 std::vector<int16_t> mt;
 static inline int sgn(cost_t c) {
  static constexpr cost_t EPS= 1e-10;
  return c < -EPS ? -1 : c > EPS ? 1 : 0;
 }
public:
 WeightedMatching(int n): mx(0), adj(n), mt(2 * n) {
  if constexpr (min_perfect)
   for (int i= n; i--;)
    for (int j= n; j--;) adj[i][j]= INF;
 }
 void add_edge(int u, int v, cost_t w) {
  mx= std::max(mx, w);
  if constexpr (min_perfect) {
   if (adj[u][v] > w) adj[u][v]= adj[v][u]= w;
  } else if (adj[u][v] < w) adj[u][v]= adj[v][u]= w;
 }
 void build() {
  const int n= mt.size() / 2;
  int16_t rt[2 * n], used[2 * n], in= 0, m= n, q[2 * n], slk[2 * n], par[2 * n], isS[2 * n];
  Mat<int16_t> blg(2 * n);
  std::fill_n(used, 2 * n, 0), std::fill_n(rt + n + 1, n - 1, 0), rt[0]= 0;
  std::vector<std::vector<int16_t>> fwr(2 * n);
  std::queue<int16_t> que;
  std::vector<cost_t> dual(2 * n);
  Mat<E> G(2 * n);
  for (int16_t i= 0; i <= n; ++i) G[i][0]= E{i, int16_t(0), 0};
  for (int16_t j= 0; j <= n; ++j) G[0][j]= E{int16_t(0), j, 0};
  if constexpr (min_perfect)
   for (int16_t i= 1; i <= n; ++i)
    for (int16_t j= 1; j <= n; ++j) G[i][j]= E{i, j, adj[i - 1][j - 1] == INF ? 0 : (mx - adj[i - 1][j - 1] + 1) * 2};
  else
   for (int16_t i= 1; i <= n; ++i)
    for (int16_t j= 1; j <= n; ++j) G[i][j]= E{i, j, adj[i - 1][j - 1] * 2};
  cost_t inf= 0;
  for (int16_t i= 1; i <= n; ++i)
   for (int16_t j= 1; j <= n; ++j) inf= std::max(inf, G[i][j].w);
  inf+= 1;
  for (int i= 1; i <= n; i++) rt[i]= i, blg[i][i]= i, dual[i]= inf;
  auto dist= [&](const E &e) { return dual[e.u] - e.w + dual[e.v]; };
  auto push= [&](int v) {
   q[0]= v;
   for (int i= 0, s= 1; i < s; ++i) {
    if (v= q[i]; v > n)
     for (int u: fwr[v]) q[s++]= u;
    else que.push(v);
   }
  };
  auto match= [&](auto self, int u, int v) -> void {
   if (mt[u]= G[u][v].v; u <= n) return;
   int x= blg[u][G[u][v].u], p= std::find(fwr[u].begin(), fwr[u].end(), x) - fwr[u].begin();
   if (p & 1) std::reverse(fwr[u].begin() + 1, fwr[u].end()), p= fwr[u].size() - p;
   for (int i= 0; i < p; ++i) self(self, fwr[u][i], fwr[u][i ^ 1]);
   self(self, x, v), std::rotate(fwr[u].begin(), fwr[u].begin() + p, fwr[u].end());
  };
  auto path= [&](const E &e) {
   if (int u= rt[e.u], v= rt[e.v], bu= u, bv= v, x; isS[v] == 1) {
    for (++in; bu; bu= rt[mt[bu]] ? rt[par[rt[mt[bu]]]] : 0) used[bu]= in;
    for (int id= n + 1; bv; bv= rt[mt[bv]] ? rt[par[rt[mt[bv]]]] : 0)
     if (used[bv] == in) {
      while (id <= m && rt[id]) ++id;
      for (int i= (m+= (id > m)); i; --i) G[id][i].w= G[i][id].w= 0;
      for (int i= n; --i;) blg[id][i]= 0;
      mt[id]= mt[bv];
      for (fwr[id].clear(); u != bv; fwr[id].push_back(u), u= rt[par[u]]) fwr[id].push_back(u), push(u= rt[mt[u]]);
      fwr[id].push_back(bv), std::reverse(fwr[id].begin(), fwr[id].end());
      for (; v != bv; fwr[id].push_back(v), v= rt[par[v]]) fwr[id].push_back(v), push(v= rt[mt[v]]);
      isS[id]= 1, dual[id]= 0, q[0]= id;
      for (int i= 0, s= 1; i < s; ++i)
       if (rt[v= q[i]]= id; v > n)
        for (int t: fwr[v]) q[s++]= t;
      for (int c: fwr[id]) {
       for (int i= 1; i <= m; ++i)
        if (sgn(G[id][i].w) == 0 || sgn(dist(G[c][i]) - dist(G[id][i])) < 0) G[id][i]= G[c][i], G[i][id]= G[i][c];
       for (int i= 1; i <= n; ++i)
        if (blg[c][i]) blg[id][i]= c;
      }
      for (int i= slk[id]= 0; i <= n; ++i)
       if (sgn(G[i][id].w) != 0 && rt[i] != id && isS[rt[i]] == 1)
        if (!slk[id] || sgn(dist(G[i][id]) - dist(G[slk[id]][id])) < 0) slk[id]= i;
      return false;
     }
    for (bu= u, bv= v;; match(match, bv= x, bu= rt[par[x]]))
     if (x= rt[mt[bu]], match(match, bu, bv); !x)
      for (;; match(match, u= x, v= rt[par[x]]))
       if (x= rt[mt[v]], match(match, v, u); !x) return true;
   } else if (!isS[v]) par[v]= e.u, isS[v]= 2, slk[v]= slk[x= rt[mt[v]]]= 0, isS[x]= 1, push(x);
   return false;
  };
 LABEL_AUGMENT:
  std::fill_n(isS, 2 * n, 0), std::fill_n(slk, 2 * n, 0), std::fill_n(par, 2 * n, 0), que= std::queue<int16_t>();
  for (int i= 1; i <= m; i++)
   if (rt[i] == i && !mt[i]) isS[i]= 1, push(i);
  if (que.empty()) return;
  for (cost_t del= inf;; del= inf) {
   for (int v, i; !que.empty();)
    for (v= que.front(), que.pop(), i= 1; i <= n; ++i)
     if (sgn(G[v][i].w) != 0 && rt[i] != rt[v]) {
      if (sgn(dist(G[v][i])) == 0) {
       if (path(G[v][i])) goto LABEL_AUGMENT;
      } else if (isS[rt[i]] != 2)
       if (!slk[rt[i]] || sgn(dist(G[v][rt[i]]) - dist(G[slk[rt[i]]][rt[i]])) < 0) slk[rt[i]]= v;
     }
   for (int i= n + 1; i <= m; ++i)
    if (rt[i] == i && isS[i] == 2 && sgn(del - dual[i] / 2) > 0) del= dual[i] / 2;
   for (int i= 1; i <= m; ++i)
    if (rt[i] == i && slk[i] && isS[i] != 2)
     if (cost_t c= dist(G[slk[i]][i]) / (1 + isS[i]); sgn(del - c) > 0) del= c;
   for (int i= 1; i <= n; ++i) {
    if (isS[rt[i]] == 1) {
     if (sgn(dual[i]-= del) <= 0) return;
    } else if (isS[rt[i]] == 2) dual[i]+= del;
   }
   for (int i= n + 1; i <= m; ++i)
    if (rt[i] == i && isS[i]) dual[i]+= isS[i] == 1 ? 2 * del : -2 * del;
   for (int i= 1; i <= m; ++i)
    if (rt[i] == i && slk[i] && rt[slk[i]] != i && sgn(dist(G[slk[i]][i])) == 0)
     if (path(G[slk[i]][i])) goto LABEL_AUGMENT;
   for (int b= n + 1, x, p, T, S; b <= m; b++)
    if (rt[b] == b && isS[b] == 2 && sgn(dual[b]) == 0) {
     isS[x= blg[b][G[b][par[b]].u]]= 2, par[x]= par[b];
     for (int c: fwr[b]) {
      q[0]= c;
      for (int i= 0, s= 1, v; i < s; ++i)
       if (rt[v= q[i]]= c; v > n)
        for (int u: fwr[v]) q[s++]= u;
     }
     p= std::find(fwr[b].begin(), fwr[b].end(), x) - fwr[b].begin(), x= fwr[b].size();
     if (p & 1) std::reverse(fwr[b].begin() + 1, fwr[b].end()), p= fwr[b].size() - p;
     for (int i= 0; i < p; i+= 2) isS[S= fwr[b][i + 1]]= 1, isS[T= fwr[b][i]]= 2, par[T]= G[S][T].u, slk[S]= slk[T]= 0, push(S);
     for (int i= p + 1, j; i < x; ++i)
      for (T= fwr[b][i], j= isS[T]= slk[T]= 0; j <= n; ++j)
       if (sgn(G[j][T].w) != 0 && rt[j] != T && isS[rt[j]] == 1)
        if (!slk[T] || sgn(dist(G[j][T]) - dist(G[slk[T]][T])) < 0) slk[T]= j;
     fwr[b].clear(), rt[b]= 0;
    }
  }
  goto LABEL_AUGMENT;
 }
 int match(int v) const { return mt[v + 1] - 1; }
 std::vector<E> weight_matching() const {
  const int n= mt.size() / 2;
  std::vector<E> ret;
  if constexpr (min_perfect) {
   if (n & 1) return {};  // no solution
   for (int16_t i= 0; i < n; ++i)
    if (int16_t j= match(i); i < j) ret.push_back(E{i, j, adj[i][j]});
   if (2 * int(ret.size()) != n) return {};  // no solution
  } else
   for (int16_t i= 0; i < n; ++i)
    if (int16_t j= match(i); i < j) ret.push_back(E{i, j, adj[i][j]});
  return ret;
 }
};
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 int x[N];
 for(int i= 0; i < N; ++i) cin >> x[i];
 vector<int> vs= {x[0]};
 for(int i= 1; i < N; ++i)
  if(x[i] - x[i - 1] > 1) vs.push_back(x[i - 1] + 1), vs.push_back(x[i]);
 vs.push_back(x[N - 1] + 1);
 int m= vs.size();
 WeightedMatching<long long, true> graph(m);
 for(int i= m; --i;)
  for(int j= i; j--;) {
   int k= vs[i] - vs[j];
   graph.add_edge(i, j, k & 1 ? is_prime(k) ? 1 : 3 : 2);
  }
 graph.build();
 long long ans= 0;
 for(auto [u, v, w]: graph.weight_matching()) ans+= w;
 cout << ans << '\n';
 return 0;
}
