#pragma once
// zmod_pf_blk8 の掃き出しを、行列式の zmod_det_hpv と同じく、要を探す列、要の行、行ごとの掛ける数を AVX2 で 4 個ずつ求め、
// 行列を huge page に置く形 (_shared/linalg/lazy_elim_v.hpp の HugeBuf と Mont) にしたもの。法は 998244353。
// 手順は zmod_pf_blk8 と同じで、列 c の要を位置 c + 1 以降の行から選んで位置 c + 1 に置き (行と同じ番号の列を入れ替えると
// 符号が変わる)、c が偶数なら -(要の値) を掛け、位置 c + 2 以降の行を消す。c が奇数の列に要がなければ次の列へ進む。
//   - 組の初めに、位置 first 以降の行の次の 8 列を、位置で引く列ごとの配列 C に 4 × 4 の転置で写す。行ごとの掛ける数 F も
//     位置で引く列ごとの配列に持つ。
//   - 位置 c + 1 と sel の行を入れ替えるときは、C、F、V の位置 c + 1 と sel も入れ替える。列 c + 1 と sel を入れ替えるとき、
//     両方が写した 8 列の中なら C の 2 列を入れ替え、c + 1 だけが中なら、入れ替えたあとの行から C の列 c + 1 を読み直す。
//   - 値は Montgomery の還元で M 未満に直し、要の行を 2^{-32} 倍、掛ける数を 2^32 倍で持つ。偶数の列で掛ける要の値も 2^{-32}
//     倍なので、最後に掛けた数だけ 2^32 を掛けて戻す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim_v.hpp"
inline u32 run(int N, const vector<vector<u32>>& M_in) {
 using Z= ZMod<MOD>;
 using namespace lazy_elim_v;
 const int n= 2 * N, w= (n + 3) & ~3, n4= w;
 const u64 R= (u64(1) << 32) % MOD;
 const Mont mt(MOD);
 HugeBuf<u64> A((size_t)n * w + 8);
 for(int i= 0; i < n; ++i) {
  u64* r= A.p + (size_t)i * w;
  for(int j= 0; j < n; ++j) r[j]= M_in[i][j];
  for(int j= n; j < w; ++j) r[j]= 0;
 }
 fill(A.p + (size_t)n * w, A.p + (size_t)n * w + 8, 0ull);
 auto row= [&](int r) { return A.p + (size_t)r * w; };
 vector<int> pos(n);  // pos[t] は位置 t にある行
 iota(pos.begin(), pos.end(), 0);
 vector<u64> C((size_t)K * n4), U((size_t)K * w + 8), zero(w + 8);
 vector<u32> F((size_t)K * n4), V(n4);
 const u64* Ub[K];
 Z res= Z::raw(1);
 int bt= 0, first= 1, c0= -1000, ne= 0;  // first は残りの行の最初の位置、c0 は C に写した最初の列、ne は掛けた要の値の数
 auto fill_cache= [&](int cs) {
  c0= cs;
  for(int t= first & ~3; t < n; t+= 4) {
   const u64* r0= row(pos[t]) + cs;
   const u64* r1= t + 1 < n ? row(pos[t + 1]) + cs : zero.data();
   const u64* r2= t + 2 < n ? row(pos[t + 2]) + cs : zero.data();
   const u64* r3= t + 3 < n ? row(pos[t + 3]) + cs : zero.data();
   for(int h= 0; h < 8; h+= 4) {
    const __m256i x0= _mm256_loadu_si256((const __m256i*)(r0 + h)), x1= _mm256_loadu_si256((const __m256i*)(r1 + h));
    const __m256i x2= _mm256_loadu_si256((const __m256i*)(r2 + h)), x3= _mm256_loadu_si256((const __m256i*)(r3 + h));
    const __m256i t0= _mm256_unpacklo_epi64(x0, x1), t1= _mm256_unpackhi_epi64(x0, x1);
    const __m256i t2= _mm256_unpacklo_epi64(x2, x3), t3= _mm256_unpackhi_epi64(x2, x3);
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 0) * n4 + t], _mm256_permute2x128_si256(t0, t2, 0x20));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 1) * n4 + t], _mm256_permute2x128_si256(t1, t3, 0x20));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 2) * n4 + t], _mm256_permute2x128_si256(t0, t2, 0x31));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 3) * n4 + t], _mm256_permute2x128_si256(t1, t3, 0x31));
   }
  }
 };
 auto flush= [&](int col) {
  if(!bt) return;
  for(int s= bt; s < K; ++s) Ub[s]= zero.data();
  for(int t= first; t < n; ++t) {
   u32 f[K], any= 0;
   for(int s= 0; s < K; ++s) any|= (f[s]= s < bt ? F[(size_t)s * n4 + t] : 0);
   if(any) lazy_elim::update_row(row(pos[t]), Ub, f, col & ~3, w, R);
  }
  for(int s= 0; s < bt; ++s) fill(&F[(size_t)s * n4], &F[(size_t)s * n4] + n4, 0u);
  bt= 0, c0= -1000;  // 行が変わったので写し直す
 };
 for(int c= 0; c + 1 < n; ++c) {
  if(c < c0 || c >= c0 + 8) fill_cache(c);
  const u64* cc= &C[(size_t)(c - c0) * n4];
  __m256i us[K];
  for(int s= 0; s < bt; ++s) us[s]= _mm256_set1_epi64x(Ub[s][c]);
  int sel= -1;
  for(int t= first & ~3; t < n4; t+= 4) {
   __m256i x= _mm256_loadu_si256((const __m256i*)(cc + t));
   for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&F[(size_t)s * n4 + t])), us[s]));
   const __m256i y= mt.redc(x);
   _mm_storeu_si128((__m128i*)&V[t], pack_lo(y));
   if(sel < 0) {
    int nz= ~_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(y, _mm256_setzero_si256()))) & 15;
    if(t < first) nz&= ~((1 << (first - t)) - 1);  // 位置 first より前は終わった行
    if(nz) sel= t + __builtin_ctz(nz);
   }
  }
  if(sel >= n) sel= -1;
  if(sel < 0) {  // 列 c が消えている。c が奇数なら積に入らないので次の列へ (位置 c + 1 の行はもう使わない)
   if(!(c & 1)) return 0;
   first= c + 2;
   continue;
  }
  if(sel != c + 1) {
   swap(pos[c + 1], pos[sel]), swap(V[c + 1], V[sel]);
   for(int q= 0; q < K; ++q) swap(C[(size_t)q * n4 + c + 1], C[(size_t)q * n4 + sel]);
   for(int s= 0; s < bt; ++s) swap(F[(size_t)s * n4 + c + 1], F[(size_t)s * n4 + sel]);
   for(int t= c + 1; t < n; ++t) {
    u64* r= row(pos[t]);
    swap(r[c + 1], r[sel]);
   }
   for(int s= 0; s < bt; ++s) swap(U[(size_t)s * w + c + 1], U[(size_t)s * w + sel]);
   const int q1= c + 1 - c0, q2= sel - c0;
   if(q1 < 8 && q2 < 8) {
    for(int t= first & ~3; t < n4; ++t) swap(C[(size_t)q1 * n4 + t], C[(size_t)q2 * n4 + t]);
   } else if(q1 < 8) {
    for(int t= first; t < n; ++t) C[(size_t)q1 * n4 + t]= row(pos[t])[c + 1];
   }
   res= -res;
  }
  const int p= pos[c + 1];
  if(!(c & 1)) res*= -Z::raw(V[c + 1]), ++ne;
  u64* up= &U[(size_t)bt * w];
  {  // 要の行 U[j] = (a[p][j] + sum_s F[s][c + 1] U[s][j]) 2^{-32} mod M
   __m256i fs[K];
   for(int s= 0; s < bt; ++s) fs[s]= _mm256_set1_epi64x(F[(size_t)s * n4 + c + 1]);
   const u64* ap= row(p);
   for(int j= c & ~3; j < w; j+= 4) {
    __m256i x= _mm256_loadu_si256((const __m256i*)(ap + j));
    for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(Ub[s] + j)), fs[s]));
    _mm256_storeu_si256((__m256i*)(up + j), mt.redc(x));
   }
  }
  {  // F[bt][t] = -V[t] cinv mod M (t >= c + 2、cinv = V[c + 1]^{-1} 2^32)。Shoup の方法で 4 個ずつ
   const u32 ci= (Z::raw(V[c + 1]).inv() * Z::raw(u32(R))).val(), cs= u32((u64(ci) << 32) / MOD);
   const __m256i Cv= _mm256_set1_epi64x(ci), CS= _mm256_set1_epi64x(cs);
   u32* fb= &F[(size_t)bt * n4];
   for(int t= (c + 2) & ~3; t < n4; t+= 4) {
    const __m256i vv= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&V[t]));
    const __m256i qq= _mm256_srli_epi64(_mm256_mul_epu32(vv, CS), 32);
    __m256i x= _mm256_sub_epi64(_mm256_mul_epu32(vv, Cv), _mm256_mul_epu32(qq, mt.M));
    x= _mm256_min_epu32(x, _mm256_sub_epi32(x, mt.M32));
    x= _mm256_sub_epi32(mt.M32, x);
    x= _mm256_min_epu32(x, _mm256_sub_epi32(x, mt.M32));
    _mm_storeu_si128((__m128i*)(fb + t), pack_lo(x));
   }
  }
  Ub[bt++]= up;
  first= c + 2;
  if(bt == K) flush(c + 1);
 }
 res*= Z(R).pow(ne);  // 掛けた要の値は 2^{-32} 倍
 return res.val();
}
