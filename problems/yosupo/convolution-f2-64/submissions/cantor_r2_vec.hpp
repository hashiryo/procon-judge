#pragma once
// Cantor 基底の additive FFT (DIF、radix-2) で、隣り合う 4 つの butterfly と各点の積を mul4 (clmul を imm 0x00 と 0x11 で 2 回、
// 4 つまとめて畳む) でまとめる版。4 要素をそのまま読み書きでき、cantor_r2_pair の permute が要らない。x64 で VPCLMULQDQ があれば 256 bit の clmul を、無ければ (arm も) 64 bit の clmul を 2 回使う。
// 掛け算の部品 (cmul、mul、mul2) は Library (6f8462e9c) の GF2p64.hpp から写した。
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
template <bool V> inline __m256i mul4(const __m256i& a, const __m256i& b) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 __m256i p0, p1;
 if constexpr(V) p0= _mm256_clmulepi64_epi128(a, b, 0x00), p1= _mm256_clmulepi64_epi128(a, b, 0x11);
 else {
  const __m128i al= _mm256_castsi256_si128(a), ah= _mm256_extracti128_si256(a, 1), bl= _mm256_castsi256_si128(b), bh= _mm256_extracti128_si256(b, 1);
  p0= _mm256_setr_m128i(_mm_clmulepi64_si128(al, bl, 0x00), _mm_clmulepi64_si128(ah, bh, 0x00));
  p1= _mm256_setr_m128i(_mm_clmulepi64_si128(al, bl, 0x11), _mm_clmulepi64_si128(ah, bh, 0x11));
 }
 const __m256i lo= _mm256_unpacklo_epi64(p0, p1), h= _mm256_unpackhi_epi64(p0, p1), d= _mm256_xor_si256(h, _mm256_slli_epi64(h, 1));
 return _mm256_xor_si256(_mm256_xor_si256(lo, _mm256_shuffle_epi8(RED256, _mm256_srli_epi64(h, 60))), _mm256_xor_si256(d, _mm256_slli_epi64(d, 3)));
}
inline __m256i ld(const u64* p) { return _mm256_loadu_si256((const __m256i*)p); }
inline void st(u64* p, const __m256i& v) { _mm256_storeu_si256((__m256i*)p, v); }
inline __m256i bc(u64 s) { return _mm256_set1_epi64x((long long)s); }
// Phase B: DIF の radix-2 butterfly (lo, hi) → (lo + s·hi, lo + (s+1)·hi) = (t, t ^ hi)。block 0 は s = 0 なので XOR だけ。
// block の中では s が一定なので、隣り合う 4 つの butterfly を mul4 でまとめる (half < 4 の段はスカラ)。
template <bool V> inline void fft(u64* f, int n) {
 if(n <= 1) return;
 bc_to_lch(f, n);
 const u64* M= TW.m.data();
 for(int k= msb(n); k >= 1; --k) {
  const int unit= 1 << k, half= unit >> 1;
  for(int i= 0; i < half; ++i) f[half + i]^= f[i];
  for(int j= 1, nb= n >> k; j < nb; ++j) {
   u64* b= f + (size_t)j * unit;
   const u64 s= M[j];
   if(half >= 4) {
    const __m256i S= bc(s);
    for(int i= 0; i < half; i+= 4) {
     const __m256i hi= ld(b + half + i), t= _mm256_xor_si256(ld(b + i), mul4<V>(hi, S));
     st(b + i, t), st(b + half + i, _mm256_xor_si256(t, hi));
    }
   } else
    for(int i= 0; i < half; ++i) b[i]^= mul(b[half + i], s), b[half + i]^= b[i];
  }
 }
}
template <bool V> inline void ifft(u64* f, int n) {
 if(n <= 1) return;
 const u64* M= TW.m.data();
 for(int k= 1, d= msb(n); k <= d; ++k) {
  const int unit= 1 << k, half= unit >> 1;
  for(int i= 0; i < half; ++i) f[half + i]^= f[i];
  for(int j= 1, nb= n >> k; j < nb; ++j) {
   u64* b= f + (size_t)j * unit;
   const u64 s= M[j];
   if(half >= 4) {
    const __m256i S= bc(s);
    for(int i= 0; i < half; i+= 4) {
     const __m256i lo= ld(b + i), h= _mm256_xor_si256(lo, ld(b + half + i));
     st(b + half + i, h), st(b + i, _mm256_xor_si256(lo, mul4<V>(h, S)));
    }
   } else
    for(int i= 0; i < half; ++i) b[half + i]^= b[i], b[i]^= mul(b[half + i], s);
  }
 }
 bc_to_mono(f, n);
}
template <bool V> inline void pointwise(u64* f, const u64* g, int n) {
 int i= 0;
 for(; i + 4 <= n; i+= 4) st(f + i, mul4<V>(ld(f + i), ld(g + i)));
 for(; i < n; ++i) f[i]= mul(f[i], g[i]);
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
