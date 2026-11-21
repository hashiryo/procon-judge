// NeoLibrary の ZMod と sparse_fps::div (neo/base/core/poly/sparse_fps.hpp)。長さ N を渡す。
#include <iostream>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/sparse_fps.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<998244353>;
 int N, K;
 cin >> N >> K;
 vector<Mint> num= {1, -1}, den(N + 2);
 den[0]= 1, den[1]= -1;
 for(int i= 0; i < K; ++i) {
  int L, R;
  cin >> L >> R;
  den[L]-= 1;
  den[R + 1]+= 1;
 }
 cout << sparse_fps::div(num, den, N)[N - 1] << '\n';
 return 0;
}
