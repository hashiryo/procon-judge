#pragma once
// int の max-plus の行列を素直な i-k-j で掛ける (GCC が列の向きを vpaddd と vpmaxsd でベクトル化する)。辺のない所は NEG = -2^29 と
// し、値を NEG 以上に保つ (積のあとに max(・, NEG))。NEG を含む和は NEG + 3 × 10^6 以下で、本当の値 (-3 × 10^6 以上) には勝たず、
// 2 つの和は -2^30 以上なので溢れない。手順は reference と同じ。積の中では n を局所の値に写す (ラムダが参照で受けた n は、
// 書き込みと重なりうるとして x64 の GCC 15 がループのたびに読み直し、ベクトル化しなかった。9V45 の run 42353789021 で 154.15 ms)。
#include "../common.hpp"
inline int solve(int n, const vector<array<int, 4>>& doors) {
 constexpr int NEG= -(1 << 29);
 using Mat= vector<int>;
 auto mul= [&, n](const Mat& x, const Mat& y) {
  Mat z((size_t)n * n, NEG);
  for(int i= 0; i < n; ++i) {
   int* __restrict zi= &z[(size_t)i * n];
   for(int k= 0; k < n; ++k) {
    const int a= x[(size_t)i * n + k];
    if(a == NEG) continue;
    const int* __restrict yk= &y[(size_t)k * n];
    for(int j= 0; j < n; ++j) zi[j]= max(zi[j], a + yk[j]);
   }
   for(int j= 0; j < n; ++j) zi[j]= max(zi[j], NEG);
  }
  return z;
 };
 Mat a((size_t)n * n, NEG);
 for(int i= 0; i < n; ++i) a[(size_t)i * n + i]= 0;
 for(auto [i, j, c, d]: doors) a[(size_t)i * n + j]= max(a[(size_t)i * n + j], c), a[(size_t)j * n + i]= max(a[(size_t)j * n + i], d);
 int lg= 0;
 while((1 << lg) <= n) ++lg;
 vector<Mat> P(lg);
 P[0]= a;
 for(int k= 1; k < lg; ++k) P[k]= mul(P[k - 1], P[k - 1]);
 Mat cur((size_t)n * n, NEG);
 for(int i= 0; i < n; ++i) cur[(size_t)i * n + i]= 0;
 int len= 0;
 for(int k= lg - 1; k >= 0; --k) {
  if(len + (1 << k) > n) continue;
  Mat t= mul(cur, P[k]);
  bool pos= false;
  for(int i= 0; i < n; ++i) pos|= t[(size_t)i * n + i] > 0;
  if(!pos) cur.swap(t), len+= 1 << k;
 }
 return len >= n ? 0 : len + 1;
}
