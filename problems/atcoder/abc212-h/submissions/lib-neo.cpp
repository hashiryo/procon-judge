#include <iostream>
#include <numeric>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/zeta.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Z= ZMod<998244353>;
 constexpr int n= 1 << 16;
 int N, K;
 cin >> N >> K;
 vector<Z> a(n);
 for(int i= 0, A; i < K; ++i) cin >> A, a[A]+= 1;
 sps::hadamard(a);
 for(auto& x: a) x= x == Z(1) ? Z(N) : (x.pow(N) - 1) / (x - 1) * x;
 sps::hadamard(a);
 Z all= K == 1 ? Z(N) : (Z(K).pow(N) - 1) / (K - 1) * K;
 cout << all - a[0] / n << '\n';
 return 0;
}
