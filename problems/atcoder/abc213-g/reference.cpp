// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// 連結グラフ
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

#include <algorithm>
#include <vector>
#include <cassert>
#include <cstdint>
namespace sps {
namespace _sps_internal {
using namespace std;
#define _ZETA(s, l) \
 if constexpr (!t) A[s + l]+= A[s]; \
 else if constexpr (t == 1) A[s + l]-= A[s]; \
 else if constexpr (t == 2) A[s]+= A[s + l]; \
 else if constexpr (t == 3) A[s]-= A[s + l]; \
 else tie(A[s], A[s + l])= make_pair(A[s] + A[s + l], A[s] - A[s + l]);
template <int t, class T> void rec(T A[], int l) {
 if (l > 127) {
  l>>= 1, rec<t>(A, l), rec<t>(A + l, l);
  for (int s= 0; s < l; ++s) _ZETA(s, l);
 } else
  for (int k= 1; k < l; k<<= 1)
   for (int i= 0; i < l; i+= k + k)
    for (int j= 0; j < k; ++j) _ZETA(i + j, k);
}
#undef _ZETA
/*  f -> g s.t. g[S] = sum_{T subseteq S} f[T]  O(n 2^n) */
template <class T> void subset_zeta(vector<T>& f) { rec<0>(f.data(), f.size()); }
/*  f -> h s.t. f[S] = sum_{T subseteq S} h[T]  O(n 2^n) */
template <class T> void subset_mobius(vector<T>& f) { rec<1>(f.data(), f.size()); }
/*  f -> g s.t. g[S] = sum_{S subseteq T} f[T]  O(n 2^n) */
template <class T> void supset_zeta(vector<T>& f) { rec<2>(f.data(), f.size()); }
/*  f -> h s.t. f[S] = sum_{S subseteq T} h[T]  O(n 2^n) */
template <class T> void supset_mobius(vector<T>& f) { rec<3>(f.data(), f.size()); }
/* h[S] = sum_{U | T == S} f[U]g[T]  O(n 2^n) */
template <class T> vector<T> or_convolve(vector<T> f, vector<T> g) {
 subset_zeta(f), subset_zeta(g);
 for (int s= f.size(); s--;) f[s]*= g[s];
 return subset_mobius(f), f;
}
/* h[S] = sum_{U & T == S} f[U]g[T]  O(n 2^n) */
template <class T> vector<T> and_convolve(vector<T> f, vector<T> g) {
 supset_zeta(f), supset_zeta(g);
 for (int s= f.size(); s--;) f[s]*= g[s];
 return supset_mobius(f), f;
}
/* f -> g s.t. g[S] = sum_{T} (-1)^{|S & T|} f[T] */
template <class T> void hadamard(vector<T>& f) { rec<4>(f.data(), f.size()); }
/* h[S] = sum_{U ^ T = S} f[U]g[T] */
template <class T> vector<T> xor_convolve(vector<T> f, vector<T> g) {
 hadamard(f), hadamard(g);
 for (int s= f.size(); s--;) f[s]*= g[s];
 hadamard(f);
 if (T iv= T(1) / f.size(); iv == 0)
  for (int s= f.size(); s--;) f[s]/= f.size();
 else
  for (int s= f.size(); s--;) f[s]*= iv;
 return f;
}
template <int t, class T> void rec_r(T A[], int l, int n, int c= 0) {
 if (l >= (n << 4)) {
  l>>= 1, rec_r<t>(A, l, n, c), rec_r<t>(A + l, l, n, c + 1);
  for (int s= l / n; s--;)
   if constexpr (!t)
    for (int d= 0, e= __builtin_popcount(s) + c + 1; d < e; ++d) A[s * n + d + l]+= A[s * n + d];
   else
    for (int d= __builtin_popcount(s) + c + 1; d < n; ++d) A[s * n + d + l]-= A[s * n + d];
 } else
  for (int k= 1, m= l / n; k < m; k<<= 1)
   for (int i= 0; i < m; i+= k + k)
    for (int j= 0; j < k; ++j)
     if constexpr (!t)
      for (int u= i + j, s= u + k, d= 0, e= __builtin_popcount(s) + c; d < e; ++d) A[s * n + d]+= A[u * n + d];
     else
      for (int u= i + j, s= u + k, d= __builtin_popcount(s) + c; d < n; ++d) A[s * n + d]-= A[u * n + d];
}
template <class T> void rnk_zeta(const T f[], T F[], int n) {
 for (int s= 1 << n; s--;) F[s * (n + 1) + __builtin_popcount(s)]= f[s];
 rec_r<0>(F, (n + 1) << n, n + 1);
}
template <class T> void rnk_mobius(T F[], T f[], int n) {
 rec_r<1>(F, (n + 1) << n, n + 1);
 for (int s= 1 << n; s--;) f[s]= F[s * (n + 1) + __builtin_popcount(s)];
}
template <class T> void cnv_(T A[], T B[], int n) {
 for (int s= 1 << (n - 1); s--;) {
  T t, *a= A + s * n, *b= B + s * n;
  for (int c= __builtin_popcount(s), d= min(2 * c, n - 1), e; d >= c; a[d--]= t)
   for (t= 0, e= d - c; e <= c; ++e) t+= a[e] * b[d - e];
 }
}
template <class T> void cnv_na(const T f[], const T g[], T h[], int N) {
 for (int s= N, t; s--;)
  for (h[t= s]= f[s] * g[0]; t; --t&= s) h[s]+= f[s ^ t] * g[t];
}
// fg, O(n^2 2^n)
template <class T> vector<T> convolve(const vector<T>& f, const vector<T>& g) {
 const int N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1))), assert(N == (int)g.size());
 vector<T> h(N);
 if (n < 11) return cnv_na(f.data(), g.data(), h.data(), N), h;
 vector<T> F((n + 1) << n);
 if (rnk_zeta(f.data(), F.data(), n); f.data() == g.data()) return cnv_(F.data(), F.data(), n + 1), rnk_mobius(F.data(), h.data(), n), h;
 vector<T> G((n + 1) << n);
 return rnk_zeta(g.data(), G.data(), n), cnv_(F.data(), G.data(), n + 1), rnk_mobius(F.data(), h.data(), n), h;
}
template <class T> void div_na(T f[], const T g[], int N) {
 for (int s= 1; s < N; ++s)
  for (int t= s; t; --t&= s) f[s]-= f[s ^ t] * g[t];
}
// 1/f, "f[empty] = 1" is required, O(n^2 2^n)
template <class T> vector<T> inv(const vector<T>& f) {
 const int N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1))), assert(f[0] == 1);
 vector<T> h(N);
 if (n < 11) return h[0]= 1, div_na(h.data(), f.data(), N), h;
 vector<T> F((n + 1) << n), G((n + 1) << n);
 rnk_zeta(f.data(), G.data(), n);
 for (int s= N; s--;) {
  T *a= F.data() + s * (n + 1), *b= G.data() + s * (n + 1);
  a[0]= 1;
  for (int d= 0, c= __builtin_popcount(s); d++ < n;)
   for (int e= max(0, d - c); e < d; ++e) a[d]-= a[e] * b[d - e];
 }
 return rnk_mobius(F.data(), h.data(), n), h;
}
// f/g, "f[empty] = 1" is required, O(n^2 2^n)
template <class T> vector<T> div(vector<T> f, const vector<T>& g) {
 const int N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1))), assert(N == (int)g.size()), assert(g[0] == 1);
 if (n < 12) return div_na(f.data(), g.data(), N), f;
 vector<T> F((n + 1) << n), G((n + 1) << n);
 rnk_zeta(f.data(), F.data(), n), rnk_zeta(g.data(), G.data(), n);
 for (int s= N; s--;) {
  T *a= F.data() + s * (n + 1), *b= G.data() + s * (n + 1);
  for (int d= 0, c= __builtin_popcount(s); d++ < n;)
   for (int e= max(0, d - c); e < d; ++e) a[d]-= a[e] * b[d - e];
 }
 return rnk_mobius(F.data(), f.data(), n), f;
}
template <class T, class P> void oncnv_(const T f[], T h[], const P& phi, int n) {
 vector<T> F((n + 1) << n), G((n + 1) << n);
 rnk_zeta(f, F.data(), n), fill_n(G.data(), 1 << n, h[0]);
 T* a= G.data() + (1 << n);
 for (int l= 1 << n; l>>= 1;) phi(l, a[l]= h[0] * f[l]), h[l]= a[l];
 for (int d= 2, s; d <= n; ++d) {
  for (rec<0>(a, 1 << n), a+= (s= 1 << n); --s;)
   if (int c= __builtin_popcount(s); c <= d && d <= 2 * c)
    for (int e= d; e--;) a[s]+= G[e << n | s] * F[s * (n + 1) + d - e];
  for (rec<1>(a, 1 << n), s= 1 << n; --s;)
   if (__builtin_popcount(s) == d) phi(s, a[s]), h[s]= a[s];
   else a[s]= 0;
 }
}
// h[S] = phi(S, sum_{T subsetneq S} h[T]f[S/T] )  O(n^2 2^n)
// phi: [](int, T&x)
template <class T, class P> vector<T> semi_relaxed_convolve(const vector<T>& f, T init, const P& phi) {
 const int N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1)));
 vector<T> h(N);
 if (h[0]= init; n < 12) {
  for (int s= 1, t; s < N; phi(s, h[s]), ++s)
   for (t= s; t; --t&= s) h[s]+= h[s ^ t] * f[t];
 } else oncnv_(f.data(), h.data(), phi, n);
 return h;
}
// h[S] = phi(S, 1/2 sum_{empty neq T subseteq S} h[T]h[S/T] )  O(n^2 2^n)
// phi: [](int, T&x)
template <class T, class P> vector<T> self_relaxed_convolve(const P& phi, int n) {
 const int e= min(n, 12);
 int i= 0, l= 1;
 vector<T> f(1 << n);
 for (int u= 1; i < e; l<<= 1, ++i)
  for (int s= 0; s < l; phi(u, f[u]), ++s, ++u)
   for (int t= s; t; --t&= s) f[u]+= f[u ^ t] * f[t];
 for (; i < n; l<<= 1, ++i) phi(l, f[l]), oncnv_(f.data(), f.data() + l, [&](int s, T& x) { phi(s | l, x); }, i);
 return f;
}
// exp(f) , "f[empty] = 0" is required,  O(n^2 2^n)
template <class T> vector<T> exp(const vector<T>& f) {
 const int N= f.size(), n= __builtin_ctz(N), e= min(n, 11);
 assert(!(N & (N - 1))), assert(f[0] == 0);
 vector<T> h(N);
 int i= 0, l= 1;
 for (h[0]= 1; i < e; l<<= 1, ++i) cnv_na(h.data(), f.data() + l, h.data() + l, l);
 for (; i < n; l<<= 1, ++i) {
  vector<T> F((i + 1) << i), G((i + 1) << i);
  rnk_zeta(h.data(), F.data(), i), rnk_zeta(f.data() + l, G.data(), i), cnv_(F.data(), G.data(), i + 1), rnk_mobius(F.data(), h.data() + l, i);
 }
 return h;
}
// log(f) , "f[empty] = 1" is required,  O(n^2 2^n)
template <class T> vector<T> log(const vector<T>& f) {
 const int N= f.size(), n= __builtin_ctz(N), e= min(n, 12);
 assert(!(N & (N - 1))), assert(f[0] == 1);
 vector<T> h= f;
 int i= 0, l= 1;
 for (h[0]= 0; i < e; l<<= 1, ++i) div_na(h.data() + l, f.data(), l);
 if (i < n) {
  vector<T> G(n << (n - 1));
  rnk_zeta(f.data(), G.data(), n - 1);
  for (; i < n; l<<= 1, ++i) {
   vector<T> F((i + 1) << i, 0);
   if constexpr (is_floating_point_v<T>) {
    fill_n(F.data(), l, h[l]= f[l]);
    T* a= F.data() + l;
    for (int m= l; m>>= 1;) h[l | m]= a[m]= f[l | m] - h[l] * f[m];
    for (int d= 2, s; d <= i; ++d) {
     for (rec<0>(a, l), a+= (s= l); --s;)
      if (int c= __builtin_popcount(s); c <= d && d <= 2 * c)
       for (int e= d; e--;) a[s]+= F[e << i | s] * G[s * n + d - e];
     for (rec<1>(a, l), s= l; --s;)
      if (__builtin_popcount(s) == d) h[l | s]= a[s]= f[l | s] - a[s];
      else a[s]= 0;
    }
   } else {
    rnk_zeta(f.data() + l, F.data(), i);
    for (int s= l; s--;) {
     T t, *a= F.data() + s * (i + 1), *b= G.data() + s * n;
     for (int d= 0, c= __builtin_popcount(s), e; d++ < i; a[d]-= t)
      for (t= 0, e= max(0, d - c); e < d; ++e) t+= a[e] * b[d - e];
    }
    rnk_mobius(F.data(), h.data() + l, i);
   }
  }
 }
 return h;
}
// F(f) =  sum_i F_i f^i/i! , "f[empty] = 0" is required, O(n^2 2^n)
template <class T> vector<T> egf_comp(const vector<T>& F, const vector<T>& f) {
 const int N= f.size(), n= __builtin_ctz(N), e= min(n, 11);
 assert(!(N & (N - 1))), assert(f[0] == 0);
 vector<T> h(N);
 T* b= h.data() + N;
 for (int i= n - F.size(); i++ < n;) h[N - (1 << i)]= F[n - i];
 int i= 0, l= 1;
 for (; i < e; l<<= 1, ++i)
  for (int j= N >> 1; j >= l; j>>= 1) cnv_na(b - j, f.data() + l, b - j - j + l, l);
 if (i < n) {
  vector<T> A(n << (n - 1)), B(n << (n - 1));
  for (; i < n; l<<= 1, ++i) {
   fill_n(B.data(), (i + 1) << i, 0), rnk_zeta(f.data() + l, B.data(), i);
   for (int j= N >> 1; j >= l; j>>= 1) fill_n(A.data(), (i + 1) << i, 0), rnk_zeta(b - j, A.data(), i), cnv_(A.data(), B.data(), i + 1), rnk_mobius(A.data(), b - j - j + l, i);
  }
 }
 return h;
}
// P(f) = sum_{i=0}^m P_i f^i ,  O(n^2 2^n)
template <class T> vector<T> poly_comp(vector<T> P, vector<T> f) {
 const int N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1)));
 vector<T> F(n + 1);
 for (int j= 0, e= P.size();; ++j, --e) {
  for (int i= e; i--;) (F[j]*= f[0])+= P[i];
  if (j == n || e <= 1) break;
  for (int i= 1; i < e; ++i) P[i - 1]= P[i] * i;
 }
 return f[0]= 0, egf_comp(F, f);
}
// f^k ,  O(n^2 2^n)
template <class T> vector<T> pow(vector<T> f, uint64_t k) {
 const unsigned N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1)));
 vector<T> F;
 unsigned i, m;
 if (n < k) {
  F.resize(n + 1), F[m= n]= 1;
  T x= f[0];
  for (uint64_t l= k - n;; x*= x)
   if (l & 1 ? F[n]*= x : 0; !(l>>= 1)) break;
 } else F.resize(k + 1), F[m= k]= 1;
 for (i= m; i--;) F[i]= F[i + 1] * f[0];
 for (T t= 1; ++i < m;) F[i + 1]*= (t*= k - i);
 return f[0]= 0, egf_comp(F, f);
}
template <class T> vector<T> _egfT(const T* b, T* h, int M, int n) {
 T *a, *d;
 vector<T> c(n + 1);
 int l= M;
 if (int i= __builtin_ctz(M); i > 10) {
  vector<T> F((i + 1) << i), G((i + 1) << i);
  for (int m, s; i > 10; fill_n(F.data(), (i + 1) << i, 0), rnk_zeta(h, F.data(), i), cnv_(F.data(), G.data(), i + 1), rnk_mobius(F.data(), h, i), b-= (l>>= 1), --i)
   for (fill_n(G.data(), (i + 1) << i, 0), rnk_zeta(b, G.data(), i), m= M; m > l; m>>= 1)
    for (a= h + (m - l), fill_n(F.data(), (i + 1) << i, 0), rnk_zeta(a + m - l, F.data(), i), cnv_(F.data(), G.data(), i + 1), rec_r<1>(F.data(), (i + 1) << i, i + 1), s= l; s--;) a[s]+= F[s * (i + 1) + __builtin_popcount(s)];
 }
 for (; l; cnv_na(h, b, h, l), b-= (l>>= 1))
  for (int m= M, s, t; m > l; m>>= 1)
   for (a= h + (m - l), d= a + (m - l), s= l; s--;)
    for (a[t= s]+= d[s] * b[0]; t; --t&= s) a[s]+= d[s ^ t] * b[t];
 for (int i= 0; i <= n; ++i) c[i]= h[(1 << (n - i)) - 1];
 return c;
}
// [X^{[n]}] f^k/k! for k=0,1,...,n , O(n^2 2^n)
template <class T> vector<T> egf_T(vector<T> f) {
 const int N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1)));
 if (n == 0) return {1};
 if (n == 1) return {0, f[1]};
 return _egfT(f.data() + (N >> 2), f.data() + (N >> 1), N >> 2, n);
}
// [X^{[n]}] f^k/k! g for k=0,1,...,n , O(n^2 2^n)
template <class T> vector<T> egf_T(const vector<T>& f, vector<T> g) {
 const int N= f.size(), n= __builtin_ctz(N);
 assert(!(N & (N - 1)));
 if (n == 0) return {g[1]};
 return _egfT(f.data() + (N >> 1), g.data(), N >> 1, n);
}
}
using _sps_internal::subset_zeta, _sps_internal::subset_mobius, _sps_internal::supset_zeta, _sps_internal::supset_mobius, _sps_internal::hadamard, _sps_internal::or_convolve, _sps_internal::and_convolve, _sps_internal::xor_convolve, _sps_internal::convolve, _sps_internal::semi_relaxed_convolve, _sps_internal::self_relaxed_convolve, _sps_internal::inv, _sps_internal::div, _sps_internal::exp, _sps_internal::log, _sps_internal::egf_comp, _sps_internal::poly_comp, _sps_internal::pow, _sps_internal::egf_T;
}
class UndirectedGraphSetPowerSeries {
 using u64= unsigned long long;
 template <class T> using Sps= std::vector<T>;
 template <class T> using Poly= std::vector<T>;
 const int n, N, m, o;
 std::vector<u64> adj;
 std::vector<int> es;
 template <class T> static inline T pow(T x, int k) {
  for(T ret(1);; x*= x)
   if(k& 1 ? ret*= x : 0; !(k>>= 1)) return ret;
 }
 template <class F> inline void bfs(int s, const F& f) const {
  for(int t= s, u, j; t;)
   for(f(u= 1 << __builtin_ctz(t)); u;) j= __builtin_ctz(u), t^= 1 << j, u^= 1 << j, u|= es[j] & t;
 }
 template <class T, class G> static inline void transform_articulation(Sps<T>& f, const G& g) {
  const int M= f.size() / 2;
  Sps<T> tmp(M);
  for(int I= M; I; I>>= 1) {
   for(int t= 0; t < M; t+= I)
    for(int u= I, t2= t << 1; u--;) tmp[t | u]= f[t2 | I | u];
   tmp= g(tmp);
   for(int t= 0; t < M; t+= I)
    for(int u= I, t2= t << 1; u--;) f[t2 | I | u]= tmp[t | u];
  }
 }
 template <class T, bool b> inline void transform_bridge(Sps<T>& f) const {
  const int M= N / 2;
  Sps<T> tmp(M), tmp2;
  for(int i= n, I= M; --i; I>>= 1) {
   for(int t= 0; t < M; t+= I)
    for(int u= I, t2= t << 1; u--;) tmp[t | u]= f[t2 | I | u];
   tmp2.assign(M, 0);
   for(int t= 0; t < M; t+= I)
    for(int j= i, J= I, t2= t << 1; J>>= 1, j--;)
     for(int s= J, J2= J * 2; s < I; s+= J2)
      for(int u= s + J; u-- > s;) {
       if constexpr(b) tmp2[t | u]+= f[t2 | u] * adj[i * m + j];
       else tmp2[t | u]-= f[t2 | u] * adj[i * m + j];
      }
   tmp= sps::convolve(tmp, sps::exp(tmp2));
   for(int t= 0; t < M; t+= I)
    for(int u= I, t2= t << 1; u--;) f[t2 | I | u]= tmp[t | u];
  }
 }
 template <class T> inline std::vector<T> cyc() const {
  std::vector<T> cyc(1 << o);
  for(int i= 0; i < o; ++i) {
   int a= i + i, b= a + 1, K= a + 2, I= 1 << i;
   std::vector<T> dp0(K << i);
   dp0[a]= 1;
   for(int s= 0; s < I; ++s) {
    T* dp0s= dp0.data() + (s * K);
    for(int u= s | I, S= u, j, j0, j1; S; S^= 1 << j) {
     j= __builtin_ctz(S), j0= j + j, j1= j0 + 1;
     const u64 *A0= adj.data() + (j0 * m), *A1= A0 + m;
     T dp0s0= dp0s[j0], dp0s1= dp0s[j1];
     cyc[u]+= dp0s0 * A0[b] + dp0s1 * A1[b];
     for(int U= I - 1 - s, k, k0, k1; U; U^= 1 << k) {
      k= __builtin_ctz(U), k0= k + k, k1= k0 + 1;
      dp0s[(K << k) + k0]+= dp0s0 * A0[k1] + dp0s1 * A1[k1], dp0s[(K << k) + k1]+= dp0s0 * A0[k0] + dp0s1 * A1[k0];
     }
    }
   }
  }
  return cyc;
 }
 template <class T> inline std::pair<std::vector<T>, std::vector<T>> cyc_pth() const {
  std::vector<T> cyc(1 << o), pth(1 << o);
  for(int i= 0; i < o; ++i) {
   int a= i + i, b= a + 1, K= a + 2, I= 1 << i;
   std::vector<T> dp0(K << i), dp1(K << i);
   dp0[a]= 1;
   for(int s= 0; s < I; ++s) {
    T *dp0s= dp0.data() + (s * K), *dp1s= dp1.data() + (s * K);
    for(int j= 0; j < K; ++j) dp1s[b]+= dp0s[j];
    for(int u= s | I, S= u, j, j0, j1; S; S^= 1 << j) {
     j= __builtin_ctz(S), j0= j + j, j1= j0 + 1;
     const u64 *A0= adj.data() + (j0 * m), *A1= A0 + m;
     T dp0s0= dp0s[j0], dp0s1= dp0s[j1], dp1s0= dp1s[j0], dp1s1= dp1s[j1];
     cyc[u]+= dp0s0 * A0[b] + dp0s1 * A1[b], pth[u]+= dp1s0 + dp1s1;
     for(int U= I - 1 - s, k, k0, k1; U; U^= 1 << k) {
      k= __builtin_ctz(U), k0= k + k, k1= k0 + 1;
      dp0s[(K << k) + k0]+= dp0s0 * A0[k1] + dp0s1 * A1[k1], dp0s[(K << k) + k1]+= dp0s0 * A0[k0] + dp0s1 * A1[k0];
      dp1s[(K << k) + k0]+= dp1s0 * A0[k1] + dp1s1 * A1[k1], dp1s[(K << k) + k1]+= dp1s0 * A0[k0] + dp1s1 * A1[k0];
     }
    }
   }
  }
  return {cyc, pth};
 }
public:
 UndirectedGraphSetPowerSeries(int n): n(n), N(1 << n), m(n + (n & 1)), o(m / 2), adj(m * m), es(n) {}
 template <class Int> UndirectedGraphSetPowerSeries(const std::vector<std::vector<Int>>& g): n(g.size()), N(1 << n), m(n + (n & 1)), o(m / 2), adj(m * m), es(n) {
  for(int i= n; i--;)
   for(int j= i; j--;) assert(g[i][j] == g[j][i]);
  for(int i= n; i--;)
   for(int j= n; j--;) adj[i * m + j]= g[i][j];
  for(int i= n; i--;)
   for(int j= n; j--;) es[i]|= !(!(adj[i * m + j])) << j;
 }
 void add_edge(int u, int v, u64 cnt= 1) {
  adj[u * m + v]= (adj[v * m + u]+= cnt), es[u]|= (1 << v), es[v]|= (1 << u);
  if(!(adj[u * m + v])) es[u]^= (1 << v), es[v]^= (1 << u);
 }
 const auto operator[](int u) const { return adj.begin() + (u * m); }
 template <class T> static inline Sps<T> only_connected(const Sps<T>& f) { return sps::log(f); }
 template <class T> static inline Sps<T> disjoint_union(const Sps<T>& f) { return sps::exp(f); }
 template <class T> static inline Sps<T> only_biconnected(Sps<T> f) { return transform_articulation(f, sps::log<T>), f; }
 template <class T> static inline Sps<T> articulation_union(Sps<T> f) { return transform_articulation(f, sps::exp<T>), f; }
 template <class T> inline Sps<T> only_2edge_connected(Sps<T> f) const { return transform_bridge<T, false>(f), f; }
 template <class T> inline Sps<T> bridge_union(Sps<T> f) const { return transform_bridge<T, true>(f), f; }
 inline Sps<u64> edge_num() const {
  Sps<u64> ret(N, 0);
  for(int i= n; i--;)
   for(int j= i; j--;) ret[(1 << i) | (1 << j)]= adj[i * m + j];
  return sps::subset_zeta(ret), ret;
 }
 inline Sps<int> connected_component_num() const {
  Sps<int> ret(N, 0);
  for(int s= N; s--;) bfs(s, [&](int) { ret[s]++; });
  return ret;
 }
 inline Sps<u64> cycle_space_rank() const {
  Sps<u64> e= edge_num(), ret(N, 0);
  Sps<int> k= connected_component_num();
  for(int s= N; s--;) ret[s]= e[s] + k[s] - __builtin_popcount(s);
  return ret;
 }
 inline Sps<u64> odd_deg_num() const {
  Sps<u64> ret(N, 0);
  for(int i= n, I= N; I>>= 1, i--;)
   for(int t= 0, I2= I << 1; t < N; t+= I2)
    for(int u= I, cnt, v, j; u--; ret[t | I | u]+= cnt & 1)
     for(cnt= 0, v= t | u; v; v^= 1 << j) cnt+= adj[i * m + (j= __builtin_ctz(v))];
  return ret;
 }
 inline Sps<u64> selfloop_num() const {
  Sps<u64> ret(N, 0);
  for(int i= 0, I= 1; i < n; ++i, I<<= 1)
   for(int u= I; u--;) ret[I | u]= ret[u] + adj[i * m + i];
  return ret;
 }
 template <class T, class Int> static inline Sps<T> space_size(const Sps<Int>& rank) {
  Sps<T> ret(rank.size());
  for(int s= rank.size(); s--;) ret[s]= pow<T>(2, rank[s]);
  return ret;
 }
 template <class T> inline Sps<T> graph() const { return space_size<T>(edge_num()); }
 template <class T> inline Sps<T> cycle_space_size() const { return space_size<T>(cycle_space_rank()); }
 template <class T> inline Sps<T> connected_graph() const { return sps::log(graph<T>()); }
 template <class T> inline Sps<T> eulerian_graph() const { return sps::log(cycle_space_size<T>()); }
 template <class T> inline Sps<T> connected_biparate_graph() const {
  Sps<T> tmp= graph<T>(), ret(N, 1);
  for(int s= N; s--;) ret[s]/= tmp[s];
  ret= sps::convolve(ret, ret);
  for(int s= N; s--;) ret[s]*= tmp[s];
  ret= sps::log(ret);
  for(int s= N; s--;) ret[s]/= 2;
  return ret;
 }
 template <class T> inline Sps<T> tree() const {
  Sps<u64> e= edge_num();
  Sps<T> ret= {0, 1};
  ret.reserve(N);
  for(int I= 2; I < N; I<<= 1) {
   Sps<T> g(ret);
   for(int s= I; --s;) g[s]*= e[s | I] - e[s] - e[I];
   g= sps::exp(g), std::copy(g.begin(), g.end(), std::back_inserter(ret));
  }
  return ret;
 }
 template <class T> inline Sps<T> forest() const { return sps::exp(tree<T>()); }
 template <class T> inline Sps<T> cycle_graph() const {
  T dp[N][n - 1];
  Sps<T> ret(N, 0);
  for(int i= n, I= N; I>>= 1, --i;) {
   for(int s= I; --s;) std::fill_n(dp[s], i, 0);
   for(int j= i; j--;) dp[1 << j][j]= adj[i * m + j];
   for(int s= 1; s < I; ++s)
    for(int t= s, j, u, r, k; t; ret[s | I]+= dp[s][j] * adj[j * m + i])
     for(t^= 1 << (j= __builtin_ctz(t)), u= r= s ^ (1 << j); u; dp[s][j]+= dp[r][k] * adj[k * m + j]) u^= 1 << (k= __builtin_ctz(u));
  }
  for(int i= n; i--;)
   for(int j= i; j--;) ret[(1 << i) | (1 << j)]-= adj[i * m + j];
  for(int s= N; --s;) ret[s]/= 2;
  return ret;
 }
 template <class T> inline Sps<T> biconnected_graph() const {
  Sps<T> ret= connected_graph<T>();
  return transform_articulation(ret, sps::log<T>), ret;
 }
 template <class T> inline Sps<T> two_edge_connected_graph() const {
  Sps<T> ret= connected_graph<T>();
  return transform_bridge<T, false>(ret), ret;
 }
 template <class T> inline Sps<T> cactus_graph() const {
  auto ret= cycle_graph<T>();
  for(int i= n; i--;)
   for(int j= i; j--;) ret[(1 << i) | (1 << j)]+= adj[i * m + j];
  return transform_articulation(ret, sps::exp<T>), ret;
 }
 template <class T> inline Sps<T> acyclic_orientations() const {
  auto k= connected_component_num();
  Sps<T> g(N, 0);
  for(int s= N; --s;)
   if(k[s] == __builtin_popcount(s)) g[s]= k[s] & 1 ? -1 : 1;
  return g[0]= 1, sps::inv(g);
 }
 template <class T> inline std::vector<T> colorings_using_exactly_k_colors_num() const {
  if(n == 0) return {0};  // impossible in any number of ways
  auto k= connected_component_num();
  std::vector<T> indep(N, 0);
  for(int s= N; --s;) indep[s]= k[s] == __builtin_popcount(s);
  return sps::egf_T(indep);
 }
 template <class T> inline Poly<T> chromatic_polynomial() const {
  auto e= colorings_using_exactly_k_colors_num<T>();
  if(e.back() == 0) return {0};
  Poly<T> ret(n + 1, 0);
  std::vector<T> tmp(n);
  tmp[0]= 1;
  for(int i= 1, j; i < n; ++i)
   for(j= i; j--; tmp[j]*= -i) ret[j + 1]+= tmp[j] * e[i], tmp[j + 1]+= tmp[j];
  for(int j= n; j--;) ret[j + 1]+= tmp[j];
  return ret;
 }
 template <class T> inline T tutte_polynomial(T x, T y) const {
  int sum[N], s, t, lim= 2, i, j;
  T fum[10'000]= {0, 1};
  std::vector<T> g= {0}, h;
  for(g.reserve(N), h.reserve(N), i= 0; i < n; h= sps::exp(h), std::copy(h.begin(), h.end(), std::back_inserter(g)), ++i) {
   for(sum[0]= j= 0; j < i; j++)
    for(s= t= 1 << j; s--;) sum[s | t]= sum[s] + adj[i * m + j];
   for(h.resize(s= 1 << i); s--; h[s]= g[s] * fum[sum[s]])
    for(; lim <= sum[s]; lim++) fum[lim]= fum[lim - 1] * y + 1;
  }
  for(x-= 1, t= ~0, j= 0, i= n; i--;) j+= adj[i * m + i];
  for(bfs((s= N) - 1, [&](int u) { t^= u; }); --s&= t;) g[s]*= x;
  return sps::exp(g)[N - 1] * pow(y, j);
 }
 template <class T> inline T perfect_matching() const { return sps::exp(cyc<T>()).back(); }
 template <class T> inline T all_matching() const {
  auto [cyc, pth]= cyc_pth<T>();
  for(int s= cyc.size(); s--;) cyc[s]+= pth[s];
  return sps::exp(cyc).back();
 }
 template <class T> std::vector<T> k_matching() const {
  auto [cyc, pth]= cyc_pth<T>();
  auto ret= sps::egf_T(pth, sps::exp(cyc));
  return std::reverse(ret.begin(), ret.end()), ret.resize(n / 2 + 1), ret;
 }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ModInt<998244353>;
 int N, M;
 cin >> N >> M;
 int all= (1 << N) - 1;
 UndirectedGraphSetPowerSeries g(N);
 for(int i= 0, a, b; i < M; i++) cin >> a >> b, g.add_edge(--a, --b);
 auto x= g.graph<Mint>();
 auto y= g.connected_graph<Mint>();
 for(int k= 1; k < N; ++k) {
  Mint ans= 0;
  for(int s= 1; s <= all; s+= 2)
   if((s >> k) & 1) ans+= y[s] * x[all ^ s];
  cout << ans << '\n';
 }
 return 0;
}
