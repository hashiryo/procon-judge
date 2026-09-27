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

#include <vector>
#include <functional>
#include <memory>
#include <optional>
#include <cstdint>
#include <cstddef>
#include <array>
#include <limits>
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

template <class T, size_t LM= 1 << 22> class RelaxedConvolution {
 std::vector<T> a, b, c;
 std::vector<NTTArray<T, LM, true>> ac, bc;
 std::function<T()> ha, hb;
 int n;
 template <class T0> static auto wrap(T0&& f, int& n, const std::vector<T>& c, std::vector<T>& e) {
  if constexpr(std::is_invocable_r_v<T, T0, int, const std::vector<T>&>) return std::bind([f](int n, const std::vector<T>& c, std::vector<T>& e) mutable { return T(e.emplace_back(f(n, c))); }, std::cref(n), std::cref(c), std::ref(e));
  else if constexpr(std::is_invocable_r_v<T, T0, int>) return std::bind([f](int n, std::vector<T>& e) mutable { return T(e.emplace_back(f(n))); }, std::cref(n), std::ref(e));
  else if constexpr(std::is_invocable_r_v<T, T0>) return std::bind([f](std::vector<T>& e) mutable { return T(e.emplace_back(f())); }, std::ref(e));
  else throw;
 }
public:
 template <class F1, class F2> RelaxedConvolution(F1&& h1, F2&& h2): c(4), ha(wrap(h1, n, c, a)), hb(wrap(h2, n, c, b)), n(0) { a.reserve(LM), b.reserve(LM), c.reserve(LM); }
 const std::vector<T>& multiplicand() const { return a; }
 const std::vector<T>& multiplier() const { return b; }
 T at(int k) { return (*this)[k]; }
 T operator[](int k) {
  while(n <= k) next();
  return c[k];
 }
 T next() {
  using GNA1= GlobalNTTArray<T, LM, 1>;
  using GNA2= GlobalNTTArray<T, LM, 2>;
  static constexpr int BASE_CASE_SIZE= 32;
  if(int l= pw2(n << 1 | 1); (int)c.size() < l) c.resize(l);
  if(n == 0) c[0]= ha() * hb();
  if(n == 1) c[1]= ha() * b[0] + a[0] * hb(), c[2]= a[1] * b[1];
  if(n == 2) c[2]+= ha() * b[0] + a[0] * hb(), c[3]= a[2] * b[1] + a[1] * b[2];
  if(n > 2) {
   if(!(n & (n - 1))) {
    int t0= n >> 1, t1= n;
    auto &c0= ac.emplace_back(), &c1= bc.emplace_back();
    c0.resize(t1), c0.set(a.data() + t0, 0, t0), c0.dft(0, t1), c1.resize(t1), c1.set(b.data() + t0, 0, t0), c1.dft(0, t1), GNA1::bf.mul(c0, c1, 0, t1), GNA1::bf.idft(0, t1);
    for(int i= t1 - 1; i--;) c[t1 + i]+= GNA1::bf.get(i);
   }
   c[n]+= ha() * b[0] + a[0] * hb(), c[n + 1]+= a[1] * b[n] + a[n] * b[1];
   for(int t0= 2, sft= 0, ofs= pw2(n + 1) >> 1, t= n + 1 - ofs; !(t & 1) && t0 < ofs; t0<<= 1, sft++, t>>= 1)
    if(int m= n + 1 - t0, t1= t0 << 1; t0 > BASE_CASE_SIZE) {
     GNA1::bf.set(a.data() + m, 0, t0), GNA1::bf.zeros(t0, t1), GNA2::bf.set(b.data() + m, 0, t0), GNA2::bf.zeros(t0, t1), GNA1::bf.dft(0, t1), GNA2::bf.dft(0, t1), GNA1::bf.mul(bc[sft], 0, t1), GNA2::bf.mul(ac[sft], 0, t1), GNA1::bf.add(GNA2::bf, 0, t1), GNA1::bf.idft(0, t1);
     for(int i= t1 - 1; i--;) c[n + 1 + i]+= GNA1::bf.get(i);
    } else
     for(int i= t0; i--;)
      for(int j= t0; j--;) c[n + 1 + i + j]+= a[m + i] * b[j + t0] + a[j + t0] * b[m + i];
  }
  return c[n++];
 }
};
template <class mod_t, size_t LM= 1 << 22> class FormalPowerSeries {
 using F= std::function<mod_t(int)>;
 using FPS= FormalPowerSeries;
 F h_;
public:
 class Resetter {
  std::shared_ptr<F> p_;
 public:
  Resetter() {}
  Resetter(std::shared_ptr<F> p): p_(p) {}
  void set(const FPS& rhs) { *p_= rhs.handle(); }
 };
 class Inde {  // indeterminate
  int p_;
 public:
  Inde(int p): p_(p) {}
  Inde(): Inde(1) {}
  Inde operator^(int p) const { return Inde(p_ * p); }
  Inde operator*(const Inde& rhs) const { return Inde(p_ + rhs.p_); }
  int pow() const { return p_; }
 };
 FormalPowerSeries(): h_([](int) { return mod_t(0); }) {}
 FormalPowerSeries(F f)
     : h_([f, cache= std::make_shared<std::vector<mod_t>>()](int k) -> mod_t {
        for(int i= (int)cache->size(); i <= k; ++i) cache->emplace_back(f(i));
        return cache->at(k);
       }) {}
 FormalPowerSeries(const std::vector<mod_t>& coef): h_([cache= std::make_shared<std::vector<mod_t>>(coef)](int k) -> mod_t { return k < (int)cache->size() ? cache->at(k) : mod_t(0); }) {}
 FormalPowerSeries(mod_t v): h_([v](int k) { return k == 0 ? v : mod_t(0); }) {}
 F handle() const { return h_; }
 static Inde x() { return Inde(); }
 Resetter reset() {
  auto p= std::make_shared<F>();
  return h_= [p](int i) { return (*p)(i); }, Resetter(p);
 }
 mod_t operator[](int k) const { return h_(k); }
 FPS operator()(const Inde& rhs) const { return scale(rhs.pow()); }
 FPS operator*(const Inde& rhs) const { return shift(rhs.pow()); }
 FPS operator*(const mod_t& rhs) const {
  return FPS([h= h_, v= rhs](int i) { return h(i) * v; });
 }
 FPS operator/(const mod_t& rhs) const {  // `rhs == 0` is not allowed
  return FPS([h= h_, v= mod_t(1) / rhs](int i) { return h(i) * v; });
 }
 FPS operator+(const mod_t& rhs) const {
  return FPS([h= h_, v= rhs](int i) { return i ? h(i) : h(i) + v; });
 }
 FPS operator-(const mod_t& rhs) const {
  return FPS([h= h_, v= rhs](int i) { return i ? h(i) : h(i) - v; });
 }
 friend FPS operator*(const Inde& lhs, const FPS& rhs) { return rhs.shift(lhs.pow()); }
 friend FPS operator-(const mod_t& lhs, const FPS& rhs) {
  return FPS([h= rhs.h_, v= lhs](int i) { return i ? -h(i) : v - h(i); });
 }
 friend FPS operator+(const mod_t& lhs, const FPS& rhs) {
  return FPS([h= rhs.h_, v= lhs](int i) { return i ? h(i) : h(i) + v; });
 }
 friend FPS operator*(const mod_t& lhs, const FPS& rhs) {
  return FPS([h= rhs.h_, v= lhs](int i) { return h(i) * v; });
 }
 friend FPS operator/(const mod_t& lhs, const FPS& rhs) { return lhs * rhs.inv(); }
 FPS scale(int k) const {
  return FPS([h= h_, k](int i) { return i % k ? mod_t(0) : h(i / k); });
 }
 FPS shift(int k) const {
  return FPS([h= h_, k](int i) { return i < k ? mod_t(0) : h(i - k); });
 }
 FPS inv() const {
  auto rc= std::make_shared<RelaxedConvolution<mod_t, LM>>([h= h_](int i) { return h(i); }, [h= h_, iv= mod_t()](int i, const auto& c) mutable { return i ? -(c[i] + h(i) * iv) * iv : (iv= mod_t(1) / h(0)); });
  return FPS([rc](int i) { return rc->next(), rc->multiplier()[i]; });  // safe
 }
 friend FPS deriv(const FPS& fps) {
  return FPS([h= fps.h_](int i) { return h(i + 1) * mod_t(i + 1); });
 }
 friend FPS integ(const FPS& fps) {
  return FPS([h= fps.h_](int i) { return i ? h(i - 1) * get_inv<mod_t, LM>(i) : mod_t(0); });
 }
 // `fps[0]==1` is required
 friend FPS log(const FPS& fps) { return integ(deriv(fps) / fps); }
 friend FPS exp(const FPS& fps) {  // `fps[0]==0` is required
  auto rc= std::make_shared<RelaxedConvolution<mod_t, LM>>([h= fps.h_](int i) { return h(i + 1) * mod_t(i + 1); }, [](int i, const auto& c) { return i ? c[i - 1] * get_inv<mod_t, LM>(i) : mod_t(1); });
  return FPS([rc](int i) { return i ? rc->at(i - 1) * get_inv<mod_t, LM>(i) : mod_t(1); });
 }
 friend FPS pow(const FPS& fps, uint64_t k) {
  if(!k) return FPS(1);
  return FPS([h= fps.h_, kk= mod_t(k), k, cnt= 0ull, s= std::optional<std::function<mod_t(int)>>()](int i) mutable {
   if(s) return (uint64_t)i < cnt ? mod_t(0) : (*s)(i - (int)cnt);
   mod_t v= h(i);
   if(v == mod_t(0)) return cnt++, mod_t(0);
   cnt*= k;
   FPS t0([os= i, iv= mod_t(1) / v, h](int i) { return h(i + os) * iv; });
   FPS t1([h0= log(t0).handle(), kk](int i) { return h0(i) * kk; });
   s.emplace([vk= v.pow(k), h1= exp(t1).handle()](int i) { return h1(i) * vk; });
   return cnt ? mod_t(0) : (*s)(i);
  });
 }
 friend FPS SEQ(const FPS& fps) {  // SEQUENCE `fps[0]==0` is required
  return FPS([h= fps.h_](int i) { return i == 0 ? mod_t(1) : -h(i); }).inv();
 }
 friend FPS MSET(const FPS& fps) {  // MULTISET `fps[0]==0` is required
  return exp(FPS([h= fps.h_, cache= std::make_shared<std::vector<mod_t>>()](int i) {
   if(i == 0) return mod_t(0);
   if((i & (i - 1)) == 0) {
    cache->resize(i * 2, mod_t(0));
    for(int j= 1; j < i; ++j) {
     mod_t hj= h(j);
     for(int k= (i + j - 1) / j, ed= (i * 2 + j - 1) / j; k < ed; k++) cache->at(j * k)+= hj * get_inv<mod_t, LM>(k);
    }
   }
   return mod_t(cache->at(i)+= h(i));
  }));
 }
 friend FPS PSET(const FPS& fps) {  //  POWERSET `fps[0]==0` is required
  return exp(FPS([h= fps.h_, cache= std::make_shared<std::vector<mod_t>>()](int i) {
   if(i == 0) return mod_t(0);
   if((i & (i - 1)) == 0) {
    cache->resize(i * 2, mod_t(0));
    for(int j= 1; j < i; ++j) {
     mod_t hj= h(j);
     for(int k= (i + j - 1) / j, ed= (i * 2 + j - 1) / j; k < ed; k++)
      if(k & 1) cache->at(j * k)+= hj * get_inv<mod_t, LM>(k);
      else cache->at(j * k)-= hj * get_inv<mod_t, LM>(k);
    }
   }
   return mod_t(cache->at(i)+= h(i));
  }));
 };
 FPS operator+(const FPS& rhs) const {
  return FPS([h0= h_, h1= rhs.h_](int i) { return h0(i) + h1(i); });
 }
 FPS operator-(const FPS& rhs) const {
  return FPS([h0= h_, h1= rhs.h_](int i) { return h0(i) - h1(i); });
 }
 FPS operator-() const {
  return FPS([h= h_](int i) { return -h(i); });
 }
 FPS operator*(const FPS& rhs) const {
  auto rc= std::make_shared<RelaxedConvolution<mod_t, LM>>([h= h_](int i) { return h(i); }, [h= rhs.h_](int i) { return h(i); });
  return FPS([rc](int) { return rc->next(); });
 }
 FPS operator/(const FPS& rhs) const {
  auto rc= std::make_shared<RelaxedConvolution<mod_t, LM>>([h= rhs.h_](int i) { return h(i); },
                                                           [h0= h_, h1= rhs.h_, iv= mod_t(), t0= mod_t()](int i, const auto& c) mutable {
                                                            if(i == 0) return t0= h0(0) * (iv= mod_t(1) / h1(0));
                                                            return (h0(i) - h1(i) * t0 - c[i]) * iv;
                                                           });
  return FPS([rc](int i) { return rc->next(), rc->multiplier()[i]; });
 }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ModInt<998244353>;
 using FPS= FormalPowerSeries<Mint, 40'010>;
 int N, M, T;
 cin >> N >> M >> T;
 std::vector<Mint> p[M];
 int a[M], b[M];
 for(int i= 0; i < M; i++) {
  cin >> a[i] >> b[i], a[i]--, b[i]--;
  p[i].resize(T, 0);
  for(int j= 0; j < T; j++) cin >> p[i][j];
 }
 auto X= FPS::x();
 FPS d[N];
 FPS::Resetter r[N];
 for(int s= 0; s < N; s++) r[s]= d[s].reset();
 for(int s= 0; s < N; s++) {
  FPS tmp;
  for(int i= 0; i < M; i++) {
   if(b[i] == s) tmp= d[a[i]] * p[i] + tmp;
   if(a[i] == s) tmp= d[b[i]] * p[i] + tmp;
  }
  tmp= tmp * X;
  if(!s) tmp= tmp + 1;
  r[s].set(tmp);
 }
 cout << d[0][T] << '\n';
 return 0;
}
