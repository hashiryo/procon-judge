#pragma once
// online_rho の、実行時に表を作る前計算を速くした版。表の中身は online_rho とまったく同じなので、1 回の log の
// 速さも同じで、変わるのは起動時の前計算の時間だけ (計測区間の外)。
//
// online_rho は前計算の歩みを 1 本ずつ進めていて、掛け算の待ちと、歩みが終わるたびの分岐予測の外れが
// そのまま時間になっていた (前計算の 8 割)。ここでは 8 本を並べ、分岐を使わずに進める。
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
 for(u32 k= 65535; k--;) r.t[id[k]]= (u32(id[k ? 65535 - k : 0]) << 16) | (u32(k) * 49826 % 65535);
 return r;
}();
template <bool V> inline u64 iv(u64 a) {
 assert(a);
 u64 a32= F32(a), b= mul(a, a32);
 auto [g, c]= unpack(mul2<V>(_mm256_set_epi64x(0, b, 0, a32), _mm256_set1_epi64x(F16(b))));
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
 LinMap m= make_mul_table(0x6bf808f7824282a2);
 for(u64 k= 0, cur= 1; k < 641; ++k, cur= m(cur)) h.t[u16((cur * 0xffef5fb99f1bf6e7) >> 50)]= k * 590 % 641;
 return h;
}();
constexpr u16 PHI_B[16]= {49349, 60640, 60091, 52204, 8753, 26688, 50952, 24030, 14026, 41051, 57150, 31936, 39252, 22252, 63476, 55223};
constexpr LinMap16<u16> PHI= LinMap16<u16>(PHI_B);
struct ClassTable65537 {
 u32 t[65535];
 u32 K0;
};
constexpr ClassTable65537 CLS65537= []() {
 ClassTable65537 r{};
 u64 cur= 1;
 u32 v= 0;
 LinMap m= make_mul_table(0x1c1e79669b95a7ce);
 for(u32 k= 0; k < 65537; ++k, cur= m(cur)) {
  const u64 fr= F16(cur);
  const u16 b1= cur ^ fr, b0= cur ^ PHI.t[0][u8(b1)] ^ PHI.t[1][b1 >> 8];
  if(b0 == 0) r.K0= v;
  else if(b1) {
   u32 idx= u16(LNINV16.t[b0]) + 65535 - u16(LNINV16.t[b1]);
   if(idx >= 65535) idx-= 65535;
   r.t[idx]= v;
  }
  v= v >= 32768 ? v - 32768 : v + 32769;
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
}
#include <algorithm>
#include <vector>
namespace gf2p64_internal {
// 位数 6700417 の部分群の log を、前計算した distinguished point の表とランダムウォークで求める
// (D. J. Bernstein, T. Lange, "Computing small discrete logarithms faster", 2012)。
//
// 歩き方は r-adding walk で、x の上位 5 bit で選んだ step[i] = G^s_i を掛けていき、x の bit 19..20 が
// 0 の点 (distinguished point。4 歩に 1 回ほど) で止まる。前計算ではランダムな出発点 G^v から 2^20 本
// 歩き、着いた distinguished point のうち多くの歩みが着いたもの 2^18 個を、log と一緒に表へ入れる。
// 問い合わせでは対象 y から同じ規則で歩き、表の点に着けば、そこまでに掛けた分を引いて log(y) が
// 出る。表に無い点に着いたら、出発点に G^restart_log を掛けてずらし、歩き直す。表の 1 点がそこへ
// 流れ込む道筋をまとめて代表するので、同じ 4 MB の表で BSGS (平均 26 回) よりずっと少ない表引きで
// 済む (手元の実験では 1 回の log あたり 6 歩、表引き 2 回)。
//
// 前計算は 300 万歩ほどあって constexpr の上限を超えるので、表は main の前に実行時に作る (計測区間の外)。
struct Rho6700417 {
 static constexpr u32 L= 6700417;
 static constexpr u64 G= 0x00f542601703f991;         // 位数 L の元 (V3base_2 の BSGS と同じ底)
 static constexpr u32 SLOTS= u32(1) << 19;           // 表の枠。x の下位 19 bit を枠の番号にする
 static constexpr int MAX_STEPS= 128;                // これより長く歩いたら、輪に入ったとみなして歩き直す
 static constexpr u64 LOG_MASK= (u64(1) << 23) - 1;  // 枠には log + 1 と、x の bit 23 以上を詰める
 u64 step[32];
 u32 step_log[32];
 u64 restart;  // 歩き直すたびに出発点へ掛ける元 G^restart_log
 u32 restart_log;
 u64 tab[SLOTS];
 // bit 19..20 は、枠の番号 (bit 0..18) とも、表に残す bit 23 以上とも重ならない
 static bool dist(u64 x) { return ((x >> 19) & 3) == 0; }
 // x が表にあれば log(x) + 1、無ければ 0
 u32 find(u64 x) const {
  for(u32 h= u32(x) & (SLOTS - 1); tab[h]; h= (h + 1) & (SLOTS - 1))
   if(((tab[h] ^ x) & ~LOG_MASK) == 0) return u32(tab[h] & LOG_MASK);
  return 0;
 }
 Rho6700417();
 u32 solve(u64 y) const;                            // 1 つの対象を 2 本の歩みで探す
 std::pair<u32, u32> solve2(u64 ya, u64 yb) const;  // 2 つの対象を 1 本ずつの歩みで探す
};
inline Rho6700417::Rho6700417(): step{}, step_log{}, restart(0), restart_log(0), tab{} {
 u64 seed= 0x2545f4914f6cdd1d;  // 決まった種の splitmix64
 const auto rnd= [&seed]() {
  u64 z= seed+= 0x9e3779b97f4a7c15;
  z= (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9, z= (z ^ (z >> 27)) * 0x94d049bb133111eb;
  return z ^ (z >> 31);
 };
 const auto pow_g= [](u64 e) {
  u64 r= 1;
  for(u64 a= G; e; e>>= 1, a= sq(a))
   if(e & 1) r= mul(r, a);
  return r;
 };
 for(int i= 0; i < 32; ++i) step_log[i]= u32(rnd() % (L - 1) + 1), step[i]= pow_g(step_log[i]);
 restart_log= u32(rnd() % (L - 1) + 1), restart= pow_g(restart_log);
 // ランダムな出発点から N 本歩いて、着いた distinguished point と、そこへ着いた本数を数える。
 // 1 本の歩みは掛け算が鎖になり、終わるたびに分岐予測も外れるので、1 本ずつ歩くと待ちがそのまま
 // 時間になる。そこで B 本を並べて進め、終わった枠にはすぐ次の出発点を入れて、どの枠も休ませない。
 // 枠の更新は分岐を使わず選択で書く。着いた点は BLK 本ぶんを出発点の番号の順に並べてから数えるので、
 // 1 本ずつ歩いたときと同じ順に数えることになり、表の中身は変わらない。
 // 作業用の表は log (23 bit) と着いた本数 (9 bit、511 で止める) を 1 つの u32 に詰めて、12 MB に抑える
 constexpr u32 T= SLOTS / 2, N= 4 * T, CAP= N, BLK= 4096;
 constexpr int B= 8;
 std::vector<u64> key(CAP), sx(BLK + 1), ex(BLK + 1);
 std::vector<u32> val(CAP), sv(BLK + 1), ev(BLK + 1);  // 添字 BLK は、書かなくてよい値の捨て場
 u32 v= u32(rnd() % L);
 const u32 dv= u32(rnd() % (L - 1) + 1);
 const u32 dvb= u32(u64(dv) * B % L);
 const u64 gdv= pow_g(dv), gdvb= pow_g(dvb);
 u64 x0= pow_g(v);
 for(u32 base= 0; base < N; base+= BLK) {
  // この組の出発点。gdv を順に掛けると 1 本の鎖になるので、最初の B 本のあとは gdv^B ずつの B 本の鎖で作る
  for(int j= 0; j < B; ++j) sx[j]= x0, sv[j]= v, x0= mul(x0, gdv), v= u32((u64(v) + dv) % L);
  for(u32 k= B; k < BLK; ++k) sx[k]= mul(sx[k - B], gdvb), sv[k]= u32((u64(sv[k - B]) + dvb) % L);
  x0= mul(sx[BLK - 1], gdv), v= u32((u64(sv[BLK - 1]) + dv) % L);
  u64 lx[B];
  u32 le[B], lk[B], ls[B], next= B, live= B;  // lk は歩いている出発点の番号 (BLK なら休み)
  for(int j= 0; j < B; ++j) lx[j]= sx[j], le[j]= sv[j], lk[j]= u32(j), ls[j]= 0;
  while(live)
   for(int j= 0; j < B; ++j) {
    const u64 x= lx[j];
    const int i= int(x >> 59);
    const u64 nx= mul(x, step[i]);
    const bool d= dist(x), fin= (d || ls[j] >= MAX_STEPS) && lk[j] < BLK, take= fin && next < BLK;
    const u32 w= fin ? lk[j] : BLK;
    ex[w]= d && ls[j] < MAX_STEPS ? x : 0, ev[w]= le[j];  // 長く歩きすぎた歩みは捨てる (online_rho と同じ)
    lx[j]= fin ? sx[take ? next : BLK] : nx;
    le[j]= fin ? sv[take ? next : BLK] : le[j] + step_log[i];
    ls[j]= fin ? 0 : ls[j] + 1;
    lk[j]= fin ? (take ? next : BLK) : lk[j];
    live-= fin && !take, next+= take;
   }
  for(u32 k= 0; k < BLK; ++k) {
   if(const u64 y= k + 16 < BLK ? ex[k + 16] : 0) {  // 16 本先の点の枠を先に読んでおく
    const u32 hy= u32((y * 0x9e3779b97f4a7c15) >> 44);
    _mm_prefetch((const char*)&key[hy], _MM_HINT_T0), _mm_prefetch((const char*)&val[hy], _MM_HINT_T0);
   }
   const u64 x= ex[k];
   if(!x) continue;
   u32 h= u32((x * 0x9e3779b97f4a7c15) >> 44);  // CAP = 2^20
   while(key[h] && key[h] != x) h= (h + 1) & (CAP - 1);
   if(!key[h]) key[h]= x, val[h]= ev[k] % L;
   if(val[h] < (u32(511) << 23)) val[h]+= u32(1) << 23;
  }
 }
 // 多くの歩みが着いた点ほど、問い合わせの歩みも着きやすい。上位 T 個を表に入れる
 std::vector<u32> idx;
 for(u32 h= 0; h < CAP; ++h)
  if(key[h]) idx.push_back(h);
 if(idx.size() > T) {
  std::nth_element(idx.begin(), idx.begin() + T, idx.end(), [&](u32 a, u32 b) { return val[a] >> 23 > val[b] >> 23; });
  idx.resize(T);
 }
 for(u32 h: idx) {
  u32 s= u32(key[h]) & (SLOTS - 1);
  while(tab[s]) s= (s + 1) & (SLOTS - 1);
  tab[s]= (key[h] & ~LOG_MASK) | ((val[h] & LOG_MASK) + 1);
 }
}
inline u32 Rho6700417::solve(u64 y) const {
 // 歩み 0 は y から、歩み 1 は y·G^restart_log から出る。歩き直すときは、次にずらした出発点をもらう
 u64 x[2]= {y, mul(y, restart)}, nx= mul(x[1], restart);
 u32 e[2]= {0, restart_log}, ne= u32(2 * u64(restart_log) % L);  // x[c] = y·G^e[c]
 int s[2]= {0, 0};
 for(;;)
  for(int c= 0; c < 2; ++c) {
   if(dist(x[c])) {
    if(const u32 f= find(x[c])) return u32((u64(f) - 1 + L - e[c] % L) % L);
   } else if(s[c] < MAX_STEPS) {
    const int i= int(x[c] >> 59);
    x[c]= mul(x[c], step[i]), e[c]+= step_log[i], ++s[c];
    continue;
   }
   // 表に無い点に着いたか、長く歩きすぎた。出発点をずらして歩き直す
   x[c]= nx, e[c]= ne, s[c]= 0;
   nx= mul(nx, restart), ne= u32((u64(ne) + restart_log) % L);
  }
}
inline std::pair<u32, u32> Rho6700417::solve2(u64 ya, u64 yb) const {
 constexpr u32 NF= ~0u;
 u64 x[2]= {ya, yb}, nx[2]= {mul(ya, restart), mul(yb, restart)};
 u32 e[2]= {0, 0}, ne[2]= {restart_log, restart_log}, r[2]= {NF, NF};  // x[c] = y_c·G^e[c]
 int s[2]= {0, 0};
 while(r[0] == NF || r[1] == NF)
  for(int c= 0; c < 2; ++c) {
   if(r[c] != NF) continue;
   if(dist(x[c])) {
    if(const u32 f= find(x[c])) {
     r[c]= u32((u64(f) - 1 + L - e[c] % L) % L);
     continue;
    }
   } else if(s[c] < MAX_STEPS) {
    const int i= int(x[c] >> 59);
    x[c]= mul(x[c], step[i]), e[c]+= step_log[i], ++s[c];
    continue;
   }
   x[c]= nx[c], e[c]= ne[c], s[c]= 0;
   nx[c]= mul(nx[c], restart), ne[c]= u32((u64(ne[c]) + restart_log) % L);
  }
 return {r[0], r[1]};
}
inline const Rho6700417 RHO6700417;  // main の前に作る
}  // namespace gf2p64_internal
#include <array>
namespace gf2p64_internal {
// 逆元の表に C を掛けたもの。0 の欄は 0 にしておくので、a の成分が 1 (log が 0) なら k の成分も 0 になる。
template <u32 P, u32 C= 1> constexpr std::array<u16, P> inv_table() {
 std::array<u16, P> r{};
 r[1]= 1;
 for(u32 i= 2; i < P; ++i) r[i]= u16((P - u64(P / i) * r[P % i] % P) % P);
 for(u32 i= 1; i < P; ++i) r[i]= u16(u64(r[i]) * C % P);
 return r;
}
constexpr auto INV3= inv_table<3>();
constexpr auto INV5= inv_table<5>();
constexpr auto INV17= inv_table<17>();
constexpr auto INV257= inv_table<257>();
constexpr auto INV641= inv_table<641, 590>();  // 590 = ((2^64-1)/641)^-1 mod 641
// x^-1 mod 65537 (x ≠ 0)。x^65535 を x^3 → x^15 → x^255 → x^65535 の順に組む (掛け算 19 回)。
inline u32 inv65537(u32 x) {
 const auto sqn= [](u64 v, int k) {
  while(k--) v= v * v % 65537;
  return v;
 };
 const u64 x3= u64(x) * x % 65537 * x % 65537, x15= sqn(x3, 2) * x3 % 65537, x255= sqn(x15, 4) * x15 % 65537;
 return u32(sqn(x255, 8) * x255 % 65537);
}
// x^-1 mod 6700417 (x ≠ 0)。拡張ユークリッド。23 bit に収まるので 32 bit の割り算で済む。
inline u32 inv6700417(u32 x) {
 u32 r0= 6700417, r1= x;
 int s0= 0, s1= 1;
 while(r1) {
  const u32 q= r0 / r1;
  std::tie(r0, r1)= std::make_pair(r1, r0 - q * r1);
  std::tie(s0, s1)= std::make_pair(s1, s0 - int(q) * s1);
 }
 return u32(s0 < 0 ? s0 + 6700417 : s0);
}
inline u64 fold(__uint128_t x) {
 const u64 lo= u64(x), t= lo + u64(x >> 64);
 return t + (t < lo);
}
// a^k = b となる k を 1 つ返す。無ければ 2^64-1。素数 p ごとの k の成分は、CRT の定数を掛けるだけで
// 済むよう ((2^64-1)/p)^-1 倍 (mod p) の形で持つ (per_prime と同じ)。
template <bool V> inline u64 query(u64 a, u64 b) {
 constexpr u64 M= ~0ull;
 if(b == 1) return 0;
 if(a == b) return 1;
 if(a == 1) return M;
 // 1 段目: a^M と b^M (F_2^16 の元) の log を引き、65535 の約数の部分で解なしならここで返す
 const u64 a32= F32(a), b32= F32(b);
 const auto [na, nb]= unpack(mul2<V>(_mm256_set_epi64x(0, b, 0, a), _mm256_set_epi64x(0, b32, 0, a32)));
 const u64 fna= F16(na), fnb= F16(nb);
 const __m256i fn2= _mm256_set_epi64x(0, fnb, 0, fna);
 const auto [ma, mb]= unpack(mul2<V>(_mm256_set_epi64x(0, nb, 0, na), fn2));
 const u32 la= LNINV16.t[u16(ma)], lb= LNINV16.t[u16(mb)];
 const u32 a3= u16(la) % 3, a5= u16(la) % 5, a17= u16(la) % 17, a257= u16(la) % 257;
 const u32 b3= u16(lb) % 3, b5= u16(lb) % 5, b17= u16(lb) % 17, b257= u16(lb) % 257;
 if((!a3 && b3) || (!a5 && b5) || (!a17 && b17) || (!a257 && b257)) return M;
 // 2 段目: x^(2^32-1) から 6700417 の成分を作る
 const auto [wa, wb]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(b32), 0, sq(a32)), fn2));
 const __m256i s2= mul2<V>(_mm256_set_epi64x(0, EMB(u16(lb >> 16)), 0, EMB(u16(la >> 16))), _mm256_set_epi64x(0, wb, 0, wa));
 const auto [sa, sb]= unpack(s2);
 const u64 s7a= F7(sa), s7b= F7(sb), t2a= sq(s7a), t2b= sq(s7b);
 const auto [t3a, t3b]= unpack(mul2<V>(_mm256_set_epi64x(0, s7b, 0, s7a), _mm256_set_epi64x(0, t2b, 0, t2a)));
 const u64 t48a= F4(t3a), t48b= F4(t3b);
 const __m256i t3v= _mm256_set_epi64x(0, t3b, 0, t3a);
 const auto [t5a, t5b]= unpack(mul2<V>(_mm256_set_epi64x(0, t2b, 0, t2a), t3v));
 const auto [t51a, t51b]= unpack(mul2<V>(_mm256_set_epi64x(0, t48b, 0, t48a), t3v));
 const auto [ya, yb]= unpack(mul2<V>(_mm256_set_epi64x(0, t5b, 0, t5a), s2));  // s^641
 if(ya == 1 && yb != 1) return M;
 const bool walk= ya != 1 && yb != 1;  // どちらかが 1 なら k mod 6700417 は 0 でよい
 // 3 段目: 641 と 65537 の成分を引いて割る。表を読む待ちが、このあとの歩みに重なる
 const auto [ua, ub]= unpack(mul2<V>(_mm256_set_epi64x(0, t51b, 0, t51a), s2));
 const auto [t72a, ca]= unpack(mul2<V>(_mm256_set_epi64x(0, F10(t51a), 0, F3(t3a)), _mm256_set_epi64x(0, ua, 0, t48a)));
 const auto [t72b, cb]= unpack(mul2<V>(_mm256_set_epi64x(0, F10(t51b), 0, F3(t3b)), _mm256_set_epi64x(0, ub, 0, t48b)));
 const auto [xa, xb]= unpack(mul2<V>(_mm256_set_epi64x(0, t72b, 0, t72a), _mm256_set_epi64x(0, cb, 0, ca)));  // s^6700417
 const u32 v0a= LN641(xa), v0b= LN641(xb), v2a= log_65537(na, fna), v2b= log_65537(nb, fnb);
 if((!v0a && v0b) || (!v2a && v2b)) return M;
 // 43690 などは 65535 の約数ごとの CRT の定数に 16384 を掛けたもの
 const u32 v1= (b3 * INV3[a3] % 3 * 43690 + b5 * INV5[a5] % 5 * 26214 + b17 * INV17[a17] % 17 * 3855 + b257 * INV257[a257] % 257 * 8160) % 65535;
 const u32 v0= v0b * INV641[v0a] % 641;
 const u64 v2= v2a ? u64(v2b) * 16384 % 65537 * inv65537(v2a) % 65537 : 0;
 __uint128_t acc= __uint128_t(0x1000100010001ull * v1) + 0x663d80ff99c27full * v0 + 0xffff0000ffffull * v2;
 // 4 段目: 6700417 の成分の log を、ランダムウォークで求める。3883315 = ((2^64-1)/6700417)^-1 mod 6700417
 if(walk) {
  const auto [a6, b6]= RHO6700417.solve2(ya, yb);
  acc+= 0x280fffffd7full * (u64(b6) * 3883315 % 6700417 * inv6700417(a6) % 6700417);
 }
 return fold(acc);
}
}  // namespace gf2p64_internal
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::query<1>(as[i], bs[i]);
 return ans;
}
