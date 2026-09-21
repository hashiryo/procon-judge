#include <iostream>
#include "mylib/algebra/ModInt_Exp.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ModInt_Exp<998244353>;
 Mint n, k, m;
 cin >> n >> k >> m;
 cout << m.pow(k.pow(n)) << '\n';
 return 0;
}
