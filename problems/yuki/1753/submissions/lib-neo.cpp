#include <iostream>
#include <numeric>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/zeta.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Z= ZMod<998244353>;
 int N;
 cin >> N;
 vector<Z> A(N + 1);
 Z sum= 0;
 for(int i= 0; i <= N; ++i) cin >> A[i], sum+= A[i];
 Z iv= Z(1) / sum;
 for(int i= 0; i <= N; ++i) A[i]*= iv;
 Z ans= 0;
 vector<Z> b(1024);
 b[0]= A[0], sps::hadamard(b);
 for(int X= 1; X < 1024; ++X) {
  vector<Z> a(1024);
  for(int i= 1; i <= N; ++i)
   if(i != X) a[i]= A[i];
  sps::hadamard(a);
  for(int i= 0; i < 1024; ++i) a[i]= b[i] / (Z(1) - a[i]);
  sps::hadamard(a);
  ans+= a[X];
 }
 ans/= 1024;
 cout << ans << '\n';
 return 0;
}
