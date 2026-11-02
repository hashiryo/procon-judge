#pragma once
// lazy_elim_v.hpp の ElimV (要を探す列、要の行、行ごとの掛ける数を AVX2 で 4 個ずつ求める前進消去) を、後退代入に使う形に
// 広げたもの。yosupo の system-of-linear-equations の提出が読む。記録は algo-notes の notes/linear-algebra.md。
//   - 要の行をすべて持ち (urow)、要の列と、要の値の 2^{-32} 倍 (pval) を返す。要の行も本当の値の 2^{-32} 倍なので、要の列が
//     作る上三角の行列の各行と右辺が同じ倍率になり、T X = Y の解は変わらない。後退代入は lazy_elim::tri_solve_blk と同じ手順。
//   - 最後の組も残りの行に足す (残りの行の右辺を前進消去のあとで読むため)。
//   - 行列と要の行は huge page の領域に置く。
#include "_shared/linalg/lazy_elim_v.hpp"
namespace lazy_elim_vk {
using u32= unsigned;
using u64= unsigned long long;
using lazy_elim_v::HugeBuf;
using lazy_elim_v::Mont;
using lazy_elim_v::pack_lo;
constexpr int K= 8;
template <class Z> struct ElimK {
 int n, w;
 HugeBuf<u64> buf, ubuf;
 std::vector<int> rem, prow, pcol;
 std::vector<u32> pval;  // 要の値の 2^{-32} 倍
 ElimK(int n_, int w_): n(n_), w(w_), buf((std::size_t)n_ * w_ + 8), ubuf((std::size_t)n_ * w_ + 8), rem(n_) {
  for(int i= 0; i < n; ++i) rem[i]= i;
 }
 u64* row(int r) { return buf.p + (std::size_t)r * w; }
 const u64* row(int r) const { return buf.p + (std::size_t)r * w; }
 const u64* urow(int t) const { return ubuf.p + (std::size_t)t * w; }
 u32 value(int r, int j) const { return Z(row(r)[j]).val(); }
 // 列 [0, lim) で要を探しながら前進消去する。要のない列は飛ばす。
 void run(int lim) {
  const u32 M= Z::mod();
  const u64 R= (u64(1) << 32) % M;
  const Mont mt(M);
  const int n4= (n + 3) & ~3;
  std::vector<u64> C((std::size_t)K * n4), zero(w + 8);
  std::vector<u32> F((std::size_t)K * n4), V(n4);
  const u64* Ub[K];
  int col= 0;
  auto fill_cache= [&](int c0) {
   const int nr= rem.size();
   for(int i= 0; i < nr; i+= 4) {
    const u64* r0= row(rem[i]) + c0;
    const u64* r1= i + 1 < nr ? row(rem[i + 1]) + c0 : zero.data();
    const u64* r2= i + 2 < nr ? row(rem[i + 2]) + c0 : zero.data();
    const u64* r3= i + 3 < nr ? row(rem[i + 3]) + c0 : zero.data();
    for(int h= 0; h < 8; h+= 4) {
     const __m256i x0= _mm256_loadu_si256((const __m256i*)(r0 + h)), x1= _mm256_loadu_si256((const __m256i*)(r1 + h));
     const __m256i x2= _mm256_loadu_si256((const __m256i*)(r2 + h)), x3= _mm256_loadu_si256((const __m256i*)(r3 + h));
     const __m256i t0= _mm256_unpacklo_epi64(x0, x1), t1= _mm256_unpackhi_epi64(x0, x1);
     const __m256i t2= _mm256_unpacklo_epi64(x2, x3), t3= _mm256_unpackhi_epi64(x2, x3);
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 0) * n4 + i], _mm256_permute2x128_si256(t0, t2, 0x20));
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 1) * n4 + i], _mm256_permute2x128_si256(t1, t3, 0x20));
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 2) * n4 + i], _mm256_permute2x128_si256(t0, t2, 0x31));
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 3) * n4 + i], _mm256_permute2x128_si256(t1, t3, 0x31));
    }
   }
  };
  while(col < lim && !rem.empty()) {
   int bt= 0, c0= col;
   fill_cache(c0);
   for(; bt < K && col < lim && !rem.empty(); ++col) {
    if(col == c0 + 8) fill_cache(c0= col);
    const int nr= rem.size(), nr4= (nr + 3) & ~3;
    const u64* cc= &C[(std::size_t)(col - c0) * n4];
    __m256i us[K];
    for(int s= 0; s < bt; ++s) us[s]= _mm256_set1_epi64x(Ub[s][col]);
    int pi= -1;
    for(int i= 0; i < nr4; i+= 4) {
     __m256i x= _mm256_loadu_si256((const __m256i*)(cc + i));
     for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&F[(std::size_t)s * n4 + i])), us[s]));
     const __m256i y= mt.redc(x);
     _mm_storeu_si128((__m128i*)&V[i], pack_lo(y));
     if(pi < 0) {
      const int nz= ~_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(y, _mm256_setzero_si256()))) & 15;
      if(nz) pi= i + __builtin_ctz(nz);
     }
    }
    if(pi >= nr) pi= -1;
    if(pi < 0) continue;
    const int p= rem[pi];
    u64* up= ubuf.p + prow.size() * (std::size_t)w;
    {
     __m256i fs[K];
     for(int s= 0; s < bt; ++s) fs[s]= _mm256_set1_epi64x(F[(std::size_t)s * n4 + pi]);
     const u64* ap= row(p);
     for(int j= col & ~3; j < w; j+= 4) {
      __m256i x= _mm256_loadu_si256((const __m256i*)(ap + j));
      for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(Ub[s] + j)), fs[s]));
      _mm256_storeu_si256((__m256i*)(up + j), mt.redc(x));
     }
    }
    {
     const u32 c= (Z::raw(V[pi]).inv() * Z::raw(u32(R))).val(), cs= u32((u64(c) << 32) / M);
     const __m256i Cv= _mm256_set1_epi64x(c), CS= _mm256_set1_epi64x(cs);
     u32* fb= &F[(std::size_t)bt * n4];
     for(int i= 0; i < nr4; i+= 4) {
      const __m256i vv= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&V[i]));
      const __m256i q= _mm256_srli_epi64(_mm256_mul_epu32(vv, CS), 32);
      __m256i t= _mm256_sub_epi64(_mm256_mul_epu32(vv, Cv), _mm256_mul_epu32(q, mt.M));
      t= _mm256_min_epu32(t, _mm256_sub_epi32(t, mt.M32));
      t= _mm256_sub_epi32(mt.M32, t);
      t= _mm256_min_epu32(t, _mm256_sub_epi32(t, mt.M32));
      _mm_storeu_si128((__m128i*)(fb + i), pack_lo(t));
     }
    }
    prow.push_back(p), pcol.push_back(col), pval.push_back(V[pi]);
    Ub[bt++]= up;
    const int last= nr - 1;
    rem[pi]= rem[last], rem.pop_back();
    for(int s= 0; s < bt; ++s) F[(std::size_t)s * n4 + pi]= F[(std::size_t)s * n4 + last], F[(std::size_t)s * n4 + last]= 0;
    for(int q= 0; q < K; ++q) C[(std::size_t)q * n4 + pi]= C[(std::size_t)q * n4 + last], C[(std::size_t)q * n4 + last]= 0;
   }
   if(!bt) break;
   for(int s= bt; s < K; ++s) Ub[s]= zero.data();
   const int jb= col & ~3, nr= rem.size();
   for(int i= 0; i < nr; ++i) {
    u32 f[K], any= 0;
    for(int s= 0; s < K; ++s) any|= (f[s]= s < bt ? F[(std::size_t)s * n4 + i] : 0);
    if(any) lazy_elim::update_row(row(rem[i]), Ub, f, jb, w, R);
   }
   for(int s= 0; s < bt; ++s) std::fill(&F[(std::size_t)s * n4], &F[(std::size_t)s * n4] + n4, 0u);
  }
 }
};
// lazy_elim::tri_solve_blk と同じ後退代入を、ElimK で。
template <class Z> std::vector<u64> tri_solve_blk(const ElimK<Z>& e, const std::vector<int>& cols, int& wx) {
 const u32 M= Z::mod();
 const u64 R= (u64(1) << 32) % M;
 const int r= e.prow.size(), nc= cols.size();
 wx= (nc + 3) & ~3;
 std::vector<u64> X((std::size_t)r * wx), acc((std::size_t)K * wx), zero(wx);
 std::vector<u32> c((std::size_t)r * K);
 for(int t1= r; t1 > 0; t1-= K) {
  const int t0= std::max(0, t1 - K), bs= t1 - t0, ns= r - t1;
  for(int q= 0; q < K; ++q) {
   u64* aq= &acc[(std::size_t)q * wx];
   std::fill(aq, aq + wx, 0);
   if(q < bs) {
    const u64* ut= e.urow(t0 + q);
    for(int k= 0; k < nc; ++k) aq[k]= ut[cols[k]];
   }
  }
  for(int s= t1; s < r; ++s)
   for(int q= 0; q < K; ++q) {
    const u32 x= q < bs ? u32(e.urow(t0 + q)[e.pcol[s]]) : 0;
    c[(std::size_t)(s - t1) * K + q]= x ? M - x : 0;
   }
  if(ns) lazy_elim::block_update(acc.data(), &X[(std::size_t)t1 * wx], c.data(), ns, wx, R);
  for(int q= bs - 1; q >= 0; --q) {
   const int t= t0 + q;
   u64* aq= &acc[(std::size_t)q * wx];
   const u64* Ub[K];
   u32 f[K];
   for(int i= 0; i < K; ++i) {
    const int s= t + 1 + i;
    if(s < t1) {
     const u32 x= u32(e.urow(t)[e.pcol[s]]);
     Ub[i]= &X[(std::size_t)s * wx], f[i]= x ? M - x : 0;
    } else Ub[i]= zero.data(), f[i]= 0;
   }
   lazy_elim::update_row(aq, Ub, f, 0, wx, R);
   const Z iv= Z::raw(e.pval[t]).inv();
   u64* xt= &X[(std::size_t)t * wx];
   for(int k= 0; k < wx; ++k) xt[k]= (Z(aq[k]) * iv).val();
  }
 }
 return X;
}
}  // namespace lazy_elim_vk
