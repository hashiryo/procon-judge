#pragma once
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#include <simde/x86/bmi.h>
#endif
#include <bit>
#include <tuple>
#include <utility>
#include <cassert>
namespace gf2p64_internal {
using u64= unsigned long long;
using u32= unsigned;
using u16= unsigned short;
using u8= unsigned char;
struct LinMap {
 u64 g[64], t[8][256];
 constexpr LinMap(const u64 b[64]): g{}, t{} {
  for(int i= 0; i < 64; ++i) {
   g[i]= b[i];
   u64* l= t[i >> 3];
   for(u8 h= 1 << (i & 7), b= h; b--;) l[h | b]= l[b] ^ g[i];
  }
 }
 inline constexpr u64 operator()(u64 a) const { return t[0][u8(a)] ^ t[1][u8(a >> 8)] ^ t[2][u8(a >> 16)] ^ t[3][u8(a >> 24)] ^ t[4][u8(a >> 32)] ^ t[5][u8(a >> 40)] ^ t[6][u8(a >> 48)] ^ t[7][u8(a >> 56)]; }
 constexpr LinMap operator*(const LinMap& r) const {
  u64 h[64]= {};
  for(int i= 0; i < 64; ++i) h[i]= (*this)(r.g[i]);
  return LinMap(h);
 }
};
constexpr u64 TO_NIM_B[64]= {0x0000000000000001, 0x5211145c804b6109, 0x7c8bc2cad259879f, 0x565854b4c60c1e0b, 0x4068acf7104c20c3, 0x662d2bd0f2739155, 0x7a90c83701fa8323, 0x21cfa750247e8755, 0x67d1044e545abf47, 0x4d9d3b5a8568f839, 0x567a9d7331b6b3c6, 0x1ca54bfdd6d1ae59, 0x454fa483275db25c, 0x6766df6fec4e9d44, 0x35cb621cec1fe7f9, 0x4c606d3e52faf263, 0x57640dc825a57954, 0x7aca87838b7f6315, 0x6d53c884ebf2b0ed, 0x3721d998bb50164b, 0x7aa7c62fd6cd53ab, 0x47cbb2c51f7c040f, 0x132063b7f5e42489, 0x0c1b36c8b2993f8a, 0x60119ecff680497a, 0x5175da444cc11791, 0x5792ff4554765b09, 0x0c9fdb8a01334e82, 0x2be0a763a68a4725, 0x3c2dc8260ad051f6, 0x6c4c9fed8816bb9c, 0x630062753ffaf766,
                             0x7b37d31b5d519225, 0x2364f7f79705691c, 0x453eb8a83e2fec71, 0x7c0121b37e828666, 0x59190d3250e66011, 0x103207f9dda18cae, 0x28233dce01c69b76, 0x4fa519899227a5e7, 0x4567ba46ee7bc6cd, 0x0a284773d021afd5, 0x63894079bbe3a824, 0x11013c7fdfaaa5c2, 0x1aa984f18574f3b0, 0x0cbaba126fd0c4db, 0x0b8797719e6dc725, 0x4a2845680aefaa72, 0x536d2535f6934e15, 0x01db7a57effcd689, 0x7e1ed0ad01e2a5ad, 0x0aedc9b3cee826f6, 0x7ba716eccf9f68e1, 0x5d5e23bc0f3dc38f, 0x0b5f2a3b88674d83, 0x2de9bafc2f00f8d4, 0x3b56712ad419c7e0, 0x3ab4be8c30c19253, 0x2708522ffaa654b0, 0x2b8bca57bf643598, 0x588825d1a5fa8e1c, 0x86adf8bf4d45962f, 0x51b4c15d8719dd73, 0xe4a2b3b59783d0aa};
constexpr u64 FROM_NIM_B[64]= {0x0000000000000001, 0x19c9369f278adc02, 0xa181e7d66f5ff795, 0x5db84357ce785d09, 0xa0bae2f9d2430cc8, 0xb7ea5a9705b771c0, 0xba4f3cd82801769d, 0x4886cde01b8241d0, 0x0a6f43f2aaf612ed, 0xebd0142f98030a32, 0xa81f89cda43f3792, 0xe99aec6b66ccb814, 0xa69d1ff025fc2f82, 0x48a81132d25db068, 0x4a900f9dcaa9644f, 0xe5ce4ea88259972a, 0xf7094c336029f04c, 0xe191dde287bc9c6b, 0xaacaff12bff239b8, 0x49bc5212be1bc1ca, 0xfe57defb454446cf, 0xa1dffcf944bdf6a7, 0xb9f1bdb5cee941ee, 0x12e5e889275c22de, 0x5bcb6b117b77eeed, 0x03eb1ab59d05ae4b, 0x02a25d7076ddd386, 0x53164a606c612245, 0xebb33f5822f66059, 0xe9be765f5747b93e, 0x552a78df373a354f, 0xbcf5ac65f31fb8bf,
                               0xe411e728becdc77b, 0xf35c26d7b57cdca6, 0x4499da83de4ca5f7, 0x40ab25bdca4ae226, 0xee004b6f1dff7218, 0x0d122da9821c5b41, 0x51fbfcb058120efe, 0xa148b1fa84905b22, 0xbb8ed3e647604d8d, 0xe2d93fef2472776f, 0x4c17a2541a10e6b5, 0x1d879e08903708e7, 0x0fbe7d0d1934da90, 0x5bf977d9c6f61d30, 0x06832fc918260412, 0x0fe22e843ebf73e3, 0x4d7ef4e4fa28d60d, 0x402250d979afbed5, 0x067902b8c8ca2d4f, 0xf38d113fe1d6bb16, 0x414f0248b02b5b7d, 0xf041922915824ce9, 0x11a72fb5e30c93d9, 0x12e54f4d63102aee, 0xbc46ac14b3141c6c, 0x1f172b3c16c645bb, 0x584b492ed4e8fa6c, 0x00a852e9a32cc133, 0xa180861bce00a45e, 0xa194b6bcb4645fb9, 0x4509002ad808a4fb, 0xc5172a0055602f69};
constexpr LinMap TO_NIM= LinMap(TO_NIM_B);
constexpr LinMap FROM_NIM= LinMap(FROM_NIM_B);
constexpr LinMap F1= []() {
 u64 g[64]= {1};
 for(int i= 32; --i;) g[i]= u64(1) << (i * 2);
 for(int i= 62; i-- > 32;) g[i]= u64(27) << ((i - 32) * 2);
 g[62]= 0xb00000000000001b, g[63]= 0xc00000000000005a;
 return LinMap(g);
}();
constexpr LinMap F2= F1 * F1;
constexpr LinMap F3= F2 * F1;
constexpr LinMap F4= F2 * F2;
constexpr LinMap F5= F4 * F1;
constexpr LinMap F7= F4 * F3;
constexpr LinMap F8= F4 * F4;
constexpr LinMap F10= F5 * F5;
constexpr LinMap F15= F8 * F7;
constexpr LinMap F16= F8 * F8;
constexpr LinMap F32= F16 * F16;
constexpr LinMap F48= F32 * F16;
constexpr LinMap F63= F48 * F15;
template <LinMap LM0, LinMap LM1= LM0> inline __m256i linmap2(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(LM1.t[1][u8(a1 >> 8)], LM1.t[0][u8(a1)], LM0.t[1][u8(a0 >> 8)], LM0.t[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(LM1.t[3][u8(a1 >> 24)], LM1.t[2][u8(a1 >> 16)], LM0.t[3][u8(a0 >> 24)], LM0.t[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(LM1.t[5][u8(a1 >> 40)], LM1.t[4][u8(a1 >> 32)], LM0.t[5][u8(a0 >> 40)], LM0.t[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(LM1.t[7][u8(a1 >> 56)], LM1.t[6][u8(a1 >> 48)], LM0.t[7][u8(a0 >> 56)], LM0.t[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
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
template <bool VPCLMUL= 1, int IMM= 0> inline __m256i mul2(const __m256i& a_vec, const __m256i& b_vec) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 __m256i prod;
 if constexpr(VPCLMUL) prod= _mm256_clmulepi64_epi128(a_vec, b_vec, IMM);
 else prod= _mm256_setr_m128i(_mm_clmulepi64_si128(_mm256_castsi256_si128(a_vec), _mm256_castsi256_si128(b_vec), IMM), _mm_clmulepi64_si128(_mm256_extracti128_si256(a_vec, 1), _mm256_extracti128_si256(b_vec, 1), IMM));
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
 auto [b6, b7]= unpack(mul2<VPCLMUL>(linmap2<F32>(t[(e >> 56) & 0xf], t[(e >> 60) & 0xf]), _mm256_set_epi64x(0, t[(e >> 28) & 0xf], 0, t[(e >> 24) & 0xf])));
 auto [b4, b5]= unpack(mul2<VPCLMUL>(linmap2<F32>(t[(e >> 48) & 0xf], t[(e >> 52) & 0xf]), _mm256_set_epi64x(0, t[(e >> 20) & 0xf], 0, t[(e >> 16) & 0xf])));
 __m256i b23= mul2<VPCLMUL>(linmap2<F32>(t[(e >> 40) & 0xf], t[(e >> 44) & 0xf]), _mm256_set_epi64x(0, t[(e >> 12) & 0xf], 0, t[(e >> 8) & 0xf]));
 __m256i b01= mul2<VPCLMUL>(linmap2<F32>(t[(e >> 32) & 0xf], t[(e >> 36) & 0xf]), _mm256_set_epi64x(0, t[(e >> 4) & 0xf], 0, t[e & 0xf]));
 auto [b2, b3]= unpack(mul2<VPCLMUL>(linmap2<F16>(b6, b7), b23));
 auto [b0, b1]= unpack(mul2<VPCLMUL>(linmap2<F8>(b2, b3), mul2<VPCLMUL>(linmap2<F16>(b4, b5), b01)));
 return mul(F4(b1), b0);
}
template <class U> struct HalfMap {
 U t[2][256];
 constexpr HalfMap(const U b[16]): t{} {
  for(int i= 0; i < 16; ++i) {
   U* l= t[i >> 3];
   for(u8 h= 1 << (i & 7), j= h; j--;) l[h | j]= l[j] ^ b[i];
  }
 }
 inline constexpr U operator()(const u16 x) const { return t[0][u8(x)] ^ t[1][x >> 8]; }
};
constexpr u64 EMB_B[]= {0x0000000000000001, 0x5fbfaec6aeac0002, 0xb06c601895640004, 0xb013b5277b7c0008, 0xb5ebb915248a0010, 0x109bb25b2c600020, 0xbf3bd95bd4190040, 0x0fc66342279b0080, 0xb6418f5e57c50100, 0xaa194bd4b83f0200, 0x1b5217b4dcc70400, 0xbb06fa73867a0800, 0x006fd55b23331000, 0x4ae8fb39198c2000, 0xfbd141b29b4f4000, 0x1d9ce1776be78000};
constexpr HalfMap<u64> EMB= HalfMap<u64>(EMB_B);
struct Ln16Inv {
 u32 t[65536];
};
constexpr Ln16Inv LNINV16= []() {
 u16 cl[]= {1, 11778, 7028, 51115, 48663, 26081, 17458, 40223}, ch[]= {30334, 42368, 14380, 2223, 49688, 11217, 44239, 63445}, Tl[256]= {0}, Th[256]= {0};
 for(u8 i= 0; i < 8; ++i)
  for(u8 h= 1 << i, b= h; b--;) Tl[h | b]= Tl[b] ^ cl[i], Th[h | b]= Th[b] ^ ch[i];
 u16 id[65535]{};
 for(u32 k= 0, cur= 1; k < 65535; ++k, cur= u16(cur << 1) ^ (0x002d & -u16(cur >> 15))) id[k]= Tl[u8(cur)] ^ Th[cur >> 8];
 Ln16Inv r{};
 for(u32 k= 65535; k--;) r.t[id[k]]= (u32(id[k ? 65535 - k : 0]) << 16) | (u32(k) * 2699 % 65535);
 return r;
}();
inline u64 iv(u64 a) {
 assert(a);
 u64 a32= F32(a), b= mul(a, a32);
 auto [g, c]= unpack(mul2(_mm256_set_epi64x(0, b, 0, a32), _mm256_set1_epi64x(F16(b))));
 return mul(EMB(LNINV16.t[u16(c)] >> 16), g);
}
constexpr LinMap make_mul_table(u64 c) {
 u64 basis[64]= {c};
 for(int i= 1; i < 64; ++i) basis[i]= (basis[i - 1] << 1) ^ (0x1b & -(basis[i - 1] >> 63));
 return LinMap(basis);
}
struct Ln641 {
 u16 t[1 << 14];
 u16 operator()(u64 a) const { return t[u16((a * 0xffef5fb99f1bf6e7) >> 50)]; }
};
constexpr Ln641 LN641= []() {
 Ln641 h{};
 LinMap t= make_mul_table(0x6bf808f7824282a2);
 for(u64 k= 0, cur= 1; k < 641; ++k, cur= t(cur)) h.t[u16((cur * 0xffef5fb99f1bf6e7) >> 50)]= k;
 return h;
}();
constexpr u16 PHI_B[16]= {49349, 60640, 60091, 52204, 8753, 26688, 50952, 24030, 14026, 41051, 57150, 31936, 39252, 22252, 63476, 55223};
constexpr HalfMap<u16> PHI= HalfMap<u16>(PHI_B);
struct ClassTable65537 {
 u32 t[65535];
 u32 K0;
};
constexpr ClassTable65537 CLS65537= []() {
 ClassTable65537 r{};
 u64 cur= 1;
 u32 v= 0;
 LinMap MUL_G17= make_mul_table(0x1c1e79669b95a7ce);
 for(u32 k= 0; k < 65537; ++k) {
  const u64 fr= F16(cur);
  const u16 b1= cur ^ fr, b0= cur ^ PHI.t[0][u8(b1)] ^ PHI.t[1][b1 >> 8];
  if(b1 == 0) {
  } else if(b0 == 0) {
   r.K0= v;
  } else {
   u32 idx= u16(LNINV16.t[b0]) + 65535 - u16(LNINV16.t[b1]);
   if(idx >= 65535) idx-= 65535;
   r.t[idx]= v;
  }
  cur= MUL_G17(cur);
  v= v >= 2 ? v - 2 : v + 65535;
 }
 return r;
}();
// n = x^(2^32+1), fn = F16(n) から log_{G_65537}(x^(2^32-1)) を返す
inline u32 log_65537(u64 n, u64 fn) {
 const u16 b1= n ^ fn;
 if(!b1) return 0;
 const u16 b0= n ^ PHI(b1);
 if(!b0) return CLS65537.K0;
 u32 idx= u16(LNINV16.t[b0]) + 65535 - u16(LNINV16.t[b1]);
 if(idx >= 65535) idx-= 65535;
 return CLS65537.t[idx];
}
struct BSGSTable6700417 {
 static constexpr u32 mask= 524287;  // 19-bit, cap = 524288
 static constexpr u64 q= 6700417;
 static constexpr u32 m= 131072;
 static constexpr int max_i= 52;
 static constexpr u64 HI_MASK= ~u64(mask);  // = 0xfffffffffff80000
 static constexpr u64 inv_base_m= 0x1489880b9cf723de;
 static constexpr u64 inv2_base_m= 0x5be693c8c2c557e3;  // = inv_base_m^2
 static constexpr u64 inv3_base_m= 0x7a8a7626c26ddc4d;  // = inv_base_m^3
 // 空きは 0。値の側を j + 1 にして実在の entry が 0 にならないようにしてある
 // (こうすると 52 万要素の初期化ループが要らず、constexpr の step も浮く)。
 struct Tab {
  u64 t[mask + 1];
 };
 static constexpr Tab TAB= []() {
  const LinMap mg= make_mul_table(0x00f542601703f991);
  Tab r{};
  u64 cur= 1;
  for(u32 j= 0; j < m; ++j) {
   u32 h= u32(cur) & mask;
   while(r.t[h]) h= (h + 1) & mask;
   r.t[h]= (cur & HI_MASK) | (u64(j) + 1);
   cur= mg(cur);
  }
  return r;
 }();
 // 定数のベクタは 1 度だけ作る (関数の中で組むと毎回即値の組み立てが走る)
 static inline const __m256i V_S01= _mm256_set_epi64x(0, 0x1489880b9cf723de, 0, 1);
 static inline const __m256i V_S23= _mm256_set_epi64x(0, 0x7a8a7626c26ddc4d, 0, 0x5be693c8c2c557e3);
 static inline const __m256i V_S4= _mm256_set1_epi64x(0xfdb44dcbca6522de);
 static inline u32 solve(u64 target) {
  // stream は (0,1) と (2,3) の 2 本のベクタで持つ。mul2 の出力 (q0, q2) が次の mul2 の
  // operand の置き場所そのものなので、段を進めるのに詰め直しが要らない。
  const __m256i tv= _mm256_set1_epi64x(target);
  __m256i A= mul2(tv, V_S01), B= mul2(tv, V_S23);  // (t0, t1), (t2, t3)
  __m256i An= mul2(A, V_S4), Bn= mul2(B, V_S4);
  __m256i An2= mul2(An, V_S4), Bn2= mul2(Bn, V_S4);
  u64 t[4], t_n[4], t_n2[4];
  std::tie(t[0], t[1])= unpack(A), std::tie(t[2], t[3])= unpack(B);
  std::tie(t_n[0], t_n[1])= unpack(An), std::tie(t_n[2], t_n[3])= unpack(Bn);
  std::tie(t_n2[0], t_n2[1])= unpack(An2), std::tie(t_n2[2], t_n2[3])= unpack(Bn2);
  for(int j= 0; j < 4; ++j) {
   _mm_prefetch((const char*)&TAB.t[u32(t[j]) & mask], _MM_HINT_T0);
   _mm_prefetch((const char*)&TAB.t[u32(t_n[j]) & mask], _MM_HINT_T0);
   _mm_prefetch((const char*)&TAB.t[u32(t_n2[j]) & mask], _MM_HINT_T0);
  }
  for(u8 i= 0; i <= max_i; i+= 4) {
   for(int j= 0; j < 4; ++j) {
    if(i + j > max_i) break;
    u64 tt= t[j];
    u32 h= u32(tt) & mask;
    while(TAB.t[h]) {
     u64 e= TAB.t[h];
     if(((e ^ tt) & HI_MASK) == 0) return u32((i + j) * m + u32(e & mask) - 1);
     h= (h + 1) & mask;
    }
   }
   A= An, B= Bn, An= An2, Bn= Bn2;
   for(int j= 0; j < 4; ++j) t[j]= t_n[j], t_n[j]= t_n2[j];
   if(i + 12 <= max_i) {
    An2= mul2(An, V_S4), Bn2= mul2(Bn, V_S4);
    std::tie(t_n2[0], t_n2[1])= unpack(An2), std::tie(t_n2[2], t_n2[3])= unpack(Bn2);
    for(int j= 0; j < 4; ++j) _mm_prefetch((const char*)&TAB.t[u32(t_n2[j]) & mask], _MM_HINT_T0);
   }
  }
  return q;
 }
};
inline u64 ln(u64 x) {
 assert(x);
 const u64 x32= F32(x), n= mul(x, x32), fn= F16(n);
 auto [x_f16, w]= unpack(mul2(_mm256_set_epi64x(0, sq(x32), 0, n), _mm256_set1_epi64x(fn)));
 const u32 lnv= LNINV16.t[u16(x_f16)];
 const u64 s= mul(EMB(u16(lnv >> 16)), w), s7= F7(s), t2= sq(s7), t3= mul(s7, t2);
 __m256i t24_48= linmap2<F3, F4>(t3, t3);
 auto [t72, t51]= unpack(mul2(t24_48, _mm256_set_epi64x(0, t3, 0, _mm256_extract_epi64(t24_48, 2))));
 auto [x_641, x_6700417]= unpack(mul2(mul2(_mm256_set_epi64x(0, t2, 0, F10(t51)), _mm256_set_epi64x(0, t3, 0, mul(t72, t51))), _mm256_set1_epi64x(s)));
 const u16 r1= u16(lnv);
 const u32 r0= LN641(x_641);
 const u32 r2= log_65537(n, fn);
 const u32 r3= BSGSTable6700417::solve(x_6700417);
 // 法の積が 2^64-1 なので、E_i ≡ 1 (mod m_i)、≡ 0 (mod 他の法) として x ≡ Σ r_i E_i (mod 2^64-1)。
 // 2^64 ≡ 1 より 128 bit の和は上下を足せば法に落ちる。r1, r2 の E_i は (2^64-1)/m_i の 2^14 倍で、
 // この法での 2^14 倍は 14 bit の回転。和が 2^64-1 の倍数になるのは余りが全部 0 (和も 0) のときだけ。
 const __uint128_t acc= __uint128_t(0xeba1bf4d145e40b2) * r0 + __uint128_t(0x945e40b26ba1bf4d) * r3 + std::rotl(u64(r1) * 0x0001000100010001, 14) + std::rotl(u64(r2) * 0x0000ffff0000ffff, 14);
 const u64 lo= u64(acc), t= lo + u64(acc >> 64);
 return t + (t < lo);
}
}
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::ln(as[i]);
 return ans;
}
