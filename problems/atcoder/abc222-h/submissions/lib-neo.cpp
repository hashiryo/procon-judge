// NeoLibrary の ZMod と sparse_fps::pow (neo/polynomial/sparse_fps.hpp)。長さ N を渡す。
#include <iostream>
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/sparse_fps.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<998244353>;
 int N;
 cin >> N;
 auto F= sparse_fps::pow<Mint>({1, 3, 1}, 2 * N, N);
 cout << F[N - 1] / N << '\n';
 return 0;
}
