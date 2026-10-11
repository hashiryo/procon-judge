#pragma once
// Bertsekas のオークション法 (ε スケーリング付き、Gauss-Seidel で 1 人ずつ入札する)。費用を最小にする代わりに、利得
// b_ij = -a_ij (n + 1) を最大にする。割り当てられていない人 i は、利得から価格を引いた値 b_ij - p_j が最大の物 j1 と、
// 2 番目の値を探し、p_j1 をその差 + ε だけ上げて j1 を取る (前の持ち主は割り当てを外れる)。段ごとに割り当てを空にし、
// 価格は引き継いで ε を ALPHA で割る。ε = 1 の段を終えた割り当ては、利得を n + 1 倍しているので最適になる。最大と
// 2 番目を探す行の走査は、AVX2 で 4 列ずつ回す。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <vector>
namespace asg_auction {
using i64= long long;
template <int ALPHA= 8> std::vector<int> solve(int n, const std::vector<i64>& a) {
 const i64 S= n + 1, NEG= -(i64(1) << 62);
 std::vector<i64> b((size_t)n * n);
 i64 C= 1;
 for(size_t k= 0; k < b.size(); ++k) b[k]= -a[k] * S, C= std::max(C, b[k] < 0 ? -b[k] : b[k]);
 std::vector<i64> p(n, 0);
 std::vector<int> owner(n), obj(n), q(n);
 i64 eps= std::max<i64>(1, C / ALPHA);
 for(;;) {
  std::fill(owner.begin(), owner.end(), -1), std::fill(obj.begin(), obj.end(), -1);
  int qh= 0, qn= n;
  for(int i= 0; i < n; ++i) q[i]= i;
  while(qn) {
   const int i= q[qh];
   qh= qh + 1 == n ? 0 : qh + 1, --qn;
   const i64* row= &b[(size_t)i * n];
   // 最大 v1 (列 j1) と 2 番目 v2。
   i64 v1= NEG, v2= NEG;
   int j1= 0, j= 0;
   {
    __m256i V1= _mm256_set1_epi64x(NEG), V2= V1, J1= _mm256_setzero_si256(), id= _mm256_setr_epi64x(0, 1, 2, 3);
    const __m256i four= _mm256_set1_epi64x(4);
    for(; j + 3 < n; j+= 4, id= _mm256_add_epi64(id, four)) {
     const __m256i v= _mm256_sub_epi64(_mm256_loadu_si256((const __m256i*)(row + j)), _mm256_loadu_si256((const __m256i*)(p.data() + j)));
     const __m256i gt1= _mm256_cmpgt_epi64(v, V1), gt2= _mm256_cmpgt_epi64(v, V2);
     V2= _mm256_blendv_epi8(_mm256_blendv_epi8(V2, v, gt2), V1, gt1);
     V1= _mm256_blendv_epi8(V1, v, gt1), J1= _mm256_blendv_epi8(J1, id, gt1);
    }
    alignas(32) i64 x1[4], x2[4], xj[4];
    _mm256_store_si256((__m256i*)x1, V1), _mm256_store_si256((__m256i*)x2, V2), _mm256_store_si256((__m256i*)xj, J1);
    for(int l= 0; l < 4; ++l) {
     if(x1[l] > v1) v2= std::max(v1, x2[l]), v1= x1[l], j1= (int)xj[l];
     else v2= std::max(v2, x1[l]);
    }
   }
   for(; j < n; ++j) {
    const i64 v= row[j] - p[j];
    if(v > v1) v2= v1, v1= v, j1= j;
    else if(v > v2) v2= v;
   }
   p[j1]+= (v2 == NEG ? 0 : v1 - v2) + eps;
   if(const int o= owner[j1]; o >= 0) {
    obj[o]= -1;
    int t= qh + qn;
    if(t >= n) t-= n;
    q[t]= o, ++qn;
   }
   owner[j1]= i, obj[i]= j1;
  }
  if(eps == 1) break;
  eps= std::max<i64>(1, eps / ALPHA);
 }
 return obj;
}
}
struct Solver {
 int n;
 const vector<i64>& a;
 vector<int> p;
 i64 c= 0;
 Solver(int n, const vector<i64>& a): n(n), a(a) {}
 void run() {
  p= asg_auction::solve<8>(n, a);
  c= 0;
  for(int i= 0; i < n; ++i) c+= a[(size_t)i * n + p[i]];
 }
 i64 cost() const { return c; }
 const vector<int>& assignment() const { return p; }
};
