// TUPC 2022 K の愚直解。A をソートし、t 個の区間に分ける O(K N^2) の DP。N <= 40 の小さな入力
// (gen.py の seed 1000 以上) でだけ使う。
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>
using namespace std;
using i64= long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, K;
 if(!(cin >> N >> K)) return 0;
 vector<i64> A(N);
 for(auto& x: A) cin >> x;
 sort(A.begin(), A.end());
 const int Kp= min(K, N);
 const i64 INF= numeric_limits<i64>::max() / 4;
 vector<i64> dp(N + 1, INF), ndp(N + 1, INF);
 dp[0]= 0;
 for(int t= 1; t <= Kp; ++t) {
  fill(ndp.begin(), ndp.end(), INF);
  for(int i= 1; i <= N; ++i)
   for(int j= 0; j < i; ++j)
    if(dp[j] < INF / 2) ndp[i]= min(ndp[i], dp[j] + A[i - 1] * (i - j));
  dp.swap(ndp);
 }
 cout << dp[N] << '\n';
 return 0;
}
