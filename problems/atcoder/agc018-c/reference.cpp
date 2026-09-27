// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <algorithm>
#include <vector>
template <typename TD, typename TR, class F> std::pair<TR, std::vector<TD>> min_Lconvex(const F &f, std::vector<TD> x, TD alpha) {
 TR f0= f(x), f1= f0, fS;
 for (int n= x.size(); alpha; f0 == f1 ? alpha>>= 1 : f0= f1) {
  std::vector<TD> x0{x};
  for (int S= 1; S < (1 << n) - 1; S++) {
   std::vector<TD> xS{x0};
   for (int i= 0; i < n; i++)
    if ((S >> i) & 1) xS[i]+= alpha;
   if ((fS= f(xS)) < f1) f1= fS, x= std::move(xS);
  }
 }
 return {f1, std::move(x)};
}
using namespace std;
// O(MAX_A log log MAX_A)
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int x[3], n= 0;
 for(int j= 0; j < 3; j++) cin >> x[j], n+= x[j];
 int c[n][3];
 for(int i= 0; i < n; i++)
  for(int j= 0; j < 3; j++) cin >> c[i][j];
 auto f= [&](const std::vector<int>& q) {
  long long ret= 0;
  for(int i= 0; i < n; i++) ret+= max({c[i][0] - q[0], c[i][1] - q[1], c[i][2] - q[2]});
  for(int j= 0; j < 3; j++) ret+= 1LL * x[j] * q[j];
  return ret;
 };
 cout << min_Lconvex<int, long long>(f, {0, 0, 0}, 1 << 29).first << '\n';
 return 0;
}
