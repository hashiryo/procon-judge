// ABC 355 G の愚直解。位置を P_y 個に展開し、t 個の中心で受ける O(K M^2) の DP。
// M = ΣP <= 60 の小さな入力 (gen.py の seed 1000 以上) でだけ使う。
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
 vector<i64> pos;
 for(int y= 1; y <= N; ++y) {
  int p;
  cin >> p;
  for(int t= 0; t < p; ++t) pos.push_back(y);
 }
 const int M= pos.size();
 if(M == 0) return cout << 0 << '\n', 0;
 const int Kp= min(K, M);
 vector<i64> sp(M + 1, 0);
 for(int i= 0; i < M; ++i) sp[i + 1]= sp[i] + pos[i];
 auto w= [&](int l, int r) {
  const int c= (l + r) / 2;
  return pos[c] * (c - l) - (sp[c] - sp[l]) + (sp[r + 1] - sp[c + 1]) - pos[c] * (r - c);
 };
 const i64 INF= numeric_limits<i64>::max() / 4;
 vector<i64> dp(M + 1, INF), ndp(M + 1, INF);
 dp[0]= 0;
 for(int t= 1; t <= Kp; ++t) {
  fill(ndp.begin(), ndp.end(), INF);
  for(int i= 1; i <= M; ++i)
   for(int j= 0; j < i; ++j)
    if(dp[j] < INF / 2) ndp[i]= min(ndp[i], dp[j] + w(j, i - 1));
  dp.swap(ndp);
 }
 cout << dp[M] << '\n';
 return 0;
}
