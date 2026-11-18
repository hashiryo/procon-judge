#pragma once
// 書き比べ: 法 2^64 (u64 の桁あふれのまま) の集合冪級数の exp と log の AVX2 の核。NeoLibrary の汎用の道 (generic.hpp の
// ランク付きのゼータ変換) と同じく、上の要素 i を含む部分を段ごとに部分集合畳み込み (exp) と割り算 (log) で求める。
//   exp: h_hi = f_i * h_lo、log: g_hi = f_hi / f_lo (f_lo[∅] = 1)。i ビットの部分集合畳み込みはランク付きのゼータ変換、各点の
//   多項式の積か割り算、メビウス変換で、どれも割り算 (整数での) を使わない。
// 持ち方は位置 4 個を 1 本の __m256i (u64 × 4) に並べた塊で、塊ごとに rank の数だけ並べる。ゼータ変換は下の 2 bit をレーンの
// 中で、上の bit を塊の間で行い、塊 q の値は rank が [popcount(q の残りの上の bit), popcount(q) + 2] の範囲 (窓) にしかないので、
// その範囲だけを足す。積は u64 の下位 64 bit だけが要るので、a b = a_lo b_lo + 2^32 (a_lo b_hi + a_hi b_lo) を vpmuludq 3 回で求める。
// 比べ終えたら、採るものを NeoLibrary に入れて消す。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
namespace sps_u64 {
using u64= unsigned long long;  // pj.hpp と同じ (Linux の uint64_t は unsigned long)
using V= __m256i;
#define U64_AI [[gnu::always_inline]] inline
U64_AI V ld(const V* p) { return _mm256_load_si256(p); }
U64_AI void st(V* p, V x) { _mm256_store_si256(p, x); }
U64_AI V add(V a, V b) { return _mm256_add_epi64(a, b); }
U64_AI V sub(V a, V b) { return _mm256_sub_epi64(a, b); }
// レーンの中の 2 bit のゼータ (UP) とメビウス
template <bool INV> U64_AI V lane_zeta(V v) {
 const V m13= _mm256_setr_epi64x(0, -1, 0, -1), m23= _mm256_setr_epi64x(0, 0, -1, -1);
 V t= _mm256_and_si256(_mm256_permute4x64_epi64(v, 0xA0), m13);  // レーン 1, 3 にレーン 0, 2
 v= INV ? sub(v, t) : add(v, t);
 t= _mm256_and_si256(_mm256_permute4x64_epi64(v, 0x44), m23);  // レーン 2, 3 にレーン 0, 1
 return INV ? sub(v, t) : add(v, t);
}
// u64 の積の和 sum_k a_k b_k を、lo = sum a_lo b_lo と c = sum (a_lo b_hi + a_hi b_lo) にためる (結果は lo + (c << 32))
U64_AI void mac(V& lo, V& c, V a, V ah, V b, V bh) {
 lo= add(lo, _mm256_mul_epu32(a, b)), c= add(c, add(_mm256_mul_epu32(a, bh), _mm256_mul_epu32(ah, b)));
}
U64_AI V fin(V lo, V c) { return add(lo, _mm256_slli_epi64(c, 32)); }
inline V* valloc(std::size_t n) { return static_cast<V*>(std::aligned_alloc(32, std::max<std::size_t>(n, 1) * sizeof(V))); }
// f の 2^m 個 (m >= 2) のランク付きのゼータ変換を F (塊 2^{m-2} 個、幅 S) に。塊 q は rank [0, popcount(q) + 2] を持つ。
inline void rz_fwd(const u64* f, V* F, int m, int S) {
 const std::size_t nb= std::size_t(1) << (m - 2);
 const V M0= _mm256_setr_epi64x(-1, 0, 0, 0), M1= _mm256_setr_epi64x(0, -1, -1, 0), M2= _mm256_setr_epi64x(0, 0, 0, -1);
 for(std::size_t q= 0; q < nb; ++q) {
  const V x= _mm256_loadu_si256((const V*)(f + 4 * q));
  V* b= F + q * S;
  const int c= __builtin_popcountll(q);
  st(b + c, lane_zeta<false>(_mm256_and_si256(x, M0)));
  st(b + c + 1, lane_zeta<false>(_mm256_and_si256(x, M1)));
  st(b + c + 2, lane_zeta<false>(_mm256_and_si256(x, M2)));
 }
 for(int kk= 0; kk < m - 2; ++kk) {
  const std::size_t d= std::size_t(1) << kk;
  for(std::size_t q0= 0; q0 < nb; q0+= 2 * d)
   for(std::size_t q= q0; q < q0 + d; ++q) {
    const V* s= F + q * S;
    V* t= F + (q + d) * S;
    const int lo= __builtin_popcountll(q >> kk), hi= __builtin_popcountll(q) + 2;
    st(t + lo, ld(s + lo));
    for(int r= lo + 1; r <= hi; ++r) st(t + r, add(ld(t + r), ld(s + r)));
   }
 }
}
// 塊 q が rank [pc(q), top(q)] を持つ F のメビウス変換をして、位置 y の rank popcount(y) を out に取り出す。
template <class Top> inline void rz_inv(V* F, u64* out, int m, int S, const Top& top) {
 const std::size_t nb= std::size_t(1) << (m - 2);
 for(int kk= 0; kk < m - 2; ++kk) {
  const std::size_t d= std::size_t(1) << kk;
  for(std::size_t q0= 0; q0 < nb; q0+= 2 * d)
   for(std::size_t q= q0; q < q0 + d; ++q) {
    const V* s= F + q * S;
    V* t= F + (q + d) * S;
    const int a= __builtin_popcountll(q) + 1, b= std::min(top(q), top(q + d));
    for(int r= a; r <= b; ++r) st(t + r, sub(ld(t + r), ld(s + r)));
   }
 }
 const V M0= _mm256_setr_epi64x(-1, 0, 0, 0), M1= _mm256_setr_epi64x(0, -1, -1, 0), M2= _mm256_setr_epi64x(0, 0, 0, -1);
 for(std::size_t q= 0; q < nb; ++q) {
  V* b= F + q * S;
  const int c= __builtin_popcountll(q), tp= top(q);
  V r= _mm256_and_si256(lane_zeta<true>(ld(b + c)), M0);
  if(c + 1 <= tp) r= _mm256_or_si256(r, _mm256_and_si256(lane_zeta<true>(ld(b + c + 1)), M1));
  if(c + 2 <= tp) r= _mm256_or_si256(r, _mm256_and_si256(lane_zeta<true>(ld(b + c + 2)), M2));
  _mm256_storeu_si256((V*)(out + 4 * q), r);
 }
}
// 素朴な部分集合畳み込み (小さい段)
inline void naive_cnv(const u64* a, const u64* b, u64* c, std::size_t L) {
 for(std::size_t s= 0; s < L; ++s) {
  u64 t= 0;
  for(std::size_t x= s;; x= (x - 1) & s) {
   t+= a[x] * b[s ^ x];
   if(!x) break;
  }
  c[s]= t;
 }
}
// exp(f) (f[0] = 0)
inline std::vector<u64> exp(const std::vector<u64>& f) {
 const std::size_t N= f.size();
 const int n= __builtin_ctzll(N);
 std::vector<u64> h(N, 0);
 h[0]= 1;
 int i= 0;
 for(; i < n && i < 6; ++i) naive_cnv(h.data(), f.data() + (std::size_t(1) << i), h.data() + (std::size_t(1) << i), std::size_t(1) << i);
 if(i == n) return h;
 const int S= n;  // rank 0..n-1 (段 i の rank は 0..i)
 V *A= valloc((std::size_t(1) << (n - 3)) * S), *B= valloc((std::size_t(1) << (n - 3)) * S);
 V ah[32], bh[32];
 for(; i < n; ++i) {
  const std::size_t L= std::size_t(1) << i, nb= L >> 2;
  rz_fwd(h.data(), A, i, S), rz_fwd(f.data() + L, B, i, S);
  for(std::size_t q= 0; q < nb; ++q) {
   V *a= A + q * S, *b= B + q * S;
   const int pc= __builtin_popcountll(q), w= pc + 2, top= std::min(i, 2 * pc + 4);
   for(int e= 0; e <= w; ++e) ah[e]= _mm256_srli_epi64(ld(a + e), 32), bh[e]= _mm256_srli_epi64(ld(b + e), 32);
   for(int d= top; d >= pc; --d) {  // 上から書けば、まだ要る a[e] (e <= d) を壊さない
    V lo= _mm256_setzero_si256(), c= lo;
    const int e0= std::max(0, d - w), e1= std::min(d, w);
    for(int e= e0; e <= e1; ++e) mac(lo, c, ld(a + e), ah[e], ld(b + d - e), bh[d - e]);
    st(a + d, fin(lo, c));
   }
  }
  rz_inv(A, h.data() + L, i, S, [&](std::size_t q) { return std::min(i, 2 * __builtin_popcountll(q) + 4); });
 }
 std::free(A), std::free(B);
 return h;
}
// log(f) (f[0] = 1)
inline std::vector<u64> log(const std::vector<u64>& f) {
 const std::size_t N= f.size();
 const int n= __builtin_ctzll(N);
 std::vector<u64> g(N, 0);
 int i= 0;
 for(; i < n && i < 6; ++i) {  // g_hi[s] = f_hi[s] - sum_{∅≠T⊆s} f[T] g_hi[s \ T]
  const std::size_t L= std::size_t(1) << i;
  for(std::size_t s= 0; s < L; ++s) {
   u64 t= f[L | s];
   for(std::size_t x= s; x; x= (x - 1) & s) t-= f[x] * g[L | (s ^ x)];
   g[L | s]= t;
  }
 }
 if(i == n) return g;
 const int S= n;
 // f の下の半分 (n - 1 bit) のゼータ変換を一度だけ (段 i ではその先頭の塊 2^{i-2} 個が i bit の変換)
 V *Bf= valloc((std::size_t(1) << (n - 3)) * S), *A= valloc((std::size_t(1) << (n - 3)) * S);
 rz_fwd(f.data(), Bf, n - 1, S);
 V bh[32], q[32], qh[32];
 for(; i < n; ++i) {
  const std::size_t L= std::size_t(1) << i, nb= L >> 2;
  rz_fwd(f.data() + L, A, i, S);
  for(std::size_t qq= 0; qq < nb; ++qq) {
   V *a= A + qq * S;
   const V* b= Bf + qq * S;
   const int w= __builtin_popcountll(qq) + 2;
   // 段 i の f_lo の変換の塊 qq の rank は [0, popcount(qq) + 2] (n - 1 bit の変換でも、塊 qq の上の bit は 0 なので同じ)
   for(int e= 1; e <= w; ++e) bh[e]= _mm256_srli_epi64(ld(b + e), 32);
   for(int d= 0; d <= i; ++d) {  // Q[d] = A[d] - sum_{e=1}^{min(d, w)} B[e] Q[d-e]
    V lo= _mm256_setzero_si256(), c= lo;
    for(int e= 1; e <= std::min(d, w); ++e) mac(lo, c, ld(b + e), bh[e], q[d - e], qh[d - e]);
    const V ad= d <= w ? ld(a + d) : _mm256_setzero_si256();
    q[d]= sub(ad, fin(lo, c)), qh[d]= _mm256_srli_epi64(q[d], 32);
   }
   for(int d= 0; d <= i; ++d) st(a + d, q[d]);
  }
  rz_inv(A, g.data() + L, i, S, [&](std::size_t) { return i; });
 }
 std::free(A), std::free(Bf);
 return g;
}
#undef U64_AI
}  // namespace sps_u64
