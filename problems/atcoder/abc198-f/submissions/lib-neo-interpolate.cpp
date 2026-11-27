// NeoLibrary の ZMod、sparse_fps (pow_of_div、div)、interpolate_iota (neo/algo/compute/poly/lagrange.hpp)。長さは 72 を渡す。
#include <iostream>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/sparse_fps.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<998244353>;
 auto a= sparse_fps::pow_of_div<Mint>({1}, {1, -1}, 6, 72);
 auto b= sparse_fps::pow_of_div<Mint>({1}, {1, 0, -1}, 3, 72);
 for(int i= 72; i--;) a[i]+= b[i] * 6;
 b= sparse_fps::pow_of_div<Mint>({1}, {1, 0, 0, -1}, 2, 72);
 for(int i= 72; i--;) a[i]+= b[i] * 8;
 b= sparse_fps::pow_of_div<Mint>({1}, {1, -1}, 2, 72);
 auto c= sparse_fps::div<Mint>(b, {1, 0, -2, 0, 1}, 72);
 for(int i= 72; i--;) a[i]+= c[i] * 3;
 c= sparse_fps::div<Mint>(b, {1, 0, 0, 0, -1}, 72);
 for(int i= 72; i--;) a[i]+= c[i] * 6;
 long long S;
 cin >> S;
 S-= 6;
 vector<Mint> y(6);
 for(int i= 0; i < 6; ++i) y[i]= a[i * 12 + S % 12];
 cout << lagrange::interpolate_iota<Mint>(y, S / 12) / 24 << '\n';
 return 0;
}
