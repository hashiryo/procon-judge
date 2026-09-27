// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <numeric>
#include <cassert>
#include <iostream>
#include <algorithm>
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

#include <type_traits>
#include <algorithm>
#include <cstdint>
template <class Int> constexpr int bsf(Int a) {
 if constexpr (sizeof(Int) == 16) {
  uint64_t lo= a & uint64_t(-1);
  return lo ? __builtin_ctzll(lo) : 64 + __builtin_ctzll(a >> 64);
 } else if constexpr (sizeof(Int) == 8) return __builtin_ctzll(a);
 else return __builtin_ctz(a);
}
template <class Int> constexpr Int binary_gcd(Int a, Int b) {
 if (a == 0 || b == 0) return a + b;
 int n= bsf(a), m= bsf(b), s= 0;
 for (a>>= n, b>>= m; a != b;) {
  Int d= a - b;
  bool f= a > b;
  s= bsf(d), b= f ? b : a, a= (f ? d : -d) >> s;
 }
 return a << std::min(n, m);
}
namespace math_internal {
template <class T> constexpr void bubble_sort(T* bg, T* ed) {
 for(int sz= ed - bg, i= 0; i < sz; i++)
  for(int j= sz; --j > i;)
   if(auto tmp= bg[j - 1]; bg[j - 1] > bg[j]) bg[j - 1]= bg[j], bg[j]= tmp;
}
template <class T, size_t _Nm> struct ConstexprArray {
 constexpr size_t size() const { return sz; }
 constexpr auto& operator[](int i) const { return dat[i]; }
 constexpr auto* begin() const { return dat; }
 constexpr auto* end() const { return dat + sz; }
protected:
 T dat[_Nm]= {};
 size_t sz= 0;
 friend ostream& operator<<(ostream& os, const ConstexprArray& r) {
  os << "[";
  for(size_t i= 0; i < r.sz; ++i) os << r[i] << ",]"[i == r.sz - 1];
  return os;
 }
};
class Factors: public ConstexprArray<pair<u64, uint16_t>, 16> {
 template <class Uint, class MP> static constexpr Uint rho(Uint n, Uint c) {
  const MP md(n);
  auto f= [&md, c](Uint x) { return md.plus(md.mul(x, x), c); };
  const Uint m= 1LL << ((63 - __builtin_clzll(n)) / 5);
  Uint x= 1, y= md.set(2), z= 1, q= md.set(1), g= 1;
  for(Uint r= 1, i= 0; g == 1; r<<= 1) {
   for(x= y, i= r; i--;) y= f(y);
   for(Uint k= 0; k < r && g == 1; g= binary_gcd<Uint>(md.get(q), n), k+= m)
    for(z= y, i= min(m, r - k); i--;) y= f(y), q= md.mul(q, md.diff(y, x));
  }
  if(g == n) do {
    z= f(z), g= binary_gcd<Uint>(md.get(md.diff(z, x)), n);
   } while(g == 1);
  return g;
 }
 static constexpr u64 find_prime_factor(u64 n) {
  if(is_prime(n)) return n;
  for(u64 i= 100; i--;)
   if(n= n < (1 << 30) ? rho<u32, MP_Mo32>(n, i + 1) : n < (1ull << 62) ? rho<u64, MP_Mo64>(n, i + 1) : n < (1ull << 62) ? rho<u64, MP_D2B1_1>(n, i + 1) : rho<u64, MP_D2B1_2>(n, i + 1); is_prime(n)) return n;
  return 0;
 }
 constexpr void init(u64 n) {
  for(u64 p= 2; p < 98 && p * p <= n; ++p)
   if(n % p == 0)
    for(dat[sz++].first= p; n % p == 0;) n/= p, ++dat[sz - 1].second;
  for(u64 p= 0; n > 1; dat[sz++].first= p)
   for(p= find_prime_factor(n); n % p == 0;) n/= p, ++dat[sz].second;
 }
public:
 constexpr Factors()= default;
 constexpr Factors(u64 n) { init(n), bubble_sort(dat, dat + sz); }
};
}
using math_internal::Factors;
constexpr uint64_t totient(const Factors& f) {
 uint64_t ret= 1, i= 0;
 for(auto [p, e]: f)
  for(ret*= p - 1, i= e; --i;) ret*= p;
 return ret;
}
constexpr auto totient(uint64_t n) { return totient(Factors(n)); }
template <class Uint= uint64_t> std::vector<Uint> enumerate_divisors(const Factors& f) {
 int k= 1;
 for(auto [p, e]: f) k*= e + 1;
 std::vector<Uint> ret(k, 1);
 k= 1;
 for(auto [p, e]: f) {
  int sz= k;
  for(Uint pw= 1; pw*= p, e--;)
   for(int j= 0; j < sz;) ret[k++]= ret[j++] * pw;
 }
 return ret;
}
template <class Uint> std::vector<Uint> enumerate_divisors(Uint n) { return enumerate_divisors<Uint>(Factors(n)); }

template <uint64_t MOD> class ModInt_Exp {
 static_assert(MOD < 1uLL << 63, "MOD must be smaller than 2^63");
 using Uint= std::conditional_t < MOD<(1ull << 32), uint32_t, uint64_t>;
 using DUint= std::conditional_t<std::is_same_v<Uint, uint64_t>, __uint128_t, uint64_t>;
 using mod_t= ModInt_Exp;
 static constexpr inline Uint mod(DUint x) { return x < MOD * 2 ? Uint(x) : Uint(x % MOD) + MOD; }
 static constexpr inline Uint mul(Uint a, Uint b) { return mod(DUint(a) * b); }
 static constexpr inline Uint pow(Uint b, Uint k) {
  for(Uint ret(1);; b= mul(b, b))
   if(k& 1 ? ret= mul(ret, b) : 0; !(k>>= 1)) return ret;
 }
 static constexpr inline uint64_t f(uint64_t x) {
  uint64_t ret= 1, i= 0, tmp= 1;
  for(const auto& [p, e]: Factors(x)) {
   for(tmp= p - 1, i= e - (p == 2 && e > 3); --i;) tmp*= p;
   ret*= tmp / binary_gcd(ret, tmp);
  }
  return ret;
 }
public:
 Uint a;
 ModInt_Exp<f(MOD)> b;
 constexpr ModInt_Exp()= default;
 constexpr ModInt_Exp(uint64_t x): a(mod(x)), b(x) {}
 constexpr ModInt_Exp(Uint a_, ModInt_Exp<f(MOD)> b_): a(a_), b(b_) {}
 constexpr Uint val() const { return a < MOD ? a : a - MOD; }
 constexpr mod_t& operator*=(const mod_t& r) { return a= mul(a, r.a), b*= r.b, *this; }
 constexpr mod_t& operator+=(const mod_t& r) { return a-= MOD & -((a+= r.a) >= MOD * 2), b+= r.b, *this; }
 constexpr mod_t operator*(const mod_t& r) const { return mod_t(*this)*= r; }
 constexpr mod_t operator+(const mod_t& r) const { return mod_t(*this)+= r; }
 constexpr mod_t pow(const mod_t& r) const { return mod_t{pow(a, r.b.a), b.pow(r.b)}; };
};
template <> struct ModInt_Exp<1> {
 using mod_t= ModInt_Exp;
 bool a;
 constexpr ModInt_Exp(): a(0) {}
 constexpr ModInt_Exp(uint64_t x): a(x) {}
 constexpr uint32_t val() { return 0; }
 constexpr mod_t& operator*=(const mod_t& r) { return a&= r.a, *this; }
 constexpr mod_t& operator+=(const mod_t& r) { return a|= r.a, *this; }
 constexpr mod_t operator*(const mod_t& r) const { return mod_t(*this)*= r; }
 constexpr mod_t operator+(const mod_t& r) const { return mod_t(*this)+= r; }
 constexpr mod_t pow(const mod_t& r) const { return {a || !r.a}; };
};
template <uint64_t MOD> std::ostream& operator<<(std::ostream& os, const ModInt_Exp<MOD>& r) { return os << r.val(); }
template <uint64_t MOD> std::istream& operator>>(std::istream& is, ModInt_Exp<MOD>& r) {
 uint64_t v;
 return is >> v, r= ModInt_Exp<MOD>(v), is;
}

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ModInt_Exp<998244353>;
 Mint n, k, m;
 cin >> n >> k >> m;
 cout << m.pow(k.pow(n)) << '\n';
 return 0;
}
