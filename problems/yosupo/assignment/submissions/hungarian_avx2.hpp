#pragma once
// hungarian_lazy の内側の走査を AVX2 で 4 列ずつ回すもの。縮約費用の計算、minv と way の更新、minv の最小とその列の
// 追跡を、64 bit の比較と blend で分岐なしに行う。way は blend できるように i64 で持つ。最小の列は 4 本の lane ごとに
// 追い、走査の終わりにまとめる。4 で割り切れない残りの列はスカラーで回す。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <vector>
namespace asg_hungarian_avx2 {
using i64= long long;
inline std::vector<int> solve(int n, const std::vector<i64>& a, i64& total) {
 constexpr i64 OFF= i64(1) << 60, USED= OFF / 2, INF= OFF / 2;
 std::vector<i64> u(n + 1), v(n + 1), veff(n + 1), minv(n + 1), tj(n + 1), way(n + 1);
 std::vector<int> p(n + 1), usedcols;
 usedcols.reserve(n + 1);
 for(int i= 1; i <= n; ++i) {
  p[0]= i;
  int j0= 0;
  i64 T= 0;
  for(int j= 0; j <= n; ++j) veff[j]= v[j], minv[j]= INF;
  usedcols.clear();
  do {
   usedcols.push_back(j0);
   tj[j0]= T, veff[j0]= -OFF, minv[j0]= USED;
   const int i0= p[j0];
   const i64* row= a.data() + (size_t)(i0 - 1) * n - 1;
   const i64 c= T - u[i0];
   const __m256i vc= _mm256_set1_epi64x(c), vj0= _mm256_set1_epi64x(j0), four= _mm256_set1_epi64x(4);
   __m256i vbest= _mm256_set1_epi64x(INF), vbidx= _mm256_setzero_si256(), vidx= _mm256_setr_epi64x(1, 2, 3, 4);
   int j= 1;
   for(; j + 3 <= n; j+= 4) {
    const __m256i r= _mm256_loadu_si256((const __m256i*)(row + j));
    const __m256i ve= _mm256_loadu_si256((const __m256i*)(veff.data() + j));
    __m256i mv= _mm256_loadu_si256((const __m256i*)(minv.data() + j));
    const __m256i cur= _mm256_add_epi64(_mm256_sub_epi64(r, ve), vc);
    const __m256i lt= _mm256_cmpgt_epi64(mv, cur);
    mv= _mm256_blendv_epi8(mv, cur, lt);
    _mm256_storeu_si256((__m256i*)(minv.data() + j), mv);
    const __m256i w= _mm256_loadu_si256((const __m256i*)(way.data() + j));
    _mm256_storeu_si256((__m256i*)(way.data() + j), _mm256_blendv_epi8(w, vj0, lt));
    const __m256i lb= _mm256_cmpgt_epi64(vbest, mv);
    vbest= _mm256_blendv_epi8(vbest, mv, lb);
    vbidx= _mm256_blendv_epi8(vbidx, vidx, lb);
    vidx= _mm256_add_epi64(vidx, four);
   }
   alignas(32) i64 bb[4], bi[4];
   _mm256_store_si256((__m256i*)bb, vbest), _mm256_store_si256((__m256i*)bi, vbidx);
   i64 best= bb[0];
   int j1= (int)bi[0];
   for(int k= 1; k < 4; ++k)
    if(bb[k] < best) best= bb[k], j1= (int)bi[k];
   for(; j <= n; ++j) {
    const i64 cur= row[j] - veff[j] + c;
    if(cur < minv[j]) minv[j]= cur, way[j]= j0;
    if(minv[j] < best) best= minv[j], j1= j;
   }
   T= best;
   j0= j1;
  } while(p[j0] != 0);
  for(int jj: usedcols) {
   const i64 d= T - tj[jj];
   u[p[jj]]+= d, v[jj]-= d;
  }
  do {
   const int j1= (int)way[j0];
   p[j0]= p[j1], j0= j1;
  } while(j0);
 }
 std::vector<int> ans(n);
 total= 0;
 for(int j= 1; j <= n; ++j) ans[p[j] - 1]= j - 1;
 for(int i= 0; i < n; ++i) total+= a[(size_t)i * n + ans[i]];
 return ans;
}
}
struct Solver {
 int n;
 const vector<i64>& a;
 i64 total= 0;
 vector<int> p;
 Solver(int n, const vector<i64>& a): n(n), a(a) {}
 void run() { p= asg_hungarian_avx2::solve(n, a, total); }
 i64 cost() const { return total; }
 const vector<int>& assignment() const { return p; }
};
