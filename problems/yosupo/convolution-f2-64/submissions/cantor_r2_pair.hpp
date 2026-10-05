#pragma once
// Cantor 基底の additive FFT (DIF、radix-2) で、隣り合う 2 つの butterfly を mul2 でまとめる版。古い fastest_full_dif_v2_loctbl
// (2026-09-30 の CI で x64 最速) と同じ形で、256 bit の clmul を Library の mul2<V> に替えた。x64 で VPCLMULQDQ があれば 256 bit の clmul を、無ければ (arm も) 64 bit の clmul を 2 回使う。
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
// Phase B: DIF の radix-2 butterfly (lo, hi) → (lo + s·hi, lo + (s+1)·hi) = (t, t ^ hi)。block 0 は s = 0 なので XOR だけ。
// 隣り合う 2 つの butterfly を mul2 (qword 0 と 2 の 2 lane) でまとめる。2 要素を読んで permute で qword 0 と 2 に広げ、
// 答えを permute で詰め直す (古い fastest_full_dif_v2_loctbl と同じ形を、Library の mul2<V> に載せ替えたもの)。
inline __m256i expand_pair(const u64* p) { return _mm256_permute4x64_epi64(_mm256_castsi128_si256(_mm_loadu_si128((const __m128i*)p)), _MM_SHUFFLE(3, 1, 2, 0)); }
inline __m128i pack_pair(const __m256i& v) { return _mm256_castsi256_si128(_mm256_permute4x64_epi64(v, _MM_SHUFFLE(3, 2, 2, 0))); }
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
   if(half >= 2) {
    const __m256i S= _mm256_set_epi64x(0, (long long)s, 0, (long long)s);
    for(int i= 0; i < half; i+= 2) {
     const __m256i hi= expand_pair(b + half + i), t= _mm256_xor_si256(expand_pair(b + i), mul2<V>(hi, S));
     _mm_storeu_si128((__m128i*)(b + i), pack_pair(t)), _mm_storeu_si128((__m128i*)(b + half + i), pack_pair(_mm256_xor_si256(t, hi)));
    }
   } else
    b[0]^= mul(b[1], s), b[1]^= b[0];
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
   if(half >= 2) {
    const __m256i S= _mm256_set_epi64x(0, (long long)s, 0, (long long)s);
    for(int i= 0; i < half; i+= 2) {
     const __m256i lo= expand_pair(b + i), h= _mm256_xor_si256(lo, expand_pair(b + half + i));
     _mm_storeu_si128((__m128i*)(b + half + i), pack_pair(h)), _mm_storeu_si128((__m128i*)(b + i), pack_pair(_mm256_xor_si256(lo, mul2<V>(h, S))));
    }
   } else
    b[1]^= b[0], b[0]^= mul(b[1], s);
  }
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
