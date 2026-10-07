#pragma once
// frobenius_ord16_f57.hpp の a^641 と a^6700417 の代わりの元を、x·F_k(y) の形の掛け算を並べた組み方を総当たりで探して見つけたものに替えた版 (探し方は frobenius_ord16_f18f49f7 と同じ)。
// y = a^3·F57(a) (a^3 = a·sq(a)) から c = y·sq(a)、d = y·F21(a)·F60(a^3)·F50(y) とする。c は frobenius_ord16_f57f51f29 と同じ F57(a^641)。表を引く回数は
// 4 回 (F57、F21、F60、F50)、sq は 1 回、掛け算は 6 回。探した範囲で依存の鎖が最も短かった組み方。F57、F21、F60、F50 は Log<D> に置く。ほかは
// frobenius_ord16_f57 と同じ。
// Library (6f8462e9c) の GF2p64.hpp から、LinMap、mul、sq と、ord() を持つ Log<D> とその材料を写し、IL16 を ORD16 に替えたもの。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#endif
#include <tuple>
#include <type_traits>
#include <cassert>
namespace gf2p64_internal {
using u64= unsigned long long;
using u32= unsigned;
using u16= unsigned short;
using u8= unsigned char;
template <class T, int N> struct Arr {
 T t[N];
};
template <class U, int N> struct Lin {
 U t[N][256];
 constexpr Lin(const U* b): t{} {
  for(int i= 0; i < N * 8; ++i)
   for(u32 h= 1 << (i & 7), j= h; j--;) t[i >> 3][h | j]= t[i >> 3][j] ^ b[i];
 }
 constexpr U operator()(u64 x) const {
  if constexpr(N == 2) return t[0][u8(x)] ^ t[1][u8(x >> 8)];
  else if constexpr(N == 4) return t[0][u8(x)] ^ t[1][u8(x >> 8)] ^ t[2][u8(x >> 16)] ^ t[3][u8(x >> 24)];
  else return t[0][u8(x)] ^ t[1][u8(x >> 8)] ^ t[2][u8(x >> 16)] ^ t[3][u8(x >> 24)] ^ t[4][u8(x >> 32)] ^ t[5][u8(x >> 40)] ^ t[6][u8(x >> 48)] ^ t[7][u8(x >> 56)];
 }
 constexpr Lin operator*(const Lin& r) const {
  U b[N * 8]{};
  for(int i= 0; i < N * 8; ++i) b[i]= (*this)(r.t[i >> 3][1 << (i & 7)]);
  return Lin(b);
 }
};
using LinMap= Lin<u64, 8>;
constexpr u64 cmul(u64 a, u64 b) {
 u64 r= 0;
 for(int i= 64; i--;) r= r << 1 ^ (0x1b & -(r >> 63)) ^ (a & -(b >> i & 1));
 return r;
}
template <class T, int N> constexpr Arr<T, N> inverse(const T* c) {
 Arr<T, N> a{}, r{};
 for(int i= 0; i < N; ++i) a.t[i]= c[i], r.t[i]= T(1) << i;
 for(int j= 0; j < N; ++j) {
  int p= j;
  while(!(a.t[p] >> j & 1)) ++p;
  if(p != j) a.t[j]^= a.t[p], r.t[j]^= r.t[p];
  for(int i= 0; i < N; ++i)
   if(i != j && a.t[i] >> j & 1) a.t[i]^= a.t[j], r.t[i]^= r.t[j];
 }
 return r;
}
constexpr LinMap F1= []() {
 u64 g[64]{};
 for(int i= 0; i < 64; ++i) g[i]= cmul(u64(1) << i, u64(1) << i);
 return LinMap(g);
}();
constexpr LinMap F2= F1 * F1, F3= F2 * F1, F4= F2 * F2, F7= F4 * F3, F8= F4 * F4, F15= F8 * F7, F16= F8 * F8, F32= F16 * F16, F48= F32 * F16, F63= F48 * F15;
inline u64 mul(u64 a, u64 b) {
 static constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 __m128i v= _mm_clmulepi64_si128(_mm_cvtsi64_si128(a), _mm_cvtsi64_si128(b), 0);
 u64 h= v[1], d= h ^ (h << 1);
 return v[0] ^ RED[h >> 60] ^ d ^ (d << 3);
}
inline u64 sq(u64 a) {
 static constexpr u8 RED_SQ[4]= {0, 27, 90, 65};
 const __m128i MASK_LO= _mm_set1_epi8(0x0f);
 const __m128i SPR= _mm_setr_epi8(0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f, (char)0xc0, (char)0xc3, (char)0xcc, (char)0xcf, (char)0xf0, (char)0xf3, (char)0xfc, (char)0xff);
 __m128i v= _mm_set_epi64x(0, a);
 __m128i x= _mm_shuffle_epi8(SPR, _mm_and_si128(_mm_unpacklo_epi8(v, _mm_srli_epi16(v, 4)), MASK_LO));
 u64 d= x[1];
 return (x[0] & 0x5555555555555555) ^ RED_SQ[a >> 62] ^ d ^ (d << 3);
}
constexpr u32 MH_B[16]= {42619, 34034, 37264, 59687, 13661, 58726, 9805, 26873, 8763, 63546, 2437, 49325, 17957, 37424, 41924, 9918};
constexpr Arr<u32, 16> MHI= inverse<u32, 16>(MH_B);
struct Ord16 {
 u16 t[65536];
 u32 x, y;
};
template <int D> struct Log {
 static constexpr Ord16 ord16(Ord16 r, u32 k, u32 e) {
  u32 f[2][256]{}, b[2][256]{};
  for(int i= 0; i < 16; ++i)
   for(u32 h= 1 << (i & 7), j= h; j--;) f[i >> 3][h | j]= f[i >> 3][j] ^ MH_B[i], b[i >> 3][h | j]= b[i >> 3][j] ^ MHI.t[i];
  u32 x= r.x, y= r.y;
  for(u32 l= k; l < e; ++l) x= f[0][x & 255] ^ f[1][x >> 8], y= b[0][y & 255] ^ b[1][y >> 8], r.t[x]= r.t[y]= (l % 3 ? 3 : 1) * (l % 5 ? 5 : 1) * (l % 17 ? 17 : 1) * (l % 257 ? 257 : 1);
  r.x= x, r.y= y;
  return r;
 }
 static constexpr Ord16 ORD16_0= ord16({{0, 1}, 1, 1}, 1, 16385), ORD16= ord16(ORD16_0, 16385, 32768);
 static constexpr LinMap F9= []() { return F8 * F1; }(), F57= F48 * F9, F21= []() { return F16 * F4 * F1; }(), F60= []() { return F48 * F8 * F4; }(), F50= []() { return F48 * F2; }();
 static inline u64 ord(u64 a) {
  assert(a);
  const u64 n= mul(a, F32(a)), fn= F16(n), a2= sq(a), x3= mul(a, a2), y= mul(x3, F57(a)), c= mul(y, a2);
  const u64 d= mul(mul(mul(y, F21(a)), F60(x3)), F50(y));
  return u64(ORD16.t[u16(mul(n, fn))]) * (fn != n ? 65537 : 1) * (F32(d) != d ? 641 : 1) * (F32(c) != c ? 6700417 : 1);
 }
};
}
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::Log<0>::ord(as[i]);
 return ans;
}
