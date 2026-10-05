#pragma once
// 素数 p ごとに a^((2^64-1)/p) を冪で求め、1 でない p を掛ける (冪 7 回)。冪は Library の GF2p64::pow と同じ pw で、x86 では
// VPCLMULQDQ があれば mul2 を 1 命令で掛ける。Library (6f40504df) の GF2p64.hpp から、LinMap、mul、sq、mul2、pw を写したもの。
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
template <bool VPCLMUL= 1> inline u64 pw(u64 a, u64 e) {
 u64 t[16]= {1, a, sq(a)};
 __m256i t12= _mm256_set_epi64x(0, t[2], 0, a);
 __m256i t34= mul2<VPCLMUL>(t12, _mm256_set1_epi64x(t[2]));
 std::tie(t[3], t[4])= unpack(t34);
 __m256i t4= _mm256_set1_epi64x(t[4]);
 __m256i t56= mul2<VPCLMUL>(t4, t12);
 std::tie(t[5], t[6])= unpack(t56);
 std::tie(t[7], t[8])= unpack(mul2<VPCLMUL>(t4, t34));
 __m256i t8= _mm256_set1_epi64x(t[8]);
 std::tie(t[9], t[10])= unpack(mul2<VPCLMUL>(t8, t12));
 std::tie(t[11], t[12])= unpack(mul2<VPCLMUL>(t8, t34));
 std::tie(t[13], t[14])= unpack(mul2<VPCLMUL>(t8, t56));
 t[15]= mul(t[7], t[8]);
 auto [b6, b7]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 60) & 0xf]), 0, F32(t[(e >> 56) & 0xf])), _mm256_set_epi64x(0, t[(e >> 28) & 0xf], 0, t[(e >> 24) & 0xf])));
 auto [b4, b5]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 52) & 0xf]), 0, F32(t[(e >> 48) & 0xf])), _mm256_set_epi64x(0, t[(e >> 20) & 0xf], 0, t[(e >> 16) & 0xf])));
 __m256i b23= mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 44) & 0xf]), 0, F32(t[(e >> 40) & 0xf])), _mm256_set_epi64x(0, t[(e >> 12) & 0xf], 0, t[(e >> 8) & 0xf]));
 __m256i b01= mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 36) & 0xf]), 0, F32(t[(e >> 32) & 0xf])), _mm256_set_epi64x(0, t[(e >> 4) & 0xf], 0, t[e & 0xf]));
 auto [b2, b3]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F16(b7), 0, F16(b6)), b23));
 auto [b0, b1]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F8(b3), 0, F8(b2)), mul2<VPCLMUL>(_mm256_set_epi64x(0, F16(b5), 0, F16(b4)), b01)));
 return mul(F4(b1), b0);
}
}
#include <vector>
using namespace std;
using u64= unsigned long long;
namespace gf2_64_ord_pow7 {
using namespace gf2p64_internal;
template <bool V> inline void solve_all(const vector<u64>& as, vector<u64>& ans) {
 constexpr u64 M= ~0ull, PR[7]= {3, 5, 17, 257, 641, 65537, 6700417};
 for(size_t i= 0; i < as.size(); ++i) {
  u64 o= 1;
  for(u64 p: PR)
   if(pw<V>(as[i], M / p) != 1) o*= p;
  ans[i]= o;
 }
}
}  // namespace gf2_64_ord_pow7
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return gf2_64_ord_pow7::solve_all<1>(as, ans), ans;
#endif
 return gf2_64_ord_pow7::solve_all<0>(as, ans), ans;
}
