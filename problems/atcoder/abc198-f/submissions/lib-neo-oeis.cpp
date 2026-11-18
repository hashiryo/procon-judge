// OEIS A054473, 実装サボり
// NeoLibrary の ZMod、berlekamp_massey、linear_recurrence。
#include <iostream>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/berlekamp_massey.hpp"
#include "neo/polynomial/linear_recurrence.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<998244353>;
 vector<Mint> a= {1, 1, 3, 5, 10, 15, 29, 41, 68, 98, 147, 202, 291, 386, 528, 688, 906, 1151, 1480, 1841, 2310, 2833, 3484, 4207, 5099, 6076, 7259, 8562, 10104, 11796};
 long long S;
 cin >> S;
 cout << linear_recurrence(berlekamp_massey(a), a, S - 6) << '\n';
 return 0;
}
