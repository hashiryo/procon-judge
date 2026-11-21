// NeoLibrary の ZMod、pow_table、interpolate_iota。
#include <iostream>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/nt/pow_table.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<int(1e9 + 7)>;
 int n, k;
 cin >> n >> k;
 auto y= pow_table<Mint>(k + 1, k);
 for(int i= 0; i < k + 1; ++i) y[i + 1]+= y[i];
 cout << interpolate_iota<Mint>(y, n) - y[0] << '\n';
 return 0;
}
