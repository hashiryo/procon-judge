#pragma once
// 書き比べ: 区間 (a ⊆ c ⊆ b) への加算と和の問いを、操作の塊ごとに列を作り直す形で (sbk1 から sbk3 が読む)。旗は 3 つ。
//   SB_ADAPT: 作り直す時期を、塊の中の加算を問いで足した数の合計が作り直しの手間 (n 2^n の何倍か) を超えたときにする。0 なら
//     sqrt(n 2^n) 個の加算ごと。
//   SB_TRI: 塊より前の分の問いと加算の分け方を、自由な bit (b \ a) をたどる、1 の bit (a) を包除で外して部分集合の和を使う、0 の bit
//     (b の補集合) を包除で外して上位集合の和を使う、の 3 つのうち少ないものにする (どれも 2^{n/3} 以下)。0 なら前の 2 つ (2^{n/2} 以下)。
//   SB_SIMD: 塊の中の加算 (a', b', k) を問い (a, b) で足す段 (a ∪ a' ⊆ b ∩ b' なら k 2^{|b ∩ b'| - |a ∪ a'|}) を AVX2 で 8 個ずつ。
//   SB_BF: SB_ADAPT の作り直しの目安の倍率 (塊の中の加算を足した数の合計が SB_BF n 2^n を超えたら作り直す)。既定は SB_SIMD なら 4、
//     でなければ 1。
// 包除の式: [a ⊆ c ⊆ b] = sum_{d ⊆ a} (-1)^{|d|} [c ⊆ b \ d] = sum_{e ⊆ ~b} (-1)^{|e|} [c ⊇ a ∪ e]。
// 加算は、塊の中の加算として覚えたうえで、同じ 3 つの分け方で「点に足す」「部分集合全部に足す」「上位集合全部に足す」の印に分け、
// 作り直すときに上位集合の和と部分集合の和で配る。比べ終えたら消す。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <cmath>
#include "pj.hpp"
#ifndef SB_ADAPT
#define SB_ADAPT 1
#endif
#ifndef SB_TRI
#define SB_TRI 1
#endif
#ifndef SB_SIMD
#define SB_SIMD 1
#endif
#ifndef SB_BF
#define SB_BF (SB_SIMD ? 4 : 1)
#endif
namespace sbk {
inline void zeta_sub(u32* f, int n) {  // f[y] <- sum_{c ⊆ y} f[c]
 const size_t N= size_t(1) << n;
 for(size_t w= 1; w < N; w<<= 1)
  for(size_t p= 0; p < N; p+= 2 * w)
   for(size_t k= 0; k < w; ++k) f[p + w + k]+= f[p + k];
}
inline void zeta_sup(u32* f, int n) {  // f[x] <- sum_{c ⊇ x} f[c]
 const size_t N= size_t(1) << n;
 for(size_t w= 1; w < N; w<<= 1)
  for(size_t p= 0; p < N; p+= 2 * w)
   for(size_t k= 0; k < w; ++k) f[p + k]+= f[p + w + k];
}
struct IntervalSum {
 int n;
 u32 full;
 vector<u32> v, s, t, mv, ms, mt;
 vector<u32> ua, ub, uk;  // 塊の中の加算 (8 個ずつに埋める)
 size_t cnt= 0;
 long long scan= 0, budget;
 vector<unsigned char> pc;
 IntervalSum(int n_, const vector<u32>& a): n(n_), full((u32(1) << n_) - 1), v(a) {
  const size_t N= size_t(1) << n;
  s.assign(N, 0), mv.assign(N, 0), ms.assign(N, 0);
  if(SB_TRI) t.assign(N, 0), mt.assign(N, 0);
  pc.assign(N, 0);
  for(size_t i= 1; i < N; ++i) pc[i]= pc[i >> 1] + (i & 1);
  // 作り直しは長さ 2^n の変換が 2 回か 4 回。塊の中の加算 1 つを足す手間は、ベクトルなら変換の 1 要素と同じくらいと見る
  const long long N2= (long long)1 << n;
  budget= SB_ADAPT ? (long long)(SB_BF) * n * N2 : (long long)max<size_t>(2, size_t(sqrt(double(size_t(n) << n))));
  build();
 }
 void build() {
  const size_t N= size_t(1) << n;
  zeta_sup(ms.data(), n);
  if(SB_TRI) zeta_sub(mt.data(), n);
  for(size_t j= 0; j < N; ++j) v[j]+= mv[j] + ms[j] + (SB_TRI ? mt[j] : 0u);
  s= v, zeta_sub(s.data(), n);
  if(SB_TRI) t= v, zeta_sup(t.data(), n), fill(mt.begin(), mt.end(), 0u);
  fill(mv.begin(), mv.end(), 0u), fill(ms.begin(), ms.end(), 0u);
  cnt= 0, scan= 0;
  ua.clear(), ub.clear(), uk.clear();
 }
 // 塊より前の分 (v、s、t) の和
 u32 base(u32 x, u32 y) const {
  const u32 fr= x ^ y, zr= full ^ y;
  const int cf= pc[fr], c1= pc[x], c0= SB_TRI ? pc[zr] : 99;
  u32 r= 0;
  if(cf <= c1 && cf <= c0) {
   for(u32 d= fr;; d= (d - 1) & fr) {
    r+= v[x | d];
    if(!d) break;
   }
  } else if(c1 <= c0) {
   for(u32 d= x;; d= (d - 1) & x) {
    r+= (pc[d] & 1) ? 0u - s[y ^ d] : s[y ^ d];
    if(!d) break;
   }
  } else {
   for(u32 e= zr;; e= (e - 1) & zr) {
    r+= (pc[e] & 1) ? 0u - t[x | e] : t[x | e];
    if(!e) break;
   }
  }
  return r;
 }
 void add(u32 x, u32 y, u32 k) {
  if((x & y) != x) return;
  if(cnt % 8 == 0) ua.resize(cnt + 8, full), ub.resize(cnt + 8, 0), uk.resize(cnt + 8, 0);  // 埋め草は a ∪ a' ⊆ b ∩ b' にならない
  ua[cnt]= x, ub[cnt]= y, uk[cnt]= k, ++cnt;
  const u32 fr= x ^ y, zr= full ^ y;
  const int cf= pc[fr], c1= pc[x], c0= SB_TRI ? pc[zr] : 99;
  if(cf <= c1 && cf <= c0) {
   for(u32 d= fr;; d= (d - 1) & fr) {
    mv[x | d]+= k;
    if(!d) break;
   }
  } else if(c1 <= c0) {
   for(u32 d= x;; d= (d - 1) & x) {
    ms[y ^ d]+= (pc[d] & 1) ? 0u - k : k;
    if(!d) break;
   }
  } else {
   for(u32 e= zr;; e= (e - 1) & zr) {
    mt[x | e]+= (pc[e] & 1) ? 0u - k : k;
    if(!e) break;
   }
  }
  if(!SB_ADAPT && (long long)cnt >= budget) build();
 }
 u32 inblock(u32 x, u32 y) const {
#if SB_SIMD
  const __m256i X= _mm256_set1_epi32(int(x)), Y= _mm256_set1_epi32(int(y)), Z= _mm256_setzero_si256();
  const __m256i lut= _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
  const __m256i m4= _mm256_set1_epi8(0x0F), one8= _mm256_set1_epi8(1), one16= _mm256_set1_epi16(1);
  __m256i acc= Z;
  for(size_t i= 0; i < cnt; i+= 8) {
   const __m256i A= _mm256_loadu_si256((const __m256i*)(ua.data() + i)), Bv= _mm256_loadu_si256((const __m256i*)(ub.data() + i)), K= _mm256_loadu_si256((const __m256i*)(uk.data() + i));
   const __m256i lo= _mm256_or_si256(X, A), hi= _mm256_and_si256(Y, Bv);
   const __m256i ok= _mm256_cmpeq_epi32(_mm256_andnot_si256(hi, lo), Z);
   const __m256i fr= _mm256_xor_si256(hi, lo);
   const __m256i c8= _mm256_add_epi8(_mm256_shuffle_epi8(lut, _mm256_and_si256(fr, m4)), _mm256_shuffle_epi8(lut, _mm256_and_si256(_mm256_srli_epi32(fr, 4), m4)));
   const __m256i c32= _mm256_madd_epi16(_mm256_maddubs_epi16(c8, one8), one16);
   acc= _mm256_add_epi32(acc, _mm256_and_si256(_mm256_sllv_epi32(K, c32), ok));
  }
  alignas(32) u32 tmp[8];
  _mm256_store_si256((__m256i*)tmp, acc);
  return tmp[0] + tmp[1] + tmp[2] + tmp[3] + tmp[4] + tmp[5] + tmp[6] + tmp[7];
#else
  u32 r= 0;
  for(size_t i= 0; i < cnt; ++i) {
   const u32 lo= x | ua[i], hi= y & ub[i];
   if((lo & hi) == lo) r+= uk[i] << pc[lo ^ hi];
  }
  return r;
#endif
 }
 u32 sum(u32 x, u32 y) {
  if((x & y) != x) return 0;
  if(SB_ADAPT && scan + (long long)cnt > budget) build();
  scan+= (long long)cnt;
  return base(x, y) + inblock(x, y);
 }
};
}  // namespace sbk
struct Solver {
 int n;
 vector<u32> a;
 vector<array<u32, 4>> ops;
 vector<u32> ans;
 Solver(int n_, const vector<u32>& a_, const vector<array<u32, 4>>& ops_): n(n_), a(a_), ops(ops_) {}
 void run() {
  sbk::IntervalSum st(n, a);
  ans.reserve(ops.size());
  for(auto& o: ops)
   if(o[0] == 1) st.add(o[1], o[2], o[3]);
   else ans.push_back(st.sum(o[1], o[2]));
 }
 vector<u32> answer() const { return ans; }
};
