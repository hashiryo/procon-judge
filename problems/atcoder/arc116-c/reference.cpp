// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
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

#include <valarray>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <numeric>
#include <cstdint>
template <class T> struct DirichletSeries {
 using Self= DirichletSeries;
 uint64_t N;  // <= K * L
 size_t K, L;
 std::valarray<T> x, X;
 DirichletSeries(uint64_t N, bool unit= false): N(N), K(N > 1 ? std::max(std::ceil(std::pow((double)N / std::log2(N), 2. / 3)), std::sqrt(N) + 1) : 1), L((N - 1 + K) / K), x(K + 1), X(K + L + 1) {
  if (assert(N > 0); unit) x[1]= 1, X= 1;
 }
 template <class F, typename= std::enable_if_t<std::is_invocable_r_v<T, F, uint64_t>>> DirichletSeries(uint64_t N, const F &sum): DirichletSeries(N) {
  for (size_t i= 1; i <= K; ++i) X[i]= sum(i);
  for (size_t i= 1; i <= L; ++i) X[K + i]= sum(uint64_t((double)N / i));
  for (size_t i= K; i; --i) x[i]= X[i] - X[i - 1];
 }
 Self operator-() const {
  Self ret(N);
  return ret.x= -x, ret.X= -X, ret;
 }
 Self &operator+=(T r) { return x[1]+= r, X+= r, *this; }
 Self &operator-=(T r) { return x[1]-= r, X-= r, *this; }
 Self &operator*=(T r) { return x*= r, X*= r, *this; }
 Self &operator/=(T r) {
  if (T iv= T(1) / r; iv == 0) x/= r, X/= r;
  else x*= iv, X*= iv;
  return *this;
 }
 Self &operator+=(const Self &r) { return assert(N == r.N), assert(K == r.K), assert(L == r.L), x+= r.x, X+= r.X, *this; }
 Self &operator-=(const Self &r) { return assert(N == r.N), assert(K == r.K), assert(L == r.L), x-= r.x, X-= r.X, *this; }
 Self operator+(T r) const { return Self(*this)+= r; }
 Self operator-(T r) const { return Self(*this)-= r; }
 Self operator*(T r) const { return Self(*this)*= r; }
 Self operator/(T r) const { return Self(*this)/= r; }
 Self operator+(const Self &r) const { return Self(*this)+= r; }
 Self operator-(const Self &r) const { return Self(*this)-= r; }
 friend Self operator+(T l, Self r) { return r+= l; }
 friend Self operator-(T l, Self r) { return r.x[1]-= l, r.X-= l, r.x= -r.x, r.X= -r.X, r; }
 friend Self operator*(T l, const Self &r) { return r * l; }
 friend Self operator/(T l, const Self &r) { return (Self(r.N, true)/= r)*= l; }
 Self operator*(const Self &r) const {
  assert(N == r.N), assert(K == r.K), assert(L == r.L);
  Self ret(N);
  uint64_t n;
  for (size_t i= K, j; i; --i)
   for (j= K / i; j; --j) ret.x[i * j]+= x[i] * r.x[j];
  for (size_t l= L, m, i; l; ret.X[K + l--]-= sum(m) * r.sum(m))
   for (i= m= std::sqrt(n= (double)N / l); i; --i) ret.X[K + l]+= x[i] * r.sum((double)n / i) + r.x[i] * sum((double)n / i);
  for (size_t i= 1; i <= K; ++i) ret.X[i]= ret.X[i - 1] + ret.x[i];
  return ret;
 }
 Self operator/(const Self &r) const { return Self(*this)/= r; }
 Self &operator*=(const Self &r) { return *this= *this * r; }
 Self &operator/=(const Self &r) {
  assert(N == r.N), assert(K == r.K), assert(L == r.L);
  for (size_t i= 1, j, ed; i <= K; i++)
   for (x[i]/= r.x[1], j= 2, ed= K / i; j <= ed; j++) x[i * j]-= x[i] * r.x[j];
  X[1]= x[1];
  for (size_t i= 2; i <= K; ++i) X[i]= X[i - 1] + x[i];
  uint64_t n;
  for (size_t l= L, m; l; X[K + l--]/= r.x[1])
   for (m= std::sqrt(n= (double)N / l), X[K + l]+= r.sum(m) * sum(m) - x[1] * r.sum(n); m > 1;) X[K + l]-= r.x[m] * sum((double)n / m) + x[m] * r.sum((double)n / m), --m;
  return *this;
 }
 Self square() const {
  Self ret(N);
  size_t i, j, l= std::sqrt(K);
  uint64_t n;
  T tmp;
  for (i= l; i; --i)
   for (j= K / i; j > i; --j) ret.x[i * j]+= x[i] * x[j];
  ret.x+= ret.x;
  for (i= l; i; --i) ret.x[i * i]+= x[i] * x[i];
  for (l= L; l; ret.X[K + l]+= ret.X[K + l], ret.X[K + l--]-= tmp * tmp)
   for (tmp= sum(i= std::sqrt(n= (double)N / l)); i; --i) ret.X[K + l]+= x[i] * sum((double)n / i);
  for (size_t i= 1; i <= K; ++i) ret.X[i]= ret.X[i - 1] + ret.x[i];
  return ret;
 }
 Self pow(uint64_t M) const {
  if (N / M > M)
   for (auto ret= Self(N, true), b= *this;; b= b.square()) {
    if (M & 1) ret*= b;
    if (!(M>>= 1)) return ret;
   }
  size_t n= 0, m, i, l, p= 2;
  uint64_t e, j;
  while (n <= M && (1ULL << n) <= N) ++n;
  T pw[65]= {1}, b= x[1], tmp;
  for (e= M - n + 1;; b*= b)
   if (e & 1 ? pw[0]*= b : T(); !(e>>= 1)) break;
  for (m= 1; m < n; ++m) pw[m]= pw[m - 1] * x[1];
  Self ret(*this);
  std::valarray<T> D= (ret.X-= x[1]), E(std::begin(D), K + 1), Y(std::begin(D) + K, L + 1), y= x, z(K + 1), Z(L + 1);
  auto A= [&](uint64_t n) { return n > K ? D[K + (double)N / n] : D[n]; };
  auto B= [&](uint64_t n) { return n > K ? Y[(double)N / n] : E[n]; };
  for (tmp= pw[n - 2] * M, l= L; l; l--) ret.X[K + l]*= tmp;
  for (i= 2; i <= K; ++i) ret.x[i]*= tmp;
  for (ret.x[1]= pw[n - 1], l= L; l; l--) ret.X[K + l]+= ret.x[1];
  for (m= 1, b= M, l= std::min<uint64_t>(L, uint64_t((double)N / p) / 2); m + 1 < n;) {
   for (b*= M - m, b/= ++m, tmp= b * pw[n - 1 - m]; l; ret.X[K + l--]+= Z[l] * tmp) {
    for (i= j= std::sqrt(e= (double)N / l); i >= p; --i) Z[l]+= y[i] * A((double)e / i);
    for (i= std::min(j, e / p); i >= 2; --i) Z[l]+= x[i] * B((double)e / i);
    if (j >= p) Z[l]-= A(j) * B(j);
   }
   for (i= K; i >= p; --i)
    for (l= K / i; l >= 2; l--) z[i * l]+= y[i] * x[l];
   for (i= p= 1 << m; i <= K; ++i) ret.x[i]+= z[i] * tmp;
   if (m + 1 == n) break;
   if (l= std::min<uint64_t>(L, uint64_t((double)N / p) / 2), y.swap(z), Y.swap(Z), std::fill_n(std::begin(Z) + 1, l, 0); p * 2 <= K) std::fill(std::begin(z) + p * 2, std::end(z), 0);
   if (p <= K)
    for (E[p]= y[p], i= p + 1; i <= K; ++i) E[i]= E[i - 1] + y[i];
  }
  for (size_t i= 1; i <= K; ++i) ret.X[i]= ret.X[i - 1] + ret.x[i];
  return ret;
 }
 inline T sum() const { return X[K + 1]; }
 inline T sum(uint64_t n) const { return n > K ? X[K + (double)N / n] : X[n]; }
 inline T operator()(uint64_t n) const { return n > K ? x[K + (double)N / n] : x[n]; }
};
// 1, zeta(s), O(K+L)
template <class T> DirichletSeries<T> get_1(uint64_t N) {
 DirichletSeries<T> ret(N);
 for (size_t i= ret.L; i; --i) ret.X[ret.K + i]= uint64_t((double)N / i);
 return std::fill(std::begin(ret.x) + 1, std::end(ret.x), T(1)), std::iota(std::begin(ret.X), std::begin(ret.X) + ret.K + 1, 0), ret;
}
// Mobius, 1/zeta(s), O(N^(2/3)log^(1/3)N))
template <class T> DirichletSeries<T> get_mu(uint64_t N) { return DirichletSeries<T>(N, true)/= get_1<T>(N); }
// n, zeta(s-1)
template <class T> DirichletSeries<T> get_Id(uint64_t N) {
 DirichletSeries<T> ret(N);
 __uint128_t a;
 for (size_t l= ret.L; l; --l) a= (double)N / l, ret.X[ret.K + l]= (a * (a + 1)) >> 1;
 std::iota(std::begin(ret.x), std::end(ret.x), 0);
 for (size_t i= 1; i <= ret.K; ++i) ret.X[i]= ret.X[i - 1] + ret.x[i];
 return ret;
}
// n^2, zeta(s-2), O(K+L)
template <class T> DirichletSeries<T> get_Id2(uint64_t N) {
 DirichletSeries<T> ret(N);
 __uint128_t a, b, c;
 for (size_t l= ret.L; l; --l) a= (double)N / l, b= (a * (a + 1)) >> 1, c= (a + a + 1), ret.X[ret.K + l]= c % 3 == 0 ? T(c / 3) * b : T(b / 3) * c;
 for (uint64_t i= ret.K; i; --i) ret.x[i]= i * i;
 for (size_t i= 1; i <= ret.K; ++i) ret.X[i]= ret.X[i - 1] + ret.x[i];
 return ret;
}
// number-of-divisors, zeta(s)zeta(s-1), O(N^(2/3)log^(1/3)N))
template <class T> DirichletSeries<T> get_d(uint64_t N) { return get_1<T>(N).square(); }
// sum-of-divisors, zeta(s)zeta(s-2), function, O(N^(2/3)log^(1/3)N))
template <class T> DirichletSeries<T> get_sigma(uint64_t N) { return get_1<T>(N) * get_Id<T>(N); }
// Euler's totient, zeta(s-1)/zeta(s), O(N^(2/3)log^(1/3)N))
template <class T> DirichletSeries<T> get_phi(uint64_t N) { return get_Id<T>(N)/= get_1<T>(N); }
template <class T>  // zeta(2s), O(K+L)
DirichletSeries<T> get_1sq(uint64_t N) {
 DirichletSeries<T> ret(N);
 for (size_t i= 1, e= ret.x.size(); i * i <= e; ++i) ret.x[i * i]= 1;
 for (size_t i= 1; i <= ret.K; ++i) ret.X[i]= ret.X[i - 1] + ret.x[i];
 for (size_t l= ret.L; l; --l) ret.X[ret.K + l]= uint64_t(std::sqrt((double)N / l));
 return ret;
}
// Liouville, zeta(2s)/zeta(s), O(N^(2/3)log^(1/3)N))
template <class T> DirichletSeries<T> get_lambda(uint64_t N) { return get_1sq<T>(N)/= get_1<T>(N); }
// square-free, zeta(s)/zeta(2s), O(N^(2/3)log^(1/3)N))
template <class T> DirichletSeries<T> get_absmu(uint64_t N) { return get_1<T>(N)/= get_1sq<T>(N); }

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<998244353>;
 long long N, M;
 cin >> N >> M;
 cout << get_1<Mint>(M).pow(N).sum() << '\n';
 return 0;
}
