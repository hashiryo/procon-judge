// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <type_traits>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <queue>
#include <cstdlib>
#include <cassert>
template <class symbol_t> class Automaton {
 std::vector<int> table;
 std::vector<int8_t> info;
 std::vector<symbol_t> alph;
 const int m;
 template <class Map, class state_t, class F, class G, class H> void build(const state_t &initial_state, const F &transition, const G &is_accept, const H &abs_reject) {
  static_assert(std::is_same_v<bool, std::invoke_result_t<G, state_t>>);
  static_assert(std::is_same_v<bool, std::invoke_result_t<H, state_t>>);
  Map encode;
  std::vector<state_t> decode;
  int ts= 0;
  decode.push_back(initial_state), encode.emplace(initial_state, ts++);
  for (int i= 0, k= 0; i < ts; ++i) {
   auto s= decode[i];
   table.resize(table.size() + m);
   for (int j= 0; j < m; ++j) {
    if (auto t= transition(s, j); abs_reject(t)) table[k++]= -1;
    else if (auto it= encode.find(t); it != encode.end()) table[k++]= it->second;
    else table[k++]= ts, decode.push_back(t), encode.emplace(t, ts++);
   }
  }
  info.resize(ts);
  for (int i= ts; i--;) info[i]= is_accept(decode[i]);
 }
 Automaton(const std::vector<symbol_t> &alphabet): alph(alphabet), m(alph.size()) {}
public:
 template <class state_t, class F, class G, std::enable_if_t<std::is_invocable_r_v<state_t, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept): alph(alphabet), m(alph.size()) {
  std::sort(alph.begin(), alph.end());
  auto tr= [&](const state_t &s, int i) { return transition(s, alph[i]); };
  auto rej= [](const state_t &) { return false; };
  if constexpr (std::is_integral_v<state_t>) build<std::unordered_map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
  else build<std::map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
 }
 template <class state_t, class F, class G, std::enable_if_t<std::is_invocable_r_v<state_t, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept, const state_t &abs_rej_state): alph(alphabet), m(alph.size()) {
  std::sort(alph.begin(), alph.end());
  auto tr= [&](const state_t &s, int i) { return transition(s, alph[i]); };
  auto rej= [abs_rej_state](const state_t &s) { return s == abs_rej_state; };
  if constexpr (std::is_integral_v<state_t>) build<std::unordered_map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
  else build<std::map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
 }
 template <class state_t, class F, class G, std::enable_if_t<std::is_invocable_r_v<std::set<state_t>, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept): alph(alphabet), m(alph.size()) {
  static_assert(std::is_same_v<bool, std::invoke_result_t<G, state_t>>);
  std::sort(alph.begin(), alph.end());
  auto tr= [&](const std::set<state_t> &s, int i) {
   std::set<state_t> ret;
   for (const auto &x: s) {
    auto ys= transition(x, alph[i]);
    ret.insert(ys.begin(), ys.end());
   }
   return ret;
  };
  auto ac= [&](const std::set<state_t> &s) { return std::any_of(s.begin(), s.end(), is_accept); };
  auto rej= [](const std::set<state_t> &s) { return s == std::set<state_t>(); };
  build<std::map<std::set<state_t>, int>, std::set<state_t>>(std::set<state_t>({initial_state}), tr, ac, rej);
 }
 template <class state_t, class F, class G, class H, std::enable_if_t<std::is_invocable_r_v<std::set<state_t>, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept, const H &eps_trans): alph(alphabet), m(alph.size()) {
  static_assert(std::is_same_v<bool, std::invoke_result_t<G, state_t>>);
  static_assert(std::is_same_v<std::set<state_t>, std::invoke_result_t<H, state_t>>);
  std::sort(alph.begin(), alph.end());
  auto eps_closure= [&](std::set<state_t> s) {
   for (std::set<state_t> t; s != t;) {
    t= s;
    for (const auto &x: t) {
     auto ys= eps_trans(x);
     s.insert(ys.begin(), ys.end());
    }
   }
   return s;
  };
  auto tr= [&](const std::set<state_t> &s, int i) {
   std::set<state_t> ret;
   for (const auto &x: s) {
    auto ys= transition(x, alph[i]);
    ret.insert(ys.begin(), ys.end());
   }
   return eps_closure(ret);
  };
  auto ac= [&](const std::set<state_t> &s) { return std::any_of(s.begin(), s.end(), is_accept); };
  auto rej= [](const std::set<state_t> &s) { return s == std::set<state_t>(); };
  build<std::map<std::set<state_t>, int>, std::set<state_t>>(eps_closure({initial_state}), tr, ac, rej);
 }
 Automaton operator&(const Automaton &r) const {
  assert(alph == r.alph);
  const int S= info.size();
  auto tr= [&](int s, int q) {
   auto [s1, s0]= std::div(s, S);
   int t1= r.table[s1 * m + q], t0= table[s0 * m + q];
   return t0 == -1 || t1 == -1 ? -1 : t1 * S + t0;
  };
  auto ac= [&](int s) {
   auto [s1, s0]= std::div(s, S);
   return info[s0] == 1 && r.info[s1] == 1;
  };
  auto rej= [](int s) { return s == -1; };
  Automaton ret(alph);
  return ret.build<std::unordered_map<int, int>, int>(0, tr, ac, rej), ret;
 }
 template <class T, class A, class F> T dp_run(int n, const A &op, const T &ti, const F &f, const T &init) const {
  static_assert(std::is_same_v<T, std::invoke_result_t<A, T, T>>);
  static_assert(std::is_same_v<T, std::invoke_result_t<F, T, symbol_t, int>>);
  const size_t S= info.size();
  std::queue<std::pair<int, int>> que;
  T dp[2][S], ret= ti;
  bool in[2][S];
  for (std::fill_n(dp[0], S, ti), std::fill_n(dp[1], S, ti), std::fill_n(in[0], S, 0), std::fill_n(in[1], S, 0), dp[0][0]= init, que.emplace(0, 0), in[0][0]= 1; que.size();) {
   auto [s, i]= que.front();
   bool b= i & 1;
   T tmp= dp[b][s];
   if (que.pop(), in[b][s]= 0, dp[b][s]= ti; i == n) {
    if (info[s] == 1) ret= op(ret, tmp);
    continue;
   }
   auto l= table.cbegin() + s * m;
   for (int j= m; j--;)
    if (int t= l[j]; t != -1)
     if (dp[!b][t]= op(dp[!b][t], f(tmp, alph[j], i)); !in[!b][t]) que.emplace(t, i + 1), in[!b][t]= 1;
  }
  return ret;
 }
 template <class T> T num(int n) const {
  return dp_run(n, [](const T &l, const T &r) { return l + r; }, T(), [](const T &x, const auto &, auto) { return x; }, T(1));
 }
};
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

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<998244353>;
 string N;
 int M;
 cin >> N >> M;
 int n= N.length();
 int c= 0;
 for(int i= 0; i < M; i++) {
  int C;
  cin >> C, c|= 1 << C;
 }
 std::vector<int> alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 auto tr0= [&](int s, int q) {
  if(s >= n) return s;
  int d= N[s] - '0';
  if(q > d) return n + 1;
  if(q < d) return n;
  return s + 1;
 };
 auto ac0= [&](int) { return true; };
 Automaton dfa_le(alp, 0, tr0, ac0, n + 1);
 auto tr1= [&](int s, int q) { return s | ((q || s) << q); };
 auto ac1= [&](int s) { return (s & c) == c; };
 Automaton dfa_variety(alp, 0, tr1, ac1);
 auto dfa= dfa_le & dfa_variety;
 using T= array<Mint, 2>;
 auto op= [](const T& l, const T& r) { return T{l[0] + r[0], l[1] + r[1]}; };
 auto f= [](const T& v, int a, int) { return T{v[0], v[1] * 10 + v[0] * a}; };
 cout << dfa.dp_run(n, op, T{0, 0}, f, T{1, 0})[1] << '\n';
 return 0;
}
