#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#include <simde/x86/bmi.h>
#endif
#include <tuple>
#include <utility>
#include <iostream>
#include <cassert>
#include "include/debug.hpp"
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
inline vector<u64> run(const vector<u64>& as, const vector<u64>& es) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2p64_internal::pw(as[i], es[i]);
 return ans;
}
