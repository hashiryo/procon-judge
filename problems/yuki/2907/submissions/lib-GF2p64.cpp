// https://yukicoder.me/problems/no/2907
// GF2p64 の逆元で掃き出す。nimber の行列を from_nimber で GF2p64 に移しても体の同型なので、選んだ列の階数は変わらない
#include <iostream>
#include <vector>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
constexpr u64 MOD= 998244353;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, T;
 cin >> N >> T;
 vector H(N, vector<GF2p64>(T));
 for(int t= 0; t < T; ++t)
  for(int k= 0; k < N; ++k) {
   u64 x;
   cin >> x;
   H[k][t]= GF2p64::from_nimber(x - 1);
  }
 vector<u64> pw(N + 1, 1);  // (2^64)^e mod MOD
 for(int e= 1; e <= N; ++e) pw[e]= pw[e - 1] * ((u64(1) << 63) % MOD * 2 % MOD) % MOD;
 // W_k - 1 が 0 でない解の数を、0 にする列の集合で包除する。選んだ列の集合 S について (-1)^(N-|S|) (2^64)^(|S|-rank(S)) を足す。
 // B[i] は軸が座標 i の行 (空なら軸が無い)。
 u64 ans= 0;
 auto dfs= [&](auto&& dfs, int k, int n, int r, vector<vector<GF2p64>> B) -> void {
  if(k == N) {
   ans= (N - n) & 1 ? (ans + MOD - pw[n - r]) % MOD : (ans + pw[n - r]) % MOD;
   return;
  }
  dfs(dfs, k + 1, n, r, B);
  auto X= H[k];
  for(int i= 0; i < T; ++i) {
   if(!X[i]) continue;
   if(B[i].empty()) {
    const GF2p64 iv= X[i].inv();
    for(int j= i; j < T; ++j) X[j]*= iv;
    B[i]= X;
    return dfs(dfs, k + 1, n + 1, r + 1, B);
   }
   const GF2p64 m= X[i];
   for(int j= i; j < T; ++j) X[j]-= m * B[i][j];
  }
  dfs(dfs, k + 1, n + 1, r, B);
 };
 dfs(dfs, 0, 0, 0, vector<vector<GF2p64>>(T));
 cout << ans << '\n';
 return 0;
}
