#include <iostream>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/zeta.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Z= ZMod<998244353>;
 int N;
 cin >> N;
 int S= 1 << N;
 vector<Z> a(S), b(S);
 for(int i= 0; i < S; i++) cin >> a[i];
 for(int i= 0; i < S; i++) cin >> b[i];
 auto c= sps::xor_convolve(a, b);
 for(int i= 0; i < S; i++) cout << c[i] << " \n"[i + 1 == S];
 return 0;
}
