#include <iostream>
#include <numeric>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/convolve.hpp"
#include "neo/set_power_series/log.hpp"
#include "neo/set_power_series/zeta.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Z= ZMod<998244353>;
 int N, M;
 cin >> N >> M;
 int n= 1 << N;
 vector<int> r(n), b(n);
 for(int i= 0, R; i < M; ++i) cin >> R, --R, r[1 << R]+= 1;
 for(int i= 0, B; i < M; ++i) cin >> B, --B, b[1 << B]+= 1;
 sps::subset_zeta(r), sps::subset_zeta(b);
 vector<Z> fact(M + 1, Z(1));
 for(int i= 1; i <= M; ++i) fact[i]= fact[i - 1] * i;
 vector<Z> f(n);
 for(int s= n; s--;) {
  if(r[s] != b[s]) continue;
  f[s]= fact[r[s]];
 }
 auto ans= sps::convolve(f, sps::log(f));
 cout << ans[n - 1] * fact[M].inv() << '\n';
 return 0;
}
