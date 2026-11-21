// NeoLibrary の ZMod、pow_table (neo/base/core/nt/pow_table.hpp)、interpolate_iota (neo/algo/compute/poly/lagrange.hpp)。
#include <iostream>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/nt/pow_table.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<int(1e9 + 7)>;
 long long N, M, K;
 cin >> N >> M >> K;
 auto y= pow_table<Mint>(M + K, K);
 for(int i= M; i--;)
  for(int j= 0; j < M + K; ++j) y[j + 1]+= y[j];
 cout << interpolate_iota<Mint>(y, N) << '\n';
 return 0;
}
