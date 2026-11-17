#include <iostream>
#include <numeric>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/exp.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Z= ZMod<998244353>;
 long long N, M;
 cin >> N >> M;
 long long S= 1 << N;
 vector<long long> D(N), f(S);
 for(int i= 0; i < N; ++i) cin >> D[i];
 f[0]= 1;
 for(int i= 0, I= 1; i < N; ++i, I<<= 1)
  for(int s= 0; s < I; ++s) {
   long long g= gcd(f[s], D[i]), a= D[i] / g;
   f[s | I]= a <= M / f[s] ? a * f[s] : M + 1;
  }
 vector<Z> fact(N + 1, Z(1));
 for(int i= 1; i <= N; ++i) fact[i]= fact[i - 1] * i;
 vector<Z> g(S);
 for(int s= 1; s < S; ++s) {
  int n= __builtin_popcount(s);
  Z tmp= fact[n - 1] * Z(M / f[s]);
  g[s]= n & 1 ? tmp : -tmp;
 }
 cout << sps::exp(g)[S - 1] << '\n';
 return 0;
}
