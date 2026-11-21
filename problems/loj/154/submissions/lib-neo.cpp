#include <iostream>
#include <numeric>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/composite.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Z= ZMod<998244353>;
 int n, m, k;
 cin >> n >> m >> k;
 vector<Z> f(1 << n);
 for(int s; m--;) cin >> s, f[s]+= 1;
 vector<Z> F(k + 1, Z(1));
 cout << sps::egf_comp(F, f)[(1 << n) - 1] << '\n';
 return 0;
}
