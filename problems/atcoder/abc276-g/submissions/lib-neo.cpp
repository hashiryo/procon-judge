// NeoLibrary の ZMod と sparse_fps::pow_of_div (neo/polynomial/sparse_fps.hpp)。長さ M + 1 を渡す。
#include <iostream>
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/sparse_fps.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<998244353>;
 int N, M;
 cin >> N >> M;
 if(M-= --N; M < 0) {
  cout << 0 << '\n';
  return 0;
 }
 auto F= sparse_fps::pow_of_div<Mint>({1, 1}, {1, 0, 0, -1}, N, M + 1);
 for(int i= 1; i <= M; i++) F[i]+= F[i - 1];
 for(int i= 1; i <= M; i++) F[i]+= F[i - 1];
 cout << F[M] << '\n';
 return 0;
}
