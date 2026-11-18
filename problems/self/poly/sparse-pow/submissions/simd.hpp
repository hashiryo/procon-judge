#pragma once
// SIMD の実装。u = f / f(0) の u^m を、v_i = (Σ_j al_j v_{i-j}) / i - Σ_j u_j v_{i-j} (al_j = (m + 1) j u_j) で、8 項ずつの
// 塊にして求める。塊より前の値からの寄与は 8 個の出力と前の値の Toeplitz の積なので、前の値 v_p を 1 つずつ 8 レーンに配り、
// 重みの列を塊の先頭からの距離 s だけずらして読んで、AVX2 で u64 の積和にためる (偶数番目の出力は s から、奇数番目は s + 1
// から読むと、_mm256_mul_epu32 が使う下位 32 bit にそろう)。8 回ためるたびに 2^32 の位を 2^32 mod M 倍して下へ畳む。
// 塊の中の寄与はスカラーで足し、1 つ前の値の項 (j = 1) だけを待ちの鎖に残す。重みは f の次数 D の長さの密な列で持つので、
// 次数が高くて項の少ない疎な f には向かない。1/i の表は 8 本の鎖の AVX2 の Montgomery の積で作る。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <utility>
#include <vector>
namespace sp_simd {
constexpr u32 M= 998244353;
constexpr u64 BX= ~u64(0) / M;
constexpr u32 MI= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
constexpr u32 R32= u32((u64(1) << 32) % M);
inline u32 red(u64 t) {
 u64 r= t - u64((__uint128_t(t) * BX) >> 64) * M;
 r= r >= 2 * u64(M) ? r - 2 * u64(M) : r;
 return u32(r >= M ? r - M : r);
}
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= u32(u64(a) * a % M))
  if(e & 1) r= u32(u64(r) * a % M);
 return r;
}
// x y 2^{-32} mod M を 8 個ずつ (x、y が 2M 未満なら結果も 2M 未満)。
inline __m256i mont8(__m256i x, __m256i y) {
 const __m256i mv= _mm256_set1_epi32(M), mi= _mm256_set1_epi32(MI);
 const __m256i te= _mm256_mul_epu32(x, y), to= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
 const __m256i ue= _mm256_add_epi64(te, _mm256_mul_epu32(_mm256_mul_epu32(te, mi), mv));
 const __m256i uo= _mm256_add_epi64(to, _mm256_mul_epu32(_mm256_mul_epu32(to, mi), mv));
 return _mm256_blend_epi32(_mm256_srli_epi64(ue, 32), uo, 0b10101010);
}
// iv[i] = 1/i (1 ≤ i < n、iv は 8 ((n + 6) / 8) + 8 個の置き場)。i ≡ b (mod 8) の累積積をレーン b の鎖で作り、総積の
// 逆元から後ろ向きに戻す (Montgomery の 2^{-32} は前向きと後ろ向きで打ち消し合う)。
inline void inverses(u32* iv, int n) {
 if(n <= 1) return;
 const __m256i mv= _mm256_set1_epi32(M), eight= _mm256_set1_epi32(8);
 const int steps= (n - 1 + 7) / 8;
 __m256i acc= _mm256_set1_epi32(1), idx= _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
 for(int t= 0; t < steps; ++t) {
  acc= mont8(acc, idx);
  _mm256_storeu_si256((__m256i*)(iv + 1 + 8 * t), acc);
  idx= _mm256_add_epi32(idx, eight);
 }
 alignas(32) u32 a[8];
 _mm256_store_si256((__m256i*)a, acc);
 for(int b= 0; b < 8; ++b) a[b]= pw(a[b] % M, M - 2);
 __m256i cur= _mm256_load_si256((const __m256i*)a);
 idx= _mm256_sub_epi32(idx, eight);
 for(int t= steps - 1; t >= 0; --t) {
  const __m256i pre= t ? _mm256_loadu_si256((const __m256i*)(iv + 1 + 8 * (t - 1))) : _mm256_set1_epi32(1);
  const __m256i r= mont8(cur, pre);
  _mm256_storeu_si256((__m256i*)(iv + 1 + 8 * t), _mm256_min_epu32(r, _mm256_sub_epi32(r, mv)));
  cur= mont8(cur, idx);
  idx= _mm256_sub_epi32(idx, eight);
 }
}
}  // namespace sp_simd
inline vector<u32> run(int n, u64 m, const vector<pair<int, u32>>& f) {
 using namespace sp_simd;
 vector<u32> v(n, 0);
 if(n == 0) return v;
 const u32 f0= f[0].second, if0= pw(f0, M - 2), m1= u32((m % M + 1) % M);
 int D= 0;
 for(auto& [j, a]: f)
  if(j < n) D= std::max(D, j);
 // 重みの列 (j = 1 は鎖で扱うので 0 にしておく)。読むのは添字 D + 8 まで。
 vector<u32> wa(D + 16, 0), wb(D + 16, 0);
 u32 a1= 0, b1= 0;
 for(size_t k= 1; k < f.size() && f[k].first < n; ++k) {
  const int j= f[k].first;
  const u32 uk= u32(u64(f[k].second) * if0 % M), ak= u32(u64(uk) * m1 % M * u32(j) % M);
  if(j == 1) a1= ak, b1= uk;
  else wa[j]= ak, wb[j]= uk;
 }
 vector<u32> iv(8 * ((n + 6) / 8) + 8);
 inverses(iv.data(), n);
 v[0]= 1;
 const __m256i r32= _mm256_set1_epi64x(R32), lo32= _mm256_set1_epi64x(0xffffffffll);
 auto fold= [&](__m256i x) { return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), r32), _mm256_and_si256(x, lo32)); };
 alignas(32) u64 ae[4], ao[4], be[4], bo[4];
 u32 prev= 1;
 for(int i0= 1; i0 < n; i0+= 8) {
  __m256i sae= _mm256_setzero_si256(), sao= sae, sbe= sae, sbo= sae;
  // 古い値から足し、塊の直前の値は最後に足す (鎖の待ちを短くする)
  for(int p= std::max(0, i0 - D), cnt= 0; p < i0; ++p) {
   const __m256i x= _mm256_set1_epi32(int(v[p]));
   const int s= i0 - p;
   sae= _mm256_add_epi64(sae, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(wa.data() + s)), x));
   sao= _mm256_add_epi64(sao, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(wa.data() + s + 1)), x));
   sbe= _mm256_add_epi64(sbe, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(wb.data() + s)), x));
   sbo= _mm256_add_epi64(sbo, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(wb.data() + s + 1)), x));
   if(++cnt == 8) sae= fold(sae), sao= fold(sao), sbe= fold(sbe), sbo= fold(sbo), cnt= 0;
  }
  _mm256_store_si256((__m256i*)ae, fold(sae)), _mm256_store_si256((__m256i*)ao, fold(sao));
  _mm256_store_si256((__m256i*)be, fold(sbe)), _mm256_store_si256((__m256i*)bo, fold(sbo));
  const int e= std::min(8, n - i0);
  for(int t= 0; t < e; ++t) {
   const int i= i0 + t;
   u64 sa= (t & 1 ? ao : ae)[t >> 1], sb= (t & 1 ? bo : be)[t >> 1];  // 2^62 未満
   for(int j= 2; j <= t && j <= D; ++j) sa+= u64(wa[j]) * v[i - j], sb+= u64(wb[j]) * v[i - j];
   const u32 ivi= iv[i];
   const u32 x= red(u64(red(sa)) * ivi), b= red(sb), dl= x >= b ? x - b : x + M - b;
   const u32 y= red(u64(a1) * ivi), gm= y >= b1 ? y - b1 : y + M - b1;
   v[i]= prev= red(u64(prev) * gm + dl);
  }
 }
 const u32 a= pw(f0, m);
 if(a != 1) {
  const __m256i va= _mm256_set1_epi32(int(u32((u64(a) << 32) % M))), mv= _mm256_set1_epi32(M);
  int i= 0;
  for(; i + 8 <= n; i+= 8) {
   const __m256i r= mont8(_mm256_loadu_si256((const __m256i*)(v.data() + i)), va);
   _mm256_storeu_si256((__m256i*)(v.data() + i), _mm256_min_epu32(r, _mm256_sub_epi32(r, mv)));
  }
  for(; i < n; ++i) v[i]= u32(u64(v[i]) * a % M);
 }
 return v;
}
