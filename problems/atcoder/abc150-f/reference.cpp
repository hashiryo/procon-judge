// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
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
#include <random>
#include <cstdint>
uint64_t rng() {
 static uint64_t x= 10150724397891781847ULL * std::random_device{}();
 return x^= x << 7, x^= x >> 9;
}
uint64_t rng(uint64_t lim) { return rng() % lim; }
int64_t rng(int64_t l, int64_t r) { return l + rng() % (r - l); }
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
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 Nimber::init();
 using RH= RollingHash<Nimber, Nimber>;
 Nimber base= rng(2, 1ll << 60);
 RH::init(base);
 int N;
 cin >> N;
 vector<Nimber> a(N), b(N);
 for(int i= 0; i < N; ++i) cin >> a[i];
 for(int i= 0; i < N; ++i) cin >> b[i];
 RH ra(a), rb(b);
 auto X= (base.pow(N) - 1) / (base - 1);
 auto bh= rb.hash();
 for(int k= 0; k < N; ++k) {
  auto x= a[k] + b[0];
  if(concat_hash(ra.sub(k), ra.sub(0, k)) + x * X == bh) cout << k << " " << x << '\n';
 }
 return 0;
}
