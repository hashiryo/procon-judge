#pragma once
// 部分集合畳み込み。rank 付きの XOR 変換で畳む (cp-algo の提出 352953 と同じ考え方)。
//   f[S] を f[S] t^{|S|} に置いて XOR 畳み込みをすると、t^{|U|} の係数が部分集合畳み込みになる。変換の添字 y を補集合に移すと
//   t が -t に変わるので、y の bit 0 が 0 の半分だけを求める (bit 0 の変換は f[2S'] と f[2S'+1] を同じ位置に置くだけ)。
//   空集合は外し、rank 0 の項 f[0] g[S] + f[S] g[0] は最後に足す。入力の rank は 1..n-1、積の rank は 2..n の n-1 個ずつ。
// 違うところ:
//   - 値は u32 で [0, 2p] に持ち、位置 8 個を 1 本の __m256i に並べる (塊 = 位置 8 個 × rank n-1 個)。
//   - 下の bit から変換すると、bit b まで済んだ位置の値は rank が [残りの上の bit の数, それ + b + 1] にしかないので、
//     その範囲 (窓) だけを変換する。逆変換は上の bit からにして、最後の数段は答えに要る rank だけを持つ。
//   - 積は [0, p] に直してから掛け、12 項までずつ u64 にためて Montgomery で還元する。
//   - 塊を 2^9 個ずつの区画に分けて下の段を区画の中で、上の段と積と逆変換の上の段を列ごとに L2 の中で行う。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif

namespace xw1 {
using u32= std::uint32_t;
using u64= std::uint64_t;
using V= __m256i;
#define XW_AI __attribute__((always_inline)) inline

constexpr u32 P= 998244353, P2= 2 * P;
constexpr u32 inv32(u32 n) {
 u32 x= n;
 for(int i= 0; i < 5; ++i) x*= 2u - n * x;
 return x;
}
constexpr u32 NPINV= 0u - inv32(P);  // -p^{-1} mod 2^32
inline u32 pw(u64 a, u64 e) {
 u64 r= 1;
 for(a%= P; e; e>>= 1, a= a * a % P)
  if(e & 1) r= r * a % P;
 return (u32)r;
}

XW_AI V ld(const u32* p) { return _mm256_load_si256((const V*)p); }
XW_AI void st(u32* p, V x) { _mm256_store_si256((V*)p, x); }
XW_AI V cst(u32 x) { return _mm256_set1_epi32((int)x); }
XW_AI V red2(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P2))); }  // [0, 4p) -> [0, 2p]
XW_AI V neg2(V x) { return _mm256_sub_epi32(cst(P2), x); }
XW_AI V add2(V a, V b) { return red2(_mm256_add_epi32(a, b)); }
XW_AI V sub2(V a, V b) { return red2(_mm256_add_epi32(a, neg2(b))); }
XW_AI V norm1(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P))); }  // [0, 2p] -> [0, p]
XW_AI V mul64(V a, V b) { return _mm256_mul_epu32(a, b); }
// e: 偶数のレーンの積の和、o: 奇数のレーンの積の和 (どちらも u64)。それぞれ x R^{-1} に還元して 8 レーンに並べる。
XW_AI V redc2(V e, V o) {
 const V pv= _mm256_set1_epi64x(P), nv= _mm256_set1_epi64x(NPINV);
 V te= _mm256_add_epi64(e, mul64(mul64(e, nv), pv));
 V to= _mm256_add_epi64(o, mul64(mul64(o, nv), pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(te, 32), to, 0xAA);
}
// 8 レーンの中の WHT (レーンの番号が bit 1..3 に当たる)。
XW_AI V lane_wht(V x) {
 V t= _mm256_shuffle_epi32(x, 0xB1);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xAA)));
 t= _mm256_shuffle_epi32(x, 0x4E);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xCC)));
 t= _mm256_permute2x128_si256(x, x, 0x01);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xF0)));
 return x;
}
XW_AI V mpc(int j) {  // popcount(レーン) == j のレーン
 auto b= [&](int r) { return __builtin_popcount(r) == j ? -1 : 0; };
 return _mm256_setr_epi32(b(0), b(1), b(2), b(3), b(4), b(5), b(6), b(7));
}

// 塊 q (c = popcount(q)) に入力 16 個を置き、レーンの WHT まで済ませて窓の rank だけを書く。入力の rank r は場所 r - 1。
XW_AI void init_block(u32* X, const u32* in, int c, int nm1) {
 V lo= _mm256_loadu_si256((const V*)in), hi= _mm256_loadu_si256((const V*)(in + 8));
 __m256 lf= _mm256_castsi256_ps(lo), hf= _mm256_castsi256_ps(hi);
 V E= _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(lf, hf, 0x88)), 0xD8);
 V O= _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(lf, hf, 0xDD)), 0xD8);
 const V M0= mpc(0), M1= mpc(1), M2= mpc(2), M3= mpc(3);
 V v[5];
 v[0]= _mm256_broadcastd_epi32(_mm256_castsi256_si128(E));
 v[1]= lane_wht(_mm256_or_si256(_mm256_and_si256(E, M1), _mm256_and_si256(O, M0)));
 v[2]= lane_wht(_mm256_or_si256(_mm256_and_si256(E, M2), _mm256_and_si256(O, M1)));
 v[3]= lane_wht(_mm256_or_si256(_mm256_and_si256(E, M3), _mm256_and_si256(O, M2)));
 V o7= _mm256_permutevar8x32_epi32(O, cst(7));
 v[4]= _mm256_blend_epi32(o7, neg2(o7), 0x96);
 for(int j= 0; j < 5; ++j)
  if(int s= c + j - 1; s >= 0 && s < nm1) st(X + 8 * s, v[j]);
}
// 塊の bit k の変換。X は bit k が 0 の塊 (rank [c, c+4+k])、Y は 1 の塊 (rank [c+1, c+5+k])、c = popcount(q >> (k+1))。
XW_AI void fwd_pair(u32* X, u32* Y, int c, int k, int nm1) {
 if(c >= 1) st(Y + 8 * (c - 1), ld(X + 8 * (c - 1)));
 const int hi= std::min(c + 3 + k, nm1 - 1);
 for(int s= c; s <= hi; ++s) {
  V a= ld(X + 8 * s), b= ld(Y + 8 * s);
  st(X + 8 * s, add2(a, b)), st(Y + 8 * s, sub2(a, b));
 }
 if(int s= c + 4 + k; s < nm1) {
  V b= ld(Y + 8 * s);
  st(X + 8 * s, b), st(Y + 8 * s, neg2(b));
 }
}
// 逆変換の塊の bit k。どちらも rank [c, c+5+k] を持ち、X (bit k が 0) は [c, c+4+k]、Y は [c+1, c+5+k] を残す。積の rank r は場所 r - 2。
XW_AI void inv_pair(u32* X, u32* Y, int c, int k, int nm1) {
 if(c >= 2) st(X + 8 * (c - 2), add2(ld(X + 8 * (c - 2)), ld(Y + 8 * (c - 2))));
 const int lo= std::max(c - 1, 0), hi= std::min(c + 2 + k, nm1 - 1);
 for(int s= lo; s <= hi; ++s) {
  V a= ld(X + 8 * s), b= ld(Y + 8 * s);
  st(X + 8 * s, add2(a, b)), st(Y + 8 * s, sub2(a, b));
 }
 if(int s= c + 3 + k; s < nm1) st(Y + 8 * s, sub2(ld(X + 8 * s), ld(Y + 8 * s)));
}
// 塊ごとの rank の積 (長さ nm1 で切り詰める)。結果は a に書く。
template <int NM1> XW_AI void prod_block(u32* a, const u32* b) {
 alignas(32) u32 A[NM1 * 8], Ah[NM1 * 8], B[NM1 * 8], Bh[NM1 * 8];
 for(int i= 0; i < NM1; ++i) {
  V x= norm1(ld(a + 8 * i)), y= norm1(ld(b + 8 * i));
  st(A + 8 * i, x), st(Ah + 8 * i, _mm256_srli_epi64(x, 32));
  st(B + 8 * i, y), st(Bh + 8 * i, _mm256_srli_epi64(y, 32));
 }
 const V z= _mm256_setzero_si256();
 for(int s= 0; s < NM1; ++s) {
  V r;
  if(s < 12) {
   V e= z, o= z;
   for(int i= 0; i <= s; ++i) {
    e= _mm256_add_epi64(e, mul64(ld(A + 8 * i), ld(B + 8 * (s - i))));
    o= _mm256_add_epi64(o, mul64(ld(Ah + 8 * i), ld(Bh + 8 * (s - i))));
   }
   r= red2(redc2(e, o));
  } else {
   V e1= z, o1= z, e2= z, o2= z;
   for(int i= 0; i < 12; ++i) {
    e1= _mm256_add_epi64(e1, mul64(ld(A + 8 * i), ld(B + 8 * (s - i))));
    o1= _mm256_add_epi64(o1, mul64(ld(Ah + 8 * i), ld(Bh + 8 * (s - i))));
   }
   for(int i= 12; i <= s; ++i) {
    e2= _mm256_add_epi64(e2, mul64(ld(A + 8 * i), ld(B + 8 * (s - i))));
    o2= _mm256_add_epi64(o2, mul64(ld(Ah + 8 * i), ld(Bh + 8 * (s - i))));
   }
   r= add2(red2(redc2(e1, o1)), red2(redc2(e2, o2)));
  }
  st(a + 8 * s, r);
 }
}
// 塊 q のレーンの逆変換と答えの取り出し。out = (Q K + g F0 + f G0) R^{-1}。
XW_AI void out_block(const u32* X, int c, int nm1, const u32* f, const u32* g, u32* out, V Kc, V F0, V G0) {
 const V M0= mpc(0), M1= mpc(1), M2= mpc(2), M3= mpc(3), z= _mm256_setzero_si256();
 V v[5];
 for(int j= 0; j < 5; ++j) {
  int s= c + j - 2;
  v[j]= s >= 0 && s < nm1 ? lane_wht(ld(X + 8 * s)) : z;
 }
 V ev= _mm256_or_si256(_mm256_or_si256(_mm256_and_si256(v[0], M0), _mm256_and_si256(v[1], M1)), _mm256_or_si256(_mm256_and_si256(v[2], M2), _mm256_and_si256(v[3], M3)));
 V od= _mm256_or_si256(_mm256_or_si256(_mm256_and_si256(v[1], M0), _mm256_and_si256(v[2], M1)), _mm256_or_si256(_mm256_and_si256(v[3], M2), _mm256_and_si256(v[4], M3)));
 V ul= _mm256_unpacklo_epi32(ev, od), uh= _mm256_unpackhi_epi32(ev, od);
 V q[2]= {_mm256_permute2x128_si256(ul, uh, 0x20), _mm256_permute2x128_si256(ul, uh, 0x31)};
 for(int t= 0; t < 2; ++t) {
  V fv= _mm256_loadu_si256((const V*)(f + 8 * t)), gv= _mm256_loadu_si256((const V*)(g + 8 * t));
  V e= _mm256_add_epi64(_mm256_add_epi64(mul64(q[t], Kc), mul64(gv, F0)), mul64(fv, G0));
  V o= _mm256_add_epi64(_mm256_add_epi64(mul64(_mm256_srli_epi64(q[t], 32), Kc), mul64(_mm256_srli_epi64(gv, 32), F0)), mul64(_mm256_srli_epi64(fv, 32), G0));
  _mm256_storeu_si256((V*)(out + 8 * t), norm1(redc2(e, o)));
 }
}

inline u32* huge_alloc(size_t bytes) {
 const size_t H= size_t(1) << 21;
 bytes= (bytes + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
#endif
 return (u32*)p;
}

template <int N> void run_n(const u32* f, const u32* g, u32* out) {
 constexpr int NM1= N - 1, NB= N - 4, BS= NM1 * 8;
 constexpr int K= NB < 9 ? NB : 9, H= NB - K;
 const size_t nblk= size_t(1) << NB, csz= size_t(1) << K;
 u32 *A= huge_alloc(nblk * BS * 4), *B= huge_alloc(nblk * BS * 4);
 for(size_t q0= 0; q0 < nblk; q0+= csz)
  for(int w= 0; w < 2; ++w) {
   u32* X= w ? B : A;
   const u32* in= w ? g : f;
   for(size_t q= q0; q < q0 + csz; ++q) init_block(X + q * BS, in + 16 * q, __builtin_popcountll(q), NM1);
   for(int k= 0; k < K; ++k)
    for(size_t q= q0; q < q0 + csz; ++q)
     if(!(q >> k & 1)) fwd_pair(X + q * BS, X + (q | size_t(1) << k) * BS, __builtin_popcountll(q >> (k + 1)), k, NM1);
  }
 if constexpr(H == 0) {
  for(size_t q= 0; q < nblk; ++q) prod_block<NM1>(A + q * BS, B + q * BS);
 } else {
  constexpr size_t hs= size_t(1) << H;
  u32 *cA= (u32*)std::aligned_alloc(64, hs * BS * 4), *cB= (u32*)std::aligned_alloc(64, hs * BS * 4);
  for(size_t j= 0; j < csz; ++j) {
   for(size_t h= 0; h < hs; ++h) {
    std::memcpy(cA + h * BS, A + ((h << K) + j) * BS, BS * 4);
    std::memcpy(cB + h * BS, B + ((h << K) + j) * BS, BS * 4);
   }
   for(int kk= 0; kk < H; ++kk)
    for(size_t h= 0; h < hs; ++h)
     if(!(h >> kk & 1)) {
      int c= __builtin_popcountll(h >> (kk + 1));
      fwd_pair(cA + h * BS, cA + (h | size_t(1) << kk) * BS, c, kk + K, NM1);
      fwd_pair(cB + h * BS, cB + (h | size_t(1) << kk) * BS, c, kk + K, NM1);
     }
   for(size_t h= 0; h < hs; ++h) prod_block<NM1>(cA + h * BS, cB + h * BS);
   for(int kk= H; kk--;)
    for(size_t h= 0; h < hs; ++h)
     if(!(h >> kk & 1)) inv_pair(cA + h * BS, cA + (h | size_t(1) << kk) * BS, __builtin_popcountll(h >> (kk + 1)), kk + K, NM1);
   for(size_t h= 0; h < hs; ++h) std::memcpy(A + ((h << K) + j) * BS, cA + h * BS, BS * 4);
  }
  std::free(cA), std::free(cB);
 }
 const u64 R1= (u64(1) << 32) % P, R2= R1 * R1 % P;
 const V Kc= cst(u32(u64(pw(2, u64(P - 1) * 64 - (N - 1))) * R2 % P)), F0= cst(u32(f[0] * R1 % P)), G0= cst(u32(g[0] * R1 % P));
 for(size_t q0= 0; q0 < nblk; q0+= csz) {
  for(int k= K; k--;)
   for(size_t q= q0; q < q0 + csz; ++q)
    if(!(q >> k & 1)) inv_pair(A + q * BS, A + (q | size_t(1) << k) * BS, __builtin_popcountll(q >> (k + 1)), k, NM1);
  for(size_t q= q0; q < q0 + csz; ++q) out_block(A + q * BS, __builtin_popcountll(q), NM1, f + 16 * q, g + 16 * q, out + 16 * q, Kc, F0, G0);
 }
 out[0]= u32(u64(f[0]) * g[0] % P);
 std::free(A), std::free(B);
}

inline std::vector<u32> subset_convolution(int n, const std::vector<u32>& f, const std::vector<u32>& g) {
 std::vector<u32> out(size_t(1) << n);
 if(n < 4) {
  for(int s= 0; s < (1 << n); ++s) {
   u64 t= 0;
   for(int a= s;; a= (a - 1) & s) {
    t+= u64(f[a]) * g[s ^ a] % P;
    if(!a) break;
   }
   out[s]= u32(t % P);
  }
  return out;
 }
 switch(n) {
#define XW_CASE(m) \
 case m: run_n<m>(f.data(), g.data(), out.data()); break;
  XW_CASE(4) XW_CASE(5) XW_CASE(6) XW_CASE(7) XW_CASE(8) XW_CASE(9) XW_CASE(10) XW_CASE(11) XW_CASE(12) XW_CASE(13) XW_CASE(14) XW_CASE(15) XW_CASE(16) XW_CASE(17) XW_CASE(18) XW_CASE(19) XW_CASE(20)
#undef XW_CASE
 }
 return out;
}
}  // namespace xw1

// ハーネスの形 (Solver: 構築 / run / answer) に合わせる。入出力は素の整数。
struct Solver {
 int n;
 std::vector<xw1::u32> a, b, c;
 Solver(int n_, const std::vector<long long>& a_, const std::vector<long long>& b_): n(n_), a(a_.begin(), a_.end()), b(b_.begin(), b_.end()) {}
 void run() { c= xw1::subset_convolution(n, a, b); }
 std::vector<long long> answer() const { return std::vector<long long>(c.begin(), c.end()); }
};
