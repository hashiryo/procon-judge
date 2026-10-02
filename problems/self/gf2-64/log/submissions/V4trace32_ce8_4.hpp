#pragma once
// V4trace32_ce6_3 の baby step を増やした版。表を 8 回の定数評価に分けて作り、baby step を 175834 個にした
// (giant step は最大 20 回)。充填率は 0.67 で、外れ 1 回で読む 4 枠の組は平均 1.78 組 (1 枠ずつなら平均 5.13 枠)。
// 表を折り返さずに埋めるので、1 回の評価で入れられる数が V4trace32_ce6_2 より増える。
// 区切りは、GCC の手数の見積もり (1 回の評価 = 30000 + 76·入れる数 + 13·探査の数、CI と同じ g++-15 で測った値) が
// 上限 2097152 の 95% に収まるように決めた。clang は 1 回の評価が上限の 3 割ほどで済み、配列の要素数 (枠 + 余白) が
// 上限 524288 の半分なので、どちらも引っかからない。コンパイル中のメモリの山は、CI の arm のフラグで GCC 495 MB、
// clang 373 MB (V4trace32_ce6_3 は 411 MB と 343 MB)。GCC は入れる数が増えるほど、評価ごとに抱える表の写しが重くなる。
// 以下は V4trace32_ce6_3 の説明。
// V4trace32_ce6_2 の表引きを SIMD にした版。ほかは V4trace32_ce6_2 と同じ。
// 表引きは 1 回の log でほとんどが外れで、外れと分かるには鍵の位置から空き枠まで見る必要がある。1 枠ずつ分岐で見る
// のをやめ、続く 4 枠を 1 回で読み、当たりの枠と空き枠を 1 回の比較で探す (外れ 1 回で読む 4 枠の組は、充填率 0.46 で
// 平均 1.13 組。1 枠ずつなら平均 2.19 枠)。4 枠を読めるよう、表は末尾で折り返さず、後ろの TR_PAD 枠の余白へはみ出す形で
// 埋める (この鍵の並びでは、はみ出しは起きない)。
//
// s = x^(2^32-1) から 641 と 6700417 の成分へ写す部分は V4trace32_ce6_2 と同じで、V4trace32_ce6 より短くしてある。
// 641 = 513 + 2^7 と 6700417 = 513 + 2^7·52343 (52343 = 17·3079、3079 = 1 + 6·513、513 = 2^9 + 1) から、s^641 も
// s^6700417 も、s^513 と 2^7 ずらした値の積になる。F7 を s と s^52343 の 2 か所に掛ける代わりに、F7 の逆の F57 を s^513 に
// 1 回だけ掛け、F57(s^641) と F57(s^6700417) を作る。LinMap 4 回、sq 1 回、掛け算 8 回だったのが、LinMap 3 回、sq 2 回、
// 掛け算 6 回になる。対数はどちらも 2^57 = 2^-7 倍になるので、LN641 の表の値と r3 の CRT の係数に 2^7 を掛けて戻す。
//
// Library の GF2p64 (LNINV16 を INV16 と LN16 に分け、CLS65537 を 2 回に分けて作るようにした版) の log を写し、
// 6700417 の成分を、トレース t(z) = z + z^-1 を鍵にする BSGS で求める版。baby step は 175834 個で、表は 8 回の定数評価に分けて作る (前の評価の表を写し、漸化式の続きから書き足す)。
// V4trace 系との違いは giant step で、64 bit の掛け算 (mul2) を使わず、鍵の 32 bit の座標の上で表を 4 回引いて進める。
//
// 位数 6700417 の部分群は x^(2^32+1) = 1 の中にあるので z^-1 = z^(2^32) で、t(z) は GF(2^32) の元になる。t(z) = t(z^-1)
// なので、表の 1 項目で G^b と G^-b を覆え、giant step の幅を 2m + 1 にできる (最大 20 回)。
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
constexpr LinMap F9= F8 * F1;
constexpr LinMap F57= F48 * F9;
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
// 引くのは F57(s^6700417) なので、値は k·590 に 2^7 を掛けておく
constexpr Ln641 LN641= []() {
 Ln641 h{};
 LinMap m= mul_linmap(0x6bf808f7824282a2);
 for(u64 k= 0, cur= 1; k < 641; ++k, cur= m(cur)) h.t[u16((cur * 0xffef5fb99f1bf6e7) >> 50)]= k * 590 * 128 % 641;
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
// 鍵 u32(t ^ (t >> 32)) は GF(2^32) の上で単射 (下位 32 bit だけでは単射にならない)。鍵は GF(2) の上で線型なので、
// GF(2^32) の決まった元を掛けることは、鍵の 32 bit の座標の上の 32x32 の行列になる (表引き 4 回)。
// baby step: t(G^(b+1)) = t(G)·t(G^b) + t(G^(b-1))。
// giant step: z_i = y·h^i (h = G^-s、s = 2m + 1) の鍵 c_i も、c_(i+1) = [t(h) 倍] c_i + c_(i-1) で進む。最初の 2 つは、
// 64 bit から鍵への線型写像 Λ(x) = 鍵(x + x^(2^32)) で c_0 = Λ(y)、c_1 = Λ(y·h) と出す。
// 表に当たったら z_i = G^b か G^-b なので、z_i と G^b を小さな表から作って比べ、符号を決める。
constexpr u32 TR_PAD= 64;  // 表の後ろの余白。探査は折り返さずにここへはみ出してよく、4 枠ずつ読んでも表の外に出ない
constexpr u32 TR_M= 175834, TR_S= 2 * TR_M + 1, TR_I= (6700417 - 1) / TR_S + 1, TR_SLOTS= 262144;
// t(G) 倍、t(h) 倍、t(h^4) 倍の、鍵の座標での基底の像と、鍵(t(G))、h
constexpr u32 TR_C1= 0x472523cd, TR_MU_B[32]= {0x472523cd, 0xc493a17e, 0x8c6bbbdb, 0xd78fbf23, 0x463851c8, 0x35e30d55, 0x29c8d62f, 0x148d91d5, 0xda9d1289, 0x3ede4353, 0xc6aa2ca4, 0x9839b8c5, 0x46a768ef, 0x6c1e196a, 0x7d93a09b, 0x2986d17c, 0x2a0bacab, 0x77c36094, 0xbdb5a712, 0xb85c168a, 0xa5a604af, 0x320e0990, 0x2cd2f85a, 0x769b22e8, 0x3b34f6c9, 0x2df74317, 0xace32974, 0xb4edf746, 0x3adaf87d, 0xe35cfc23, 0xb9e4c426, 0xdd68b4b6};
constexpr u32 TR_MT1_B[32]= {0xf66482d5, 0x19ef30e3, 0xd0f00510, 0xc429cb8d, 0x16d317d3, 0x35b4e4b6, 0x9f22b192, 0xd8d0bedf, 0xebc55b25, 0x2157e601, 0x9549fa35, 0x6d919410, 0x4a60f31b, 0xf541146a, 0x9c4e9f43, 0x0573d785, 0x9a87dcdc, 0x05507dda, 0x40d52746, 0x70574538, 0x4bc2960d, 0xc3d9005d, 0x71b85aa0, 0xe48d93d0, 0x781b640b, 0x72b00e5d, 0xad73ae26, 0x51881957, 0xe59125b0, 0x866253d0, 0x1cd38182, 0x0f7041d2};
constexpr u32 TR_MT4_B[32]= {0x7191dd9a, 0x65639f8f, 0x289e51b9, 0xc2e2416a, 0xf15db167, 0x6291fe24, 0x3924d32c, 0x1f20d7b6, 0x6f548ae0, 0x316ba5ff, 0xbc5276de, 0xae7f4045, 0xf317db87, 0x8dddcc97, 0x425128b1, 0xa0b6ffdc, 0x7a445e21, 0xfe504617, 0x8a07fb28, 0x761fc783, 0x92434729, 0x34aaf9d2, 0x8c289ed8, 0x7a259612, 0xdf69fd80, 0x39f341fb, 0x35a12c80, 0x154c3736, 0x629766e7, 0x0bf39cfa, 0x0cac1b86, 0xa5e2e568};
constexpr u64 TR_H1= 0x2f6e3e1f787d8b3e;
// 鍵の座標への線型写像。N byte の入力を、byte ごとの表で引いて 32 bit にする
template <int N> struct Lin32 {
 u32 t[N][256];
 constexpr Lin32(const u32 b[N * 8]): t{} {
  for(int i= 0; i < N * 8; ++i)
   for(u32 h= 1 << (i & 7), j= h; j--;) t[i >> 3][h | j]= t[i >> 3][j] ^ b[i];
 }
 constexpr u32 operator()(u64 x) const {
  u32 r= 0;
  for(int k= 0; k < N; ++k) r^= t[k][u8(x >> 8 * k)];
  return r;
 }
};
constexpr Lin32<4> TR_M1= Lin32<4>(TR_MT1_B), TR_M4= Lin32<4>(TR_MT4_B);
// Λ(e_i) = 鍵(e_i + F32(e_i))。F32 の基底の像から作る
constexpr Lin32<8> TR_LAM= []() {
 u32 b[64]{};
 for(int i= 0; i < 64; ++i) {
  const u64 v= (u64(1) << i) ^ F32.g[i];
  b[i]= u32(v ^ (v >> 32));
 }
 return Lin32<8>(b);
}();
// 枠には 鍵 << 32 | b を入れる。p, c は漸化式の直前の 2 項 (鍵の座標)、b は次に入れる番号
struct TraceTab {
 u64 t[TR_SLOTS + TR_PAD];
 u32 p, c, b;
};
// 前の評価の表は参照で受け、中で写す。値で受けると GCC は写しを余分に抱え、コンパイル中のメモリが増える
// (10 回の評価の表づくりだけで 260 MB が 216 MB になった。clang は変わらない)
constexpr TraceTab trace_part(const TraceTab& r0, u32 e) {
 TraceTab r= r0;
 u32 m[4][256]{};
 for(int i= 0; i < 32; ++i)
  for(u32 h= 1 << (i & 7), j= h; j--;) m[i >> 3][h | j]= m[i >> 3][j] ^ TR_MU_B[i];
 u32 p= r.p, c= r.c, b= r.b;
 for(; b <= e; ++b) {
  u32 h= c & (TR_SLOTS - 1);
  while(r.t[h]) ++h;
  r.t[h]= u64(c) << 32 | b;
  const u32 n= m[0][c & 255] ^ m[1][c >> 8 & 255] ^ m[2][c >> 16 & 255] ^ m[3][c >> 24] ^ p;
  p= c, c= n;
 }
 r.p= p, r.c= c, r.b= b;
 return r;
}
// 後ろの評価ほど表が埋まって探査が増えるので、入れる数を少しずつ減らし、どの評価も同じくらいの手数にする
constexpr TraceTab TR_TAB_0= trace_part(TraceTab{{}, 0, TR_C1, 1}, 25575), TR_TAB_1= trace_part(TR_TAB_0, 50576), TR_TAB_2= trace_part(TR_TAB_1, 74833), TR_TAB_3= trace_part(TR_TAB_2, 98162), TR_TAB_4= trace_part(TR_TAB_3, 120266), TR_TAB_5= trace_part(TR_TAB_4, 140877), TR_TAB_6= trace_part(TR_TAB_5, 159525), TR_TAB= trace_part(TR_TAB_6, TR_M);
// 符号を決めるための G^b = lo[b & 255]·hi[b >> 8] と h^i = hp[i]
constexpr LinMap TR_MUL_G= mul_linmap(0x00f542601703f991), TR_MUL_G256= mul_linmap(0x5f915310c81c09e9), TR_MUL_H= mul_linmap(TR_H1);
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
// 鍵 k の枠を探し、見つかれば b を、無ければ 0 を返す。続く 4 枠を 1 回で読み、32 bit ごとに k << 32 と比べる。
// 上位 32 bit が k の枠が当たりで、下位 32 bit が 0 の枠が空き (b >= 1 なので、下位が 0 なのは空きの枠だけ)。
// 鍵は表に 1 つずつしか無く、当たりの枠は鍵の位置から最初の空きまでの間にあるので、同じ 4 枠に当たりと空きが
// あっても当たりを返してよい
inline u32 trace_lookup(u32 k) {
 const __m256i K= _mm256_set1_epi64x((long long)(u64(k) << 32));
 for(u32 h= k & (TR_SLOTS - 1);; h+= 4) {
  const u32 f= _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_loadu_si256((const __m256i*)&TR_TAB.t[h]), K)));
  if(f & 0xaa) return u32(TR_TAB.t[h + (__builtin_ctz(f & 0xaa) >> 1)]);
  if(f & 0x55) return 0;
 }
}
// 鍵 k の z_i を表で探す。見つかれば log を、無ければ 6700417 を返す
inline u32 trace_find(u64 y, u32 k, u32 i) {
 constexpr u32 L= 6700417;
 if(!k) return u32(u64(TR_S) * i % L);  // t(z_i) = 0 なので z_i = 1
 if(const u32 b= trace_lookup(k)) {
  const u64 z= mul(y, TR_POW.hp[i]), g= mul(TR_POW.lo[b & 255], TR_POW.hi[b >> 8]);
  return u32((u64(TR_S) * i + (z == g ? b : L - b)) % L);
 }
 return L;
}
inline void trace_prefetch(u32 k) { _mm_prefetch((const char*)&TR_TAB.t[k & (TR_SLOTS - 1)], _MM_HINT_T0); }
inline u32 log_6700417(u64 y) {
 constexpr u32 L= 6700417;
 u32 a[4];
 a[0]= TR_LAM(y), a[1]= TR_LAM(mul(y, TR_H1));
 a[2]= TR_M1(a[1]) ^ a[0], a[3]= TR_M1(a[2]) ^ a[1];
 for(int r= 0; r < 4; ++r) trace_prefetch(a[r]);
 if constexpr(TR_I <= 4) {
  for(u32 r= 0; r < TR_I; ++r)
   if(const u32 e= trace_find(y, a[r], r); e != L) return e;
  return L;
 } else {
  // 4 本を並べ、h^4 刻みで進める。b は a の 4 つ先
  u32 b[4];
  b[0]= TR_M1(a[3]) ^ a[2], b[1]= TR_M1(b[0]) ^ a[3], b[2]= TR_M1(b[1]) ^ b[0], b[3]= TR_M1(b[2]) ^ b[1];
  for(int r= 0; r < 4; ++r) trace_prefetch(b[r]);
  for(u32 i= 0; i < TR_I; i+= 4) {
   u32 n[4];
   for(int r= 0; r < 4; ++r) n[r]= TR_M4(b[r]) ^ a[r], trace_prefetch(n[r]);
   for(u32 r= 0; r < 4 && i + r < TR_I; ++r)
    if(const u32 e= trace_find(y, a[r], i + r); e != L) return e;
   for(int r= 0; r < 4; ++r) a[r]= b[r], b[r]= n[r];
  }
  return L;
 }
}
template <bool V> inline u64 ln(u64 x) {
 assert(x);
 const u64 x32= F32(x), n= mul(x, x32), fn= F16(n);
 auto [x_f16, w]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(x32), 0, n), _mm256_set1_epi64x(fn)));
 const u16 xf= u16(x_f16);
 // w1 = s^513、e57 = F57(w1)、w2 = s^1539、y = s·e57 = F57(s^641)、w3 = s^3079、w4 = s^52343、e57·w4 = F57(s^6700417)
 const u64 s= mul(EMB(INV16.t[xf]), w), w1= mul(s, F9(s)), e57= F57(w1);
 auto [w2, y]= unpack(mul2<V>(_mm256_set_epi64x(0, e57, 0, sq(w1)), _mm256_set_epi64x(0, s, 0, w1)));
 u32 r3= log_6700417(y);
 const u64 w3= mul(s, sq(w2)), w4= mul(w3, F4(w3));
 u64 r0= LN641(mul(e57, w4)), r2= log_65537(n, fn);
 // r3 の係数は 0x945e40b26ba1bf4d を 7 bit 左に回したもの (2^64 - 1 を法として 2^7 倍)
 const __uint128_t acc= 0x663d80ff99c27f * r0 + __uint128_t(0x2f205935d0dfa6ca) * r3 + 0x1000100010001ull * LN16.t[xf] + 0xffff0000ffff * r2;
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
