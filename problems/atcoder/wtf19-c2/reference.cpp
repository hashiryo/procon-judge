// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/wtf19-open/tasks/wtf19_c2
// nimber pow log
#include <iostream>
#include <vector>
#include <numeric>
#include <utility>
#include <cassert>
#include <iostream>
class Nimber {
 using u64= unsigned long long;
 using u32= unsigned;
 using u16= unsigned short;
 static inline u16 pw[65536], ln[65536];
 template <u16 h= 3> static inline u16 half(u16 A) { return A ? pw[(ln[A] + h) % 65535] : 0; }
 template <u16 h= 0> static inline u16 mul(u16 A, u16 B) { return A && B ? pw[(ln[A] + ln[B] + h) % 65535] : 0; }
 template <u16 h= 0> static inline u16 mul(u16 A, u16 B, u16 C) { return A && B && C ? pw[(ln[A] + ln[B] + ln[C] + h) % 65535] : 0; }
 static inline u16 inv(u16 A) { return assert(A), pw[65535 - ln[A]]; }
 static inline u16 sqrt(u16 A) { return A ? pw[u16((65537 * u32(ln[A])) >> 1)] : 0; }
 static inline u64 mul(u64 A, u64 B) {
  u16 a0= u16(A), a1= u16(A >> 16), a2= u16(A >> 32), a3= A >> 48, b0= u16(B), b1= u16(B >> 16), b2= u16(B >> 32), b3= B >> 48, x0= a1 ^ a0, x1= a3 ^ a2, y0= b1 ^ b0, y1= b3 ^ b2, c0= mul(a0, b0), c1= mul(x0, y0) ^ c0, c2= mul<0>(a2 ^ a0, b2 ^ b0), c3= mul<0>(x0 ^ x1, y0 ^ y1) ^ c2 ^ c1;
  return c2^= (c0^= mul<3>(a1, b1)) ^ mul<3>(u16(a3 ^ a1), u16(b3 ^ b1)), c1^= mul<6>(a3, b3) ^ mul<3>(x1, y1), c0^= mul<6>(a2, b2) ^ mul<6>(x1, y1), (u64(c3) << 48) | (u64(c2) << 32) | (u32(c1) << 16) | c0;
 }
 static inline u64 inv(u64 A) {
  u16 a0= u16(A), a1= u16(A >> 16), a2= u16(A >> 32), a3= A >> 48, x= a2 ^ a3, y= a1 ^ a3, w= a0 ^ a2, v= a0 ^ a1, b3= mul(a1, a2, a1 ^ x), b2= mul(a0, a2, a0 ^ x), b1= mul(a0, a1, a0 ^ y), b0= mul(a0, v, w), t= mul<3>(w, x, x);
  return b0^= b1 ^ b2, b1^= b3, b2^= b3, b0^= b3^= mul(a0, a0, a3), b1^= t ^ mul<3>(a1, y, y), b0^= t ^ mul<3>(v, y, y), b3^= t= mul<3>(a1, a3, y) ^ mul<3>(a2, x, x), b2^= t ^ mul<3>(a0, a3, a3) ^ mul<3>(a1, a1, a2), b3^= mul<6>(a3, a3, x), b2^= mul<6>(a3, x, x), b1^= mul<6>(a3, a3, y ^ w), b0^= mul<6>(y, x, x), b2^= mul<9>(a3, a3, a3), b0^= mul<9>(a3, a3, y), t= mul<6>(x, b3) ^ mul<6>(a3, b2) ^ mul<3>(a1, b1) ^ mul(a0, b0), t= inv(t), (u64(mul(b3, t)) << 48) | (u64(mul(b2, t)) << 32) | (u32(mul(b1, t)) << 16) | mul(b0, t);
 }
 static inline u64 square(u64 A) {
  u16 a0= u16(A), a1= u16(A >> 16), a2= u16(A >> 32), a3= A >> 48;
  return a3= mul(a3, a3), a2= mul(a2, a2), a1= mul(a1, a1), a0= mul(a0, a0), a0^= half(a1) ^ half<6>(a3), a2^= half(a3), a1^= half(a3 ^ a2), (u64(a3) << 48) | (u64(a2) << 32) | (u32(a1) << 16) | a0;
 }
 static inline u64 pow(u64 A, u64 k) {
  for (u64 ret= 1;; A= square(A))
   if (k & 1 ? ret= mul(ret, A) : 0; !(k>>= 1)) return ret;
 }
 template <int mod> static inline int mdif(int a, int b) { return a+= mod & -((a-= b) < 0); }
 template <int mod> static inline int mmul(int a, int b) { return u64(a) * b % mod; }
 static inline int log16(u16 A, u16 B) {
  int a= ln[A], b= ln[B], x= 1;
  if (a == 0) return b == 0 ? 1 : -1;
  for (int q, z, u, y= 0, t= 65535; t;) z= x, u= a, x= y, y= z - y * (q= a / t), a= t, t= u - t * q;
  return b % a ? -1 : u32(b / a) * (x < 0 ? 65535 + x : x) % 65535;
 }
 template <int period, int size> static inline int bsgs(u64 x, u64 y) {
  static constexpr int mask= size - 1;
  std::pair<u64, int> vs[size];
  int os[size + 1]= {};
  u64 so[size], big= 1;
  for (int i= 0; i < size; ++i, big= mul(big, x)) ++os[(so[i]= big) & mask];
  for (int i= 0; i < size; ++i) os[i + 1]+= os[i];
  for (int i= 0; i < size; ++i) vs[--os[so[i] & mask]]= {so[i], i};
  for (int t= 0; t < period; t+= size, y= mul(y, big))
   for (int m= (y & mask), i= os[m], ret; i < os[m + 1]; ++i)
    if (y == vs[i].first) return (ret= vs[i].second - t) < 0 ? ret + period : ret;
  return -1;
 }
 static inline u64 log(u64 A, u64 B) {
  if (B == 1) return 0;
  if (!A && !B) return 1;
  if (!A || !B) return u64(-1);
  static constexpr int P0= 641, P1= 65535, P2= 65537, P3= 6700417, iv10= 40691, iv21= 32768, iv20= 45242, iv32= 3317441, iv31= 3350208, iv30= 3883315;
  int a0= bsgs<P0, 16>(pow(A, 0x663d80ff99c27f), pow(B, 0x663d80ff99c27f));
  if (a0 == -1) return u64(-1);
  int a1= log16(pow(A, 0x1000100010001), pow(B, 0x1000100010001));
  if (a1 == -1) return u64(-1);
  int a2= bsgs<P2, 256>(pow(A, 0xffff0000ffff), pow(B, 0xffff0000ffff));
  if (a2 == -1) return u64(-1);
  int a3= bsgs<P3, 2048>(pow(A, 0x280fffffd7f), pow(B, 0x280fffffd7f));
  if (a3 == -1) return u64(-1);
  int x1= mmul<P1>(mdif<P1>(a1, a0), iv10), x2= mdif<P2>(mmul<P2>(mdif<P2>(a2, a0), iv20), mmul<P2>(x1, iv21)), x3= mdif<P3>(mdif<P3>(mmul<P3>(mdif<P3>(a3, a0), iv30), mmul<P3>(x1, iv31)), mmul<P3>(x2, iv32));
  return u64(P0) * (u64(P1) * (u64(P2) * x3 + x2) + x1) + a0;
 }
 u64 x;
public:
 static inline void init(u32 x= 0, u32 y= 0) {
  constexpr u16 f2n[16]= {0x0001u, 0x2827u, 0x392bu, 0x8000u, 0x20fdu, 0x4d1du, 0xde4au, 0x0a17u, 0x3464u, 0xe3a9u, 0x6d8du, 0x34bcu, 0xa921u, 0xa173u, 0x0ebcu, 0x0e69u};
  for (int i= pw[0]= pw[65535]= 1; i < 65535; ++i) pw[i]= (pw[i - 1] << 1) ^ (0x1681fu & (-(pw[i - 1] >= 0x8000u)));
  for (int i= 1; i < 65535; ln[pw[i]= y]= i, i++)
   for (x= pw[i], y= 0; x; x&= x - 1) y^= f2n[__builtin_ctz(x)];
 }
 Nimber(u64 x_= 0): x(x_) {}
 Nimber &operator+=(const Nimber &r) { return x^= r.x, *this; }
 Nimber &operator-=(const Nimber &r) { return x^= r.x, *this; }
 Nimber &operator*=(const Nimber &r) { return x= mul(x, r.x), *this; }
 Nimber &operator/=(const Nimber &r) { return x= mul(x, inv(r.x)), *this; }
 Nimber operator+(const Nimber &r) const { return Nimber(x ^ r.x); }
 Nimber operator-(const Nimber &r) const { return Nimber(x ^ r.x); }
 Nimber operator*(const Nimber &r) const { return Nimber(mul(x, r.x)); }
 Nimber operator/(const Nimber &r) const { return Nimber(mul(x, inv(r.x))); }
 Nimber operator-() const { return *this; }
 Nimber inv() const { return Nimber(inv(x)); }
 Nimber square() const { return Nimber(square(x)); }
 Nimber sqrt() const {
  u16 a0= u16(x), a1= u16(x >> 16), a2= u16(x >> 32), a3= x >> 48;
  return a1^= half(a3 ^ a2), a2^= half(a3), a0^= half(a1) ^ half<6>(a3), Nimber((u64(sqrt(a3)) << 48) | (u64(sqrt(a2)) << 32) | (u32(sqrt(a1)) << 16) | sqrt(a0));
 }
 u64 val() const { return x; }
 Nimber pow(u64 k) const { return Nimber(pow(x, k)); }
 u64 log(const Nimber &r) const { return log(x, r.x); }
 bool operator==(const Nimber &r) const { return x == r.x; }
 bool operator!=(const Nimber &r) const { return x != r.x; }
 bool operator<(const Nimber &r) const { return x < r.x; }
 bool operator>(const Nimber &r) const { return x > r.x; }
 bool operator<=(const Nimber &r) const { return x <= r.x; }
 bool operator>=(const Nimber &r) const { return x >= r.x; }
 friend std::ostream &operator<<(std::ostream &os, const Nimber &r) { return os << r.x; }
 friend std::istream &operator>>(std::istream &is, Nimber &r) { return is >> r.x, is; }
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
int main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ModInt<0xffffffffffffffff>;
 Nimber::init();
 static constexpr long long OFS= 1e17;

 Nimber a= -1, b= a.pow(11);
 Mint s= a.log(a + 1), t= b.log(b + 1);

 int N;
 cin >> N;
 vector<long long> x(N), y(N);
 for(int i= 0; i < N; i++) cin >> x[i] >> y[i], x[i]+= OFS, y[i]+= OFS;
 auto f= [&](Nimber z) -> Nimber {
  Nimber sum= 0;
  for(int i= N; i--;) sum+= z.pow(x[i]) * (z + 1).pow(y[i]);
  return sum;
 };
 Mint u= a.log(f(a)), v= b.log(f(b));
 Mint Y= (u - v) / (s - t), X= u - s * Y;
 cout << (long long)X.val() - OFS << " " << (long long)Y.val() - OFS << '\n';
 return 0;
}
