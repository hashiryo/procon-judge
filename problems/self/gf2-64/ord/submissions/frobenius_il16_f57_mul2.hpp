#pragma once
// frobenius_il16_f57.hpp の 8 回の掛け算のうち、材料が同じころに揃う 2 組を mul2 (__m256i の 2 lane の掛け算) にまとめた版。log (Log<D>::ln) と同じく 1 つの元の中
// だけでまとめ、元をまたいではまとめない。(a·F9(a), a·F32(a)) と (n·F16(n), F57(a^513)·a) の 2 組を mul2 で掛ける。残りの 4 回 (a^1539、a^3079、a^52343、
// a^(6700417·2^57)) は互いを待つ 1 本の鎖なので、スカラのまま掛ける。mul2 は Library と同じく、x86 では VPCLMULQDQ があれば 1 命令で、無ければ (arm も)
// 64 bit の clmul を 2 回使う。ほかは frobenius_il16_f57 と同じ。
// Library (6f8462e9c) の GF2p64.hpp から、LinMap、mul、sq、mul2、unpack と、IL16 と ord() を持つ Log<D> とその材料を写したもの。
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
template <bool V> inline __m256i mul2(const __m256i& a_vec, const __m256i& b_vec) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 __m256i prod;
 if constexpr(V) prod= _mm256_clmulepi64_epi128(a_vec, b_vec, 0);
 else prod= _mm256_setr_m128i(_mm_clmulepi64_si128(_mm256_castsi256_si128(a_vec), _mm256_castsi256_si128(b_vec), 0), _mm_clmulepi64_si128(_mm256_extracti128_si256(a_vec, 1), _mm256_extracti128_si256(b_vec, 1), 0));
 __m256i h= _mm256_srli_si256(prod, 8);
 __m256i d= _mm256_xor_si256(h, _mm256_slli_epi64(h, 1));
 return _mm256_xor_si256(_mm256_xor_si256(prod, _mm256_shuffle_epi8(RED256, _mm256_srli_epi64(h, 60))), _mm256_xor_si256(d, _mm256_slli_epi64(d, 3)));
}
inline std::pair<u64, u64> unpack(const __m256i& vec) { return std::make_pair(u64(_mm256_extract_epi64(vec, 0)), u64(_mm256_extract_epi64(vec, 2))); }
constexpr u32 MH_B[16]= {42619, 34034, 37264, 59687, 13661, 58726, 9805, 26873, 8763, 63546, 2437, 49325, 17957, 37424, 41924, 9918};
constexpr Arr<u32, 16> MHI= inverse<u32, 16>(MH_B);
template <int D> struct Log {
 static constexpr Arr<u32, 65536> IL16= []() {
  u32 f[2][256]{}, b[2][256]{};
  for(int i= 0; i < 16; ++i)
   for(u32 h= 1 << (i & 7), j= h; j--;) f[i >> 3][h | j]= f[i >> 3][j] ^ MH_B[i], b[i >> 3][h | j]= b[i >> 3][j] ^ MHI.t[i];
  Arr<u32, 65536> r{};
  r.t[1]= 1 << 16;
  for(u32 l= 1, x= 1, y= 1; l < 32768; ++l) x= f[0][x & 255] ^ f[1][x >> 8], y= b[0][y & 255] ^ b[1][y >> 8], r.t[x]= y << 16 | l, r.t[y]= x << 16 | (65535 - l);
  return r;
 }();
 static constexpr LinMap F9= []() { return F8 * F1; }(), F57= F48 * F9;
 template <bool V> static inline u64 ord(u64 a) {
  assert(a);
  const auto [w1, n]= unpack(mul2<V>(_mm256_set1_epi64x(a), _mm256_set_epi64x(0, F32(a), 0, F9(a))));
  const u64 fn= F16(n), e57= F57(w1), w2= mul(w1, sq(w1)), w3= mul(a, sq(w2)), w4= mul(w3, F4(w3)), d= mul(e57, w4);
  const auto [m, c]= unpack(mul2<V>(_mm256_set_epi64x(0, a, 0, n), _mm256_set_epi64x(0, e57, 0, fn)));
  const u32 l= IL16.t[u16(m)] & 65535;
  return u64(l % 3 ? 3 : 1) * (l % 5 ? 5 : 1) * (l % 17 ? 17 : 1) * (l % 257 ? 257 : 1) * (fn != n ? 65537 : 1) * (F32(d) != d ? 641 : 1) * (F32(c) != c ? 6700417 : 1);
 }
};
}
#include <vector>
using namespace std;
using u64= unsigned long long;
template <bool V> inline void solve_all(const vector<u64>& as, vector<u64>& ans) {
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::Log<0>::ord<V>(as[i]);
}
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return solve_all<1>(as, ans), ans;
#endif
 return solve_all<0>(as, ans), ans;
}
