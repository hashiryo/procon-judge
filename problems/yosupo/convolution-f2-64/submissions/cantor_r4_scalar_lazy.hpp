#pragma once
// cantor_r4_scalar の radix-4 の butterfly で、4 回の積のうち 1 回は畳まずに 128 bit のまま足し、畳む回数を 4 回から 3 回にした版。
// 古い memo.md の「4 基底なら reduction を遅延できる」の案。掛け算の部品 (cmul、mul、mul2) は Library (6f8462e9c) の GF2p64.hpp から写した。
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
constexpr u64 cmul(u64 a, u64 b) {
 u64 r= 0;
 for(int i= 64; i--;) r= r << 1 ^ (0x1b & -(r >> 63)) ^ (a & -(b >> i & 1));
 return r;
}
inline u64 mul(u64 a, u64 b) {
 static constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 __m128i v= _mm_clmulepi64_si128(_mm_cvtsi64_si128(a), _mm_cvtsi64_si128(b), 0);
 u64 h= v[1], d= h ^ (h << 1);
 return v[0] ^ RED[h >> 60] ^ d ^ (d << 3);
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
}
#include <array>
#include <vector>
namespace conv_f2_64 {
using namespace gf2p64_internal;
// Cantor 基底。β_l = CHAIN[62-l]、β_l^2 + β_l = β_{l-1}、β_0 = 1 (CHAIN[0] = x から x^2 + x を繰り返す)。
constexpr int CHAIN_LEN= 63;
constexpr std::array<u64, CHAIN_LEN> CHAIN= []() {
 std::array<u64, CHAIN_LEN> c{};
 c[0]= 2;
 for(int k= 1; k < CHAIN_LEN; ++k) c[k]= cmul(c[k - 1], c[k - 1]) ^ c[k - 1];
 return c;
}();
inline int msb(u64 n) { return 63 - __builtin_clzll(n); }
// DIF の twiddle。M[j] = Σ_{j の bit L が立つ} β_{L+1} (j < 2^(d-1)) で、段によらず 1 本で足りる。要る長さまで伸ばす。
struct Twiddle {
 std::vector<u64> m;
 void init(int d) {
  const size_t sz= d <= 1 ? 1 : size_t(1) << (d - 1);
  if(m.size() >= sz) return;
  m.assign(sz, 0);
  for(size_t j= 1; j < sz; ++j) m[j]= m[j & (j - 1)] ^ CHAIN[61 - __builtin_ctzll(j)];
 }
};
inline Twiddle TW;
// Phase A: 単項式の係数を LCH の基底 (自然順) に移す。Cantor 基底では部分空間の多項式が s_k(x) = Σ_{l ⊆ k} x^(2^l) と 0/1 の
// 係数だけになるので、s_{k-1} で割る操作が XOR だけで済む。level - 1 の bit が 1 つなら、割る相手の項は 1 つだけ。
inline void bc_to_lch(u64* p, int n) {
 for(int level= msb(n); level >= 2; --level) {
  const int len= 1 << level, half= len >> 1, sub= level - 1;
  if(__builtin_popcount(sub) == 1) {
   for(int base= 0; base < n; base+= len)
    for(int i= half - 1; i >= 0; --i) p[base + i + 1]^= p[base + half + i];
  } else {
   int subs[16], sn= 0;
   for(int s= sub;;) {
    s= (s - 1) & sub, subs[sn++]= 1 << s;
    if(!s) break;
   }
   for(int base= 0; base < n; base+= len)
    for(int i= half - 1; i >= 0; --i)
     if(const u64 q= p[base + half + i])
      for(int t= 0; t < sn; ++t) p[base + i + subs[t]]^= q;
  }
 }
}
// bc_to_lch の逆。XOR を逆の順に流す。
inline void bc_to_mono(u64* p, int n) {
 for(int level= 2, d= msb(n); level <= d; ++level) {
  const int len= 1 << level, half= len >> 1, sub= level - 1;
  if(__builtin_popcount(sub) == 1) {
   for(int base= 0; base < n; base+= len)
    for(int i= 0; i < half; ++i) p[base + i + 1]^= p[base + half + i];
  } else {
   int subs[16], sn= 0;
   for(int s= sub;;) {
    s= (s - 1) & sub, subs[sn++]= 1 << s;
    if(!s) break;
   }
   for(int base= 0; base < n; base+= len)
    for(int i= 0; i < half; ++i)
     if(const u64 q= p[base + half + i])
      for(int t= 0; t < sn; ++t) p[base + i + subs[t]]^= q;
  }
 }
}
// Phase B: DIF の radix-4 butterfly。段 k と k-1 を 1 回で進め、要素を読み書きする回数を radix-2 の半分にする (掛け算の数は同じ)。
// 4 分の 1 ずつの Q0..Q3 から、v = M[j] で A0 = Q0 + v·Q2、A1 = Q1 + v·Q3、A2 = A0 + Q2、A3 = A1 + Q3 を作り、u = M[2j] で
// R0 = A0 + u·A1、R1 = R0 + A1、u' = M[2j+1] で R2 = A2 + u'·A3、R3 = R2 + A3 とする。block 0 は v = u = 0。段の数が奇数なら
// 最後に radix-2 を 1 段。古い fastest_simple_dif_radix4_loctbl と同じ手順。
inline void r4_block(u64* b, int q, u64 v, u64 ul, u64 uh, bool first) {
 for(int i= 0; i < q; ++i) {
  const u64 Q0= b[i], Q1= b[q + i], Q2= b[2 * q + i], Q3= b[3 * q + i];
  const u64 A0= first ? Q0 : Q0 ^ mul(Q2, v), A1= first ? Q1 : Q1 ^ mul(Q3, v), A2= A0 ^ Q2, A3= A1 ^ Q3;
  const u64 R0= first ? A0 : A0 ^ mul(A1, ul), R2= A2 ^ mul(A3, uh);
  b[i]= R0, b[q + i]= R0 ^ A1, b[2 * q + i]= R2, b[3 * q + i]= R2 ^ A3;
 }
}
inline void r4_block_inv(u64* b, int q, u64 v, u64 ul, u64 uh, bool first) {
 for(int i= 0; i < q; ++i) {
  const u64 R0= b[i], R1= b[q + i], R2= b[2 * q + i], R3= b[3 * q + i];
  const u64 A3= R2 ^ R3, A2= R2 ^ mul(A3, uh), A1= R0 ^ R1, A0= first ? R0 : R0 ^ mul(A1, ul), Q2= A0 ^ A2, Q3= A1 ^ A3;
  b[i]= first ? A0 : A0 ^ mul(Q2, v), b[q + i]= first ? A1 : A1 ^ mul(Q3, v), b[2 * q + i]= Q2, b[3 * q + i]= Q3;
 }
}
// 畳む回数を減らす: 積は 128 bit のまま XOR でき、畳む操作は線型なので、足してから畳んでも答えは同じ。順変換では v·Q2 が
// 次の掛け算に入らないので 128 bit のまま持ち、R0 = Q0 + 畳み(v·Q2 + u·A1)、R2 = Q0 + Q2 + 畳み(v·Q2 + u'·A3) とする
// (畳むのは v·Q3、R0、R2 の 3 回で、今までの 4 回より 1 回少ない)。逆変換では Q2 = R0 + R2 + 畳み(u·A1 + u'·A3)、
// Q0 = R0 + 畳み(u·A1 + v·Q2)、Q1 = A1 + 畳み(v·Q3) とする (こちらも 3 回)。block 0 は積が 1 つだけなので今までの形。
inline __m128i clm(u64 a, u64 b) { return _mm_clmulepi64_si128(_mm_cvtsi64_si128(a), _mm_cvtsi64_si128(b), 0); }
inline u64 red(const __m128i& v) {
 static constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 const u64 h= v[1], d= h ^ (h << 1);
 return v[0] ^ RED[h >> 60] ^ d ^ (d << 3);
}
inline void r4_lazy(u64* b, int q, u64 v, u64 ul, u64 uh) {
 for(int i= 0; i < q; ++i) {
  const u64 Q0= b[i], Q1= b[q + i], Q2= b[2 * q + i], Q3= b[3 * q + i];
  const __m128i P2= clm(Q2, v);
  const u64 A1= Q1 ^ red(clm(Q3, v)), A3= A1 ^ Q3, R0= Q0 ^ red(_mm_xor_si128(P2, clm(A1, ul))), R2= Q0 ^ Q2 ^ red(_mm_xor_si128(P2, clm(A3, uh)));
  b[i]= R0, b[q + i]= R0 ^ A1, b[2 * q + i]= R2, b[3 * q + i]= R2 ^ A3;
 }
}
inline void r4_lazy_inv(u64* b, int q, u64 v, u64 ul, u64 uh) {
 for(int i= 0; i < q; ++i) {
  const u64 R0= b[i], R1= b[q + i], R2= b[2 * q + i], R3= b[3 * q + i], A3= R2 ^ R3, A1= R0 ^ R1;
  const __m128i P1= clm(A1, ul);
  const u64 Q2= R0 ^ R2 ^ red(_mm_xor_si128(P1, clm(A3, uh))), Q3= A1 ^ A3;
  b[i]= R0 ^ red(_mm_xor_si128(P1, clm(Q2, v))), b[q + i]= A1 ^ red(clm(Q3, v)), b[2 * q + i]= Q2, b[3 * q + i]= Q3;
 }
}
template <bool V> inline void fft(u64* f, int n) {
 if(n <= 1) return;
 bc_to_lch(f, n);
 const u64* M= TW.m.data();
 int k= msb(n);
 for(; k >= 2; k-= 2)
  for(int j= 0, nb= n >> k, q= 1 << (k - 2); j < nb; ++j) {
   u64* b= f + (size_t)j * (4 * q);
   if(!j) r4_block(b, q, 0, 0, M[1], true);
   else r4_lazy(b, q, M[j], M[2 * j], M[2 * j + 1]);
  }
 if(k == 1)
  for(int j= 0, nb= n >> 1; j < nb; ++j) {
   u64* b= f + (size_t)j * 2;
   if(j) b[0]^= mul(b[1], M[j]);
   b[1]^= b[0];
  }
}
template <bool V> inline void ifft(u64* f, int n) {
 if(n <= 1) return;
 const u64* M= TW.m.data();
 const int d= msb(n);
 if(d & 1)
  for(int j= 0, nb= n >> 1; j < nb; ++j) {
   u64* b= f + (size_t)j * 2;
   b[1]^= b[0];
   if(j) b[0]^= mul(b[1], M[j]);
  }
 for(int k= (d & 1) ? 3 : 2; k <= d; k+= 2)
  for(int j= 0, nb= n >> k, q= 1 << (k - 2); j < nb; ++j) {
   u64* b= f + (size_t)j * (4 * q);
   if(!j) r4_block_inv(b, q, 0, 0, M[1], true);
   else r4_lazy_inv(b, q, M[j], M[2 * j], M[2 * j + 1]);
  }
 bc_to_mono(f, n);
}
template <bool V> inline void pointwise(u64* f, const u64* g, int n) {
 for(int i= 0; i < n; ++i) f[i]= mul(f[i], g[i]);
}
template <bool V> inline std::vector<u64> convolve(std::vector<u64> f, std::vector<u64> g) {
 const int n= f.size(), m= g.size();
 if(!n || !m) return {};
 int s= 1;
 while(s < n + m - 1) s<<= 1;
 f.resize(s), g.resize(s), TW.init(msb(s));
 fft<V>(f.data(), s), fft<V>(g.data(), s), pointwise<V>(f.data(), g.data(), s), ifft<V>(f.data(), s);
 f.resize(n + m - 1);
 return f;
}
}  // namespace conv_f2_64
inline std::vector<u64> run(int, int, const std::vector<u64>& a, const std::vector<u64>& b) {
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return conv_f2_64::convolve<1>(a, b);
#endif
 return conv_f2_64::convolve<0>(a, b);
}
