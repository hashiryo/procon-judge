#pragma once
// zmod_inv_relabel の前進消去を、要を探す列、要の行、行ごとの掛ける数を AVX2 で 4 個ずつ求め、行列と要の行を huge page に置く
// 形 (_shared/linalg/lazy_elim_vk.hpp の ElimK の領域と後退代入) にしたもの。法は 998244353。
// 単位行列の列の付け替えは、zmod_inv_relabel と同じく右半分の列だけを触る。組の初めに写す要を探す列の配列は左半分の列なので、
// 付け替えの影響を受けない (n の近くで写した右半分の列は読まない)。要の行と要の値は 2^{-32} 倍で持つが、上三角の行列の各行と
// 右辺が同じ倍率なので、後退代入の解は変わらない。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim_vk.hpp"
inline InverseResult run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 using namespace lazy_elim_vk;
 const int w= (2 * n + 3) & ~3, n4= (n + 3) & ~3;
 const u64 R= (u64(1) << 32) % MOD;
 const Mont mt(MOD);
 ElimK<Z> e(n, w);
 for(int i= 0; i < n; ++i) {
  u64* r= e.row(i);
  fill(r, r + w, 0ull);
  for(int j= 0; j < n; ++j) r[j]= a[i][j];
  r[n + i]= 1;
 }
 fill(e.row(n), e.row(n) + 8, 0ull);
 vector<u64> C((size_t)K * n4), zero(w + 8);
 vector<u32> F((size_t)K * n4), V(n4);
 vector<int> pos2row(n), row2pos(n);
 iota(pos2row.begin(), pos2row.end(), 0), iota(row2pos.begin(), row2pos.end(), 0);
 const u64* Ub[K];
 auto fill_cache= [&](int c0) {
  const int nr= e.rem.size();
  for(int i= 0; i < nr; i+= 4) {
   const u64* r0= e.row(e.rem[i]) + c0;
   const u64* r1= i + 1 < nr ? e.row(e.rem[i + 1]) + c0 : zero.data();
   const u64* r2= i + 2 < nr ? e.row(e.rem[i + 2]) + c0 : zero.data();
   const u64* r3= i + 3 < nr ? e.row(e.rem[i + 3]) + c0 : zero.data();
   for(int h= 0; h < 8; h+= 4) {
    const __m256i x0= _mm256_loadu_si256((const __m256i*)(r0 + h)), x1= _mm256_loadu_si256((const __m256i*)(r1 + h));
    const __m256i x2= _mm256_loadu_si256((const __m256i*)(r2 + h)), x3= _mm256_loadu_si256((const __m256i*)(r3 + h));
    const __m256i t0= _mm256_unpacklo_epi64(x0, x1), t1= _mm256_unpackhi_epi64(x0, x1);
    const __m256i t2= _mm256_unpacklo_epi64(x2, x3), t3= _mm256_unpackhi_epi64(x2, x3);
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 0) * n4 + i], _mm256_permute2x128_si256(t0, t2, 0x20));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 1) * n4 + i], _mm256_permute2x128_si256(t1, t3, 0x20));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 2) * n4 + i], _mm256_permute2x128_si256(t0, t2, 0x31));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 3) * n4 + i], _mm256_permute2x128_si256(t1, t3, 0x31));
   }
  }
 };
 for(int col= 0; col < n;) {
  int bt= 0, c0= col;
  fill_cache(c0);
  for(; bt < K && col < n; ++col) {
   if(col == c0 + 8) fill_cache(c0= col);
   const int nr= e.rem.size(), nr4= (nr + 3) & ~3;
   const u64* cc= &C[(size_t)(col - c0) * n4];
   __m256i us[K];
   for(int s= 0; s < bt; ++s) us[s]= _mm256_set1_epi64x(Ub[s][col]);
   int pi= -1;
   for(int i= 0; i < nr4; i+= 4) {
    __m256i x= _mm256_loadu_si256((const __m256i*)(cc + i));
    for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&F[(size_t)s * n4 + i])), us[s]));
    const __m256i y= mt.redc(x);
    _mm_storeu_si128((__m128i*)&V[i], pack_lo(y));
    if(pi < 0) {
     const int nz= ~_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(y, _mm256_setzero_si256()))) & 15;
     if(nz) pi= i + __builtin_ctz(nz);
    }
   }
   if(pi < 0 || pi >= nr) return {false, {}};
   const int p= e.rem[pi], t= e.prow.size(), q= row2pos[p];
   if(q != t) {  // 行 p の 1 を位置 t に、位置 t にあった行 r2 の 1 を位置 q に移す
    const int r2= pos2row[t];
    u64 *ap= e.row(p), *ar= e.row(r2);
    ap[n + q]= 0, ap[n + t]= 1, ar[n + t]= 0, ar[n + q]= 1;
    pos2row[q]= r2, row2pos[r2]= q, pos2row[t]= p, row2pos[p]= t;
   }
   u64* up= e.ubuf.p + (size_t)t * w;
   {
    __m256i fs[K];
    for(int s= 0; s < bt; ++s) fs[s]= _mm256_set1_epi64x(F[(size_t)s * n4 + pi]);
    const u64* ap= e.row(p);
    const int je= min(w, (n + t + 1 + 3) & ~3);
    for(int j= col & ~3; j < je; j+= 4) {
     __m256i x= _mm256_loadu_si256((const __m256i*)(ap + j));
     for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(Ub[s] + j)), fs[s]));
     _mm256_storeu_si256((__m256i*)(up + j), mt.redc(x));
    }
    fill(up + je, up + w, 0ull);  // 右半分の位置 t より先は 0 (後退代入が読む)
   }
   {
    const u32 c= (Z::raw(V[pi]).inv() * Z::raw(u32(R))).val(), cs= u32((u64(c) << 32) / MOD);
    const __m256i Cv= _mm256_set1_epi64x(c), CS= _mm256_set1_epi64x(cs);
    u32* fb= &F[(size_t)bt * n4];
    for(int i= 0; i < nr4; i+= 4) {
     const __m256i vv= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&V[i]));
     const __m256i qq= _mm256_srli_epi64(_mm256_mul_epu32(vv, CS), 32);
     __m256i x= _mm256_sub_epi64(_mm256_mul_epu32(vv, Cv), _mm256_mul_epu32(qq, mt.M));
     x= _mm256_min_epu32(x, _mm256_sub_epi32(x, mt.M32));
     x= _mm256_sub_epi32(mt.M32, x);
     x= _mm256_min_epu32(x, _mm256_sub_epi32(x, mt.M32));
     _mm_storeu_si128((__m128i*)(fb + i), pack_lo(x));
    }
   }
   e.prow.push_back(p), e.pcol.push_back(col), e.pval.push_back(V[pi]);
   Ub[bt++]= up;
   const int last= nr - 1;
   e.rem[pi]= e.rem[last], e.rem.pop_back();
   for(int s= 0; s < bt; ++s) F[(size_t)s * n4 + pi]= F[(size_t)s * n4 + last], F[(size_t)s * n4 + last]= 0;
   for(int qq= 0; qq < K; ++qq) C[(size_t)qq * n4 + pi]= C[(size_t)qq * n4 + last], C[(size_t)qq * n4 + last]= 0;
  }
  for(int s= bt; s < K; ++s) Ub[s]= zero.data();
  const int jb= col & ~3, je= min(w, (n + (int)e.prow.size() + 3) & ~3), nr= e.rem.size();
  for(int i= 0; i < nr; ++i) {
   u32 f[K], any= 0;
   for(int s= 0; s < K; ++s) any|= (f[s]= s < bt ? F[(size_t)s * n4 + i] : 0);
   if(any) lazy_elim::update_row(e.row(e.rem[i]), Ub, f, jb, je, R);
  }
  for(int s= 0; s < bt; ++s) fill(&F[(size_t)s * n4], &F[(size_t)s * n4] + n4, 0u);
 }
 vector<int> cols(n);
 iota(cols.begin(), cols.end(), n);
 int wx;
 const auto X= tri_solve_blk(e, cols, wx);
 InverseResult res{true, vector<vector<u32>>(n, vector<u32>(n))};
 for(int t= 0; t < n; ++t)
  for(int q= 0; q < n; ++q) res.mat[t][pos2row[q]]= u32(X[(size_t)t * wx + q]);
 return res;
}
