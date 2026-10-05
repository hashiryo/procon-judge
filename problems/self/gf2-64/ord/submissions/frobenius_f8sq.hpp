#pragma once
// frobenius.hpp と同じ手順を、Library の GF2p64 の部品 (名前空間の LinMap の F2、F4、F7、F8、F16、F32 と mul、sq) で書いた版。
// Library に ord() を入れるときの形。Library には F9 と F10 の LinMap が無いので、F9 = sq∘F8、F10 = sq∘sq∘F8 とする
// (frobenius.hpp は frob9 と frob10 の表を引く)。表は名前空間の LinMap だけで、log の表は使わない。
// Library (6f40504df) の GF2p64.hpp から、LinMap、mul、sq を写したもの。
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
}
#include <vector>
using namespace std;
using u64= unsigned long long;
namespace gf2_64_ord_frob_f8sq {
using namespace gf2p64_internal;
inline u64 ord(u64 a) {
 const u64 n= mul(a, F32(a)), fn= F16(n), m= mul(n, fn);
 const u64 f8= F8(m), t= mul(m, f8), f4= F4(t), u= mul(t, f4), f2= F2(u), v= mul(u, f2);
 const u64 c= mul(mul(a, F7(a)), sq(F8(a)));
 const u64 x3= mul(a, sq(a)), x15= mul(x3, F2(x3)), x51= mul(x3, F4(x3));
 const u64 d= mul(mul(sq(F16(x51)), sq(sq(F8(x15)))), mul(F7(x3), a));
 return u64(v != 1 ? 3 : 1) * (f2 != u ? 5 : 1) * (f4 != t ? 17 : 1) * (f8 != m ? 257 : 1) * (fn != n ? 65537 : 1) * (F32(d) != d ? 641 : 1) * (F32(c) != c ? 6700417 : 1);
}
}  // namespace gf2_64_ord_frob_f8sq
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2_64_ord_frob_f8sq::ord(as[i]);
 return ans;
}
