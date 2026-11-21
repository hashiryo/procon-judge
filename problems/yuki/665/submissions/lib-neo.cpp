// より厳しい制約: http://codeforces.com/contest/622/problem/F
// NeoLibrary の ZMod、pow_table、interpolate_iota。
#include <iostream>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/nt/pow_table.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<int(1e9 + 7)>;
 long long n, k;
 cin >> n >> k;
 auto pws= pow_table<Mint>(k + 1, k);
 for(int i= 0; i < k + 1; ++i) pws[i + 1]+= pws[i];
 cout << interpolate_iota<Mint>(pws, n) << '\n';
 return 0;
}
