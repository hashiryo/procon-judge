#pragma once
// Library の GF2p64 (LNINV16 を INV16 と LN16 に分け、CLS65537 を 2 回に分けて作るようにした版) の log を写し、
// 6700417 の成分を Bernstein と Lange の方法で求める版。前計算を今の上限 (GCC の -fconstexpr-ops-limit=2097152)
// で 1 回の定数評価に収まる大きさにして、表は constexpr のまま作る。歩き方、定数、問い合わせの手順は V3rho_ce_5 と同じ。
//
// V3rho_ce_5 は 2^19 本の歩みで 2^18 点ほどの表 (4 MB) を作るが、ここでは distinguished point を 16 歩に 1 回
// (x の bit 19..22 が 0) にし、歩みを 768 本に減らした (表は 4096 枠、32 KB)。前計算の歩みが覆う点が 1.2 万ほどに
// なるので、1 回の log は歩み 1 本に数えて 500 歩前後に増える。
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
// 位数 6700417 の部分群の log を、前計算した distinguished point の表とランダムウォークで求める
// (D. J. Bernstein, T. Lange, "Computing small discrete logarithms faster", 2012)。
constexpr u32 RHO_L= 6700417;
constexpr u64 RHO_STEP[32]= {0x648f1423d066f928, 0x5140a1726a2dbe6b, 0xfa56f38ce61a8196, 0x2f44d1fcfca9df83, 0x6c83c361c38f4cbd, 0xa736b38928e66791, 0x3c82faa3ef48bde4, 0x199aee92f8bc4c92, 0x277a7a6b0babe211, 0xbffe8bf1d1b17e67, 0xe44cdbf25ea609cb, 0xacb9e63f0557061e, 0x40d245160a6b61ed, 0x4f95f5dede9f9813, 0xb24744ddd35d5717, 0x85194775e556e953, 0xea65dfda4b074acb, 0x3e12d639f4221b83, 0xfb5e2866f02d88a5, 0xeb8e091192317a58, 0xc99b9f73ff258550, 0x57b634c2d5c2168b, 0xf59631c54807af81, 0x2800ab663b682205, 0xb8f21c99e880890f, 0x6367cf47e0fd56e3, 0xfbe0531679a18e55, 0xf1c5114fbfb12c62, 0x14b1fc71e01693e0, 0xaf7e15b7b3e1db88, 0x204ea6eac90f41d4, 0x862435c71ca1b284};
constexpr u32 RHO_STEP_LOG[32]= {4325469, 4339069, 3823330, 358891, 6440671, 6594993, 2070398, 3303315, 2778511, 6286735, 1186077, 758906, 6492893, 5609243, 1689973, 5727879, 4061074, 5204591, 2891882, 4552218, 3782916, 3104878, 5461058, 2688088, 6553423, 6609727, 822680, 149505, 4066850, 1953817, 6553018, 3931321};
constexpr u64 RHO_RESTART= 0xaf2910f9656d8696;  // 歩き直すたびに出発点へ掛ける元
constexpr u32 RHO_RESTART_LOG= 1822205;
constexpr u64 RHO_START= 0xbe55263f4cbcdd46;  // 前計算の最初の出発点
constexpr u32 RHO_START_LOG= 4126168;
constexpr u64 RHO_DSTART= 0x0ec30ae97a8e7a80;  // 前計算の出発点を 1 本ごとにずらす元
constexpr u32 RHO_DSTART_LOG= 4795639;
constexpr u32 RHO_SLOTS= 4096, RHO_N= 768;       // 表の枠と前計算の歩みの本数
constexpr u64 RHO_LOG_MASK= (u64(1) << 23) - 1;  // 枠には log + 1 と、x の bit 23 以上を詰める
constexpr int RHO_MAX_STEPS= 256;                // これより長く歩いたら、輪に入ったとみなして歩き直す
constexpr bool rho_dist(u64 x) { return ((x >> 19) & 15) == 0; }
// constexpr の中で歩くための、STEP[i] 倍と DSTART 倍の線形写像。32 個を 1 つの初期化で作ると 1 回の評価の
// 上限を超えるので、1 個ずつ別の変数にして、その場所を並べる
template <int I> constexpr LinMap RHO_MAP= mul_linmap(RHO_STEP[I]);
struct RhoMaps {
 const LinMap* p[32];
};
constexpr RhoMaps RHO_MAPS= []<std::size_t... I>(std::index_sequence<I...>) { return RhoMaps{{&RHO_MAP<I>...}}; }(std::make_index_sequence<32>{});
constexpr LinMap RHO_DMAP= mul_linmap(RHO_DSTART);
// 出発点 G^(START_LOG + k·DSTART_LOG) から RHO_N 本歩き、着いた distinguished point をすべて表に入れる
struct RhoTab {
 u64 t[RHO_SLOTS];
};
constexpr RhoTab RHO_TAB= []() {
 RhoTab r{};
 u64 x0= RHO_START;
 u32 v= RHO_START_LOG;
 for(u32 k= 0; k < RHO_N; ++k) {
  u64 x= x0;
  u32 e= v;
  int s= 0;
  for(; s < RHO_MAX_STEPS && ((x >> 19) & 15) != 0; ++s) {
   const u32 i= x >> 59;
   const LinMap& m= *RHO_MAPS.p[i];
   x= m.t[0][x & 255] ^ m.t[1][x >> 8 & 255] ^ m.t[2][x >> 16 & 255] ^ m.t[3][x >> 24 & 255] ^ m.t[4][x >> 32 & 255] ^ m.t[5][x >> 40 & 255] ^ m.t[6][x >> 48 & 255] ^ m.t[7][x >> 56], e+= RHO_STEP_LOG[i];
  }
  if(s < RHO_MAX_STEPS) {
   u32 h= u32(x) & (RHO_SLOTS - 1);
   while(r.t[h] && ((r.t[h] ^ x) & ~RHO_LOG_MASK)) h= (h + 1) & (RHO_SLOTS - 1);
   if(!r.t[h]) r.t[h]= (x & ~RHO_LOG_MASK) | (e % RHO_L + 1);
  }
  x0= RHO_DMAP.t[0][x0 & 255] ^ RHO_DMAP.t[1][x0 >> 8 & 255] ^ RHO_DMAP.t[2][x0 >> 16 & 255] ^ RHO_DMAP.t[3][x0 >> 24 & 255] ^ RHO_DMAP.t[4][x0 >> 32 & 255] ^ RHO_DMAP.t[5][x0 >> 40 & 255] ^ RHO_DMAP.t[6][x0 >> 48 & 255] ^ RHO_DMAP.t[7][x0 >> 56], v= (v + RHO_DSTART_LOG) % RHO_L;
 }
 return r;
}();
struct Rho6700417 {
 static constexpr u32 L= RHO_L;
 static constexpr bool dist(u64 x) { return rho_dist(x); }
 // x が表にあれば log(x) + 1、無ければ 0
 static u32 find(u64 x) {
  for(u32 h= u32(x) & (RHO_SLOTS - 1); RHO_TAB.t[h]; h= (h + 1) & (RHO_SLOTS - 1))
   if(((RHO_TAB.t[h] ^ x) & ~RHO_LOG_MASK) == 0) return u32(RHO_TAB.t[h] & RHO_LOG_MASK);
  return 0;
 }
 u32 solve(u64 y) const;  // 1 つの対象を 2 本の歩みで探す
};
inline u32 Rho6700417::solve(u64 y) const {
 // 歩み 0 は y から、歩み 1 は y·RESTART から出る。歩き直すときは、次にずらした出発点をもらう
 u64 x[2]= {y, mul(y, RHO_RESTART)}, nx= mul(x[1], RHO_RESTART);
 u32 e[2]= {0, RHO_RESTART_LOG}, ne= u32(2 * u64(RHO_RESTART_LOG) % L);  // x[c] = y·G^e[c]
 int s[2]= {0, 0};
 for(;;)
  for(int c= 0; c < 2; ++c) {
   if(dist(x[c])) {
    if(const u32 f= find(x[c])) return u32((u64(f) - 1 + L - e[c] % L) % L);
   } else if(s[c] < RHO_MAX_STEPS) {
    const int i= int(x[c] >> 59);
    x[c]= mul(x[c], RHO_STEP[i]), e[c]+= RHO_STEP_LOG[i], ++s[c];
    continue;
   }
   // 表に無い点に着いたか、長く歩きすぎた。出発点をずらして歩き直す
   x[c]= nx, e[c]= ne, s[c]= 0;
   nx= mul(nx, RHO_RESTART), ne= u32((u64(ne) + RHO_RESTART_LOG) % L);
  }
}
constexpr Rho6700417 RHO6700417{};
template <bool V> inline u64 ln(u64 x) {
 assert(x);
 const u64 x32= F32(x), n= mul(x, x32), fn= F16(n);
 auto [x_f16, w]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(x32), 0, n), _mm256_set1_epi64x(fn)));
 const u16 xf= u16(x_f16);
 const u64 s= mul(EMB(INV16.t[xf]), w), s7= F7(s), t2= sq(s7), t3= mul(s7, t2), t48= F4(t3);
 auto [t5, t51]= unpack(mul2<V>(_mm256_set_epi64x(0, t48, 0, t2), _mm256_set1_epi64x(t3)));
 auto [x_6700417, a]= unpack(mul2<V>(_mm256_set_epi64x(0, t51, 0, t5), _mm256_set1_epi64x(s)));
 auto [t72, b]= unpack(mul2<V>(_mm256_set_epi64x(0, F10(t51), 0, F3(t3)), _mm256_set_epi64x(0, a, 0, t48)));
 const u64 r0= LN641(mul(t72, b)), r2= log_65537(n, fn);
 const u32 r3= RHO6700417.solve(x_6700417);
 const __uint128_t acc= 0x663d80ff99c27f * r0 + __uint128_t(0x945e40b26ba1bf4d) * r3 + 0x1000100010001ull * LN16.t[xf] + 0xffff0000ffff * r2;
 const u64 lo= u64(acc), t= lo + u64(acc >> 64);
 return t + (t < lo);
}
}  // namespace gf2p64_internal
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::ln<1>(as[i]);
 return ans;
}
