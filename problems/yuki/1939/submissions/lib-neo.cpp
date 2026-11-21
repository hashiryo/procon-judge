// NeoLibrary の ZMod と sparse_fps::pow。長さ N + 1 を渡す。
#include <iostream>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/sparse_fps.hpp"
using namespace std;
int main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<998244353>;
 int N, M;
 cin >> N >> M;
 std::vector<Mint> f(N + 1);
 f[0]= 1;
 for(int i= 0; i < M; i++) {
  int l;
  cin >> l;
  f[l]= 1;
 }
 auto g= sparse_fps::pow(f, N + 1, N + 1);
 cout << g[N] / (N + 1) << '\n';
 return 0;
}
