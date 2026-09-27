// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
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

#include <algorithm>
#include <cstdint>
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
namespace nt_internal {
using namespace std;
vector<int> ps, lf;
void sieve(int N) {
 static int n= 2;
 if(n > N) return;
 if(lf.resize((N + 1) >> 1); n == 2) ps.push_back(n++);
 int M= (N - 1) / 2;
 for(int j= 1, e= ps.size(); j < e; ++j) {
  int p= ps[j];
  if(int64_t(p) * p > N) break;
  for(auto k= int64_t(p) * max(n / p / 2 * 2 + 1, p) / 2; k <= M; k+= p) lf[k]+= p * !lf[k];
 }
 for(; n <= N; n+= 2)
  if(!lf[n >> 1]) {
   ps.push_back(lf[n >> 1]= n);
   for(auto j= int64_t(n) * n / 2; j <= M; j+= n) lf[j]+= n * !lf[j];
  }
}
ConstListRange<int> enumerate_primes() { return {ps.cbegin(), ps.cend()}; }
ConstListRange<int> enumerate_primes(int N) {
 sieve(N);
 return {ps.cbegin(), upper_bound(ps.cbegin(), ps.cend(), N)};
}
int least_prime_factor(int n) { return n & 1 ? sieve(n), lf[(n >> 1)] : 2; }
// f(p,e) := f(p^e)
template <class T, class F> vector<T> completely_multiplicative_table(int N, const F& f) {
 vector<T> ret(N + 1);
 sieve(N);
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lf[i] == n ? f(n, 1) : ret[lf[i]] * ret[n / lf[i]];
 if(int n= 4; 2 <= N)
  for(T t= ret[2]= f(2, 1); n <= N; n+= 2) ret[n]= t * ret[n >> 1];
 return ret[1]= 1, ret;
}
}
using nt_internal::enumerate_primes, nt_internal::least_prime_factor, nt_internal::completely_multiplicative_table;
// O(N log k / log N + N)
template <class T> static std::vector<T> pow_table(int N, uint64_t k) {
 if(k == 0) return std::vector<T>(N + 1, 1);
 auto f= [k](int p, int) {
  T ret= 1, b= p;
  for(auto e= k;; b*= b) {
   if(e & 1) ret*= b;
   if(!(e>>= 1)) return ret;
  }
 };
 return completely_multiplicative_table<T>(N, f);
}

namespace nt_internal {
vector<int> lfw;
vector<char> lfe;
void set_lfe(int N) {
 static int n= 3, i= 1;
 if(n > N) return;
 for(sieve(N), lfw.resize((N + 1) >> 1), copy(lf.begin() + i, lf.begin() + ((N + 1) >> 1), lfw.begin() + i), lfe.resize(((N + 1) >> 1), 1); n <= N; n+= 2, ++i)
  if(int j= (n / lf[i]) >> 1; lf[i] == lf[j]) lfe[i]+= lfe[j], lfw[i]*= lfw[j];
}
// O(log n)
vector<pair<int, short>> factorize(int n) {
 vector<pair<int, short>> ret;
 if(short t; !(n & 1)) ret.emplace_back(2, t= __builtin_ctz(n)), n>>= t;
 if(int i= n >> 1; n > 1)
  for(set_lfe(n); n > 1; i= n >> 1) ret.emplace_back(lf[i], lfe[i]), n/= lfw[i];
 return ret;
}
// f(p,e) := f(p^e)
template <class T, class F> vector<T> multiplicative_table(int N, const F& f) {
 vector<T> ret(N + 1);
 set_lfe(N);
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lfw[i] == n ? f(lf[i], lfe[i]) : ret[lfw[i]] * ret[n / lfw[i]];
 for(int n= 2, t; n <= N; n+= 2) t= __builtin_ctz(n), ret[n]= n & (n - 1) ? ret[n & -n] * ret[n >> t] : f(2, t);
 return ret[1]= 1, ret;
}
// O(N)
template <class T= int> vector<T> mobius_table(int N) {
 vector<T> ret(N + 1);
 set_lfe(N), ret[1]= 1;
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lfw[i] == n ? -(lfe[i] == 1) : ret[lfw[i]] * ret[n / lfw[i]];
 for(int n= 2; n <= N; n+= 4) ret[n]= -ret[n >> 1];
 return ret;
}
// O(N)
template <class T= int> vector<T> totient_table(int N) {
 vector<T> ret(N + 1);
 set_lfe(N), ret[1]= 1;
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lfw[i] == n ? lf[i] == n ? n - 1 : ret[n / lf[i]] * lf[i] : ret[lfw[i]] * ret[n / lfw[i]];
 for(int n= 2; n <= N; n+= 4) ret[n]= ret[n >> 1];
 for(int n= 4; n <= N; n+= 4) ret[n]= ret[n >> 1] << 1;
 return ret;
}
}
using nt_internal::factorize, nt_internal::multiplicative_table, nt_internal::mobius_table, nt_internal::totient_table;
// f -> g s.t. g(n) = sum_{m|n} f(m), O(N log log N)
template <typename T> void divisor_zeta(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= 1, e= a.size(); p * j < e; ++j) a[p * j]+= a[j];
}
// a -> h s.t. a(n) = sum_{m|n} h(m), O(N log log N)
template <typename T> void divisor_mobius(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= (a.size() - 1) / p; j; --j) a[p * j]-= a[j];
}
// O(N log log N)
template <typename T> std::vector<T> lcm_convolve(std::vector<T> a, std::vector<T> b) {
 std::size_t N= std::max(a.size(), b.size());
 for(a.resize(N), b.resize(N), divisor_zeta(a), divisor_zeta(b); N--;) a[N]*= b[N];
 return divisor_mobius(a), a;
}
// a -> g s.t. g(n) = sum_{n|m} a(m), O(N log log N)
template <typename T> static void multiple_zeta(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= (a.size() - 1) / p; j; --j) a[j]+= a[p * j];
}
// a -> h s.t. a(n) = sum_{n|m} h(m), O(N log log N)
template <typename T> static void multiple_mobius(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= 1, e= a.size(); p * j < e; ++j) a[j]-= a[p * j];
}
// O(N log log N)
template <typename T> static std::vector<T> gcd_convolve(std::vector<T> a, std::vector<T> b) {
 std::size_t N= std::max(a.size(), b.size());
 for(a.resize(N), b.resize(N), multiple_zeta(a), multiple_zeta(b); N--;) a[N]*= b[N];
 return multiple_mobius(a), a;
}

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<int(1e9) + 7>;
 int N, K;
 cin >> N >> K;
 vector<Mint> a(K + 1);
 for(int i= 1; i <= K; ++i) a[i]= Mint(K / i).pow(N);
 multiple_mobius(a);
 Mint ans= 0;
 for(int i= 1; i <= K; ++i) ans+= a[i] * i;
 cout << ans << '\n';
 return 0;
}
