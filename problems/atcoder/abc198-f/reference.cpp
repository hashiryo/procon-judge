// 期待出力を作る参照実装。submissions/lib-interpolate.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
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

#include <cstdint>
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

#include <cassert>
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

namespace math_internal {
template <class Int, class MP> constexpr i64 inner_sqrt(Int a, Int p) {
 const MP md(p);
 Int e= (p - 1) >> 1, one= md.set(1);
 if(a= md.set(a); md.norm(pow(a, e, md)) != one) return -1;
 Int b= 0, d= md.diff(0, a), ret= one, r2= 0, b2= one;
 while(md.norm(pow(d, e, md)) == one) b= md.plus(b, one), d= md.diff(md.mul(b, b), a);
 auto mult= [&md, d](Int& u1, Int& u2, Int v1, Int v2) {
  Int tmp= md.plus(md.mul(u1, v1), md.mul(md.mul(u2, v2), d));
  u2= md.plus(md.mul(u1, v2), md.mul(u2, v1)), u1= tmp;
 };
 for(++e;; mult(b, b2, b, b2)) {
  if(e & 1) mult(ret, r2, b, b2);
  if(!(e>>= 1)) return ret= md.get(ret), ret * 2 < p ? ret : p - ret;
 }
}
}
constexpr long long mod_sqrt(long long a, long long p) {
 assert(p > 0), assert(a >= 0), assert(is_prime(p)), a%= p;
 if(a <= 1 || p == 2) return a;
 if(p < (1 << 30)) return math_internal::inner_sqrt<unsigned, math_internal::MP_Mo32>(a, p);
 if(p < (1ll << 62)) return math_internal::inner_sqrt<unsigned long long, math_internal::MP_Mo64>(a, p);
 return math_internal::inner_sqrt<unsigned long long, math_internal::MP_D2B1_1>(a, p);
}

namespace sfps {
namespace sfps_internal {
using namespace std;
template <class K> using Dat= vector<pair<int, K>>;
template <class K> Dat<K> to_sfps(const vector<K>& f, int b, int N) {
 Dat<K> p;
 for(int e= min<int>(N + 1, f.size()); b < e; ++b)
  if(f[b] != K(0)) p.emplace_back(b, f[b]);
 return p;
}
// f/g, O(N S_g)
template <class K> vector<K> div(vector<K> f, const vector<K>& g, int N) {
 assert(g[0] != K(0)), f.resize(N + 1);
 auto p= to_sfps(g, 1, N);
 const K iv= K(1) / g[0];
 for(int i= 0; i <= N; f[i++]*= iv)
  for(auto&& [j, v]: p) {
   if(i < j) break;
   f[i]-= f[i - j] * v;
  }
 return f;
}
template <class mod_t> void pw_mul_pw(mod_t* f, const Dat<mod_t>& p, const Dat<mod_t>& q, mod_t a, mod_t b, int N) {
 static_assert(is_modint_v<mod_t>);
 vector<mod_t> x(N + 1), y(N);
 f[0]= 1;
 for(int i= 0, k= 1; i < N; i= k++, f[i]*= FactorialPrecalculation<mod_t>::inv(i)) {
  for(auto&& [j, v]: q) {
   if(i < j) break;
   y[i]+= f[i - j] * v;
  }
  (y[i]+= f[i])*= a;
  for(auto&& [j, v]: p) {
   if(k < j) break;
   x[k]+= v * (y[k - j] * j - x[k - j]);
  }
  f[k]= x[k];
  for(auto&& [j, v]: q) {
   if(k < j) break;
   f[k]+= v * (b * j - k) * f[k - j];
  }
 }
}
template <class K> int mn_dg(const vector<K>& f) {
 for(int z= 0, e= f.size(); z < e; ++z)
  if(f[z] != K(0)) return z;
 return -1;
}
template <class mod_t> mod_t pw(mod_t a, int64_t k) { return k < 0 ? mod_t(1) / a.pow(-k) : a.pow(k); }
// f^k (k can be negative), O(N S_f)
template <class mod_t> vector<mod_t> pow(const vector<mod_t>& f, int64_t k, int N) {
 vector<mod_t> F(N + 1);
 if(!k) return F[0]= 1, F;
 int x= mn_dg(f);
 assert(!(x < 0 && k < 0));
 if(x < 0) return F;
 auto o= __int128_t(x) * k;
 if(o > N) return F;
 mod_t p0= f[x], ip= mod_t(1) / p0, a= pw(p0, k);
 auto p= to_sfps(f, x + 1, N - o + x);
 for(auto& [j, v]: p) v*= ip, j-= x;
 pw_mul_pw<mod_t>(F.data() + o, p, {}, k, 0, N - o);
 for(int i= o; i <= N; ++i) F[i]*= a;
 return F;
}
// f^k g^l (k, l can be negative), O(N(S_f+S_g))
template <class mod_t> vector<mod_t> pow_mul_pow(const vector<mod_t>& f, int64_t k, const vector<mod_t>& g, int64_t l, int N) {
 if(!k) return pow(g, l, N);
 if(!l) return pow(f, k, N);
 int x= mn_dg(f), y= mn_dg(g);
 vector<mod_t> F(N + 1);
 assert(!(x < 0 && k < 0)), assert(!(y < 0 && l < 0));
 if(x < 0 || y < 0) return F;
 auto o= __int128_t(x) * k + __int128_t(y) * l;
 if(assert(o >= 0); o > N) return F;
 mod_t p0= f[x], q0= g[y], ip= mod_t(1) / p0, iq= mod_t(1) / q0, a= pw(p0, k) * pw(q0, l);
 auto p= to_sfps(f, x + 1, N - o + x), q= to_sfps(g, y + 1, N - o + y);
 for(auto& [j, v]: p) v*= ip, j-= x;
 for(auto& [j, v]: q) v*= iq, j-= y;
 pw_mul_pw<mod_t>(F.data() + o, p, q, k, l + 1, N - o);
 for(int i= o; i <= N; ++i) F[i]*= a;
 return F;
}
// √(f/g), O(N(S_f+S_g))
template <class mod_t> vector<mod_t> sqrt_of_div(const vector<mod_t>& f, const vector<mod_t>& g, int N) {
 int x= mn_dg(f), y= mn_dg(g), o= (x - y) >> 1;
 vector<mod_t> F(N + 1);
 if(assert(y >= 0); x < 0) return F;
 if(assert(x >= y); (x - y) & 1) return {};  // no solution
 mod_t p0= f[x], ip= mod_t(1) / p0, iq= mod_t(1) / g[y], c= p0 * iq, a= mod_sqrt(c.val(), mod_t::mod()), i2= mod_t(1) / 2;
 if(a * a != c) return {};  // no solution
 auto p= to_sfps(f, x + 1, N - o + x), q= to_sfps(g, y + 1, N - o + y);
 for(auto& [j, v]: p) v*= ip, j-= x;
 for(auto& [j, v]: q) v*= iq, j-= y;
 pw_mul_pw<mod_t>(F.data() + o, p, q, i2, -i2 + 1, N - o);
 for(int i= o; i <= N; ++i) F[i]*= a;
 return F;
}
// log(f), O(N S_f)
template <class mod_t> vector<mod_t> log(vector<mod_t> f, int N) {
 assert(f[0] == mod_t(1));
 auto p= to_sfps(f, 1, N);
 f.resize(N + 1);
 for(int i= 2; i <= N; ++i) {
  f[i]*= i;
  for(auto&& [j, v]: p) {
   if(i <= j) break;
   f[i]-= f[i - j] * v;
  }
 }
 for(int i= 2; i <= N; ++i) f[i]*= FactorialPrecalculation<mod_t>::inv(i);
 return f[0]= 0, f;
}
// exp(f/g), O(N(S_f+S_g))
template <class mod_t> vector<mod_t> exp_of_div(const vector<mod_t>& f, const vector<mod_t>& g, int N) {
 int x= mn_dg(f), y= mn_dg(g);
 assert(y >= 0), assert(x < 0 || x > y);
 auto p= to_sfps(f, 0, N + y), q= to_sfps(g, y + 1, N + y);
 mod_t iv= mod_t(1) / g[y];
 for(auto& [j, v]: p) v*= iv, j-= y;
 for(auto& [j, v]: q) v*= iv, j-= y;
 vector<mod_t> F(N + 1), dF(N), a(N), b(N);
 F[0]= 1;
 for(int i= 0; i < N; ++i) {
  for(auto&& [j, v]: p) {
   if(i < j) break;
   b[i]+= v * F[i - j];
  }
  for(auto&& [j, v]: q) {
   if(i < j) break;
   a[i]-= v * (a[i - j] + b[i + 1 - j] * j);
   dF[i]-= v * dF[i - j];
  }
  for(auto&& [j, v]: p) {
   if(i + 1 < j) break;
   dF[i]+= v * j * F[i + 1 - j];
  }
  F[i + 1]= (dF[i]+= a[i]) * FactorialPrecalculation<mod_t>::inv(i + 1);
 }
 return F;
}
}
using sfps_internal::div, sfps_internal::pow, sfps_internal::pow_mul_pow, sfps_internal::pow_mul_pow, sfps_internal::sqrt_of_div, sfps_internal::log, sfps_internal::exp_of_div;
// 1/f, O(N S_f)
template <class K> std::vector<K> inv(const std::vector<K>& f, int N) { return div<K>({1}, f, N); }
// (f/g)^k (k can be negative), O(N(S_f+S_g))
template <class mod_t> std::vector<mod_t> pow_of_div(const std::vector<mod_t>& f, const std::vector<mod_t>& g, int64_t k, int N) { return pow_mul_pow(f, k, g, -k, N); }
// √f, O(N S_f)
template <class mod_t> std::vector<mod_t> sqrt(const std::vector<mod_t>& f, int N) { return sqrt_of_div<mod_t>(f, {1}, N); }
// exp(f), O(N S_f)
template <class mod_t> std::vector<mod_t> exp(const std::vector<mod_t>& f, int N) { return exp_of_div<mod_t>(f, {1}, N); }
}

// given: f(0),f(1),...,f(n-1), c   output: f(c)    O(n)
template <class mod_t> mod_t sample_points_shift(std::vector<mod_t> y, mod_t c) {
 using F= FactorialPrecalculation<mod_t>;
 int n= y.size();
 if((int)c.val() < n) return y[c.val()];
 for(int i= n; i--;) y[i]*= F::finv(i) * F::finv(n - i - 1);
 for(int i= 1; i < n; i+= 2) y[n - i - 1]= -y[n - i - 1];
 mod_t t= 1, ret= 0;
 for(int i= n; i--;) y[i]*= t, t*= c - i;
 t= 1;
 for(int i= 0; i < n; ++i) ret+= y[i] * t, t*= c - i;
 return ret;
}

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ModInt<998244353>;
 auto a= sfps::pow_of_div<Mint>({1}, {1, -1}, 6, 72);
 auto b= sfps::pow_of_div<Mint>({1}, {1, 0, -1}, 3, 72);
 for(int i= 72; i--;) a[i]+= b[i] * 6;
 b= sfps::pow_of_div<Mint>({1}, {1, 0, 0, -1}, 2, 72);
 for(int i= 72; i--;) a[i]+= b[i] * 8;
 b= sfps::pow_of_div<Mint>({1}, {1, -1}, 2, 72);
 auto c= sfps::div<Mint>(b, {1, 0, -2, 0, 1}, 72);
 for(int i= 72; i--;) a[i]+= c[i] * 3;
 c= sfps::div<Mint>(b, {1, 0, 0, 0, -1}, 72);
 for(int i= 72; i--;) a[i]+= c[i] * 6;
 long long S;
 cin >> S;
 S-= 6;
 vector<Mint> y(6);
 for(int i= 0; i < 6; ++i) y[i]= a[i * 12 + S % 12];
 cout << sample_points_shift<Mint>(y, S / 12) / 24 << '\n';
 return 0;
}
