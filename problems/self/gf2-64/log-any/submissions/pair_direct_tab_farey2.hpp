#pragma once
// pair_direct_tab_farey.hpp の、素数ごとに割って CRT で組む部分を詰めた版。ほかは pair_direct_tab_farey と同じ。
// 1. 65537 の逆元の表と、6700417 の FareyInv の小さい a の逆元の表に、CRT の定数 (16384 と 3883315) を掛けて入れておく。
//    逆元の表は inv(i) = -(p / i)·inv(p mod i) で埋めていて、この式は逆元について線形なので、最初の値を定数にするだけで済む。
// 2. 65535 の成分を、3、5、17、257 の 4 つではなく 255 (= 3·5·17) と 257 の 2 つに分けて割る。剰余を取る回数と表引きが半分になる。
// 3. 各成分は p で割った余りに落とさず、CRT の係数を 128 bit で掛けて、最後の fold でまとめて 2^64-1 で割る。
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
constexpr LinMap F2= F1 * F1, F4= F2 * F2, F8= F4 * F4, F16= F8 * F8, F32= F16 * F16, F48= F32 * F16;
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
constexpr u64 EMB_B[]= {0x0000000000000001, 0x5fbfaec6aeac0002, 0xb06c601895640004, 0xb013b5277b7c0008, 0xb5ebb915248a0010, 0x109bb25b2c600020, 0xbf3bd95bd4190040, 0x0fc66342279b0080, 0xb6418f5e57c50100, 0xaa194bd4b83f0200, 0x1b5217b4dcc70400, 0xbb06fa73867a0800, 0x006fd55b23331000, 0x4ae8fb39198c2000, 0xfbd141b29b4f4000, 0x1d9ce1776be78000};
constexpr Lin<u64, 2> EMB= Lin<u64, 2>(EMB_B);
constexpr u32 MH_B[16]= {42619, 34034, 37264, 59687, 13661, 58726, 9805, 26873, 8763, 63546, 2437, 49325, 17957, 37424, 41924, 9918};
constexpr Arr<u32, 16> MHI= inverse<u32, 16>(MH_B);
struct Ln641 {
 u16 t[1 << 14];
 u16 operator()(u64 a) const { return t[u16((a * 0xffef5fb99f1bf6e7) >> 50)]; }
};
constexpr u16 PHI_B[16]= {49349, 60640, 60091, 52204, 8753, 26688, 50952, 24030, 14026, 41051, 57150, 31936, 39252, 22252, 63476, 55223};
constexpr u32 MC_B[32]= {0xca137f44, 0x02f9ac22, 0x24119ddf, 0x677fa964, 0x1c3c90b8, 0x61acd330, 0x087e6d0e, 0x98f43405, 0x17ef3800, 0x46a70e74, 0xfdd52d61, 0x9767f2ed, 0xa06bb110, 0xf0ef2346, 0x88d7f773, 0x3bdf87f2, 0xb557b556, 0xaedbaed9, 0xb9ceb9ca, 0xce1bce13, 0x8c848c94, 0xb29cb2bc, 0x65706530, 0xacf1ac71, 0x2fef2eef, 0x48d34ad3, 0xd0b4d4b4, 0x658a6d8a, 0x117b017b, 0xd3a9f3a9, 0x7fa43fa4, 0xbc2d3c2d};
struct ClassTable65537 {
 u16 t[65535];
 u32 K0, s;
};
constexpr u32 OR_L= 6700417, OR_N= 58900, OR_S= 4337141, OR_I= 13, OR_BB= 14, OR_SLOTS= 8u << OR_BB, OR_PAD= 64;
constexpr u32 OR_C1= 0x40c1ee2e, OR_MG2_B[32]= {0x800e5536, 0xf206dc7f, 0xebe52465, 0x3db3b546, 0x7b06720d, 0xc4276035, 0x6532ca3c, 0x9bbfa71f, 0x3641a727, 0xf919cc6c, 0x5bcde957, 0xdc1e972a, 0xc64b8dfb, 0x11dc26a8, 0x37e7ab06, 0x597ccd7c, 0x0c0191c3, 0x16083352, 0x01f518c9, 0x2b2f7dd3, 0x982e7286, 0xc34076ee, 0x206f6920, 0x98d2c1d7, 0x9b52d4b3, 0x93cc096c, 0x98b2f2bf, 0x03b9fa4e, 0xccaf01d0, 0x5d3fe80e, 0xb91c8774, 0xfdefb3a2};
constexpr u32 OR_MT1_B[32]= {0x72e12e3b, 0xe16f64aa, 0x5ec400dd, 0x5baaa4c1, 0xd8a77c7b, 0x475e9b5e, 0xc2705cee, 0xdc1b78ce, 0xf58842a1, 0xfa371ae4, 0x05e8731b, 0x77e28090, 0x02ce177a, 0xa927bdce, 0x0696177e, 0x661b6e7f, 0xcad3c1f4, 0xcf993c9d, 0x914d1c64, 0xbc31e925, 0x78f615ca, 0x4014dc84, 0x1b3f5a7f, 0x70b2d2b6, 0xed3323f5, 0x897807b1, 0x3cb6dcf0, 0xf6526d26, 0x4000fe3a, 0x1f4b257b, 0x8ac1c7d9, 0x65bfe9a2};
constexpr u32 OR_LAM_B[64]= {0x00000000, 0x9793a0ad, 0x2f27415b, 0x2d35097e, 0x5e4e82b6, 0x43132bcc, 0x5a6a12fc, 0xd864f1ed, 0xbc9d056c, 0x861eeb46, 0x86265798, 0xbea091e6, 0xb4d425f8, 0x837f73c7, 0xb0c9e3db, 0x262565ac, 0x793a0ad9, 0xde69177e, 0x0c3dd68d, 0x9a2d0e52, 0x0c4caf31, 0xf3c09abe, 0x7d4123cd, 0x59261485, 0x69a84bf1, 0xc9c9bee2, 0x06fee78f, 0x34f6bd91, 0x6193c7b7, 0x18006846, 0x4c4acb58, 0xef7fe856, 0xf27415b2, 0x8db5b88a, 0xbcd22efd, 0xc11b2fe7, 0x187bad1a, 0x05982b31, 0x345a1ca5, 0x225c23c2, 0x18995e62, 0x0c48b0d6, 0xe781357d, 0xf5a83712, 0xfa82479a, 0x33217c80, 0xb24c290a, 0xc9295abc, 0xd35097e2, 0x26059b47, 0x93937dc5, 0x7ee7dd66, 0x0dfdcf1e, 0xc9a22bf2, 0x69ed7b22, 0x6779ca9a, 0xc3278f6e, 0xa69df839, 0x3000d08c, 0xcc04cc86, 0x989596b0, 0xe1a94462, 0xdeffd0ad, 0x202b58ec};
constexpr u64 OR_H1= 0x9944cc327ae8fc24;
#ifdef __x86_64__
inline u64 orbit_canon_avx2(u32 w) {
 const __m256i W= _mm256_set1_epi32(int(w));
 const __m256i r0= _mm256_or_si256(_mm256_sllv_epi32(W, _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)), _mm256_srlv_epi32(W, _mm256_setr_epi32(32, 31, 30, 29, 28, 27, 26, 25)));
 const __m256i r1= _mm256_or_si256(_mm256_slli_epi32(r0, 8), _mm256_srli_epi32(r0, 24));
 const __m256i r2= _mm256_or_si256(_mm256_slli_epi32(r0, 16), _mm256_srli_epi32(r0, 16));
 const __m256i r3= _mm256_or_si256(_mm256_slli_epi32(r0, 24), _mm256_srli_epi32(r0, 8));
 __m256i m= _mm256_min_epu32(_mm256_min_epu32(r0, r1), _mm256_min_epu32(r2, r3));
 m= _mm256_min_epu32(m, _mm256_permute2x128_si256(m, m, 1));
 m= _mm256_min_epu32(m, _mm256_shuffle_epi32(m, 0x4e));
 m= _mm256_min_epu32(m, _mm256_shuffle_epi32(m, 0xb1));
 const u32 f= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(r0, m)))) | u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(r1, m)))) << 8 | u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(r2, m)))) << 16 | u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(r3, m)))) << 24;
 return u64(__builtin_ctz(f)) << 32 | u32(_mm256_cvtsi256_si32(m));
}
#endif
constexpr u64 orbit_canon(u32 w) {
#ifdef __x86_64__
 if(!std::is_constant_evaluated()) return orbit_canon_avx2(w);
#endif
 const u32 nw= ~w;
 u32 z= nw;
 for(u32 t= 1, nz; t < 32 && (nz= z & (nw << t | nw >> (32 - t))); ++t) z= nz;
 u32 j= (31 - __builtin_ctz(z)) & 31, best= j ? w << j | w >> (32 - j) : w, k= j;
 for(z&= z - 1; z; z&= z - 1) {
  j= (31 - __builtin_ctz(z)) & 31;
  if(const u32 v= j ? w << j | w >> (32 - j) : w; v < best) best= v, k= j;
 }
 return u64(k) << 32 | best;
}
struct alignas(64) OrbitTab {
 u64 t[OR_SLOTS + OR_PAD];
 u8 cnt[(OR_SLOTS + OR_PAD) / 8];
 u32 p, c, n;
};
struct OrbitPow {
 u64 lo[256], mid[256], hi[(OR_L >> 16) + 1], hp[OR_I];
};
inline u32 orbit_home(u32 key) { return ((key * 0x9e3779b1u) >> (32 - OR_BB)) * 8; }
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
 static constexpr Ln641 LN641= []() {
  Ln641 h{};
  for(u64 k= 0, cur= 1; k < 641; ++k, cur= cmul(cur, 0x6bf808f7824282a2)) h.t[u16((cur * 0xffef5fb99f1bf6e7) >> 50)]= k * 590 * 128 % 641;
  return h;
 }();
 static constexpr Lin<u16, 2> PHI= []() { return Lin<u16, 2>(PHI_B); }();
 static constexpr ClassTable65537 cls65537(ClassTable65537 r, u32 k, u32 e) {
  u32 m[4][256]{};
  for(int i= 0; i < 32; ++i)
   for(u32 h= 1 << (i & 7), j= h; j--;) m[i >> 3][h | j]= m[i >> 3][j] ^ MC_B[i];
  u32 s= r.s, b0, b1, l1;
  for(; k < e; ++k) {
   s= m[0][s & 255] ^ m[1][s >> 8 & 255] ^ m[2][s >> 16 & 255] ^ m[3][s >> 24], b0= s & 65535, b1= s >> 16, l1= 65535 - (IL16.t[b1] & 65535);
   if(b0) r.t[((IL16.t[b0] & 65535) + l1) % 65535]= u16(k - 1);
   else r.K0= k;
   if(b0 ^ b1) r.t[((IL16.t[b0 ^ b1] & 65535) + l1) % 65535]= u16(65536 - k);
   else r.K0= 65537 - k;
  }
  r.s= s;
  return r;
 }
 static constexpr ClassTable65537 CLS65537_0= cls65537({{}, 0, 1}, 1, 16385), CLS65537= cls65537(CLS65537_0, 16385, 32769);
 static inline u32 log_65537(u64 n, u64 fn) {
  const u16 b1= n ^ fn;
  if(!b1) return 0;
  const u16 b0= n ^ PHI(b1);
  if(!b0) return CLS65537.K0;
  u32 idx= (IL16.t[b0] & 65535) + 65535 - (IL16.t[b1] & 65535);
  if(idx >= 65535) idx-= 65535;
  return CLS65537.t[idx] + 1u;
 }
 static constexpr Lin<u32, 4> OR_M1= []() { return Lin<u32, 4>(OR_MT1_B); }();
 static constexpr Lin<u32, 8> OR_LAM= []() { return Lin<u32, 8>(OR_LAM_B); }();
 static constexpr OrbitTab orbit_part(const OrbitTab& r0, u32 e) {
  OrbitTab r= r0;
  u32 m[4][256]{};
  for(int i= 0; i < 32; ++i)
   for(u32 h= 1 << (i & 7), j= h; j--;) m[i >> 3][h | j]= m[i >> 3][j] ^ OR_MG2_B[i];
  u32 p= r.p, c= r.c, n= r.n;
  for(; n <= e; ++n) {
   const u64 ck= orbit_canon(c);
   const u32 best= u32(ck), k= u32(ck >> 32);
   u32 g= (best * 0x9e3779b1u) >> (32 - OR_BB);
   while(r.cnt[g] == 8) ++g;
   r.t[g * 8 + r.cnt[g]++]= u64(best) << 32 | u32((u64(2 * n - 1) << k) % OR_L);
   const u32 x= m[0][c & 255] ^ m[1][c >> 8 & 255] ^ m[2][c >> 16 & 255] ^ m[3][c >> 24] ^ p;
   p= c, c= x;
  }
  r.p= p, r.c= c, r.n= n;
  return r;
 }
 static constexpr OrbitTab OR_TAB_0= orbit_part(OrbitTab{{}, {}, OR_C1, OR_C1, 1}, 5890), OR_TAB_1= orbit_part(OR_TAB_0, 11780), OR_TAB_2= orbit_part(OR_TAB_1, 17670), OR_TAB_3= orbit_part(OR_TAB_2, 23560), OR_TAB_4= orbit_part(OR_TAB_3, 29450), OR_TAB_5= orbit_part(OR_TAB_4, 35340), OR_TAB_6= orbit_part(OR_TAB_5, 41230), OR_TAB_7= orbit_part(OR_TAB_6, 47120), OR_TAB_8= orbit_part(OR_TAB_7, 53010), OR_TAB= orbit_part(OR_TAB_8, OR_N);
 static constexpr OrbitPow OR_POW= []() {
  OrbitPow r{};
  r.lo[0]= r.mid[0]= r.hi[0]= r.hp[0]= 1;
  for(int i= 1; i < 256; ++i) r.lo[i]= cmul(r.lo[i - 1], 0x00f542601703f991), r.mid[i]= cmul(r.mid[i - 1], 0x5f915310c81c09e9);
  for(u32 i= 1; i <= (OR_L >> 16); ++i) r.hi[i]= cmul(r.hi[i - 1], 0x40cee54547d2d10a);
  for(u32 i= 1; i < OR_I; ++i) r.hp[i]= cmul(r.hp[i - 1], OR_H1);
  return r;
 }();
 static constexpr Lin<u32, 8> OR_LAMH= []() {
  u32 b[64]{};
  for(int i= 0; i < 64; ++i) b[i]= OR_LAM(cmul(u64(1) << i, OR_H1));
  return Lin<u32, 8>(b);
 }();
 static constexpr Arr<u32, 32> OR_P2INV= []() {
  Arr<u32, 32> r{{1}};
  for(int k= 1; k < 32; ++k) r.t[k]= u64(r.t[k - 1]) * (OR_L / 2 + 1) % OR_L;
  return r;
 }();
 static inline u32 orbit_lookup(u32 key) {
  const __m256i K= _mm256_set1_epi64x((long long)(u64(key) << 32));
  for(u32 h= orbit_home(key);; h+= 8) {
   const __m256i* q= (const __m256i*)&OR_TAB.t[h];
   const u32 f= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_loadu_si256(q), K)))) | u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_loadu_si256(q + 1), K)))) << 8;
   if(f & 0xaaaa) return u32(OR_TAB.t[h + (__builtin_ctz(f & 0xaaaa) >> 1)]);
   if(f & 0x5555) return 0;
  }
 }
 static inline u32 orbit_find(u64 y, u32 c, u64 ck, u32 i) {
  constexpr u32 L= OR_L;
  if(!c) return u32(u64(OR_S) * i % L);
  const u32 e= orbit_lookup(u32(ck));
  if(!e) return L;
  const u32 e1= u32(u64(e) * OR_P2INV.t[ck >> 32] % L);
  const u64 z= i ? mul(y, OR_POW.hp[i]) : y, g= mul(mul(OR_POW.lo[e1 & 255], OR_POW.mid[e1 >> 8 & 255]), OR_POW.hi[e1 >> 16]);
  return u32((u64(OR_S) * i + (z == g ? e1 : L - e1)) % L);
 }
 static inline void orbit_prefetch(u64 ck) { _mm_prefetch((const char*)&OR_TAB.t[orbit_home(u32(ck))], _MM_HINT_T0); }
 static inline u32 log_6700417(u64 y) {
  u32 c0= OR_LAM(y), c1= OR_LAMH(y);
  for(u32 i= 0; i < OR_I; i+= 2) {
   const u64 k0= c0 ? orbit_canon(c0) : 0, k1= c1 ? orbit_canon(c1) : 0;
   orbit_prefetch(k0), orbit_prefetch(k1);
   if(const u32 e= orbit_find(y, c0, k0, i); e != OR_L) return e;
   if(i + 1 < OR_I)
    if(const u32 e= orbit_find(y, c1, k1, i + 1); e != OR_L) return e;
   const u32 c2= OR_M1(c1) ^ c0;
   c1= OR_M1(c2) ^ c1, c0= c2;
  }
  return OR_L;
 }
 template <bool V> static inline u64 ln(u64 x) {
  assert(x);
  const u64 x32= F32(x), n= mul(x, x32), fn= F16(n);
  auto [x_f16, w]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(x32), 0, n), _mm256_set1_epi64x(fn)));
  const u16 xf= u16(x_f16);
  const u32 il= IL16.t[xf];
  const u64 s= mul(EMB(u16(il >> 16)), w), w1= mul(s, F9(s)), e57= F57(w1);
  auto [w2, y]= unpack(mul2<V>(_mm256_set_epi64x(0, e57, 0, sq(w1)), _mm256_set_epi64x(0, s, 0, w1)));
  u32 r3= log_6700417(y);
  const u64 w3= mul(s, sq(w2)), w4= mul(w3, F4(w3));
  u64 r0= LN641(mul(e57, w4)), r2= log_65537(n, fn);
  const __uint128_t acc= 0x663d80ff99c27f * r0 + __uint128_t(0x2f205935d0dfa6ca) * r3 + 0x1000100010001ull * (il & 65535) + 0xffff0000ffff * r2;
  const u64 lo= u64(acc), t= lo + u64(acc >> 64);
  return t + (t < lo);
 }
};
}
namespace gf2p64_internal {
using L= Log<0>;
// 逆元の表に C を掛けたもの。0 の欄は 0 にしておくので、a の成分が 1 (log が 0) なら k の成分も 0 になる。
template <u32 P, u32 C= 1> constexpr Arr<u16, P> inv_table() {
 Arr<u16, P> r{};
 r.t[1]= 1;
 for(u32 i= 2; i < P; ++i) r.t[i]= u16((P - u64(P / i) * r.t[P % i] % P) % P);
 for(u32 i= 1; i < P; ++i) r.t[i]= u16(u64(r.t[i]) * C % P);
 return r;
}
// 65535 の成分を 255 (= 3·5·17) と 257 に分けて割る表。値には CRT の定数を掛けておく (8224 と 8160 は、255 と 257 の CRT の
// 定数 32896 と 32640 に 16384 を掛けたもの、mod 65535)。255 は擬似逆元 x^15 (λ(255) = 16 なので、x を割る素数の成分は 0、
// ほかは逆元) で、上の 3 bit に x を割る素数 (3、5、17) の印を持つ。
constexpr Arr<u32, 255> T255= []() {
 Arr<u32, 255> r{};
 for(u32 x= 0; x < 255; ++x) {
  u32 y= x;
  for(int i= 1; i < 15; ++i) y= y * x % 255;
  r.t[x]= 8224 * y % 65535 | u32(x % 3 == 0) << 16 | u32(x % 5 == 0) << 17 | u32(x % 17 == 0) << 18;
 }
 return r;
}();
constexpr Arr<u16, 257> T257= []() {
 const auto inv= inv_table<257>();
 Arr<u16, 257> r{};
 for(u32 x= 1; x < 257; ++x) r.t[x]= u16(8160 * inv.t[x] % 65535);
 return r;
}();
constexpr Arr<u16, 641> INV641= inv_table<641, 590>();  // 590 = ((2^64-1)/641)^-1 mod 641
struct Inv65537 {
 u16 t[65537];
};
// 16384·x^-1 mod 65537 から 1 を引いた表 (x ≥ 1。16384 = ((2^64-1)/65537)^-1 mod 65537 は CRT の定数で、値は 1〜65536 なので
// 1 を引いて u16 に収める)。inv(i) = -(p / i)·inv(p mod i) は逆元について線形なので、最初の値を 16384 にして同じ式で前から
// 順に埋める。constexpr の上限に収めるため、半分ずつ 2 回に分けて作る。
constexpr Inv65537 inv65537_part(const Inv65537& r0, u32 lo, u32 hi) {
 Inv65537 r= r0;
 for(u32 i= lo; i < hi; ++i) r.t[i]= u16((65537 - u64(65537 / i) * (r.t[65537 % i] + 1) % 65537) % 65537 - 1);
 return r;
}
constexpr Inv65537 INV65537_0= inv65537_part(Inv65537{{0, 16383}}, 2, 32769), INV65537= inv65537_part(INV65537_0, 32769, 65537);
// x^-1 mod P を、表引き 2 回と掛け算で求める (maspy さんの「O(1) mod inv」の方法。https://maspypy.com/o1-mod-inv-mod-pow)。分母が N 以下の分数 (Farey 数列) で
// x/P を近似すると x ≡ a / b (b ≤ N、|a| < P/N) と書けるので、x^-1 = b·a^-1 になる。[0, 1] を N² 個の区間に分け、区間
// ごとに上の端以下で最大の分数と下の端以上で最小の分数を持ち、|a| が小さいほうを使う。x = 0 なら 0 を返す。C を与えると、小さい a の逆元の表に
// C を掛けて入れておき、C·x^-1 を返す (表を埋める式は逆元について線形なので、最初の値を C にするだけで済む)。
template <u32 P, u32 N, u32 C= 1> struct FareyInv {
 static constexpr u32 B= N * N, A= P / N;
 static constexpr Arr<u32, B> FR= []() {
  Arr<u32, B> r{};
  for(u32 a= 0, b= 1, c= 1, d= N;;) {
   // 隣り合う a/b < c/d のあいだから始まる区間 i (a/b < i/B ≤ c/d) は、下の端以上で最小の分数が c/d で、上の端以下で
   // 最大の分数は、c/d ≤ (i+1)/B なら c/d、そうでなければ a/b
   for(u32 i= a * B / b + 1, e= c == d ? B - 1 : c * B / d; i <= e; ++i) r.t[i]= ((i + 1) * d >= c * B ? c | d << 8 : a | b << 8) | (c | d << 8) << 16;
   if(c == 1 && d == 1) break;
   const u32 k= (N + b) / d, e= k * c - a, f= k * d - b;
   a= c, b= d, c= e, d= f;
  }
  r.t[0]= 1 << 8 | 1 << 24;
  return r;
 }();
 static constexpr Arr<u32, A + 1> INV= []() {
  Arr<u32, A + 1> r{};
  r.t[1]= C;
  for(u32 i= 2; i <= A; ++i) r.t[i]= u32(P - u64(P / i) * r.t[P % i] % P);
  return r;
 }();
 static inline u32 inv(u32 x) {
  const u32 f= FR.t[u64(x) * B / P], c1= f & 255, b1= f >> 8 & 255, c2= f >> 16 & 255, b2= f >> 24;
  const long long a1= (long long)b1 * x - (long long)c1 * P, a2= (long long)b2 * x - (long long)c2 * P;
  const long long m1= a1 < 0 ? -a1 : a1, m2= a2 < 0 ? -a2 : a2;
  const bool s= m1 <= m2;
  const u32 r= u32(u64(s ? b1 : b2) * INV.t[s ? m1 : m2] % P);
  return (s ? a1 : a2) < 0 && r ? P - r : r;
 }
};
inline u64 fold(__uint128_t x) {
 const u64 lo= u64(x), t= lo + u64(x >> 64);
 return t + (t < lo);
}
// a^k = b となる k を 1 つ返す。無ければ 2^64-1。素数 p ごとの k の成分は、CRT の定数を掛けるだけで
// 済むよう ((2^64-1)/p)^-1 倍 (mod p) の形で持つ。43690 などは 65535 の約数ごとの CRT の定数に
// 16384 = ((2^64-1)/65535)^-1 mod 65535 を掛けたもの。16384 は 65537 での、3883315 は 6700417 での同じ値。
template <bool V> inline u64 query(u64 a, u64 b) {
 constexpr u64 M= ~0ull;
 if(b == 1) return 0;
 if(a == b) return 1;
 if(a == 1) return M;
 const u64 a32= F32(a), b32= F32(b);
 const auto [na, nb]= unpack(mul2<V>(_mm256_set_epi64x(0, b, 0, a), _mm256_set_epi64x(0, b32, 0, a32)));
 const u64 fna= F16(na), fnb= F16(nb);
 const __m256i fn2= _mm256_set_epi64x(0, fnb, 0, fna);
 const auto [ma, mb]= unpack(mul2<V>(_mm256_set_epi64x(0, nb, 0, na), fn2));
 const u32 ila= L::IL16.t[u16(ma)], ilb= L::IL16.t[u16(mb)], la= ila & 65535, lb= ilb & 65535;
 const u32 a255= la % 255, b255= lb % 255, a257= la % 257, b257= lb % 257;
 const auto [wa, wb]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(b32), 0, sq(a32)), fn2));
 const __m256i s2= mul2<V>(_mm256_set_epi64x(0, EMB(u16(ilb >> 16)), 0, EMB(u16(ila >> 16))), _mm256_set_epi64x(0, wb, 0, wa));
 const auto [sa, sb]= unpack(s2);
 const __m256i w12= mul2<V>(s2, _mm256_set_epi64x(0, L::F9(sb), 0, L::F9(sa)));
 const auto [w1a, w1b]= unpack(w12);
 const __m256i e2= _mm256_set_epi64x(0, L::F57(w1b), 0, L::F57(w1a));
 const auto [ya, yb]= unpack(mul2<V>(e2, s2));
 const auto [w2a, w2b]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(w1b), 0, sq(w1a)), w12));
 const auto [w3a, w3b]= unpack(mul2<V>(s2, _mm256_set_epi64x(0, sq(w2b), 0, sq(w2a))));
 const auto [w4a, w4b]= unpack(mul2<V>(_mm256_set_epi64x(0, w3b, 0, w3a), _mm256_set_epi64x(0, F4(w3b), 0, F4(w3a))));
 const auto [za, zb]= unpack(mul2<V>(e2, _mm256_set_epi64x(0, w4b, 0, w4a)));
 const u32 v0a= L::LN641(za), v0b= L::LN641(zb), v2a= L::log_65537(na, fna), v2b= L::log_65537(nb, fnb);
 const u32 ra= L::log_6700417(ya), rb= L::log_6700417(yb), t255= T255.t[a255];
 if((t255 >> 16 & ~(T255.t[b255] >> 16)) || (!a257 && b257) || (!v0a && v0b) || (!v2a && v2b) || (!ra && rb)) return M;
 // 各成分は p で割った余りに落とさずに CRT の係数を 128 bit で掛け、最後の fold でまとめて 2^64-1 で割る。((2^64-1)/p)·p ≡ 0
 // なので値は変わらない。そのかわり 0 が 2^64-1 として出ることがあるので、最後に 0 に直す。
 const u64 v1= b255 * (t255 & 65535) + b257 * T257.t[a257], v0= v0b * INV641.t[v0a], v2= v2a ? u64(v2b) * (INV65537.t[v2a] + 1u) : 0;
 const u64 v3= u64(rb) * FareyInv<6700417, 128, 3883315>::inv(ra);
 const u64 k= fold(__uint128_t(0x1000100010001ull) * v1 + __uint128_t(0x663d80ff99c27full) * v0 + __uint128_t(0xffff0000ffffull) * v2 + __uint128_t(0x280fffffd7full) * v3);
 return k == M ? 0 : k;
}
}
#include <vector>
using namespace std;
using u64= unsigned long long;
template <bool V> inline void solve_all(const vector<u64>& as, const vector<u64>& bs, vector<u64>& ans) {
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::query<V>(as[i], bs[i]);
}
// Library の GF2p64::log と同じく、x86 では VPCLMULQDQ があれば mul2 の 2 本を 1 命令で掛け、arm では
// 64 bit の clmul を 2 回使う形 (mul2<0>) にする。
inline vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
 vector<u64> ans(as.size());
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return solve_all<1>(as, bs, ans), ans;
#endif
 return solve_all<0>(as, bs, ans), ans;
}
