// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <array>
#include <limits>
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

#include <iostream>
#include <utility>
#include <type_traits>
#include <cassert>
template <class Uint> constexpr inline Uint mod_inv(Uint a, Uint mod) {
 std::make_signed_t<Uint> x= 1, y= 0, z= 0;
 for (Uint q= 0, b= mod, c= 0; b;) z= x, x= y, y= z - y * (q= a / b), c= a, a= b, b= c - b * q;
 return assert(a == 1), x < 0 ? mod - (-x) % mod : x % mod;
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

template <class mod_t, size_t LM> mod_t get_inv(int n) {
 static_assert(is_modint_v<mod_t>);
 static const auto m= mod_t::mod();
 static mod_t* dat= new mod_t[LM];
 static int l= 1;
 if(l == 1) dat[l++]= 1;
 for(; l <= n; ++l) dat[l]= dat[m % l] * (m - m / l);
 return dat[n];
}
namespace math_internal {
#define CE constexpr
#define ST static
#define TP template
#define BSF(_, n) __builtin_ctz##_(n)
TP<class mod_t> struct NTT {
#define _DFT(a, b, c, ...) \
 mod_t r, u, *x0, *x1; \
 for(int a= n, b= 1, s, i; a>>= 1; b<<= 1) \
  for(s= 0, r= I, x0= x;; r*= c[BSF(, s)], x0= x1 + p) { \
   for(x1= x0 + (i= p); i--;) __VA_ARGS__; \
   if(++s == e) break; \
  }
 ST inline void dft(int n, mod_t x[]) { _DFT(p, e, r2, x1[i]= x0[i] - (u= r * x1[i]), x0[i]+= u); }
 ST inline void idft(int n, mod_t x[]) {
  _DFT(e, p, ir2, u= x0[i] - x1[i], x0[i]+= x1[i], x1[i]= r * u)
  for(const mod_t iv= I / n; n--;) x[n]*= iv;
 }
#undef _DFT
 ST inline void even_dft(int n, mod_t x[]) {
  for(int i= 0, j= 0; i < n; i+= 2) x[j++]= iv2 * (x[i] + x[i + 1]);
 }
 ST inline void odd_dft(int n, mod_t x[], mod_t r= iv2) {
  for(int i= 0, j= 0;; r*= ir2[BSF(, ++j)])
   if(x[j]= r * (x[i] - x[i + 1]); (i+= 2) == n) break;
 }
 ST inline void dft_doubling(int n, mod_t x[], int i= 0) {
  mod_t k= I, t= rt[BSF(, n << 1)];
  for(copy_n(x, n, x + n), idft(n, x + n); i < n; ++i) x[n + i]*= k, k*= t;
  dft(n, x + n);
 }
protected:
 ST CE u64 md= mod_t::mod();
 static_assert(md & 1);
 static_assert(is_prime(md));
 ST CE u8 E= BSF(ll, md - 1);
 ST CE mod_t w= [](u8 e) {
  for(mod_t r= 2;; r+= 1)
   if(auto s= r.pow((md - 1) / 2); s != 1 && s * s == 1) return r.pow((md - 1) >> e);
  return mod_t();
 }(E);
 static_assert(w != mod_t());
 ST CE mod_t I= 1, iv2= (md + 1) / 2, iw= w.pow((1ULL << E) - 1);
 ST CE auto roots(mod_t w) {
  array<mod_t, E + 1> x= {};
  for(u8 e= E; e; w*= w) x[e--]= w;
  return x[0]= w, x;
 }
 TP<u32 N> ST CE auto ras(const array<mod_t, E + 1>& rt, const array<mod_t, E + 1>& irt, int i= N) {
  array<mod_t, E + 1 - N> x= {};
  for(mod_t ro= 1; i <= E; ro*= irt[i++]) x[i - N]= rt[i] * ro;
  return x;
 }
 ST CE auto rt= roots(w), irt= roots(iw);
 ST CE auto r2= ras<2>(rt, irt), ir2= ras<2>(irt, rt);
};
TP<class T, u8 t, class B> struct NI: public B {
 using B::B;
#define FUNC(op, name, HG, ...) \
 inline void name(__VA_ARGS__) { \
  HG(op, 1); \
  if CE(t > 1) HG(op, 2); \
  if CE(t > 2) HG(op, 3); \
  if CE(t > 3) HG(op, 4); \
  if CE(t > 4) HG(op, 5); \
 }
#define REP for(int i= b; i < e; ++i)
#define DFT(fft, _) B::ntt##_::fft(e - b, this->dt##_ + b)
#define ZEROS(op, _) fill_n(this->dt##_ + b, e - b, typename B::m##_())
#define SET(op, _) copy(x + b, x + e, this->dt##_ + b)
#define SET_S(op, _) this->dt##_[i]= x;
#define SUBST(op, _) copy(r.dt##_ + b, r.dt##_ + e, this->dt##_ + b)
#define ASGN(op, _) REP this->dt##_[i] op##= r.dt##_[i]
#define ASN(nm, op) TP<class C> FUNC(op, nm, ASGN, const NI<T, t, C>& r, int b, int e)
#define BOP(op, _) REP this->dt##_[i]= l.dt##_[i] op r.dt##_[i]
#define OP(nm, op) TP<class C, class D> FUNC(op, nm, BOP, const NI<T, t, C>& l, const NI<T, t, D>& r, int b, int e)
 OP(add, +) OP(dif, -) OP(mul, *) ASN(add, +) ASN(dif, -) ASN(mul, *) FUNC(dft, dft, DFT, int b, int e) FUNC(idft, idft, DFT, int b, int e) FUNC(__, zeros, ZEROS, int b, int e) FUNC(__, set, SET, const T x[], int b, int e) FUNC(__, set, SET_S, int i, T x) TP<class C> FUNC(__, subst, SUBST, const NI<T, t, C>& r, int b, int e) inline void get(T x[], int b, int e) const {
  if CE(t == 1) copy(this->dt1 + b, this->dt1 + e, x + b);
  else REP x[i]= get(i);
 }
#define TMP(_) B::iv##_##1 * (this->dt##_[i] - r1)
 inline T get(int i) const {
  if CE(t > 1) {
   u64 r1= this->dt1[i].val(), r2= (TMP(2)).val();
   T a= 0;
   if CE(t > 2) {
    u64 r3= (TMP(3) - B::iv32 * r2).val();
    if CE(t > 3) {
     u64 r4= (TMP(4) - B::iv42 * r2 - B::iv43 * r3).val();
     if CE(t > 4) a= T(B::m4::mod()) * (TMP(5) - B::iv52 * r2 - B::iv53 * r3 - B::iv54 * r4).val();
     a= (a + r4) * B::m3::mod();
    }
    a= (a + r3) * B::m2::mod();
   }
   return (a + r2) * B::m1::mod() + r1;
  } else return this->dt1[i];
 }
#undef TMP
#undef DFT
#undef ZEROS
#undef SET
#undef SET_S
#undef SUBST
#undef ASGN
#undef ASN
#undef BOP
#undef OP
#undef FUNC
#undef REP
};
#define ARR(_) \
 using m##_= ModInt<M##_>; \
 using ntt##_= NTT<m##_>; \
 m##_* dt##_= new m##_[LM];
#define IV2 ST CE m2 iv21= m2(1) / m1::mod();
#define IV3 ST CE m3 iv32= m3(1) / m2::mod(), iv31= iv32 / m1::mod();
#define IV4 ST CE m4 iv43= m4(1) / m3::mod(), iv42= iv43 / m2::mod(), iv41= iv42 / m1::mod();
#define IV5 ST CE m5 iv54= m5(1) / m4::mod(), iv53= iv54 / m3::mod(), iv52= iv53 / m2::mod(), iv51= iv52 / m1::mod();
TP<u8 t, u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM, bool v> struct NB {
 ARR(1)
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<2, M1, M2, M3, M4, M5, LM, 0> {
 ARR(1) ARR(2) IV2
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<3, M1, M2, M3, M4, M5, LM, 0> {
 ARR(1) ARR(2) ARR(3) IV2 IV3
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<4, M1, M2, M3, M4, M5, LM, 0> {
 ARR(1) ARR(2) ARR(3) ARR(4) IV2 IV3 IV4
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<5, M1, M2, M3, M4, M5, LM, 0> {
 ARR(1) ARR(2) ARR(3) ARR(4) ARR(5) IV2 IV3 IV4 IV5
};
#undef ARR
#define VC(_) \
 using m##_= ModInt<M##_>; \
 using ntt##_= NTT<m##_>; \
 vector<m##_> bf##_; \
 m##_* dt##_;
#define RS resize
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<1, M1, M2, M3, M4, M5, LM, 1> {
 NB(): dt1(bf1.data()) {}
 void RS(int n) { bf1.RS(n), dt1= bf1.data(); }
 u32 size() const { return bf1.size(); }
 VC(1)
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<2, M1, M2, M3, M4, M5, LM, 1> {
 NB(): dt1(bf1.data()), dt2(bf2.data()) {}
 void RS(int n) { bf1.RS(n), dt1= bf1.data(), bf2.RS(n), dt2= bf2.data(); }
 u32 size() const { return bf1.size(); }
 VC(1) VC(2) IV2
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<3, M1, M2, M3, M4, M5, LM, 1> {
 NB(): dt1(bf1.data()), dt2(bf2.data()), dt3(bf3.data()) {}
 void RS(int n) { bf1.RS(n), dt1= bf1.data(), bf2.RS(n), dt2= bf2.data(), bf3.RS(n), dt3= bf3.data(); }
 u32 size() const { return bf1.size(); }
 VC(1) VC(2) VC(3) IV2 IV3
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<4, M1, M2, M3, M4, M5, LM, 1> {
 NB(): dt1(bf1.data()), dt2(bf2.data()), dt3(bf3.data()), dt4(bf4.data()) {}
 void RS(int n) { bf1.RS(n), dt1= bf1.data(), bf2.RS(n), dt2= bf2.data(), bf3.RS(n), dt3= bf3.data(), bf4.RS(n), dt4= bf4.data(); }
 u32 size() const { return bf1.size(); }
 VC(1) VC(2) VC(3) VC(4) IV2 IV3 IV4
};
TP<u64 M1, u32 M2, u32 M3, u32 M4, u32 M5, u32 LM> struct NB<5, M1, M2, M3, M4, M5, LM, 1> {
 NB(): dt1(bf1.data()), dt2(bf2.data()), dt3(bf3.data()), dt4(bf4.data()), dt5(bf5.data()) {}
 void RS(int n) { bf1.RS(n), dt1= bf1.data(), bf2.RS(n), dt2= bf2.data(), bf3.RS(n), dt3= bf3.data(), bf4.RS(n), dt4= bf4.data(), bf5.RS(n), dt5= bf5.data(); }
 u32 size() const { return bf1.size(); }
 VC(1) VC(2) VC(3) VC(4) VC(5) IV2 IV3 IV4 IV5
};
#undef VC
#undef IV2
#undef IV3
#undef IV4
#undef IV5
TP<class T, u32 LM> CE bool is_nttfriend() {
 if CE(!is_staticmodint_v<T>) return 0;
 else return (T::mod() & is_prime(T::mod())) && LM <= (1ULL << BSF(ll, T::mod() - 1));
}
TP<class T, enable_if_t<is_arithmetic_v<T>, nullptr_t> = nullptr> CE u64 mv() { return numeric_limits<T>::max(); }
TP<class T, enable_if_t<is_staticmodint_v<T>, nullptr_t> = nullptr> CE u64 mv() { return T::mod(); }
TP<class T, u32 LM, u32 M1, u32 M2, u32 M3, u32 M4> CE u8 nt() {
 if CE(!is_nttfriend<T, LM>()) {
  CE u128 m= mv<T>(), mm= m * m;
  if CE(mm <= M1 / LM) return 1;
  else if CE(mm <= u64(M1) * M2 / LM) return 2;
  else if CE(mm <= u128(M1) * M2 * M3 / LM) return 3;
  else if CE(mm <= u128(M1) * M2 * M3 * M4 / LM) return 4;
  else return 5;
 } else return 1;
}
#undef BSF
#undef RS
CE u32 MOD1= 998244353, MOD2= 897581057, MOD3= 880803841, MOD4= 754974721, MOD5= 645922817;
TP<class T, u32 LM> CE u8 nttarr_type= nt<T, LM, MOD1, MOD2, MOD3, MOD4>();
TP<class T, u32 LM> CE u8 nttarr_cat= is_nttfriend<T, LM>() && (mv<T>() > (1 << 30)) ? 0 : nttarr_type<T, LM>;
TP<class T, u32 LM, bool v> using NTTArray= NI<T, nttarr_type<T, LM>, conditional_t<is_nttfriend<T, LM>(), NB<1, mv<T>(), 0, 0, 0, 0, LM, v>, NB<nttarr_type<T, LM>, MOD1, MOD2, MOD3, MOD4, MOD5, LM, v>>>;
#undef CE
#undef ST
#undef TP
}
using math_internal::is_nttfriend, math_internal::nttarr_type, math_internal::nttarr_cat, math_internal::NTT, math_internal::NTTArray;
template <class T, size_t LM, int id= 0> struct GlobalNTTArray {
 static inline NTTArray<T, LM, 0> bf;
};
template <class T, size_t LM, size_t LM2, int id= 0> struct GlobalNTTArray2D {
 static inline NTTArray<T, LM, 0>* bf= new NTTArray<T, LM, 0>[LM2];
};
template <class T, size_t LM, int id= 0> struct GlobalArray {
 static inline T* bf= new T[LM];
};
constexpr unsigned pw2(unsigned n) { return --n, n|= n >> 1, n|= n >> 2, n|= n >> 4, n|= n >> 8, n|= n >> 16, ++n; }

namespace math_internal {
class BigInt {
 static constexpr u64 BASE= 1e15;
 static constexpr int8_t D= 15;
 using Vec= vector<u64>;
 bool neg;
 Vec dat;
 BigInt(bool n, const Vec& d): neg(n), dat(d) {}
public:
 BigInt(): neg(false), dat() {}
 BigInt(__int128_t v): neg(v < 0) {
  for(v= v < 0 ? -v : v; v; v/= BASE) dat.push_back(v % BASE);
 }
 BigInt(const string& s): neg(false) {
  int p= 0, i= s.size(), j;
  for(; p < i && (s[p] == '-' || s[p] == '+'); ++p)
   if(s[p] == '-') neg= !neg;
  for(u64 x= 0; i > p; i-= D, dat.push_back(x), x= 0)
   for(j= max(p, i - D); j < i;) x= x * 10 + s[j++] - '0';
  shrink();
 }
 inline void shrink() {
  while(!dat.empty() && !dat.back()) dat.pop_back();
  if(dat.empty()) neg= false;
 }
 string to_str() const {
  if(is_zero()) return "0";
  stringstream ss;
  if(neg) ss << '-';
  ss << (dat.empty() ? 0 : dat.back());
  for(int i= dat.size() - 1; i-- > 0;) ss << setw(D) << setfill('0') << dat[i];
  string ret;
  return ss >> ret, ret;
 }
 bool is_zero() const { return dat.empty() || (dat.size() == 1 && !dat[0]); }
 bool operator<(const BigInt& r) const {
  if(neg != r.neg) return neg;
  if(dat.size() != r.dat.size()) return (dat.size() < r.dat.size()) ^ neg;
  for(int i= dat.size(); i--;)
   if(dat[i] != r.dat[i]) return (dat[i] < r.dat[i]) ^ neg;
  return false;
 }
 bool operator>(const BigInt& r) const { return r < *this; }
 bool operator<=(const BigInt& r) const { return !(r < *this); }
 bool operator>=(const BigInt& r) const { return !(*this < r); }
 bool operator==(const BigInt& r) const { return (neg == r.neg && dat == r.dat) || (is_zero() && r.is_zero()); }
 bool operator!=(const BigInt& r) const { return !(*this == r); }
 BigInt abs() const { return BigInt(false, dat); }
 BigInt operator-() const { return BigInt(!neg, dat); }
 BigInt operator+(const BigInt& r) const {
  if(neg != r.neg) return *this - (-r);
  auto [ret, tmp]= dat.size() > r.dat.size() ? make_pair(*this, &r) : make_pair(r, this);
  int car= 0, i, n= ret.dat.size(), m= tmp->dat.size();
  for(i= 0; i < m; i++) ret.dat[i]-= BASE & -(car= ((ret.dat[i]+= car + tmp->dat[i]) >= BASE));
  if(car) {
   while(i < n && ret.dat[i] == BASE - 1) ret.dat[i++]= 0;
   i < n ? ++ret.dat[i] : (ret.dat.push_back(1), 0);
  }
  return ret;
 }
 BigInt operator-(const BigInt& r) const {
  if(neg != r.neg) return *this + (-r);
  if(r.is_zero()) return *this;
  if(is_zero()) return -r;
  auto [ret, tmp]= abs() > r.abs() ? make_pair(*this, &r) : make_pair(r, this);
  int car= 0, i, n= ret.dat.size(), m= tmp->dat.size();
  for(i= 0; i < m; i++) ret.dat[i]+= BASE & -(car= ((ret.dat[i]-= car + tmp->dat[i]) >> 63));
  while(car && i < n && !ret.dat[i]) ret.dat[i++]= BASE - 1;
  return ret.neg^= (tmp == this), ret.dat[i]-= car, ret.shrink(), ret;
 }
 long long operator%(long long r) const {
  long long ret= 0;
  for(int i= dat.size(); i--;) ret= ((u128)ret * BASE + dat[i]) % r;
  return ret;
 }
 BigInt operator*(const BigInt& r) const {
  using mint1= ModInt<MOD1>;
  using mint2= ModInt<MOD2>;
  using mint3= ModInt<MOD3>;
  using mint4= ModInt<MOD4>;
  using ntt1= NTT<mint1>;
  using ntt2= NTT<mint2>;
  using ntt3= NTT<mint3>;
  using ntt4= NTT<mint4>;
  static constexpr mint2 iv21= mint2(1) / MOD1;
  static constexpr mint3 iv32= mint3(1) / MOD2, iv31= iv32 / MOD1;
  static constexpr mint4 iv43= mint4(1) / MOD3, iv42= iv43 / MOD2, iv41= iv42 / MOD1;
  if(is_zero() || r.is_zero()) return 0;
  const int n= dat.size(), m= r.dat.size(), sz= n + m - 1;
  static mint1 f1[1 << 20], g1[1 << 20];
  static mint2 f2[1 << 20], g2[1 << 20];
  static mint3 f3[1 << 20], g3[1 << 20];
  static mint4 f4[1 << 20], g4[1 << 20];
  static u128 h[1 << 20];
  if(int i= n, j; min(n, m) >= 135) {
   const int l= pw2(max(n, m)), fl= std::pow(l, 0.535) * 8.288, len= sz <= l + fl ? l : pw2(sz);
   for(i= n; i--;) f1[i]= dat[i];
   for(i= n; i--;) f2[i]= dat[i];
   for(i= n; i--;) f3[i]= dat[i];
   for(i= n; i--;) f4[i]= dat[i];
   fill_n(f1 + n, len - n, mint1()), ntt1::dft(len, f1), fill_n(f2 + n, len - n, mint2()), ntt2::dft(len, f2), fill_n(f3 + n, len - n, mint3()), ntt3::dft(len, f3), fill_n(f4 + n, len - n, mint4()), ntt4::dft(len, f4);
   if(this != &r) {
#define TMP(k) \
 for(i= m; i--;) g##k[i]= r.dat[i]; \
 fill_n(g##k + m, len - m, mint##k()), ntt##k::dft(len, g##k); \
 for(i= len; i--;) f##k[i]*= g##k[i];
    TMP(1) TMP(2) TMP(3) TMP(4)
#undef TMP
   } else {
    for(i= len; i--;) f1[i]*= f1[i];
    for(i= len; i--;) f2[i]*= f2[i];
    for(i= len; i--;) f3[i]*= f3[i];
    for(i= len; i--;) f4[i]*= f4[i];
   }
   for(ntt1::idft(len, f1), ntt2::idft(len, f2), ntt3::idft(len, f3), ntt4::idft(len, f4), i= len; i < sz; f1[i - len]-= h[i], f2[i - len]-= h[i], f3[i - len]-= h[i], f4[i - len]-= h[i], ++i)
    for(h[i]= 0, j= i - m + 1; j < n; j++) h[i]+= (u128)dat[j] * r.dat[i - j];
   for(i= min(sz, len); i--;) {
    u32 r1= f1[i].val(), r2= (iv21 * (f2[i] - r1)).val(), r3= (iv31 * (f3[i] - r1) - iv32 * r2).val();
    h[i]= ((u128)((u64)(iv41 * (f4[i] - r1) - iv42 * r2 - iv43 * r3).val() * MOD3 + r3) * MOD2 + r2) * MOD1 + r1;
   }
  } else
   for(fill_n(h, sz, 0); i--;)
    for(j= m; j--;) h[i + j]+= (u128)dat[i] * r.dat[j];
  BigInt ret(neg ^ r.neg, Vec(sz));
  u128 car= 0;
  for(int i= 0; i < sz; ++i, car/= BASE) ret.dat[i]= (car+= h[i]) % BASE;
  for(; car; car/= BASE) ret.dat.push_back(car % BASE);
  return ret;
 }
 BigInt operator/(const BigInt& r) const {
  if(assert(!r.is_zero()); r.dat.size() == 1) {
   BigInt qu(neg ^ r.neg, Vec(dat.size()));
   u128 d= 0, q;
   u64 r0= r.dat[0];
   for(int i= dat.size(); i--;) (d*= BASE)+= dat[i], q= d / r0, d= d % r0, qu.dat[i]= q;
   return qu.shrink(), qu;
  }
  BigInt a= this->abs(), b= r.abs();
  if(a < b) return 0;
  const u64 norm= BASE / (b.dat.back() + 1);
  const u32 s= (a*= norm).dat.size(), t= (b*= norm).dat.size(), deg= s - t + 2;
  const u64 yb= b.dat.back();
  u32 k= deg;
  while(k > 64) k= (k + 1) / 2;
  BigInt z(0, Vec(k + 2)), rem(0, Vec(t));
  rem.dat.back()= 1;
  for(int i= z.dat.size(); i--;) {
   if(rem.dat.size() == t) {
    if(b <= rem) z.dat[i]= 1, rem-= b;
   } else if(rem.dat.size() > t) {
    u64 q= ((u128)rem.dat[rem.dat.size() - 1] * BASE + rem.dat[rem.dat.size() - 2]) / yb;
    BigInt yq= b * q;
    while(rem < yq) --q, yq-= b;
    rem-= yq, z.dat[i]= q;
   }
   if(i) rem.dat.insert(rem.dat.begin(), 0);
  }
  for(z.shrink(); k < deg; k<<= 1) {
   int d= min(t, 2 * k + 1);
   BigInt x= z * z, w2= z + z;
   Vec w_(k + 1);
   x.dat.insert(x.dat.begin(), 0), x*= BigInt(0, Vec(b.dat.end() - d, b.dat.end())), x.dat.erase(x.dat.begin(), x.dat.begin() + d), copy(w2.dat.begin(), w2.dat.end(), back_inserter(w_)), z= BigInt(0, w_) - x, z.dat.erase(z.dat.begin());
  }
  z.dat.erase(z.dat.begin(), z.dat.begin() + k - deg);
  BigInt q= a * z;
  for(q.dat.erase(q.dat.begin(), q.dat.begin() + t + deg), z= b * q; a < z;) q-= 1, z-= b;
  for(rem= a - z; b <= rem;) q+= 1, rem-= b;
  return q.shrink(), q.neg= neg ^ r.neg, q;
 }
 BigInt operator%(const BigInt& r) const { return *this - (*this / r) * r; }
 BigInt& operator+=(const BigInt& r) { return *this= *this + r; }
 BigInt& operator-=(const BigInt& r) { return *this= *this - r; }
 BigInt& operator*=(const BigInt& r) { return *this= *this * r; }
 BigInt& operator/=(const BigInt& r) { return *this= *this / r; }
 BigInt& operator%=(const BigInt& r) { return *this= *this % r; }
 friend istream& operator>>(istream& is, BigInt& v) {
  string s;
  return is >> s, v= BigInt(s), is;
 }
 friend ostream& operator<<(ostream& os, const BigInt& v) { return os << v.to_str(), os; }
 explicit operator int() { return is_zero() ? 0 : neg ? -dat[0] : dat[0]; }
};
}
using math_internal::BigInt;

#include <map>
#include <unordered_map>
#include <array>
#include <cassert>
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

namespace period_internal {
template <class Map> struct PeriodB {
 using Iter= typename Map::const_iterator;
 Map mp;
};
template <class T> using PerB= std::conditional_t<std::is_integral_v<T>, PeriodB<std::unordered_map<T, int>>, PeriodB<std::map<T, int>>>;
}
template <class T= int> class Period: period_internal::PerB<T> {
 using typename period_internal::PerB<T>::Iter;
 using Path= std::vector<std::pair<int, int>>;
 std::vector<int> t, rt;
 std::vector<T> dc;
 HeavyLightDecomposition hld;
 static std::vector<int> iota(int n) {
  std::vector<int> v(n);
  for(int i= n; i--;) v[i]= i;
  return v;
 }
public:
 Period()= default;
 template <class F> Period(const F& f, const std::vector<T>& inits) {
  int n= 0;
  auto id= [&](const T& x) {
   if(auto it= this->mp.find(x); it != this->mp.end()) return it->second;
   return dc.emplace_back(x), t.push_back(-1), rt.push_back(-1), this->mp[x]= n++;
  };
  for(const T& s: inits)
   if(int v= id(s), w; rt[v] == -1) {
    for(w= v;; rt[w]= -2, w= t[w]= id(f(dc[w])))
     if(rt[w] != -1) {
      if(rt[w] != -2) w= rt[w];
      break;
     }
    for(int u= v; rt[u] == -2; u= t[u]) rt[u]= w;
   }
  Graph g(n + 1, n);
  for(int v= n; v--;) g[v]= {(rt[v] == v ? n : t[v]), v};
  hld= HeavyLightDecomposition(g.adjacency_vertex(1), n);
 }
 Period(const std::vector<int>& functional): Period([&](int x) { return functional[x]; }, iota(functional.size())) { static_assert(std::is_same_v<T, int>); }
 int operator()(const T& x) const {
  Iter it= this->mp.find(x);
  assert(it != this->mp.end());
  return t.size() - hld.to_seq(it->second);
 }
 size_t size() const { return t.size(); }
 // f^k(x)
 template <class Int, class= std::void_t<decltype(std::declval<Int>() % std::declval<int>())>> T jump(const T& x, Int k) const {
  Iter it= this->mp.find(x);
  assert(it != this->mp.end());
  int v= it->second, d= hld.depth(v) - 1;
  if(k <= d) return dc[hld.la(v, (int)k)];
  int b= t[v= rt[v]], l= (k-= d) % hld.depth(b);
  if(l == 0) return dc[v];
  return dc[hld.la(b, l - 1)];
 }
 // x, f(x), f(f(x)), ... f^k(x)
 // (x,...,f^i(x)), (f^(i+1)(x),...,f^(j-1)(x)) x cycle, (f^j(x),...,f^k(x))
 // sequence of half-open intervals [l,r)
 template <class Int, class= std::void_t<decltype(std::declval<Int>() % std::declval<int>())>> std::tuple<Path, Path, Int, Path> path(const T& x, Int k) const {
  Iter it= this->mp.find(x);
  assert(it != this->mp.end());
  int v= it->second, n= t.size(), d= hld.depth(v) - 1;
  std::array<Path, 3> pth;
  Int cnt= 0;
  if(k > d) {
   int r= rt[v], b= t[r], c= hld.depth(b), l= (k-= d) % c;
   if(pth[0]= hld.path(v, r), pth[1]= hld.path(b, r), cnt= k / c; l) pth[2]= hld.path(b, hld.la(b, l - 1));
  } else pth[0]= hld.path(v, hld.la(v, (int)k));
  for(int s= 3; s--;)
   for(auto& [l, r]: pth[s]) l= n - l, r= n - r + 1;
  return {pth[0], pth[1], cnt, pth[2]};
 }
 Path path_upto_cycle(const T& x) const {
  Iter it= this->mp.find(x);
  assert(it != this->mp.end());
  int v= it->second, n= t.size(), r= rt[v], b= t[r], w= hld.lca(b, v);
  auto p1= hld.path(v, r);
  if(b != w) {
   auto p2= hld.path(b, hld.jump(w, b, 1));
   p1.insert(p1.end(), p2.begin(), p2.end());
  }
  for(auto& [l, r]: p1) l= n - l, r= n - r + 1;
  return p1;
 }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 string S;
 cin >> S;
 int N= S.length();
 vector<int> to(N);
 for(int i= 0; i < N; ++i) to[i]= S[i] == 'L' ? i - 1 : i + 1;
 Period g(to);
 BigInt K("1" + string(100, '0'));
 vector cnt(N, 0);
 for(int i= 0; i < N; ++i) ++cnt[g.jump(i, K)];
 for(int i= 0; i < N; ++i) cout << cnt[i] << " \n"[i == N - 1];
 return 0;
}
