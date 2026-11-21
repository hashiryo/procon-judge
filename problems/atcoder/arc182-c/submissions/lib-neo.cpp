#include <iostream>
#include <numeric>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/composite.hpp"
#include "neo/base/core/set_fps/inv.hpp"
#include "neo/base/core/set_fps/zeta.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Z= ZMod<998244353>;
 long long N, M;
 cin >> N >> M;
 if(M == 1) return cout << Z(N) << '\n', 0;
 vector<int> p= {2, 3, 5, 7, 11, 13};
 vector<Z> f(1 << 6);
 for(int i= 1; i <= M; ++i) {
  vector<Z> g(1 << 6);
  g[0]= 1;
  for(int j= 0, n= i; j < 6; ++j) {
   Z m= 0;
   while(n % p[j] == 0) n/= p[j], m+= 1;
   for(int J= 1 << j, s= J; s--;) g[s | J]= g[s] * m;
  }
  for(int s= 1 << 6; s--;) f[s]+= g[s];
 }
 auto g= sps::pow(f, N + 1);
 for(int s= 1 << 6; s--;) g[s]-= f[s];
 Z iv= Z(1) / (M - 1);
 f[0]= 1;
 for(int s= 1 << 6; --s;) f[s]*= iv;
 for(int s= 1 << 6; s--;) g[s]*= iv;
 g= sps::div(g, f);
 sps::subset_zeta(g);
 cout << g.back() << '\n';
 return 0;
}
