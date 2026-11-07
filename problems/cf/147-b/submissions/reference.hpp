#pragma once
// 期待出力を作る参照実装。max-plus の行列を long long の配列で持ち、辺のない所 (-INF) を飛ばす素朴な i-k-j の積で掛ける。
// P[k] を「2^k 歩以下」の行列とし、len 歩以下に正の閉じた歩みがない最大の len (n 以下) を上の bit から決める。
// brute.hpp と小さい入力で突き合わせてある (pj testdata crosscheck --brute brute.hpp)。
#include "../common.hpp"
inline int solve(int n, const vector<array<int, 4>>& doors) {
 constexpr long long NEG= LLONG_MIN / 4;
 using Mat= vector<long long>;
 auto mul= [&](const Mat& x, const Mat& y) {
  Mat z((size_t)n * n, NEG);
  for(int i= 0; i < n; ++i)
   for(int k= 0; k < n; ++k) {
    const long long a= x[(size_t)i * n + k];
    if(a == NEG) continue;
    for(int j= 0; j < n; ++j) {
     const long long b= y[(size_t)k * n + j];
     if(b != NEG) z[(size_t)i * n + j]= max(z[(size_t)i * n + j], a + b);
    }
   }
  return z;
 };
 Mat a((size_t)n * n, NEG);
 for(int i= 0; i < n; ++i) a[(size_t)i * n + i]= 0;
 for(auto [i, j, c, d]: doors) a[(size_t)i * n + j]= max<long long>(a[(size_t)i * n + j], c), a[(size_t)j * n + i]= max<long long>(a[(size_t)j * n + i], d);
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
