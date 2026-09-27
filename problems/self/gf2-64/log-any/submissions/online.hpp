#pragma once
// クエリを 1 件ずつしか処理できない (run はクエリを n 回呼ぶだけ) という縛りで、per_prime の工夫を
// 1 件の中に収めた版。部品は naive.hpp と同じ (V3base_2 の ln() の中身)。
//
// 1. 解なしの判定を段階に分ける。ランダムな組の解なしの 99.7% は 65535 の約数で決まるので、a^M と
//    b^M の log を LNINV16 で引いた時点で判定して返す。残りの素数は、その成分を作ったところで見る。
// 2. 6700417 の成分が出たら、BSGS の最初の 12 歩ぶんの prefetch を a と b の両方で先に出し、表の
//    読み込みを待つあいだに 641 と 65537 の成分を引いて割る。
// 3. a と b の同じ計算は mul2 の 2 本の lane に並べる。スカラーの mul 8 本が mul2 4 本になる。線形写像
//    (F32 など) はスカラーのまま (V3base で linmap2 をやめているのに合わせた)。
// 4. 逆元はクエリごとに求める。65537 は Fermat の小定理 (掛け算 19 回)、6700417 は拡張ユークリッド。
//    per_prime の一括逆元はクエリをまたぐので使えない。
// そのほか、b = 1、a = b、a = 1 は先に返す。素数ごとに割ってから CRT を 1 回かけるのと、a と b の
// BSGS を 8 本の stream で混ぜるのは per_prime と同じ。
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
struct Ln6700417 {
 u64 t[524288];
 template <bool V> inline u32 solve(u64 target) const {
  const __m256i V_S01= _mm256_set_epi64x(0, 0x1489880b9cf723de, 0, 1);
  const __m256i V_S23= _mm256_set_epi64x(0, 0x7a8a7626c26ddc4d, 0, 0x5be693c8c2c557e3);
  const __m256i V_S4= _mm256_set1_epi64x(0xfdb44dcbca6522de);
  const __m256i tv= _mm256_set1_epi64x(target);
  __m256i A= mul2<V>(tv, V_S01), B= mul2<V>(tv, V_S23);  // (t0, t1), (t2, t3)
  __m256i An= mul2<V>(A, V_S4), Bn= mul2<V>(B, V_S4);
  __m256i An2= mul2<V>(An, V_S4), Bn2= mul2<V>(Bn, V_S4);
  u64 s[4], s_n[4], s_n2[4];
  std::tie(s[0], s[1])= unpack(A), std::tie(s[2], s[3])= unpack(B);
  std::tie(s_n[0], s_n[1])= unpack(An), std::tie(s_n[2], s_n[3])= unpack(Bn);
  std::tie(s_n2[0], s_n2[1])= unpack(An2), std::tie(s_n2[2], s_n2[3])= unpack(Bn2);
  for(int j= 0; j < 4; ++j) {
   _mm_prefetch((const char*)&t[u32(s[j]) & 524287], _MM_HINT_T0);
   _mm_prefetch((const char*)&t[u32(s_n[j]) & 524287], _MM_HINT_T0);
   _mm_prefetch((const char*)&t[u32(s_n2[j]) & 524287], _MM_HINT_T0);
  }
  for(u8 i= 0; i <= 52; i+= 4) {
   for(int j= 0; j < 4; ++j) {
    if(i + j > 52) break;
    u64 tt= s[j];
    u32 h= u32(tt) & 524287;
    while(t[h]) {
     u64 e= t[h];
     if(((e ^ tt) & 0xfffffffffff80000) == 0) return u32((i + j) * 131072 + u32(e & 524287) - 1);
     h= (h + 1) & 524287;
    }
   }
   A= An, B= Bn, An= An2, Bn= Bn2;
   for(int j= 0; j < 4; ++j) s[j]= s_n[j], s_n[j]= s_n2[j];
   if(i <= 40) {
    An2= mul2<V>(An, V_S4), Bn2= mul2<V>(Bn, V_S4);
    std::tie(s_n2[0], s_n2[1])= unpack(An2), std::tie(s_n2[2], s_n2[3])= unpack(Bn2);
    for(int j= 0; j < 4; ++j) _mm_prefetch((const char*)&t[u32(s_n2[j]) & 524287], _MM_HINT_T0);
   }
  }
  return 6700417;
 }
};
constexpr Ln6700417 LN6700417= []() {
 LinMap m= make_mul_table(0x00f542601703f991);
 Ln6700417 r{};
 u64 cur= 1;
 for(u32 j= 0; j < 131072; ++j, cur= m(cur)) {
  u32 h= u32(cur) & 524287;
  while(r.t[h]) h= (h + 1) & 524287;
  r.t[h]= (cur & 0xfffffffffff80000) | (u64(j) + 1);
 }
 return r;
}();
template <bool V> inline u64 ln(u64 x) {
 assert(x);
 const u64 x32= F32(x), n= mul(x, x32), fn= F16(n);
 auto [x_f16, w]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(x32), 0, n), _mm256_set1_epi64x(fn)));
 const u32 lnv= LNINV16.t[u16(x_f16)];
 const u64 s= mul(EMB(u16(lnv >> 16)), w), s7= F7(s), t2= sq(s7), t3= mul(s7, t2), t48= F4(t3);
 auto [t5, t51]= unpack(mul2<V>(_mm256_set_epi64x(0, t48, 0, t2), _mm256_set1_epi64x(t3)));
 auto [x_6700417, a]= unpack(mul2<V>(_mm256_set_epi64x(0, t51, 0, t5), _mm256_set1_epi64x(s)));
 u32 r3= LN6700417.solve<V>(x_6700417);
 auto [t72, b]= unpack(mul2<V>(_mm256_set_epi64x(0, F10(t51), 0, F3(t3)), _mm256_set_epi64x(0, a, 0, t48)));
 u64 r0= LN641(mul(t72, b)), r2= log_65537(n, fn);
 const __uint128_t acc= 0x663d80ff99c27f * r0 + __uint128_t(0x945e40b26ba1bf4d) * r3 + 0x1000100010001ull * u16(lnv) + 0xffff0000ffff * r2;
 const u64 lo= u64(acc), t= lo + u64(acc >> 64);
 return t + (t < lo);
}
}
#include <array>
#include <vector>
namespace gf2p64_internal {
// 2 つの対象の log を同時に探す BSGS (per_prime の solve2) を、最初の 12 歩ぶんの giant step を作って
// prefetch を出す start と、表を引く finish に分けたもの。あいだに別の計算を挟めるようにしてある。
template <bool V> struct Bsgs2 {
 static constexpr u32 NF= ~0u;
 __m256i An[2], Bn[2], An2[2], Bn2[2];
 u64 s[2][4], s_n[2][4], s_n2[2][4];
 inline void start(u64 ta, u64 tb) {
  const u64* t= LN6700417.t;
  const __m256i V_S01= _mm256_set_epi64x(0, 0x1489880b9cf723de, 0, 1);
  const __m256i V_S23= _mm256_set_epi64x(0, 0x7a8a7626c26ddc4d, 0, 0x5be693c8c2c557e3);
  const __m256i V_S4= _mm256_set1_epi64x(0xfdb44dcbca6522de);
  const u64 tg[2]= {ta, tb};
  for(int q= 0; q < 2; ++q) {
   const __m256i tv= _mm256_set1_epi64x(tg[q]);
   const __m256i A= mul2<V>(tv, V_S01), B= mul2<V>(tv, V_S23);  // (t0, t1), (t2, t3)
   An[q]= mul2<V>(A, V_S4), Bn[q]= mul2<V>(B, V_S4);
   An2[q]= mul2<V>(An[q], V_S4), Bn2[q]= mul2<V>(Bn[q], V_S4);
   std::tie(s[q][0], s[q][1])= unpack(A), std::tie(s[q][2], s[q][3])= unpack(B);
   std::tie(s_n[q][0], s_n[q][1])= unpack(An[q]), std::tie(s_n[q][2], s_n[q][3])= unpack(Bn[q]);
   std::tie(s_n2[q][0], s_n2[q][1])= unpack(An2[q]), std::tie(s_n2[q][2], s_n2[q][3])= unpack(Bn2[q]);
   for(int j= 0; j < 4; ++j) {
    _mm_prefetch((const char*)&t[u32(s[q][j]) & 524287], _MM_HINT_T0);
    _mm_prefetch((const char*)&t[u32(s_n[q][j]) & 524287], _MM_HINT_T0);
    _mm_prefetch((const char*)&t[u32(s_n2[q][j]) & 524287], _MM_HINT_T0);
   }
  }
 }
 inline std::pair<u32, u32> finish() {
  const u64* t= LN6700417.t;
  const __m256i V_S4= _mm256_set1_epi64x(0xfdb44dcbca6522de);
  u32 r[2]= {NF, NF};
  for(u8 i= 0; i <= 52; i+= 4) {
   for(int q= 0; q < 2; ++q) {
    for(int j= 0; j < 4 && i + j <= 52 && r[q] == NF; ++j) {
     const u64 tt= s[q][j];
     for(u32 h= u32(tt) & 524287; t[h]; h= (h + 1) & 524287)
      if(((t[h] ^ tt) & 0xfffffffffff80000) == 0) {
       r[q]= u32((i + j) * 131072 + u32(t[h] & 524287) - 1);
       break;
      }
    }
   }
   if(r[0] != NF && r[1] != NF) break;
   for(int q= 0; q < 2; ++q) {
    if(r[q] != NF) continue;
    An[q]= An2[q], Bn[q]= Bn2[q];
    for(int j= 0; j < 4; ++j) s[q][j]= s_n[q][j], s_n[q][j]= s_n2[q][j];
    if(i <= 40) {
     An2[q]= mul2<V>(An[q], V_S4), Bn2[q]= mul2<V>(Bn[q], V_S4);
     std::tie(s_n2[q][0], s_n2[q][1])= unpack(An2[q]), std::tie(s_n2[q][2], s_n2[q][3])= unpack(Bn2[q]);
     for(int j= 0; j < 4; ++j) _mm_prefetch((const char*)&t[u32(s_n2[q][j]) & 524287], _MM_HINT_T0);
    }
   }
  }
  return {r[0], r[1]};
 }
};
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
 // 2 段目: x^(2^32-1) から 6700417 の成分を作り、BSGS の prefetch を先に出す
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
 const bool bsgs= ya != 1 && yb != 1;  // どちらかが 1 なら k mod 6700417 は 0 でよい
 Bsgs2<V> st;
 if(bsgs) st.start(ya, yb);
 // 3 段目 (prefetch を待つあいだ): 641 と 65537 の成分を引いて割る
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
 // 4 段目: BSGS。3883315 = ((2^64-1)/6700417)^-1 mod 6700417
 if(bsgs) {
  const auto [a6, b6]= st.finish();
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
