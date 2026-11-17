// 素直な 3^N の DP (ライブラリを使わない)。dp[S] = max_{T ⊆ S、T は S の最小の要素を含む} w[T] + dp[S \ T]。
#include <algorithm>
#include <cstdio>
#include <vector>
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<long long> a(n * n);
 for(auto& v: a)
  if(scanf("%lld", &v) != 1) return 1;
 const int N= 1 << n;
 std::vector<long long> w(N, 0), dp(N, 0);
 for(int s= 1; s < N; ++s) {
  const int hi= 31 - __builtin_clz(s), rest= s ^ (1 << hi);
  w[s]= w[rest];
  for(int j= 0; j < hi; ++j)
   if(rest >> j & 1) w[s]+= a[hi * n + j];
 }
 for(int s= 1; s < N; ++s) {
  const int lo= s & -s, rest= s ^ lo;
  long long best= -(1LL << 62);
  for(int t= rest;; t= (t - 1) & rest) {
   best= std::max(best, w[lo | t] + dp[rest ^ t]);
   if(!t) break;
  }
  dp[s]= best;
 }
 printf("%lld\n", dp[N - 1]);
}
