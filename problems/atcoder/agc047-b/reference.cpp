// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <vector>
#include <string>
#include <array>
#include <cassert>
template <class K, class Int= int> class RollingHash {
 static inline std::vector<K> pw, hsh;
 static inline K bs;
 static inline std::vector<Int> str;
 static inline void set_pw(int n) {
  if (int m= pw.size(); m <= n)
   for (pw.resize(n + 1); m <= n; ++m) pw[m]= pw[m - 1] * bs;
 }
 int bg, n;
 RollingHash(int b, int n): bg(b), n(n) {}
 template <class C> static int bin_srch(int ok, int ng, const C &check) {
  for (int x; ng - ok > 1;) x= (ok + ng) / 2, (check(x) ? ok : ng)= x;
  return ok;
 }
 template <size_t I> static K concat(const std::array<RollingHash, I> &v) {
  K ret= 0;
  for (size_t i= 0; i < I; ++i) ret= ret * pw[v[i].n] + v[i].hash();
  return ret;
 }
public:
 static void init(K b) { bs= b, pw.assign(1, 1), hsh.assign(1, 0); }
 static K base_pow(int i) { return set_pw(i), pw[i]; }
 RollingHash()= default;
 RollingHash(const std::vector<Int> &v): bg(hsh.size() - 1), n(v.size()) {
  str.insert(str.end(), v.begin(), v.end()), set_pw(n), hsh.resize(bg + n + 1);
  for (int i= 0; i < n; ++i) hsh[bg + i + 1]= hsh[bg + i] * bs + v[i];
 }
 RollingHash(const std::string &s): RollingHash(std::vector<Int>(s.begin(), s.end())) {}
 inline size_t length() const { return n; }
 inline K hash() const { return hsh[bg + n] - hsh[bg] * pw[n]; }
 RollingHash sub(int b, int m) const {
  assert(b + m <= n), assert(m >= 0);
  return {bg + b, m};
 }
 RollingHash sub(int b) const {
  assert(b <= n);
  return {bg + b, n - b};
 }
 template <class... Args> friend std::enable_if_t<std::conjunction_v<std::is_same<Args, RollingHash>...>, K> concat_hash(const Args &...rh) { return concat(std::array{rh...}); }
 friend int lcp(const RollingHash &l, const RollingHash &r) {
  return bin_srch(0, std::min(l.n, r.n) + 1, [&](int x) { return l.sub(0, x) == r.sub(0, x); });
 }
 friend int lcs(const RollingHash &l, const RollingHash &r) {
  return bin_srch(0, std::min(l.n, r.n) + 1, [&](int x) { return l.sub(l.n - x) == r.sub(r.n - x); });
 }
 bool operator==(const RollingHash &r) const { return hash() == r.hash(); }
 bool operator!=(const RollingHash &r) const { return !(*this == r); }
 bool operator<(const RollingHash &r) const {
  int k= lcp(*this, r);
  return k == std::min(n, r.n) ? n < r.n : str[bg + k] < str[r.bg + k];
 }
 friend bool concat_cmp(const RollingHash &l, const RollingHash &r) {
  int k= lcp(l, r);
  if (l.n < r.n) {
   if (k < l.n) return str[l.bg + k] < str[r.bg + k];
   if (k= lcp(r, r.sub(l.n)); k < r.n - l.n) return str[r.bg + k] < str[r.bg + l.n + k];
   if (k= lcp(r.sub(r.n - l.n), l); k < l.n) return str[r.bg + r.n - l.n + k] < str[l.bg + k];
  } else {
   if (k < r.n) return str[l.bg + k] < str[r.bg + k];
   if (k= lcp(l.sub(r.n), l); k < l.n - r.n) return str[l.bg + r.n + k] < str[l.bg + k];
   if (k= lcp(r, l.sub(l.n - r.n)); k < r.n) return str[r.bg + k] < str[l.bg + l.n - r.n + k];
  }
  return false;
 }
 std::string to_str() const {  // for debug
  std::string ret;
  for (int i= bg; i < bg + n; ++i) ret+= (char)str[i];
  return ret;
 }
};
#include <tuple>
#include <array>
#include <utility>
template <class... Ks> struct Pointwise: std::tuple<Ks...> {
 static constexpr int N= sizeof...(Ks);
 using Self= Pointwise;
 using std::tuple<Ks...>::tuple;
 template <class T> Pointwise(const T &v) { fill(v, std::make_index_sequence<N>()); }
 template <class T, std::size_t... I> std::array<int, N> fill(const T &v, std::index_sequence<I...>) { return {{(void(std::get<I>(*this)= v), 0)...}}; }
#define HELPER(name, op) \
 template <std::size_t... I> std::array<int, N> name(const Self &y, std::index_sequence<I...>) { return {{(void(std::get<I>(*this) op##= std::get<I>(y)), 0)...}}; } \
 Self &operator op##=(const Self & r) { return name(r, std::make_index_sequence<N>()), *this; }
 HELPER(add_assign, +)
 HELPER(dif_assign, -)
 HELPER(mul_assign, *)
 HELPER(div_assign, /)
#undef HELPER
 Self operator+(const Self &r) const { return Self(*this)+= r; }
 Self operator-(const Self &r) const { return Self(*this)-= r; }
 Self operator*(const Self &r) const { return Self(*this)*= r; }
 Self operator/(const Self &r) const { return Self(*this)/= r; }
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

#include <random>
#include <cstdint>
uint64_t rng() {
 static uint64_t x= 10150724397891781847ULL * std::random_device{}();
 return x^= x << 7, x^= x >> 9;
}
uint64_t rng(uint64_t lim) { return rng() % lim; }
int64_t rng(int64_t l, int64_t r) { return l + rng() % (r - l); }
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<998244353>;
 using K= Pointwise<Mint, Mint>;
 using RH= RollingHash<K>;
 RH::init({rng(), rng()});
 int N;
 cin >> N;
 vector<string> S(N);
 for(int i= 0; i < N; ++i) cin >> S[i];
 vector<RH> rh(N);
 for(int i= 0; i < N; ++i) rh[i]= RH(S[i]);
 map<K, vector<int>> mp;
 for(int i= 0; i < N; ++i) {
  int c= S[i][0] - 'a';
  K h= rh[i].sub(1).hash();
  auto& v= mp[h];
  if(v.empty()) v.resize(26);
  v[c]++;
 }
 long long ans= 0;
 for(int i= 0; i < N; ++i) {
  int n= S[i].length();
  bool used[26];
  fill_n(used, 26, false);
  for(int j= 0; j < n; ++j) {
   used[S[i][j] - 'a']= true;
   K h= rh[i].sub(j + 1).hash();
   for(int k= 0; k < 26; ++k)
    if(used[k])
     if(auto it= mp.find(h); it != mp.end()) ans+= it->second[k];
  }
 }
 ans-= N;
 cout << ans << '\n';
 return 0;
}
