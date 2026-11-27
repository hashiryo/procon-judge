// https://atcoder.jp/contests/s8pc-3/tasks/s8pc_3_g
// NeoLibrary の ZMod、sparse_fps (div、pow、pow_of_div)、interpolate_iota、div_at。長さは今の Library の N に 1 足して渡す。
#include <iostream>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/sparse_fps.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
#include "neo/algo/compute/poly/linear_recurrence.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<998244353>;
 long long n, m;
 cin >> n >> m, --n, --m;
 if(n == 0) return cout << div_at<Mint>({1}, {1, -1, -1}, m) << '\n', 0;
 auto f= sparse_fps::div<Mint>({1}, {1, -1, -1}, n + 2);
 auto b= sparse_fps::div<Mint>(sparse_fps::pow<Mint>({1, -1}, n, n + 1), {1, -1, -1}, n + 2);
 Mint D= Mint(1) / (b[n] * b[n] - b[n - 1] * b[n + 1]);
 Mint A= b[n] * f[n] - b[n - 1] * f[n + 1];
 Mint B= -b[n + 1] * f[n] + b[n] * f[n + 1];
 A*= D, B*= D;
 auto c= sparse_fps::div<Mint>({A, B}, {1, -1, -1}, n + 1);
 b= sparse_fps::div(sparse_fps::pow_of_div<Mint>({1}, {1, -1}, n, n + 1), {1, -1, -1}, n + 1);
 for(int i= 0; i <= n; ++i) b[i]-= c[i];
 cout << lagrange::interpolate_iota<Mint>(b, m) + div_at<Mint>({A, B}, {1, -1, -1}, m) << '\n';
 return 0;
}
