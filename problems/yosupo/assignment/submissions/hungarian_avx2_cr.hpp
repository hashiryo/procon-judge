#pragma once
// hungarian_avx2 に 2 つの手を重ねたもの。1 つは、始める前に列のポテンシャル v を列ごとの最小にすること (column
// reduction)。使った列だけポテンシャルが下がる hungarian では、空いている列がいつも割高に見え、増加路が割り当て済みの
// 列を全部通りやすい。もう 1 つは、minv が同じ列のうち空いている列を選ぶこと。minv の代わりに、2 倍して割り当て済み
// なら 1 を足した鍵を持ち、鍵の最小を取る。割り当て済みかどうかは 1 行を足す間は変わらないので、veff の側に
// 2 倍と 1 bit を先に入れておけば、内側の走査は hungarian_avx2 と同じ形のまま (row を 2 倍する足し算が 1 つ増える)。
// hand_plus と hand_minus は、列ごとの最小を引くと行の中の縮約費用がすべて等しくなるので、同点の空いている列で
// すぐに止まる。
// 以下は hungarian_avx2 の説明。hungarian_lazy の内側の走査を AVX2 で 4 列ずつ回すもの。縮約費用の計算、minv と way の
// 更新、minv の最小とその列の追跡を、64 bit の比較と blend で分岐なしに行う。way は blend できるように i64 で持つ。
// 最小の列は 4 本の lane ごとに追い、走査の終わりにまとめる。4 で割り切れない残りの列はスカラーで回す。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <vector>
namespace asg_hungarian_avx2_cr {
using i64= long long;
inline std::vector<int> solve(int n, const std::vector<i64>& a, i64& total) {
 constexpr i64 OFF= i64(1) << 60, USED= OFF / 2, INF= OFF / 2;
 std::vector<i64> u(n + 1), v(n + 1), veff(n + 1), minv(n + 1), tj(n + 1), way(n + 1);
 std::vector<int> p(n + 1), usedcols;
 usedcols.reserve(n + 1);
 for(int j= 1; j <= n; ++j) v[j]= a[j - 1];
 for(int r= 1; r < n; ++r) {
  const i64* row= a.data() + (size_t)r * n - 1;
  for(int j= 1; j <= n; ++j) v[j]= std::min(v[j], row[j]);
 }
 for(int i= 1; i <= n; ++i) {
  p[0]= i;
  int j0= 0;
  i64 T= 0;
  for(int j= 0; j <= n; ++j) veff[j]= 2 * v[j] - (p[j] != 0), minv[j]= INF;
  usedcols.clear();
  do {
   usedcols.push_back(j0);
   tj[j0]= T, veff[j0]= -OFF, minv[j0]= USED;
   const int i0= p[j0];
   const i64* row= a.data() + (size_t)(i0 - 1) * n - 1;
   const i64 c= 2 * (T - u[i0]);
   const __m256i vc= _mm256_set1_epi64x(c), vj0= _mm256_set1_epi64x(j0), four= _mm256_set1_epi64x(4);
   __m256i vbest= _mm256_set1_epi64x(INF), vbidx= _mm256_setzero_si256(), vidx= _mm256_setr_epi64x(1, 2, 3, 4);
   int j= 1;
   for(; j + 3 <= n; j+= 4) {
    const __m256i r= _mm256_loadu_si256((const __m256i*)(row + j));
    const __m256i ve= _mm256_loadu_si256((const __m256i*)(veff.data() + j));
    __m256i mv= _mm256_loadu_si256((const __m256i*)(minv.data() + j));
    const __m256i cur= _mm256_add_epi64(_mm256_sub_epi64(_mm256_add_epi64(r, r), ve), vc);
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
    const i64 cur= 2 * row[j] - veff[j] + c;
    if(cur < minv[j]) minv[j]= cur, way[j]= j0;
    if(minv[j] < best) best= minv[j], j1= j;
   }
   T= best >> 1;
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
 void run() { p= asg_hungarian_avx2_cr::solve(n, a, total); }
 i64 cost() const { return total; }
 const vector<int>& assignment() const { return p; }
};
