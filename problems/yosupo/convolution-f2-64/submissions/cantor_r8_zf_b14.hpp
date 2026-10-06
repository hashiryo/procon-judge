#pragma once
// cantor_r4_zf_b14 の butterfly を radix-8 (3 段を 1 回の読み書きで) にした版。畳む回数は 3 段 8 要素あたり 8 回 (radix-4 は 9 回)、
// block の中の 14 段は 5 pass (radix-4 は 7 pass) になる。余りの段は上に radix-4 を置く。以下は cantor_r4_zf_b14 と同じ。
// cantor_r4_vec_lazy に、Schönhage の提出の工夫を移した版。
// (1) 入力が長さ L = 2^k0 に収まるとき、上の d - k0 段は下半分を上半分に写すだけなので掛けずに飛ばし、最初の段で base から各 block へ直接書く。
//     基底変換も [0, L) だけで済む。入力を値で受けて resize で 0 埋めするのもやめる。
// (2) 基底変換を分岐なしの 4 本ずつの XOR にする。読む位置と書く位置の距離は half/2 以上あるので、level 4 以上は 4 本ずつ進めてよい。level 2, 3 は 8 個ずつまとめて回す。
// (3) 最下段の radix-4 (q = 1) は block ごとに twiddle が違うのでスカラーだった。block 4 つを 4x4 で転置し、lane ごとに別の twiddle で回す。
// (4) 深さ優先: 2^BB 個の block に入る段 (基底変換の level も) は block ごとにまとめて回す。f は block ごとに順変換の残り、g との各点の積、逆変換の最初までを続けて済ませる。
// 段の並べ方は plan を見よ。x64 で VPCLMULQDQ があれば 256 bit の clmul を、無ければ 64 bit の clmul を 2 回使う。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#endif
#include <algorithm>
#include <array>
#include <memory>
#include <vector>
namespace cantor_zf {
using u64= unsigned long long;
using u8= unsigned char;
constexpr int BB= 14;  // block の大きさ 2^BB
constexpr u64 cmul(u64 a, u64 b) {
 u64 r= 0;
 for(int i= 64; i--;) r= r << 1 ^ (0x1b & -(r >> 63)) ^ (a & -(b >> i & 1));
 return r;
}
inline __m128i clm(u64 a, u64 b) { return _mm_clmulepi64_si128(_mm_cvtsi64_si128((long long)a), _mm_cvtsi64_si128((long long)b), 0); }
inline u64 red(const __m128i& v) {
 static constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 const u64 h= v[1], d= h ^ (h << 1);
 return u64(v[0]) ^ RED[h >> 60] ^ d ^ (d << 3);
}
// Cantor 基底。β_l = CHAIN[62-l]、β_l^2 + β_l = β_{l-1}、β_0 = 1。
constexpr int CHAIN_LEN= 63;
constexpr std::array<u64, CHAIN_LEN> CHAIN= []() {
 std::array<u64, CHAIN_LEN> c{};
 c[0]= 2;
 for(int k= 1; k < CHAIN_LEN; ++k) c[k]= cmul(c[k - 1], c[k - 1]) ^ c[k - 1];
 return c;
}();
inline int msb(u64 n) { return 63 - __builtin_clzll(n); }
inline int clog2(u64 n) { return n <= 1 ? 0 : msb(n - 1) + 1; }
// DIF の twiddle。M[j] = Σ_{j の bit L が立つ} β_{L+1}。
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
inline __m256i ld(const u64* p) { return _mm256_loadu_si256((const __m256i*)p); }
inline void st(u64* p, const __m256i& v) { _mm256_storeu_si256((__m256i*)p, v); }
inline __m256i bc(u64 s) { return _mm256_set1_epi64x((long long)s); }
inline __m256i x(const __m256i& a, const __m256i& b) { return _mm256_xor_si256(a, b); }
// 4 つの積を畳む前の形 (下位 64 bit の 4 つと上位 64 bit の 4 つ) で持つ。
struct U4 {
 __m256i lo, hi;
};
template <bool V> inline U4 clm4(const __m256i& a, const __m256i& b) {
 __m256i p0, p1;
 if constexpr(V) p0= _mm256_clmulepi64_epi128(a, b, 0x00), p1= _mm256_clmulepi64_epi128(a, b, 0x11);
 else {
  const __m128i al= _mm256_castsi256_si128(a), ah= _mm256_extracti128_si256(a, 1), bl= _mm256_castsi256_si128(b), bh= _mm256_extracti128_si256(b, 1);
  p0= _mm256_setr_m128i(_mm_clmulepi64_si128(al, bl, 0x00), _mm_clmulepi64_si128(ah, bh, 0x00));
  p1= _mm256_setr_m128i(_mm_clmulepi64_si128(al, bl, 0x11), _mm_clmulepi64_si128(ah, bh, 0x11));
 }
 return {_mm256_unpacklo_epi64(p0, p1), _mm256_unpackhi_epi64(p0, p1)};
}
inline U4 operator^(const U4& a, const U4& b) { return {x(a.lo, b.lo), x(a.hi, b.hi)}; }
inline __m256i red4(const U4& p) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 const __m256i d= x(p.hi, _mm256_slli_epi64(p.hi, 1));
 return x(x(p.lo, _mm256_shuffle_epi8(RED256, _mm256_srli_epi64(p.hi, 60))), x(d, _mm256_slli_epi64(d, 3)));
}
template <bool V> inline __m256i mul4(const __m256i& a, const __m256i& b) { return red4(clm4<V>(a, b)); }
// 小さいときの素朴な積 (畳むのは出力ごとに 1 回)。
inline std::vector<u64> naive(const std::vector<u64>& a, const std::vector<u64>& b) {
 const int n= a.size(), m= b.size(), N= n + m - 1;
 std::vector<u64> c(N);
 for(int k= 0; k < N; ++k) {
  __m128i t= _mm_setzero_si128();
  for(int i= std::max(0, k - m + 1); i <= std::min(k, n - 1); ++i) t= _mm_xor_si128(t, clm(a[i], b[k - i]));
  c[k]= red(t);
 }
 return c;
}
// ---------------- 基底変換 ----------------
// level の割る多項式 s_{level-1} の、x^half 以外の項の指数 2^l (l は level-1 の真部分集合)。
inline int shifts_of(int level, int* sh) {
 const int sub= level - 1;
 int sn= 0;
 for(int s= sub;;) {
  s= (s - 1) & sub, sh[sn++]= 1 << s;
  if(!s) break;
 }
 return sn;
}
// 単項式 → LCH の 1 level (level >= 4) を [lo, hi) で。後ろから 4 本ずつ。
inline void lch_level(u64* p, size_t lo, size_t hi, int level) {
 int sh[16];
 const int sn= shifts_of(level, sh), half= 1 << (level - 1);
 for(size_t base= lo; base < hi; base+= size_t(2) * half)
  for(int c= half - 4; c >= 0; c-= 4) {
   u64* b= p + base + c;
   const __m256i q= ld(b + half);
   for(int t= 0; t < sn; ++t) st(b + sh[t], x(ld(b + sh[t]), q));
  }
}
inline void mono_level(u64* p, size_t lo, size_t hi, int level) {
 int sh[16];
 const int sn= shifts_of(level, sh), half= 1 << (level - 1);
 for(size_t base= lo; base < hi; base+= size_t(2) * half)
  for(int c= 0; c < half; c+= 4) {
   u64* b= p + base + c;
   const __m256i q= ld(b + half);
   for(int t= 0; t < sn; ++t) st(b + sh[t], x(ld(b + sh[t]), q));
  }
}
// level 3, 2 を 8 個ずつ (level 3: i = 3..0 で e[i+1] ^= e[4+i]、level 2: 4 個ずつ e[2] ^= e[3], e[1] ^= e[2])。
inline void lch_low(u64* p, size_t lo, size_t hi) {
 for(size_t i= lo; i < hi; i+= 8) {
  u64* e= p + i;
  u64 e1= e[1], e2= e[2], e3= e[3], e4= e[4], e5= e[5], e6= e[6], e7= e[7];
  e4^= e7, e3^= e6, e2^= e5, e1^= e4;
  e2^= e3, e1^= e2, e6^= e7, e5^= e6;
  e[1]= e1, e[2]= e2, e[3]= e3, e[4]= e4, e[5]= e5, e[6]= e6;
 }
}
inline void mono_low(u64* p, size_t lo, size_t hi) {
 for(size_t i= lo; i < hi; i+= 8) {
  u64* e= p + i;
  u64 e1= e[1], e2= e[2], e3= e[3], e4= e[4], e5= e[5], e6= e[6], e7= e[7];
  e1^= e2, e2^= e3, e5^= e6, e6^= e7;
  e1^= e4, e2^= e5, e3^= e6, e4^= e7;
  e[1]= e1, e[2]= e2, e[3]= e3, e[4]= e4, e[5]= e5, e[6]= e6;
 }
}
// [0, 2^k0) を単項式から LCH へ (k0 >= 4)。2^bb より大きい level は全体で、残りは 2^bb の block ごとに。
inline void bc_lch(u64* p, int k0, int bb) {
 const int B= std::min(bb, k0);
 const size_t L= size_t(1) << k0, BS= size_t(1) << B;
 for(int level= k0; level > B; --level) lch_level(p, 0, L, level);
 for(size_t s= 0; s < L; s+= BS) {
  for(int level= B; level >= 4; --level) lch_level(p, s, s + BS, level);
  lch_low(p, s, s + BS);
 }
}
inline void bc_mono(u64* p, int d, int bb) {
 const int B= std::min(bb, d);
 const size_t S= size_t(1) << d, BS= size_t(1) << B;
 for(size_t s= 0; s < S; s+= BS) {
  mono_low(p, s, s + BS);
  for(int level= 4; level <= B; ++level) mono_level(p, s, s + BS, level);
 }
 for(int level= B + 1; level <= d; ++level) mono_level(p, 0, S, level);
}
// ---------------- butterfly ----------------
// DIF の radix-4 (段 k と k-1)。block 0 は v = u = 0 なので掛け算が 1 つ。順変換は読む場所 s と書く場所 b を分けられる (上の段を飛ばしたとき、
// どの block も base から読む)。畳む回数は lazy に 3 回 (cantor_r4_vec_lazy と同じ)。
template <bool V> inline void r4f_first(const u64* s, u64* b, int q, u64 uh) {
 const __m256i Uh= bc(uh);
 for(int i= 0; i < q; i+= 4) {
  const __m256i Q0= ld(s + i), Q1= ld(s + q + i), Q2= ld(s + 2 * q + i), Q3= ld(s + 3 * q + i);
  const __m256i A3= x(Q1, Q3), R2= x(x(Q0, Q2), mul4<V>(A3, Uh));
  st(b + i, Q0), st(b + q + i, x(Q0, Q1)), st(b + 2 * q + i, R2), st(b + 3 * q + i, x(R2, A3));
 }
}
template <bool V> inline void r4f_lazy(const u64* s, u64* b, int q, u64 v, u64 ul, u64 uh) {
 const __m256i Vv= bc(v), Ul= bc(ul), Uh= bc(uh);
 for(int i= 0; i < q; i+= 4) {
  const __m256i Q0= ld(s + i), Q1= ld(s + q + i), Q2= ld(s + 2 * q + i), Q3= ld(s + 3 * q + i);
  const U4 P2= clm4<V>(Q2, Vv);
  const __m256i A1= x(Q1, red4(clm4<V>(Q3, Vv))), A3= x(A1, Q3);
  const __m256i R0= x(Q0, red4(P2 ^ clm4<V>(A1, Ul))), R2= x(x(Q0, Q2), red4(P2 ^ clm4<V>(A3, Uh)));
  st(b + i, R0), st(b + q + i, x(R0, A1)), st(b + 2 * q + i, R2), st(b + 3 * q + i, x(R2, A3));
 }
}
template <bool V> inline void r4i_first(u64* b, int q, u64 uh) {
 const __m256i Uh= bc(uh);
 for(int i= 0; i < q; i+= 4) {
  const __m256i R0= ld(b + i), R1= ld(b + q + i), R2= ld(b + 2 * q + i), R3= ld(b + 3 * q + i);
  const __m256i A3= x(R2, R3), A1= x(R0, R1), Q2= x(x(R0, R2), mul4<V>(A3, Uh));
  st(b + q + i, A1), st(b + 2 * q + i, Q2), st(b + 3 * q + i, x(A1, A3));
 }
}
template <bool V> inline void r4i_lazy(u64* b, int q, u64 v, u64 ul, u64 uh) {
 const __m256i Vv= bc(v), Ul= bc(ul), Uh= bc(uh);
 for(int i= 0; i < q; i+= 4) {
  const __m256i R0= ld(b + i), R1= ld(b + q + i), R2= ld(b + 2 * q + i), R3= ld(b + 3 * q + i), A3= x(R2, R3), A1= x(R0, R1);
  const U4 P1= clm4<V>(A1, Ul);
  const __m256i Q2= x(x(R0, R2), red4(P1 ^ clm4<V>(A3, Uh))), Q3= x(A1, A3);
  st(b + i, x(R0, red4(P1 ^ clm4<V>(Q2, Vv)))), st(b + q + i, x(A1, red4(clm4<V>(Q3, Vv)))), st(b + 2 * q + i, Q2), st(b + 3 * q + i, Q3);
 }
}
// radix-2 (段 k、h = 2^(k-1))。
template <bool V, bool FIRST> inline void r2f(const u64* s, u64* b, int h, u64 w) {
 const __m256i W= bc(w);
 for(int i= 0; i < h; i+= 4) {
  __m256i u= ld(s + i);
  const __m256i v= ld(s + h + i);
  if constexpr(!FIRST) u= x(u, mul4<V>(v, W));
  st(b + i, u), st(b + h + i, x(u, v));
 }
}
template <bool V, bool FIRST> inline void r2i(u64* b, int h, u64 w) {
 const __m256i W= bc(w);
 for(int i= 0; i < h; i+= 4) {
  const __m256i u= ld(b + i), v= x(u, ld(b + h + i));
  if constexpr(!FIRST) st(b + i, x(u, mul4<V>(v, W)));
  st(b + h + i, v);
 }
}
// 4x4 の転置 (行 r0..r3 の t 番目を集めて列 t に)。自分自身が逆。
inline void tr4(__m256i& r0, __m256i& r1, __m256i& r2, __m256i& r3) {
 const __m256i t0= _mm256_unpacklo_epi64(r0, r1), t1= _mm256_unpackhi_epi64(r0, r1), t2= _mm256_unpacklo_epi64(r2, r3), t3= _mm256_unpackhi_epi64(r2, r3);
 r0= _mm256_permute2x128_si256(t0, t2, 0x20), r1= _mm256_permute2x128_si256(t1, t3, 0x20), r2= _mm256_permute2x128_si256(t0, t2, 0x31), r3= _mm256_permute2x128_si256(t1, t3, 0x31);
}
// 最下段の radix-4 (段 2, 1、q = 1) を block 4 つずつ。lane l が block j+l で、twiddle も lane ごと (M[0] = 0 なので block 0 も同じ式でよい)。
template <bool V> inline void r4l1f(u64* f, size_t j0, size_t j1) {
 const u64* M= TW.m.data();
 for(size_t j= j0; j < j1; j+= 4) {
  u64* b= f + 4 * j;
  __m256i Q0= ld(b), Q1= ld(b + 4), Q2= ld(b + 8), Q3= ld(b + 12);
  tr4(Q0, Q1, Q2, Q3);
  const __m256i Vv= ld(M + j), m0= ld(M + 2 * j), m1= ld(M + 2 * j + 4);
  const __m256i Ul= _mm256_permute4x64_epi64(_mm256_unpacklo_epi64(m0, m1), 0xD8), Uh= _mm256_permute4x64_epi64(_mm256_unpackhi_epi64(m0, m1), 0xD8);
  const U4 P2= clm4<V>(Q2, Vv);
  const __m256i A1= x(Q1, red4(clm4<V>(Q3, Vv))), A3= x(A1, Q3);
  __m256i R0= x(Q0, red4(P2 ^ clm4<V>(A1, Ul))), R2= x(x(Q0, Q2), red4(P2 ^ clm4<V>(A3, Uh))), R1= x(R0, A1), R3= x(R2, A3);
  tr4(R0, R1, R2, R3);
  st(b, R0), st(b + 4, R1), st(b + 8, R2), st(b + 12, R3);
 }
}
template <bool V> inline void r4l1i(u64* f, size_t j0, size_t j1) {
 const u64* M= TW.m.data();
 for(size_t j= j0; j < j1; j+= 4) {
  u64* b= f + 4 * j;
  __m256i R0= ld(b), R1= ld(b + 4), R2= ld(b + 8), R3= ld(b + 12);
  tr4(R0, R1, R2, R3);
  const __m256i Vv= ld(M + j), m0= ld(M + 2 * j), m1= ld(M + 2 * j + 4);
  const __m256i Ul= _mm256_permute4x64_epi64(_mm256_unpacklo_epi64(m0, m1), 0xD8), Uh= _mm256_permute4x64_epi64(_mm256_unpackhi_epi64(m0, m1), 0xD8);
  const __m256i A3= x(R2, R3), A1= x(R0, R1);
  const U4 P1= clm4<V>(A1, Ul);
  __m256i Q2= x(x(R0, R2), red4(P1 ^ clm4<V>(A3, Uh))), Q3= x(A1, A3), Q0= x(R0, red4(P1 ^ clm4<V>(Q2, Vv))), Q1= x(A1, red4(clm4<V>(Q3, Vv)));
  tr4(Q0, Q1, Q2, Q3);
  st(b, Q0), st(b + 4, Q1), st(b + 8, Q2), st(b + 12, Q3);
 }
}
// ---------------- radix-8 ----------------
// 段 k, k-1, k-2 を 1 回の読み書きで。Q0..Q7 は q 個ずつの 8 分の 1。段 k は (Qi, Qi+4) を v = M[j] で、段 k-1 は (0,2) (1,3) を u0 = M[2j]、
// (4,6) (5,7) を u1 = M[2j+1] で、段 k-2 は (0,1) (2,3) (4,5) (6,7) を w0..w3 = M[4j..4j+3] で。積は 12 個 (4 本ずつなら clm4 が 12 回)。
// 次に掛ける値と出力だけを畳み、ほかは 128 bit のまま足し込むと、畳むのが 8 回で済む (radix-4 を 1.5 回なら 9 回)。
// FIRST (block 0) は v = u0 = w0 = 0 で、0 との積を省く。
template <bool V, bool FIRST> inline void r8f_core(const __m256i& Q0, const __m256i& Q1, const __m256i& Q2, const __m256i& Q3, const __m256i& Q4, const __m256i& Q5, const __m256i& Q6, const __m256i& Q7, const __m256i& Vv, const __m256i& U0, const __m256i& U1, const __m256i& W0, const __m256i& W1, const __m256i& W2, const __m256i& W3, __m256i* R) {
 // 段 k: n_i = Q_i + v Q_{i+4}、n_{i+4} = n_i + Q_{i+4}。n2, n3 は次に掛けるので畳む。n0 = Q0 + P0、n1 = Q1 + P1 は畳まずに持つ。
 const __m256i n2= FIRST ? Q2 : x(Q2, red4(clm4<V>(Q6, Vv))), n3= FIRST ? Q3 : x(Q3, red4(clm4<V>(Q7, Vv))), n6= x(n2, Q6), n7= x(n3, Q7);
 // 段 k-1: B0 = Q0 + X02、B2 = Q0 + n2 + X02、B1 = Q1 + X13、B3 = B1 + n3 (B4..B7 は Q0 + Q4、Q1 + Q5 を足した同じ形)。B1, B5 は次に掛けるので畳む。
 U4 X02, X46, X57;
 __m256i B1;
 if constexpr(FIRST) X46= clm4<V>(n6, U1), X57= clm4<V>(n7, U1), B1= Q1;
 else {
  const U4 P0= clm4<V>(Q4, Vv), P1= clm4<V>(Q5, Vv);
  X02= P0 ^ clm4<V>(n2, U0), X46= P0 ^ clm4<V>(n6, U1), X57= P1 ^ clm4<V>(n7, U1), B1= x(Q1, red4(P1 ^ clm4<V>(n3, U0)));
 }
 const __m256i B5= x(x(Q1, Q5), red4(X57)), B3= x(B1, n3), B7= x(B5, n7), Q04= x(Q0, Q4);
 // 段 k-2: R_{2t} = B_{2t} + w_t B_{2t+1}、R_{2t+1} = R_{2t} + B_{2t+1}。
 if constexpr(FIRST) R[0]= Q0, R[2]= x(x(Q0, n2), red4(clm4<V>(B3, W1)));
 else R[0]= x(Q0, red4(X02 ^ clm4<V>(B1, W0))), R[2]= x(x(Q0, n2), red4(X02 ^ clm4<V>(B3, W1)));
 R[4]= x(Q04, red4(X46 ^ clm4<V>(B5, W2))), R[6]= x(x(Q04, n6), red4(X46 ^ clm4<V>(B7, W3)));
 R[1]= x(R[0], B1), R[3]= x(R[2], B3), R[5]= x(R[4], B5), R[7]= x(R[6], B7);
}
// r8f_core の逆。段 k-2、k-1、k の順に戻す。畳むのは 8 回。
template <bool V, bool FIRST> inline void r8i_core(const __m256i& R0, const __m256i& R1, const __m256i& R2, const __m256i& R3, const __m256i& R4, const __m256i& R5, const __m256i& R6, const __m256i& R7, const __m256i& Vv, const __m256i& U0, const __m256i& U1, const __m256i& W0, const __m256i& W1, const __m256i& W2, const __m256i& W3, __m256i* Q) {
 // 段 k-2: B_{2t+1} = R_{2t} + R_{2t+1}、B_{2t} = R_{2t} + Y_{2t} (Y_{2t} = w_t B_{2t+1})。
 const __m256i B1= x(R0, R1), B3= x(R2, R3), B5= x(R4, R5), B7= x(R6, R7);
 const U4 Y2= clm4<V>(B3, W1), Y4= clm4<V>(B5, W2), Y6= clm4<V>(B7, W3);
 // 段 k-1: n2 = B0 + B2、n0 = B0 + u0 n2 (n6, n4 も同じ)。n2, n6 は次に掛けるので畳む。
 U4 Z0, Z1;
 __m256i n2;
 if constexpr(FIRST) n2= x(x(R0, R2), red4(Y2));
 else {
  const U4 Y0= clm4<V>(B1, W0);
  n2= x(x(R0, R2), red4(Y0 ^ Y2));
  Z0= Y0 ^ clm4<V>(n2, U0), Z1= clm4<V>(x(B1, B3), U0);
 }
 const __m256i n3= x(B1, B3), n6= x(x(R4, R6), red4(Y4 ^ Y6)), n7= x(B5, B7);
 const U4 Z4= Y4 ^ clm4<V>(n6, U1), Z5= clm4<V>(n7, U1);
 // 段 k: Q_{i+4} = n_i + n_{i+4}、Q_i = n_i + v Q_{i+4}。
 const __m256i Q6= x(n2, n6), Q7= x(n3, n7);
 if constexpr(FIRST) {
  Q[4]= x(x(R0, R4), red4(Z4)), Q[5]= x(x(B1, B5), red4(Z5));
  Q[0]= R0, Q[1]= B1, Q[2]= n2, Q[3]= n3;
 } else {
  Q[4]= x(x(R0, R4), red4(Z0 ^ Z4)), Q[5]= x(x(B1, B5), red4(Z1 ^ Z5));
  Q[0]= x(R0, red4(Z0 ^ clm4<V>(Q[4], Vv))), Q[1]= x(B1, red4(Z1 ^ clm4<V>(Q[5], Vv)));
  Q[2]= x(n2, red4(clm4<V>(Q6, Vv))), Q[3]= x(n3, red4(clm4<V>(Q7, Vv)));
 }
 Q[6]= Q6, Q[7]= Q7;
}
template <bool V, bool FIRST> inline void r8f(const u64* s, u64* b, int q, u64 v, u64 u0, u64 u1, u64 w0, u64 w1, u64 w2, u64 w3) {
 const __m256i Vv= bc(v), U0= bc(u0), U1= bc(u1), W0= bc(w0), W1= bc(w1), W2= bc(w2), W3= bc(w3);
 for(int i= 0; i < q; i+= 4) {
  __m256i R[8];
  r8f_core<V, FIRST>(ld(s + i), ld(s + q + i), ld(s + 2 * q + i), ld(s + 3 * q + i), ld(s + 4 * q + i), ld(s + 5 * q + i), ld(s + 6 * q + i), ld(s + 7 * q + i), Vv, U0, U1, W0, W1, W2, W3, R);
  for(int t= 0; t < 8; ++t) st(b + t * q + i, R[t]);
 }
}
template <bool V, bool FIRST> inline void r8i(u64* b, int q, u64 v, u64 u0, u64 u1, u64 w0, u64 w1, u64 w2, u64 w3) {
 const __m256i Vv= bc(v), U0= bc(u0), U1= bc(u1), W0= bc(w0), W1= bc(w1), W2= bc(w2), W3= bc(w3);
 for(int i= 0; i < q; i+= 4) {
  __m256i Q[8];
  r8i_core<V, FIRST>(ld(b + i), ld(b + q + i), ld(b + 2 * q + i), ld(b + 3 * q + i), ld(b + 4 * q + i), ld(b + 5 * q + i), ld(b + 6 * q + i), ld(b + 7 * q + i), Vv, U0, U1, W0, W1, W2, W3, Q);
  for(int t= 0; t < 8; ++t) st(b + t * q + i, Q[t]);
 }
}
// 最下段の radix-8 (段 3, 2, 1、q = 1) を block 4 つずつ。前半 4 要素と後半 4 要素をそれぞれ 4x4 で転置し、lane l が block j+l。
// twiddle も lane ごとに: v = M[j+l]、u0, u1 = M[2(j+l)], M[2(j+l)+1]、w_t = M[4(j+l)+t] (M+4j からの 16 個を 4x4 で転置)。
struct Tw8 {
 __m256i Vv, U0, U1, W0, W1, W2, W3;
 explicit Tw8(size_t j) {
  const u64* M= TW.m.data();
  const __m256i m0= ld(M + 2 * j), m1= ld(M + 2 * j + 4);
  Vv= ld(M + j), U0= _mm256_permute4x64_epi64(_mm256_unpacklo_epi64(m0, m1), 0xD8), U1= _mm256_permute4x64_epi64(_mm256_unpackhi_epi64(m0, m1), 0xD8);
  W0= ld(M + 4 * j), W1= ld(M + 4 * j + 4), W2= ld(M + 4 * j + 8), W3= ld(M + 4 * j + 12);
  tr4(W0, W1, W2, W3);
 }
};
template <bool V> inline void r8l1f(u64* f, size_t j0, size_t j1) {
 for(size_t j= j0; j < j1; j+= 4) {
  u64* b= f + 8 * j;
  __m256i a0= ld(b), a1= ld(b + 8), a2= ld(b + 16), a3= ld(b + 24), c0= ld(b + 4), c1= ld(b + 12), c2= ld(b + 20), c3= ld(b + 28), R[8];
  tr4(a0, a1, a2, a3), tr4(c0, c1, c2, c3);
  const Tw8 w(j);
  r8f_core<V, false>(a0, a1, a2, a3, c0, c1, c2, c3, w.Vv, w.U0, w.U1, w.W0, w.W1, w.W2, w.W3, R);
  tr4(R[0], R[1], R[2], R[3]), tr4(R[4], R[5], R[6], R[7]);
  st(b, R[0]), st(b + 8, R[1]), st(b + 16, R[2]), st(b + 24, R[3]), st(b + 4, R[4]), st(b + 12, R[5]), st(b + 20, R[6]), st(b + 28, R[7]);
 }
}
template <bool V> inline void r8l1i(u64* f, size_t j0, size_t j1) {
 for(size_t j= j0; j < j1; j+= 4) {
  u64* b= f + 8 * j;
  __m256i a0= ld(b), a1= ld(b + 8), a2= ld(b + 16), a3= ld(b + 24), c0= ld(b + 4), c1= ld(b + 12), c2= ld(b + 20), c3= ld(b + 28), Q[8];
  tr4(a0, a1, a2, a3), tr4(c0, c1, c2, c3);
  const Tw8 w(j);
  r8i_core<V, false>(a0, a1, a2, a3, c0, c1, c2, c3, w.Vv, w.U0, w.U1, w.W0, w.W1, w.W2, w.W3, Q);
  tr4(Q[0], Q[1], Q[2], Q[3]), tr4(Q[4], Q[5], Q[6], Q[7]);
  st(b, Q[0]), st(b + 8, Q[1]), st(b + 16, Q[2]), st(b + 24, Q[3]), st(b + 4, Q[4]), st(b + 12, Q[5]), st(b + 20, Q[6]), st(b + 28, Q[7]);
 }
}
// ---------------- 段の並べ方 ----------------
struct Pass {
 int k, r;  // 段 k から r = 2, 4, 8 で (log2 r) 段
};
// 段 hi から lo までを上から。radix-8 を基本にし、余りは上に radix-4 を 1 つか 2 つ置く (最下段は q = 1 の radix-8 か radix-4 になる)。
inline int plan(int lo, int hi, Pass* ps) {
 int n= 0;
 const int c= hi - lo + 1;
 if(c == 1) {
  ps[n++]= {hi, 2};
  return n;
 }
 for(int t= c % 3 == 1 ? 2 : c % 3 == 2 ? 1 : 0; t--; hi-= 2) ps[n++]= {hi, 4};
 for(; hi >= lo + 2; hi-= 3) ps[n++]= {hi, 8};
 return n;
}
// [lo, hi) の block に 1 pass。fused なら、どの block も f の先頭 (上の段を飛ばした base) から読む。base を最後に上書きするよう後ろから。
template <bool V> inline void fwd_pass(u64* f, size_t lo, size_t hi, const Pass& p, bool fused) {
 const u64* M= TW.m.data();
 const int k= p.k;
 const size_t j0= lo >> k, j1= hi >> k;
 if(p.r == 8) {
  const int q= 1 << (k - 3);
  if(q == 1) return r8l1f<V>(f, j0, j1);
  for(size_t j= j1; j-- > j0;) {
   u64* b= f + (j << k);
   const u64* s= fused ? f : b;
   if(!j) r8f<V, true>(s, b, q, 0, 0, M[1], 0, M[1], M[2], M[3]);
   else r8f<V, false>(s, b, q, M[j], M[2 * j], M[2 * j + 1], M[4 * j], M[4 * j + 1], M[4 * j + 2], M[4 * j + 3]);
  }
 } else if(p.r == 4) {
  const int q= 1 << (k - 2);
  if(q == 1) return r4l1f<V>(f, j0, j1);
  for(size_t j= j1; j-- > j0;) {
   u64* b= f + (j << k);
   const u64* s= fused ? f : b;
   if(!j) r4f_first<V>(s, b, q, M[1]);
   else r4f_lazy<V>(s, b, q, M[j], M[2 * j], M[2 * j + 1]);
  }
 } else {
  const int h= 1 << (k - 1);
  for(size_t j= j1; j-- > j0;) {
   u64* b= f + (j << k);
   const u64* s= fused ? f : b;
   if(!j) r2f<V, true>(s, b, h, 0);
   else r2f<V, false>(s, b, h, M[j]);
  }
 }
}
template <bool V> inline void inv_pass(u64* f, size_t lo, size_t hi, const Pass& p) {
 const u64* M= TW.m.data();
 const int k= p.k;
 const size_t j0= lo >> k, j1= hi >> k;
 if(p.r == 8) {
  const int q= 1 << (k - 3);
  if(q == 1) return r8l1i<V>(f, j0, j1);
  for(size_t j= j0; j < j1; ++j) {
   u64* b= f + (j << k);
   if(!j) r8i<V, true>(b, q, 0, 0, M[1], 0, M[1], M[2], M[3]);
   else r8i<V, false>(b, q, M[j], M[2 * j], M[2 * j + 1], M[4 * j], M[4 * j + 1], M[4 * j + 2], M[4 * j + 3]);
  }
 } else if(p.r == 4) {
  const int q= 1 << (k - 2);
  if(q == 1) return r4l1i<V>(f, j0, j1);
  for(size_t j= j0; j < j1; ++j) {
   u64* b= f + (j << k);
   if(!j) r4i_first<V>(b, q, M[1]);
   else r4i_lazy<V>(b, q, M[j], M[2 * j], M[2 * j + 1]);
  }
 } else {
  const int h= 1 << (k - 1);
  for(size_t j= j0; j < j1; ++j) {
   u64* b= f + (j << k);
   if(!j) r2i<V, true>(b, h, 0);
   else r2i<V, false>(b, h, M[j]);
  }
 }
}
// 順変換の上の段 (2^bb を超える block の段) を全体に。入力が [0, 2^k0) に収まるので段 k0 から始める。
template <bool V> inline void fwd_top(u64* f, int d, int k0, int bb) {
 if(k0 <= bb) return;
 Pass ps[64];
 const int np= plan(bb + 1, k0, ps);
 for(int i= 0; i < np; ++i) fwd_pass<V>(f, 0, size_t(1) << d, ps[i], i == 0 && k0 < d);
}
// 順変換の下の段を block [lo, hi) に。上の段が無く (k0 <= bb) 飛ばした段があるなら、最初の pass は base から読む。
template <bool V> inline void fwd_bottom(u64* f, size_t lo, size_t hi, int d, int k0, int bb) {
 Pass ps[64];
 const int np= plan(1, std::min(bb, k0), ps);
 for(int i= 0; i < np; ++i) fwd_pass<V>(f, lo, hi, ps[i], i == 0 && k0 <= bb && k0 < d);
}
template <bool V> inline void inv_bottom(u64* f, size_t lo, size_t hi, int bb) {
 Pass ps[64];
 const int np= plan(1, bb, ps);
 for(int i= np; i--;) inv_pass<V>(f, lo, hi, ps[i]);
}
template <bool V> inline void inv_top(u64* f, int d, int bb) {
 if(d <= bb) return;
 Pass ps[64];
 const int np= plan(bb + 1, d, ps);
 for(int i= np; i--;) inv_pass<V>(f, 0, size_t(1) << d, ps[i]);
}
template <bool V> inline void pointwise(u64* f, const u64* g, size_t lo, size_t hi) {
 for(size_t i= lo; i < hi; i+= 4) st(f + i, mul4<V>(ld(f + i), ld(g + i)));
}
template <bool V> inline std::vector<u64> convolve(const std::vector<u64>& a, const std::vector<u64>& b) {
 const size_t n= a.size(), m= b.size();
 if(!n || !m) return {};
 const size_t N= n + m - 1;
 if(N <= 64) return naive(a, b);
 const int d= clog2(N), bb= std::min(BB, d), kf= std::max(clog2(n), 4), kg= std::max(clog2(m), 4);
 const size_t S= size_t(1) << d, BS= size_t(1) << bb;
 TW.init(d);
 std::vector<u64> f(S);  // 出力を兼ねる。[n, 2^kf) の 0 はここで入る
 std::copy(a.begin(), a.end(), f.begin());
 std::unique_ptr<u64[]> gbuf(new u64[S]);  // 0 で埋めない (読む前に書く)
 u64 *F= f.data(), *G= gbuf.get();
 std::copy(b.begin(), b.end(), G), std::fill(G + m, G + (size_t(1) << kg), 0);
 bc_lch(G, kg, bb), fwd_top<V>(G, d, kg, bb);
 for(size_t s= S; s;) s-= BS, fwd_bottom<V>(G, s, s + BS, d, kg, bb);
 bc_lch(F, kf, bb), fwd_top<V>(F, d, kf, bb);
 for(size_t s= S; s;) s-= BS, fwd_bottom<V>(F, s, s + BS, d, kf, bb), pointwise<V>(F, G, s, s + BS), inv_bottom<V>(F, s, s + BS, bb);
 inv_top<V>(F, d, bb), bc_mono(F, d, bb);
 f.resize(N);
 return f;
}
}  // namespace cantor_zf
inline std::vector<u64> run(int, int, const std::vector<u64>& a, const std::vector<u64>& b) {
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return cantor_zf::convolve<1>(a, b);
#endif
 return cantor_zf::convolve<0>(a, b);
}
