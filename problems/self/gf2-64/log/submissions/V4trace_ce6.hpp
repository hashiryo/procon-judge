#pragma once
// Library の GF2p64 (LNINV16 を INV16 と LN16 に分け、CLS65537 を 2 回に分けて作るようにした版) の log を写し、
// 6700417 の成分を、トレース t(z) = z + z^-1 を鍵にする BSGS で求める版。baby step は 119650 個で、表は 6 回の定数評価に分けて作る (前の評価の表を写し、漸化式の続きから書き足す)。
//
// 位数 6700417 の部分群は x^(2^32+1) = 1 の中にあるので z^-1 = z^(2^32) で、t(z) は GF(2^32) の元になる。t(z) = t(z^-1)
// なので、表の 1 項目で G^b と G^-b を覆え、giant step の幅を 2m + 1 にできる (最大 28 回)。
// 1 回の log の最初に、t(y) は y + y^-1 として掛け算なしで出し、2 周目の初期値は giant step が 4 回を超えるときだけ作る。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#include <simde/x86/bmi.h>
#endif
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
constexpr LinMap F7= F4 * F3;
constexpr LinMap F8= F4 * F4;
constexpr LinMap F10= F8 * F2;
constexpr LinMap F15= F8 * F7;
constexpr LinMap F16= F8 * F8;
constexpr LinMap F32= F16 * F16;
constexpr LinMap F48= F32 * F16;
constexpr LinMap F63= F48 * F15;
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
template <bool V, int IMM= 0> inline __m256i mul2(const __m256i& a_vec, const __m256i& b_vec) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 __m256i prod;
 if constexpr(V) prod= _mm256_clmulepi64_epi128(a_vec, b_vec, IMM);
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
 auto [b6, b7]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 60) & 0xf]), 0, F32(t[(e >> 56) & 0xf])), _mm256_set_epi64x(0, t[(e >> 28) & 0xf], 0, t[(e >> 24) & 0xf])));
 auto [b4, b5]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 52) & 0xf]), 0, F32(t[(e >> 48) & 0xf])), _mm256_set_epi64x(0, t[(e >> 20) & 0xf], 0, t[(e >> 16) & 0xf])));
 __m256i b23= mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 44) & 0xf]), 0, F32(t[(e >> 40) & 0xf])), _mm256_set_epi64x(0, t[(e >> 12) & 0xf], 0, t[(e >> 8) & 0xf]));
 __m256i b01= mul2<VPCLMUL>(_mm256_set_epi64x(0, F32(t[(e >> 36) & 0xf]), 0, F32(t[(e >> 32) & 0xf])), _mm256_set_epi64x(0, t[(e >> 4) & 0xf], 0, t[e & 0xf]));
 auto [b2, b3]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F16(b7), 0, F16(b6)), b23));
 auto [b0, b1]= unpack(mul2<VPCLMUL>(_mm256_set_epi64x(0, F8(b3), 0, F8(b2)), mul2<VPCLMUL>(_mm256_set_epi64x(0, F16(b5), 0, F16(b4)), b01)));
 return mul(F4(b1), b0);
}
template <class U> struct LinMap16 {
 U t[2][256];
 constexpr LinMap16(const U b[16]): t{} {
  for(int i= 0; i < 16; ++i) {
   U* l= t[i >> 3];
   for(u8 h= 1 << (i & 7), j= h; j--;) l[h | j]= l[j] ^ b[i];
  }
 }
 inline constexpr U operator()(const u16 x) const { return t[0][u8(x)] ^ t[1][x >> 8]; }
};
constexpr u64 EMB_B[]= {0x0000000000000001, 0x5fbfaec6aeac0002, 0xb06c601895640004, 0xb013b5277b7c0008, 0xb5ebb915248a0010, 0x109bb25b2c600020, 0xbf3bd95bd4190040, 0x0fc66342279b0080, 0xb6418f5e57c50100, 0xaa194bd4b83f0200, 0x1b5217b4dcc70400, 0xbb06fa73867a0800, 0x006fd55b23331000, 0x4ae8fb39198c2000, 0xfbd141b29b4f4000, 0x1d9ce1776be78000};
constexpr LinMap16<u64> EMB= LinMap16<u64>(EMB_B);
constexpr u32 MG_B[16]= {11778, 26543, 52504, 2252, 62850, 2916, 10521, 61878, 56874, 42750, 3345, 46259, 4395, 25479, 2375, 31014}, MGI_B[16]= {31924, 38226, 20204, 7599, 29649, 43528, 31690, 42995, 5042, 974, 38821, 36722, 24051, 36426, 48847, 56272};
struct Inv16 {
 u16 t[65536];
};
constexpr Inv16 INV16= []() {
 u32 f[2][256]{}, b[2][256]{};
 for(int i= 0; i < 16; ++i)
  for(u32 h= 1 << (i & 7), j= h; j--;) f[i >> 3][h | j]= f[i >> 3][j] ^ MG_B[i], b[i >> 3][h | j]= b[i >> 3][j] ^ MGI_B[i];
 Inv16 r{};
 r.t[1]= 1;
 for(u32 k= 32767, x= 1, y= 1; k--;) x= f[0][x & 255] ^ f[1][x >> 8], y= b[0][y & 255] ^ b[1][y >> 8], r.t[x]= y, r.t[y]= x;
 return r;
}();
template <bool V> inline u64 iv(u64 a) {
 assert(a);
 u64 a32= F32(a), b= mul(a, a32);
 auto [g, c]= unpack(mul2<V>(_mm256_set_epi64x(0, b, 0, a32), _mm256_set1_epi64x(F16(b))));
 return mul(EMB(INV16.t[u16(c)]), g);
}
constexpr u32 MH_B[16]= {42619, 34034, 37264, 59687, 13661, 58726, 9805, 26873, 8763, 63546, 2437, 49325, 17957, 37424, 41924, 9918}, MHI_B[16]= {65259, 13521, 41942, 64933, 45949, 48995, 32680, 14796, 1131, 41664, 58865, 25754, 5510, 38977, 46447, 39768};
struct Ln16 {
 u16 t[65536];
};
constexpr Ln16 LN16= []() {
 u32 f[2][256]{}, b[2][256]{};
 for(int i= 0; i < 16; ++i)
  for(u32 h= 1 << (i & 7), j= h; j--;) f[i >> 3][h | j]= f[i >> 3][j] ^ MH_B[i], b[i >> 3][h | j]= b[i >> 3][j] ^ MHI_B[i];
 Ln16 r{};
 for(u32 l= 1, x= 1, y= 1; l < 32768; ++l) x= f[0][x & 255] ^ f[1][x >> 8], y= b[0][y & 255] ^ b[1][y >> 8], r.t[x]= l, r.t[y]= 65535 - l;
 return r;
}();
constexpr LinMap mul_linmap(u64 c) {
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
 LinMap m= mul_linmap(0x6bf808f7824282a2);
 for(u64 k= 0, cur= 1; k < 641; ++k, cur= m(cur)) h.t[u16((cur * 0xffef5fb99f1bf6e7) >> 50)]= k * 590 % 641;
 return h;
}();
constexpr u16 PHI_B[16]= {49349, 60640, 60091, 52204, 8753, 26688, 50952, 24030, 14026, 41051, 57150, 31936, 39252, 22252, 63476, 55223};
constexpr LinMap16<u16> PHI= LinMap16<u16>(PHI_B);
constexpr u32 MC_B[32]= {0xca137f44, 0x02f9ac22, 0x24119ddf, 0x677fa964, 0x1c3c90b8, 0x61acd330, 0x087e6d0e, 0x98f43405, 0x17ef3800, 0x46a70e74, 0xfdd52d61, 0x9767f2ed, 0xa06bb110, 0xf0ef2346, 0x88d7f773, 0x3bdf87f2, 0xb557b556, 0xaedbaed9, 0xb9ceb9ca, 0xce1bce13, 0x8c848c94, 0xb29cb2bc, 0x65706530, 0xacf1ac71, 0x2fef2eef, 0x48d34ad3, 0xd0b4d4b4, 0x658a6d8a, 0x117b017b, 0xd3a9f3a9, 0x7fa43fa4, 0xbc2d3c2d};
struct ClassTable65537 {
 u32 t[65535];
 u32 K0, s;
};
constexpr ClassTable65537 cls65537(ClassTable65537 r, u32 k, u32 e) {
 u32 m[4][256]{};
 for(int i= 0; i < 32; ++i)
  for(u32 h= 1 << (i & 7), j= h; j--;) m[i >> 3][h | j]= m[i >> 3][j] ^ MC_B[i];
 u32 s= r.s, b0, b1, l1;
 for(; k < e; ++k) s= m[0][s & 255] ^ m[1][s >> 8 & 255] ^ m[2][s >> 16 & 255] ^ m[3][s >> 24], b0= s & 65535, b1= s >> 16, l1= 65535 - LN16.t[b1], (b0 ? r.t[(LN16.t[b0] + l1) % 65535] : r.K0)= k, (b0 ^ b1 ? r.t[(LN16.t[b0 ^ b1] + l1) % 65535] : r.K0)= 65537 - k;
 r.s= s;
 return r;
}
constexpr ClassTable65537 CLS65537_0= cls65537({{}, 0, 1}, 1, 16385), CLS65537= cls65537(CLS65537_0, 16385, 32769);
inline u32 log_65537(u64 n, u64 fn) {
 const u16 b1= n ^ fn;
 if(!b1) return 0;
 const u16 b0= n ^ PHI(b1);
 if(!b0) return CLS65537.K0;
 u32 idx= LN16.t[b0] + 65535 - LN16.t[b1];
 if(idx >= 65535) idx-= 65535;
 return CLS65537.t[idx];
}
// 位数 6700417 の部分群の log を、トレース t(z) = z + z^-1 を鍵にする BSGS で求める。t(z) は GF(2^32) の元で、
// 鍵 u32(t ^ (t >> 32)) は GF(2^32) の上で単射 (下位 32 bit だけでは単射にならない)。
// baby step: t(G^(b+1)) = t(G)·t(G^b) + t(G^(b-1))。鍵の 32 bit の座標で t(G) 倍は線型写像なので、表引き 4 回で進む。
// giant step: z_i = y·h^i (h = G^-s、s = 2m + 1) のトレースも t(z_(i+8)) = t(h^4)·t(z_(i+4)) + t(z_i) で、4 本を並べて進める。
// 表に当たったら z_i = G^b か G^-b なので、z_i と G^b を小さな表から作って比べ、符号を決める。
constexpr u32 TR_M= 119650, TR_S= 2 * TR_M + 1, TR_I= (6700417 - 1) / TR_S + 1, TR_SLOTS= 262144;
constexpr u32 TR_C1= 0x472523cd, TR_MU_B[32]= {0x472523cd, 0xc493a17e, 0x8c6bbbdb, 0xd78fbf23, 0x463851c8, 0x35e30d55, 0x29c8d62f, 0x148d91d5, 0xda9d1289, 0x3ede4353, 0xc6aa2ca4, 0x9839b8c5, 0x46a768ef, 0x6c1e196a, 0x7d93a09b, 0x2986d17c, 0x2a0bacab, 0x77c36094, 0xbdb5a712, 0xb85c168a, 0xa5a604af, 0x320e0990, 0x2cd2f85a, 0x769b22e8, 0x3b34f6c9, 0x2df74317, 0xace32974, 0xb4edf746, 0x3adaf87d, 0xe35cfc23, 0xb9e4c426, 0xdd68b4b6};
// h^k、h^-k (k = 0..7) と t(h^4)
constexpr u64 TR_H[8]= {0x0000000000000001, 0xe846cb851f677e44, 0x1b94a5ce63e5d1fa, 0x329ae325b6e33e9b, 0x09b48ea7fcad2258, 0x83cf879a32943a99, 0x327e89ae19232cee, 0xa964e03cc32931c5};
constexpr u64 TR_HI[8]= {0x0000000000000001, 0x525c7977d6f5cdbd, 0xfa79f960918b838d, 0x75545e571f71c381, 0x40ac819941df9c10, 0xdce60166b2e6cae3, 0xc7ffbfb0fc18eadd, 0xadc0a99116637943};
constexpr u64 TR_T4= 0x49180f3ebd72be48;
// 枠には 鍵 << 32 | b を入れる。p, c は漸化式の直前の 2 項 (鍵の座標)、b は次に入れる番号
struct TraceTab {
 u64 t[TR_SLOTS];
 u32 p, c, b;
};
constexpr TraceTab trace_part(TraceTab r, u32 e) {
 u32 m[4][256]{};
 for(int i= 0; i < 32; ++i)
  for(u32 h= 1 << (i & 7), j= h; j--;) m[i >> 3][h | j]= m[i >> 3][j] ^ TR_MU_B[i];
 u32 p= r.p, c= r.c, b= r.b;
 for(; b <= e; ++b) {
  u32 h= c & (TR_SLOTS - 1);
  while(r.t[h]) h= (h + 1) & (TR_SLOTS - 1);
  r.t[h]= u64(c) << 32 | b;
  const u32 n= m[0][c & 255] ^ m[1][c >> 8 & 255] ^ m[2][c >> 16 & 255] ^ m[3][c >> 24] ^ p;
  p= c, c= n;
 }
 r.p= p, r.c= c, r.b= b;
 return r;
}
// 後ろの評価ほど表が埋まって探査が増えるので、入れる数を少しずつ減らし、どの評価も同じくらいの手数にする
constexpr TraceTab TR_TAB_0= trace_part({{}, 0, TR_C1, 1}, 21502), TR_TAB_1= trace_part(TR_TAB_0, 42557), TR_TAB_2= trace_part(TR_TAB_1, 63007), TR_TAB_3= trace_part(TR_TAB_2, 82794), TR_TAB_4= trace_part(TR_TAB_3, 101694), TR_TAB= trace_part(TR_TAB_4, TR_M);
// 符号を決めるための G^b = lo[b & 255]·hi[b >> 8] と h^i = hp[i]
constexpr LinMap TR_MUL_G= mul_linmap(0x00f542601703f991), TR_MUL_G256= mul_linmap(0x5f915310c81c09e9), TR_MUL_H= mul_linmap(0xe846cb851f677e44);
struct TracePow {
 u64 lo[256], hi[TR_M / 256 + 1], hp[TR_I];
};
constexpr TracePow TR_POW= []() {
 TracePow r{};
 r.lo[0]= r.hi[0]= r.hp[0]= 1;
 for(int i= 1; i < 256; ++i) r.lo[i]= TR_MUL_G(r.lo[i - 1]);
 for(u32 i= 1; i <= TR_M / 256; ++i) r.hi[i]= TR_MUL_G256(r.hi[i - 1]);
 for(u32 i= 1; i < TR_I; ++i) r.hp[i]= TR_MUL_H(r.hp[i - 1]);
 return r;
}();
template <bool V> inline u32 log_6700417(u64 y) {
 constexpr u32 L= 6700417;
 const u64 yi= F32(y);
 const __m256i Y= _mm256_set_epi64x(0, yi, 0, y);
 constexpr int NT= TR_I > 4 ? 8 : 4;  // 初期値は、2 周目に入るときだけ 8 個作る
 u64 t[8]= {y ^ yi};                  // t(y) は掛け算なしで出る
 for(int k= 1; k < NT; ++k) {
  const auto [a, b]= unpack(mul2<V>(Y, _mm256_set_epi64x(0, TR_HI[k], 0, TR_H[k])));
  t[k]= a ^ b;  // t(y·h^k)
 }
 const __m256i T4= _mm256_set1_epi64x(TR_T4);
 __m256i A0= _mm256_set_epi64x(0, t[1], 0, t[0]), B0= _mm256_set_epi64x(0, t[3], 0, t[2]);
 __m256i A1= _mm256_set_epi64x(0, t[5], 0, t[4]), B1= _mm256_set_epi64x(0, t[7], 0, t[6]);
 for(int k= 0; k < NT; ++k) _mm_prefetch((const char*)&TR_TAB.t[u32(t[k] ^ (t[k] >> 32)) & (TR_SLOTS - 1)], _MM_HINT_T0);
 for(u32 i= 0; i < TR_I; i+= 4) {
  const __m256i A2= _mm256_xor_si256(mul2<V>(A1, T4), A0), B2= _mm256_xor_si256(mul2<V>(B1, T4), B0);
  u64 v[4], w[4];
  std::tie(v[0], v[1])= unpack(A0), std::tie(v[2], v[3])= unpack(B0);
  std::tie(w[0], w[1])= unpack(A2), std::tie(w[2], w[3])= unpack(B2);
  for(int r= 0; r < 4; ++r) _mm_prefetch((const char*)&TR_TAB.t[u32(w[r] ^ (w[r] >> 32)) & (TR_SLOTS - 1)], _MM_HINT_T0);
  for(u32 r= 0; r < 4 && i + r < TR_I; ++r) {
   if(!v[r]) return u32(u64(TR_S) * (i + r) % L);  // z_i = 1
   const u32 key= u32(v[r] ^ (v[r] >> 32));
   for(u32 h= key & (TR_SLOTS - 1); TR_TAB.t[h]; h= (h + 1) & (TR_SLOTS - 1))
    if(u32(TR_TAB.t[h] >> 32) == key) {
     const u32 b= u32(TR_TAB.t[h]);
     const u64 z= mul(y, TR_POW.hp[i + r]), g= mul(TR_POW.lo[b & 255], TR_POW.hi[b >> 8]);
     return u32((u64(TR_S) * (i + r) + (z == g ? b : L - b)) % L);
    }
  }
  A0= A1, B0= B1, A1= A2, B1= B2;
 }
 return L;
}
template <bool V> inline u64 ln(u64 x) {
 assert(x);
 const u64 x32= F32(x), n= mul(x, x32), fn= F16(n);
 auto [x_f16, w]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(x32), 0, n), _mm256_set1_epi64x(fn)));
 const u16 xf= u16(x_f16);
 const u64 s= mul(EMB(INV16.t[xf]), w), s7= F7(s), t2= sq(s7), t3= mul(s7, t2), t48= F4(t3);
 auto [t5, t51]= unpack(mul2<V>(_mm256_set_epi64x(0, t48, 0, t2), _mm256_set1_epi64x(t3)));
 auto [x_6700417, a]= unpack(mul2<V>(_mm256_set_epi64x(0, t51, 0, t5), _mm256_set1_epi64x(s)));
 u32 r3= log_6700417<V>(x_6700417);
 auto [t72, b]= unpack(mul2<V>(_mm256_set_epi64x(0, F10(t51), 0, F3(t3)), _mm256_set_epi64x(0, a, 0, t48)));
 u64 r0= LN641(mul(t72, b)), r2= log_65537(n, fn);
 const __uint128_t acc= 0x663d80ff99c27f * r0 + __uint128_t(0x945e40b26ba1bf4d) * r3 + 0x1000100010001ull * LN16.t[xf] + 0xffff0000ffff * r2;
 const u64 lo= u64(acc), t= lo + u64(acc >> 64);
 return t + (t < lo);
}
}
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::ln<1>(as[i]);
 return ans;
}
