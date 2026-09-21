#include <iostream>
#include "mylib/algebra/characteristic_polynomial.hpp"
#include "mylib/algebra/ModInt.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<998244353>;
 int N;
 cin >> N;
 Matrix<Mint> a(N, N);
 for(int i= 0; i < N; i++)
  for(int j= 0; j < N; j++) cin >> a[i][j];
 auto p= characteristic_polynomial(a);
 for(int i= 0; i <= N; i++) cout << p[i] << " \n"[i == N];
 return 0;
}
