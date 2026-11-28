#include <iostream>
#include <vector>
#include "neo/base/core/geo/Convex.hpp"
using namespace std;
// 小さい N 用の愚直解。部分集合をすべて試し、凸包の頂点がその集合と一致するものだけを数える。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 constexpr long long MOD= 998244353;
 int N;
 cin >> N;
 vector<Point<long long>> ps(N);
 for(auto& p: ps) cin >> p;
 vector<long long> pw(N + 1);
 pw[0]= 1;
 for(int i= 0; i < N; ++i) pw[i + 1]= pw[i] * 2 % MOD;
 long long ans= 0;
 for(int s= 1; s < (1 << N); ++s) {
  if(__builtin_popcount((unsigned)s) < 3) continue;
  vector<Point<long long>> q;
  for(int i= 0; i < N; ++i)
   if((s >> i) & 1) q.push_back(ps[i]);
  Convex<long long> h(q);
  if((int)h.size() != (int)q.size()) continue;
  int n= 0;
  for(const auto& p: ps)
   if(h.where(p) >= 0) ++n;
  ans= (ans + pw[n - (int)q.size()]) % MOD;
 }
 cout << ans << '\n';
 return 0;
}
