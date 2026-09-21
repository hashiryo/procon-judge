#include <iostream>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/sparse_fps.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<998244353>;
 int N;
 cin >> N;
 auto F= sfps::pow<Mint>({1, 3, 1}, 2 * N, N - 1);
 cout << F[N - 1] / N << '\n';
 return 0;
}
